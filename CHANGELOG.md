# Changelog

All notable changes to this module are recorded here, one section per release.

## [0.1.1] - 2026-11-01

The RP2350 port runs secure-only and expects
`configMAX_SYSCALL_INTERRUPT_PRIORITY=16`; the configuration selects both when
`PICO_RP2350` is defined. Documentation records the port values and the Pico
USB-CDC pump priority.
