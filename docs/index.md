# Freertos

RoboHub module.

**Version:** 0.1.1

## RP2350 notes

The RP2350 port is selected by the Pico platform and needs
`configRUN_FREERTOS_SECURE_ONLY=1` and
`configMAX_SYSCALL_INTERRUPT_PRIORITY=16`. `config/FreeRTOSConfig.h` sets both
when `PICO_RP2350` is defined, leaving other ports on their own values. The Pico
BSP's USB-CDC pump runs above ordinary task priorities so a console writer
cannot starve it.
