# robohub-freertos

FreeRTOS as the RoboHub OSAL: the middleware that gives `robohub-core` real
threads, real delays and real mutexes.

## How it works

`robohub-common` declares an operating-system abstraction in
`robohub/osal.h` — a thread, a delay, a recursive mutex. `robohub-core` carries
a **weak** default for every function: no threads to create, a delay that is the
hardware timer, and a mutex that locks nothing because bare metal runs one
thing. Linking this module brings **strong** definitions, and the linker prefers
them. Nothing above changes.

```c
/* robohub-common declares it */
rh_bool   robohub_osal_rtos_present(void);
void      robohub_osal_start(void);
int       robohub_osal_thread_create(robohub_thread_t*, const robohub_osal_thread_config*);
void      robohub_osal_sleep_ms(rh_u32 ms);
rh_u32    robohub_osal_now_ms(void);
robohub_mutex_t robohub_osal_mutex_create(void);
void      robohub_osal_mutex_lock(robohub_mutex_t);
void      robohub_osal_mutex_unlock(robohub_mutex_t);
```

With this linked, `robohub_core_run()` asks `robohub_osal_rtos_present()` and,
finding an RTOS, gives each periodic task its own FreeRTOS task and runs its
loop in one more, then starts the scheduler. Remove the dependency and the same
runtime runs the cooperative loop instead. The event bus locks around publish,
subscribe and unsubscribe with a recursive mutex, so a handler may itself
publish.

## The kernel

The build fetches `FreeRTOS-Kernel` into `vendor/`, the way a platform fetches
its vendor SDK. A machine that keeps one elsewhere names it:

```sh
FREERTOS_KERNEL_PATH=/path/to/FreeRTOS-Kernel rho build -target <board> .
```

The **port** and the **heap** are the platform's to choose, because the port
names the part:

```sh
rho build -target <board> .        # FREERTOS_PORT defaults to GCC_ARM_CM33
```

The platform that builds for FreeRTOS also supplies `FreeRTOSConfig.h` on the
include path.

If the kernel is not fetched, this module still builds — its one source compiles
to nothing — and the bare-metal defaults stand. The module is only linked when a
project declares it.

## Adding it to a project

```yaml
depends:
  - name: robohub-freertos
    type: c
    path: /path/to/robohub-freertos

c_connection:
  variants:
    <target>:
      libraries:
        - robohub-core
        - robohub-freertos
```

## License

MIT. See `LICENSE`.
