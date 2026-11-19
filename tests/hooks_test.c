/*
 * hooks_test.c — the kernel diagnostics, on the host.
 *
 * Two things a board cannot easily show:
 *
 *   - the defaults: the stack-overflow check (method 2) and the malloc-failed
 *     hook are on in the base configuration, so RTOS-0006's hooks are reached
 *     without a platform having to ask for them;
 *   - the order: each fatal path writes its line, then drains the console with
 *     interrupts enabled, and only then disables interrupts and halts. A hook
 *     that disables interrupts first leaves an interrupt-driven console (USB
 *     CDC, a UART's FIFO) with the line still queued, and the board stops
 *     silently.
 *
 * The kernel's interrupt macros, the halt and the console are replaced by ones
 * that record what was called (tests/stubs, the console below); the halt
 * returns here through longjmp.
 *
 * @project RoboHub Platform
 * @license MIT
 */

// @verifies RTOS-0006, RTOS-0007

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "robohub/hal/console.h"

/* The defaults, checked where the configuration is read. */
#if configCHECK_FOR_STACK_OVERFLOW != 2
#error "configCHECK_FOR_STACK_OVERFLOW must default to 2 (method 2)"
#endif
#if configUSE_MALLOC_FAILED_HOOK != 1
#error "configUSE_MALLOC_FAILED_HOOK must default to 1"
#endif

void vApplicationStackOverflowHook(TaskHandle_t task, char* task_name);
void vApplicationMallocFailedHook(void);
void robohub_freertos_panic(const char* file, int line);

static char events[64][48];
static int count;
static char written[512];
static jmp_buf halted;
static int failures;

/* Whether interrupts are enabled, as the stub port macros leave it. Each path
 * starts with them masked, as the stack check is reached from the context
 * switch, so a drain that ran without enabling them would be seen. */
static int interrupts_enabled;

void test_record(const char* event)
{
    if (strcmp(event, "disable_interrupts") == 0) {
        interrupts_enabled = 0;
    } else if (strcmp(event, "enable_interrupts") == 0) {
        interrupts_enabled = 1;
    }
    if (count < 64) {
        snprintf(events[count++], sizeof events[0], "%s", event);
    }
}

void test_halt(void)
{
    test_record("halt");
    longjmp(halted, 1);
}

/* The console the hooks write to: every write recorded, its text kept. */
RoboHubConsoleError robohub_console_puts(const char* text)
{
    test_record("puts");
    strncat(written, text, sizeof written - strlen(written) - 1);
    return ROBOHUB_CONSOLE_OK;
}

/* The drain, recorded, with the interrupt state it was called in. */
void robohub_freertos_console_drain(void)
{
    test_record(interrupts_enabled ? "drain(interrupts on)" : "drain(interrupts off)");
}

static int index_of(const char* event, int from)
{
    for (int i = from; i < count; i++) {
        if (strcmp(events[i], event) == 0) {
            return i;
        }
    }
    return -1;
}

static int last_index_of(const char* event)
{
    for (int i = count - 1; i >= 0; i--) {
        if (strcmp(events[i], event) == 0) {
            return i;
        }
    }
    return -1;
}

static void check(int condition, const char* what, const char* path)
{
    if (!condition) {
        printf("FAIL: %s: %s\n  events:", path, what);
        for (int i = 0; i < count; i++) {
            printf(" %s", events[i]);
        }
        printf("\n  written: %s\n", written);
        failures++;
    }
}

static void check_order(const char* path, const char* expected_text)
{
    int last_puts = last_index_of("puts");
    int drain = index_of("drain(interrupts on)", 0);
    int disable = index_of("disable_interrupts", 0);
    int halt = index_of("halt", 0);

    check(strstr(written, expected_text) != NULL, "the line names the cause", path);
    check(last_puts >= 0, "something is written", path);
    check(drain > last_puts, "the console is drained after the last write", path);
    check(index_of("drain(interrupts off)", 0) < 0, "the drain runs with interrupts enabled", path);
    check(disable > drain, "interrupts are disabled only after the drain", path);
    check(halt > disable, "the board halts with interrupts disabled", path);
}

static void reset(void)
{
    count = 0;
    written[0] = '\0';
    interrupts_enabled = 0;
}

int main(void)
{
    reset();
    if (setjmp(halted) == 0) {
        vApplicationStackOverflowHook(0, "sensor");
    }
    check_order("stack overflow", "panic: task stack overflow (sensor)");

    reset();
    if (setjmp(halted) == 0) {
        vApplicationMallocFailedHook();
    }
    check_order("malloc failed", "panic: heap allocation failed");

    reset();
    if (setjmp(halted) == 0) {
        robohub_freertos_panic("queue.c", 42);
    }
    check_order("assert", "assertion failed at queue.c");

    if (failures != 0) {
        printf("%d failure(s)\n", failures);
        return 1;
    }
    printf("ok: hook defaults on, and every fatal path writes, drains, then halts\n");
    return 0;
}
