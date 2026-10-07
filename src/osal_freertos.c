/*
 * osal_freertos.c — FreeRTOS as the RoboHub OSAL.
 *
 * Each call is the kernel's own, with the units converted at this edge and
 * nowhere else: a stack is asked for in bytes and given in words, milliseconds
 * are given in ticks. The mutex is the recursive kind, because the event bus
 * locks itself and a handler that publishes must not deadlock against the bus
 * it is already inside.
 *
 * The functions here are strong. The runtime carries weak defaults for every one
 * of them; linking this module is what makes the strong definitions the ones
 * that are reached, and that is the whole of how an RTOS is chosen.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/osal.h"

#if defined(RH_FREERTOS_KERNEL_PRESENT)

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

rh_bool robohub_osal_rtos_present(void)
{
    return RH_TRUE;
}

void robohub_osal_start(void)
{
    vTaskStartScheduler();
    /* A scheduler that returns ran out of heap to start with. There is nothing
     * useful left to do here, and returning would drop the caller out of its
     * program. */
    for (;;) {
    }
}

int robohub_osal_thread_create(robohub_thread_t* thread,
                               const robohub_osal_thread_config* config)
{
    if (thread == 0 || config == 0 || config->entry == 0) {
        return -1;
    }

    /* FreeRTOS counts a stack in words, the contract in bytes. */
    const rh_u32 words =
        (config->stack_size + (rh_u32)sizeof(StackType_t) - 1u) / (rh_u32)sizeof(StackType_t);

    TaskHandle_t handle = 0;
    BaseType_t result = xTaskCreate(config->entry,
                                    config->name,
                                    (configSTACK_DEPTH_TYPE)words,
                                    config->arg,
                                    (UBaseType_t)config->priority,
                                    &handle);
    if (result != pdPASS) {
        return -1;
    }
    *thread = (robohub_thread_t)handle;
    return 0;
}

void robohub_osal_sleep_ms(rh_u32 ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

rh_u32 robohub_osal_now_ms(void)
{
    return (rh_u32)(xTaskGetTickCount() * (TickType_t)portTICK_PERIOD_MS);
}

robohub_mutex_t robohub_osal_mutex_create(void)
{
    return (robohub_mutex_t)xSemaphoreCreateRecursiveMutex();
}

void robohub_osal_mutex_destroy(robohub_mutex_t mutex)
{
    if (mutex != 0) {
        vSemaphoreDelete((SemaphoreHandle_t)mutex);
    }
}

void robohub_osal_mutex_lock(robohub_mutex_t mutex)
{
    if (mutex != 0) {
        (void)xSemaphoreTakeRecursive((SemaphoreHandle_t)mutex, portMAX_DELAY);
    }
}

void robohub_osal_mutex_unlock(robohub_mutex_t mutex)
{
    if (mutex != 0) {
        (void)xSemaphoreGiveRecursive((SemaphoreHandle_t)mutex);
    }
}

#endif /* RH_FREERTOS_KERNEL_PRESENT */
