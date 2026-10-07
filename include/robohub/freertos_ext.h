/**
 * @file freertos_ext.h
 * @brief The native FreeRTOS handles behind the OSAL, for callers that need them.
 *
 * The OSAL is opaque on purpose: a program written against it does not know
 * whether a thread is a FreeRTOS task or a bare-metal function, and that is what
 * lets the same program run with or without a kernel. But a program sometimes
 * has to hand a handle to something outside this ecosystem — a vendor library,
 * a tracing tool, a debugger that speaks TaskHandle_t.
 *
 * This header is the way through, and it is opt-in: include it and the two types
 * behind the handles are FreeRTOS's. Include nothing and they stay `void*`.
 * There is no cost to not using it, and a caller that does is saying, in the
 * include, that it is written for FreeRTOS and not merely for the OSAL.
 *
 * @project RoboHub Platform
 * @author Aurel Miron
 * @version 0.1
 * @date 2026
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_ROBOHUB_FREERTOS_EXT_H
#define INCLUDE_ROBOHUB_FREERTOS_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "robohub/osal.h"

/* The FreeRTOS task behind a thread handle. */
static inline TaskHandle_t robohub_freertos_task_handle(robohub_thread_t thread)
{
    return (TaskHandle_t)thread;
}

/* The FreeRTOS semaphore behind a mutex handle. */
static inline SemaphoreHandle_t robohub_freertos_mutex_handle(robohub_mutex_t mutex)
{
    return (SemaphoreHandle_t)mutex;
}

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ROBOHUB_FREERTOS_EXT_H */
