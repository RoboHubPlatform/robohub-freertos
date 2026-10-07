/*
 * freertos_hooks.c — the kernel's diagnostics, routed to the RoboHub console.
 *
 * FreeRTOS calls these when it finds a task has overflowed its stack or the
 * heap has run out, and only if the configuration asked for the checks. They
 * are fatal: there is no recovering a stack that is already gone, and carrying
 * on is how a corrupted program looks like a mystery. So each says what it
 * found and then stops, which is the most a board with no debugger can do.
 *
 * They are defined whether or not the configuration enables the checks, so a
 * project that turns `configCHECK_FOR_STACK_OVERFLOW` or
 * `configUSE_MALLOC_FAILED_HOOK` on links without writing them itself.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "FreeRTOS.h"

#if defined(RH_FREERTOS_KERNEL_PRESENT)

#include "task.h"
#include "robohub/hal/console.h"

/* Best effort on the console, then stop. A hook may be called before the
 * console is up, and from a context that is already broken; the write is
 * attempted because a board that can say "stack overflow" is worth the risk of
 * a garbled line, and nothing after it runs. */
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

    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
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

#if defined(__GNUC__) || defined(__clang__)
#define RH_FREERTOS_WEAK __attribute__((weak))
#else
#define RH_FREERTOS_WEAK
#endif

/*
 * The assertion panic, in its default form: say where and stop. It is weak, so
 * a platform that can show more — an LED, a hardware fault channel — defines it
 * strongly and its version is the one reached.
 *
 * Interrupts are left alone on purpose. The console is serviced by an interrupt,
 * and a panic that disables interrupts before it prints is a panic nobody reads;
 * the loop is quiet but the line already went out.
 */
RH_FREERTOS_WEAK void robohub_freertos_panic(const char* file, int line)
{
    (void)line;
    (void)robohub_console_puts("\n[robohub-freertos] assertion failed at ");
    if (file != 0) {
        (void)robohub_console_puts(file);
    }
    (void)robohub_console_puts("\n");
    for (;;) {
    }
}

#endif /* RH_FREERTOS_KERNEL_PRESENT */
