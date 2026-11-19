# Changelog

All notable changes to this module are recorded here, one section per release.

## [0.1.1] - 2026-11-01

The RP2350 port runs secure-only and expects
`configMAX_SYSCALL_INTERRUPT_PRIORITY=16`; the configuration selects both when
`PICO_RP2350` is defined. Documentation records the port values and the Pico
USB-CDC pump priority.

## [0.1.2] - 2026-11-19

The stack-overflow check (method 2) and the malloc-failed hook are on by default (`configCHECK_FOR_STACK_OVERFLOW 2`, `configUSE_MALLOC_FAILED_HOOK 1`); before, both were 0, so the panic hooks (RTOS-0006) were never reached unless a platform asked, none did, and an overflowed stack corrupted the program without a word. A platform that cannot afford them defines them as 0 first. The hooks disabled interrupts straight after writing their line, so a USB or interrupt-driven UART console stopped with the line still queued; every fatal path — overflow, failed allocation and the assertion panic — now writes, enables interrupts, calls the weak `robohub_freertos_console_drain()` (a bounded wait, `RH_FREERTOS_PANIC_DRAIN_MS`, by default), and only then disables interrupts and halts. The assertion panic used to spin with interrupts as it found them, which left a line written inside a critical section unsent. New host suite `hooks` (tests/tests.yml) checks the defaults and that order (RTOS-0007).

The documentation gave the default port as `GCC_ARM_CM33`; CMake's default (`ROBOHUB_FREERTOS_PORT`) is `GCC_ARM_CM33_NTZ_NONSECURE`, and the documentation now says so, with the flags and heap that go with it and that a platform's own kernel brings its own port. It also spoke of "one source" compiling to nothing without a kernel; there are two (`src/osal_freertos.c`, `src/freertos_hooks.c`), and `src/freertos_hooks.c` did not compile to nothing — it included `FreeRTOS.h` before its `RH_FREERTOS_KERNEL_PRESENT` guard, so a build without the kernel's headers failed on it (RTOS-0003). The include is inside the guard now; new host suite `bare` (tests/tests.yml) compiles every listed source without the kernel and checks it defines nothing.
