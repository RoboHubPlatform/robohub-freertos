/*
 * freertos_hooks.c — the kernel's diagnostics, routed to the RoboHub console.
 *
 * FreeRTOS calls these when it finds a task has overflowed its stack or the
 * heap has run out; the base configuration asks for both checks (RTOS-0007). They
 * are fatal: there is no recovering a stack that is already gone, and carrying
 * on is how a corrupted program looks like a mystery. So each says what it
 * found and then stops, which is the most a board with no debugger can do.
 *
 * They are defined whether or not the configuration enables the checks, so a
 * project that turns `configCHECK_FOR_STACK_OVERFLOW` or
 * `configUSE_MALLOC_FAILED_HOOK` off still links.
 *
 * @project RoboHub Platform
 * @license MIT
 */

// @req RTOS-0003
#if defined(RH_FREERTOS_KERNEL_PRESENT)

#include "FreeRTOS.h"
#include "task.h"
#include "robohub/hal/console.h"

#if defined(__GNUC__) || defined(__clang__)
#define RH_FREERTOS_WEAK __attribute__((weak))
#else
#define RH_FREERTOS_WEAK
#endif

/* How the board stops, once it has said why. Interrupts are already disabled
 * when this is reached. A macro so the host test can return from it. */
#ifndef RH_FREERTOS_HALT
#define RH_FREERTOS_HALT() for (;;) { }
#endif

/* How long the default drain gives the console, in milliseconds. */
#ifndef RH_FREERTOS_PANIC_DRAIN_MS
#define RH_FREERTOS_PANIC_DRAIN_MS 100u
#endif

/*
 * Give the console the time to put out what was just written.
 *
 * A console write returns once the bytes are queued — in a UART's FIFO or ring
 * buffer, or a USB CDC endpoint's — and what moves them out is an interrupt.
 * So a fatal path writes, drains with interrupts enabled, and only then
 * disables them and halts; disabling first stops the board with the line still
 * queued, which is the silent hang the line was written to prevent.
 *
 * The default waits RH_FREERTOS_PANIC_DRAIN_MS, counted against
 * configCPU_CLOCK_HZ (no tick: the kernel may be what failed). It is weak: a
 * platform that can flush for real — the Pico pumps TinyUSB until the CDC FIFO
 * is empty — defines it strongly.
 */
RH_FREERTOS_WEAK void robohub_freertos_console_drain(void)
{
    /* A volatile counter costs a few cycles a turn; four is the floor on a
     * Cortex-M, so this waits at most the time asked for, and at least a
     * quarter of it. */
    volatile uint32_t turns = (uint32_t)((configCPU_CLOCK_HZ / 4u / 1000u) * RH_FREERTOS_PANIC_DRAIN_MS);
    while (turns != 0u) {
        turns--;
    }
}

/* Every fatal path ends here: interrupts on so the console can drain, the drain,
 * then interrupts off and the halt. A hook may be called with interrupts masked
 * (the stack check runs in the context switch, inside the kernel's critical
 * section), which is why they are enabled for the drain: the kernel is past
 * saving, and an interrupt that runs now can do less harm than a board that
 * stops without saying why. */
static void freertos_halt(void)
{
    taskENABLE_INTERRUPTS();
    robohub_freertos_console_drain();
    taskDISABLE_INTERRUPTS();
    RH_FREERTOS_HALT();
}

/* Best effort on the console, then stop. A hook may be called before the
 * console is up, and from a context that is already broken; the write is
 * attempted because a board that can say "stack overflow" is worth the risk of
 * a garbled line, and nothing after it runs. */
// @req RTOS-0006
static void freertos_panic(const char* what, const char* detail)
{
    (void)robohub_console_puts("\n[robohub-freertos] panic: ");
    (void)robohub_console_puts(what);
    if (detail != 0) {
        (void)robohub_console_puts(" (");
        (void)robohub_console_puts(detail);
        (void)robohub_console_puts(")");
    }
    (void)robohub_console_puts("\n");
    freertos_halt();
}

void vApplicationStackOverflowHook(TaskHandle_t task, char* task_name)
{
    (void)task;
    freertos_panic("task stack overflow", task_name);
}

void vApplicationMallocFailedHook(void)
{
    freertos_panic("heap allocation failed", 0);
}

void vApplicationIdleHook(void)
{
    /* Nothing to do. An idle that sleeps belongs to the platform, which knows
     * what the part can power down; a hook here that guessed would be a guess
     * about hardware this module does not know. */
}

/*
 * The assertion panic, in its default form: say where and stop. It is weak, so
 * a platform that can show more — an LED, a hardware fault channel — defines it
 * strongly and its version is the one reached.
 *
 * It stops the way the hooks do: the line, the drain with interrupts enabled,
 * then interrupts off and the halt. An assertion inside a kernel critical
 * section is reached with interrupts masked, and the console that is serviced
 * by an interrupt would never put the line out if they stayed that way.
 */
// @req RTOS-0006
RH_FREERTOS_WEAK void robohub_freertos_panic(const char* file, int line)
{
    (void)line;
    (void)robohub_console_puts("\n[robohub-freertos] assertion failed at ");
    if (file != 0) {
        (void)robohub_console_puts(file);
    }
    (void)robohub_console_puts("\n");
    freertos_halt();
}

#endif /* RH_FREERTOS_KERNEL_PRESENT */
