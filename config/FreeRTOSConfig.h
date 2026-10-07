/*
 * FreeRTOSConfig.h — the base configuration of the RoboHub FreeRTOS OSAL.
 *
 * One file cannot serve every part: a clock is the board's, a heap is the
 * part's memory, and the interrupt and FPU settings are the core's. So this
 * file is the third of three tiers, and each tier may override the one below:
 *
 *   Tier 1  an application's `FreeRTOSConfig_user.h`, if it has one, included
 *           first so anything in it wins;
 *   Tier 2  the platform's hardware facts, `ROBOHUB_CPU_CLOCK_HZ` and
 *           `ROBOHUB_HEAP_SIZE`, which a board support package defines;
 *   Tier 3  the defaults below, chosen for a Cortex-M33 with no MPU and no
 *           TrustZone and enough memory to start.
 *
 * Every value is guarded, so a platform or application redefines what differs
 * rather than restating the file. A board that wants none of this points
 * FREERTOS_CONFIG_DIR at a directory of its own and this one is not read.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Tier 1: the application's own overrides, when it keeps any. */
#if defined(__has_include)
#  if __has_include("FreeRTOSConfig_user.h")
#    include "FreeRTOSConfig_user.h"
#  endif
#endif

#include <stddef.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/*  Tier 2 — the platform's hardware                                           */
/* -------------------------------------------------------------------------- */

/* The clock, from the board that knows it. The default is the RP2350's. */
#ifndef configCPU_CLOCK_HZ
#  ifdef ROBOHUB_CPU_CLOCK_HZ
#    define configCPU_CLOCK_HZ                 ROBOHUB_CPU_CLOCK_HZ
#  else
#    define configCPU_CLOCK_HZ                 ( 150000000UL )
#  endif
#endif

/* The heap, from the part that has to hold it. */
#ifndef configTOTAL_HEAP_SIZE
#  ifdef ROBOHUB_HEAP_SIZE
#    define configTOTAL_HEAP_SIZE              ROBOHUB_HEAP_SIZE
#  else
#    define configTOTAL_HEAP_SIZE              ( ( size_t ) ( 48 * 1024 ) )
#  endif
#endif

/* -------------------------------------------------------------------------- */
/*  Tier 3 — the defaults                                                     */
/* -------------------------------------------------------------------------- */

/* Scheduler. */
#ifndef configUSE_PREEMPTION
#define configUSE_PREEMPTION                    1
#endif
#ifndef configUSE_TIME_SLICING
#define configUSE_TIME_SLICING                  1
#endif
#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#endif
#ifndef configUSE_TICKLESS_IDLE
#define configUSE_TICKLESS_IDLE                 0
#endif
#ifndef configIDLE_SHOULD_YIELD
#define configIDLE_SHOULD_YIELD                 1
#endif
#ifndef configUSE_TASK_NOTIFICATIONS
#define configUSE_TASK_NOTIFICATIONS            1
#endif

/* Tick. */
#ifndef configTICK_RATE_HZ
#define configTICK_RATE_HZ                      ( 1000 )
#endif
#ifndef configMAX_PRIORITIES
#define configMAX_PRIORITIES                    ( 8 )
#endif
#ifndef configMINIMAL_STACK_SIZE
#define configMINIMAL_STACK_SIZE                ( 256 )
#endif
#ifndef configMAX_TASK_NAME_LEN
#define configMAX_TASK_NAME_LEN                 ( 16 )
#endif
#ifndef configTICK_TYPE_WIDTH_IN_BITS
#define configTICK_TYPE_WIDTH_IN_BITS           TICK_TYPE_WIDTH_32_BITS
#endif

/* The RP2350 port is an SMP port; it wants a core count even when one core is
 * used. This runtime takes one — the second is the program's to claim — and a
 * build that wants both sets 2. */
#ifndef configNUMBER_OF_CORES
#define configNUMBER_OF_CORES                   1
#endif

/* Memory. */
#ifndef configSUPPORT_DYNAMIC_ALLOCATION
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#endif
#ifndef configSUPPORT_STATIC_ALLOCATION
#define configSUPPORT_STATIC_ALLOCATION         0
#endif
#ifndef configAPPLICATION_ALLOCATED_HEAP
#define configAPPLICATION_ALLOCATED_HEAP        0
#endif

/* Primitives. */
#ifndef configUSE_MUTEXES
#define configUSE_MUTEXES                       1
#endif
#ifndef configUSE_RECURSIVE_MUTEXES
#define configUSE_RECURSIVE_MUTEXES             1
#endif
#ifndef configUSE_COUNTING_SEMAPHORES
#define configUSE_COUNTING_SEMAPHORES           1
#endif
#ifndef configQUEUE_REGISTRY_SIZE
#define configQUEUE_REGISTRY_SIZE               8
#endif
#ifndef configUSE_TIMERS
#define configUSE_TIMERS                        1
#endif
/* The Raspberry Pi RP2350 port defers work out of an interrupt through the
 * timer service, so the timer task and its queue are part of the port and not
 * an optional extra. */
#ifndef configTIMER_TASK_PRIORITY
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#endif
#ifndef configTIMER_QUEUE_LENGTH
#define configTIMER_QUEUE_LENGTH                10
#endif
#ifndef configTIMER_TASK_STACK_DEPTH
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE * 2 )
#endif

