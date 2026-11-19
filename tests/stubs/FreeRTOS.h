/*
 * A stand-in for the kernel's FreeRTOS.h, for the host test of the hooks.
 *
 * It reads the real configuration (config/FreeRTOSConfig.h), so the defaults the
 * test checks are the ones a board gets, and replaces the port's interrupt
 * macros and the halt with ones that record what happened, so the order the
 * hooks do things in can be checked without a part.
 *
 * @project RoboHub Platform
 * @license MIT
 */
#ifndef ROBOHUB_FREERTOS_TEST_STUB_FREERTOS_H
#define ROBOHUB_FREERTOS_TEST_STUB_FREERTOS_H

#include "FreeRTOSConfig.h"

void test_record(const char* event);
void test_halt(void);

#define taskDISABLE_INTERRUPTS() test_record("disable_interrupts")
#define taskENABLE_INTERRUPTS() test_record("enable_interrupts")
#define RH_FREERTOS_HALT() test_halt()

typedef void* TaskHandle_t;

#endif