/* The API this runtime uses. FreeRTOS compiles each function out unless its
 * INCLUDE_ macro is 1, so the ones reached through the OSAL are turned on;
 * anything else stays out of the image. */
#ifndef INCLUDE_vTaskDelay
#define INCLUDE_vTaskDelay                      1
#endif
#ifndef INCLUDE_vTaskDelayUntil
#define INCLUDE_vTaskDelayUntil                 1
#endif
#ifndef INCLUDE_xTaskGetTickCount
#define INCLUDE_xTaskGetTickCount               1
#endif
#ifndef INCLUDE_vTaskSuspend
#define INCLUDE_vTaskSuspend                    1
#endif
#ifndef INCLUDE_vTaskDelete
#define INCLUDE_vTaskDelete                     1
#endif
#ifndef INCLUDE_vTaskPrioritySet
#define INCLUDE_vTaskPrioritySet                1
#endif
#ifndef INCLUDE_uxTaskPriorityGet
#define INCLUDE_uxTaskPriorityGet               1
#endif
#ifndef INCLUDE_xTaskGetCurrentTaskHandle
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#endif
#ifndef INCLUDE_xTaskGetSchedulerState
#define INCLUDE_xTaskGetSchedulerState          1
#endif
#ifndef INCLUDE_xTimerPendFunctionCall
#define INCLUDE_xTimerPendFunctionCall          1
#endif

/* Hooks and checks. */
#ifndef configUSE_IDLE_HOOK
#define configUSE_IDLE_HOOK                     0
#endif
#ifndef configUSE_TICK_HOOK
#define configUSE_TICK_HOOK                     0
#endif
#ifndef configUSE_MALLOC_FAILED_HOOK
#define configUSE_MALLOC_FAILED_HOOK            0
#endif
#ifndef configCHECK_FOR_STACK_OVERFLOW
#define configCHECK_FOR_STACK_OVERFLOW          0
#endif
#ifndef configUSE_TRACE_FACILITY
#define configUSE_TRACE_FACILITY                0
#endif
#ifndef configGENERATE_RUN_TIME_STATS
#define configGENERATE_RUN_TIME_STATS           0
#endif

/* Assertions. A failed assert is a program that cannot continue, and on a board
 * with no debugger the worst thing it can do is stop and say nothing. The panic
 * is the platform's — a blink, a line on the console — so that a kernel which
 * stopped is a kernel that said why. */
#ifndef configASSERT
void robohub_freertos_panic(const char* file, int line);
#define configASSERT( x ) \
    if( ( x ) == 0 ) { robohub_freertos_panic( __FILE__, __LINE__ ); }
#endif

/* The core's own facts. These are the Cortex-M33 port's; a part with an FPU
 * differently enabled, an MPU, or TrustZone overrides them. */
#ifndef configENABLE_FPU
#  ifdef ROBOHUB_FPU
#    define configENABLE_FPU                    ROBOHUB_FPU
#  else
#    define configENABLE_FPU                    1
#  endif
#endif
#ifndef configENABLE_MPU
#define configENABLE_MPU                        0
#endif
#ifndef configENABLE_TRUSTZONE
#define configENABLE_TRUSTZONE                  0
#endif
#ifndef configRUN_FREERTOS_SECURE_ONLY
#define configRUN_FREERTOS_SECURE_ONLY          0
#endif

/* The M33 implements four priority bits, so a priority is shifted up to sit in
 * the top of the eight-bit field the NVIC reads. */
#ifndef configPRIO_BITS
#define configPRIO_BITS                         4
#endif
#ifndef configLIBRARY_LOWEST_INTERRUPT_PRIORITY
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#endif
#ifndef configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY     5
#endif

/* On the RP2350 the Pico SDK's own interrupts — USB CDC among them — must sit
 * at a priority the kernel will not mask and cannot preempt wrongly, so the two
 * boundaries are the values the SDK expects rather than the port's arithmetic.
 * Any other core uses the arithmetic. */
#if defined(PICO_BOARD) || defined(ROBOHUB_PLATFORM_PICO)
#ifndef configKERNEL_INTERRUPT_PRIORITY
#define configKERNEL_INTERRUPT_PRIORITY         ( 255 )
#endif
#ifndef configMAX_SYSCALL_INTERRUPT_PRIORITY
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( 191 )
#endif
/* The port's bridges to the SDK's synchronisation and time, so the two agree
 * about what a tick is and what a lock is. */
#ifndef configSUPPORT_PICO_SYNC_INTEROP
#define configSUPPORT_PICO_SYNC_INTEROP         1
#endif
#ifndef configSUPPORT_PICO_TIME_INTEROP
#define configSUPPORT_PICO_TIME_INTEROP         1
#endif
#else
#ifndef configKERNEL_INTERRUPT_PRIORITY
#define configKERNEL_INTERRUPT_PRIORITY \
    ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )
#endif
#ifndef configMAX_SYSCALL_INTERRUPT_PRIORITY
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )
#endif
#endif

/* The handlers the port installs. */
#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler
#define xPortSysTickHandler                     SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
