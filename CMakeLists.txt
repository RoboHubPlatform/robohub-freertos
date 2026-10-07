cmake_minimum_required(VERSION 3.19)
project(robohub-freertos C)

# ===========================================================================
# robohub-freertos — FreeRTOS as the RoboHub OSAL, for any RoboHub platform.
# ===========================================================================
#
# This module is not written for one board. What it needs to know about the
# machine it is built for — which kernel, which port, which core, what clock,
# how much heap — comes from a small contract the selected platform fills in
# when a project enables FreeRTOS. Absent a platform, the module falls back to
# an upstream kernel and its own defaults, so it also builds on its own.
#
# The contract, all of it optional, read from the enclosing build:
#
#   ROBOHUB_FREERTOS_PLATFORM_KERNEL  BOOL    the platform has defined
#                                             `FreeRTOS-Kernel` itself (it added
#                                             its own port — the RP2350 port, an
#                                             STM32 port, an Xtensa one); use it
#                                             and do not fetch.
#   ROBOHUB_FREERTOS_KERNEL_PATH      PATH    a kernel checkout to build instead
#                                             of fetching.
#   ROBOHUB_FREERTOS_PORT             STRING  the upstream kernel's port name.
#   ROBOHUB_FREERTOS_HEAP             STRING  heap_1 .. heap_5. Default 4.
#   ROBOHUB_FREERTOS_ARCH_FLAGS       STRING  the core's -m flags.
#   FREERTOS_CONFIG_DIR               PATH    the directory holding
#                                             FreeRTOSConfig.h.
#   ROBOHUB_CPU_CLOCK_HZ              NUMBER  the core clock, passed to the
#                                             configuration.
#   ROBOHUB_HEAP_SIZE                 NUMBER  the FreeRTOS heap, passed to the
#                                             configuration.
#
# A platform sets these when `RHO_CONFIG_RH_FREERTOS` is on and leaves them
# alone otherwise, so a bare-metal build never pays for any of it.

# ---------------------------------------------------------------------------
# Locating a sibling module
# ---------------------------------------------------------------------------
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps.cmake")
  include("${CMAKE_CURRENT_SOURCE_DIR}/deps.cmake")
endif()

# robohub_locate_module(<name> <out-var>): find a sibling module and set the
# variable to its directory. `<name>` is the module's name, so the variable is
# its upper-case form with `_DIR` appended.
function(robohub_locate_module module out_var)
  string(TOUPPER "${module}" upper)
  string(REPLACE "-" "_" upper "${upper}")
  if(${out_var})
    return()
  endif()
  if(DEFINED ENV{${upper}_DIR})
    set(${out_var} "$ENV{${upper}_DIR}" PARENT_SCOPE)
    return()
  endif()
  # A sibling checkout, searched upward: a module may sit nested — a driver
  # inside a collection — with the sibling several levels above it.
  set(dir "${CMAKE_CURRENT_SOURCE_DIR}")
  foreach(level RANGE 6)
    get_filename_component(parent "${dir}/.." ABSOLUTE)
    if(EXISTS "${parent}/${module}/CMakeLists.txt")
      set(${out_var} "${parent}/${module}" PARENT_SCOPE)
      return()
    endif()
    set(dir "${parent}")
  endforeach()
  include(FetchContent)
  message(STATUS "[${PROJECT_NAME}] fetching ${module}")
  FetchContent_Declare(${module}
    GIT_REPOSITORY "https://github.com/RoboHubPlatform/${module}.git"
    GIT_TAG main GIT_SHALLOW TRUE GIT_PROGRESS TRUE)
  FetchContent_MakeAvailable(${module})
  set(${out_var} "${${module}_SOURCE_DIR}" PARENT_SCOPE)
endfunction()

robohub_locate_module(robohub-common ROBOHUB_COMMON_DIR)
if(NOT ROBOHUB_COMMON_DIR)
  message(FATAL_ERROR "robohub-common not found and could not be fetched")
endif()
message(STATUS "[${PROJECT_NAME}] robohub-common at ${ROBOHUB_COMMON_DIR}")

if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory("${ROBOHUB_COMMON_DIR}" robohub-common)
endif()

# ---------------------------------------------------------------------------
# The contract's defaults
# ---------------------------------------------------------------------------
# A platform fills these in; a build with no platform gets values that work for
# the Cortex-M33 the default port targets.
set(ROBOHUB_FREERTOS_PLATFORM_KERNEL OFF CACHE BOOL
    "the selected platform defines FreeRTOS-Kernel itself")
set(ROBOHUB_FREERTOS_KERNEL_PATH "" CACHE PATH
    "a FreeRTOS-Kernel checkout; empty fetches the pinned release")
set(ROBOHUB_FREERTOS_PORT "GCC_ARM_CM33_NTZ_NONSECURE" CACHE STRING
    "the upstream kernel's port to build")
set(ROBOHUB_FREERTOS_HEAP "4" CACHE STRING
    "the FreeRTOS heap implementation to build")
set(ROBOHUB_FREERTOS_ARCH_FLAGS
    "-mcpu=cortex-m33;-mthumb;-mfloat-abi=softfp;-mfpu=fpv5-sp-d16"
    CACHE STRING "the core's -m flags")
set(FREERTOS_CONFIG_DIR "" CACHE PATH
    "a directory holding FreeRTOSConfig.h; empty uses this module's config/")

# ---------------------------------------------------------------------------
# The configuration the kernel is built with
# ---------------------------------------------------------------------------
if(FREERTOS_CONFIG_DIR AND EXISTS "${FREERTOS_CONFIG_DIR}/FreeRTOSConfig.h")
  set(ROBOHUB_FREERTOS_CONFIG_DIR_USED "${FREERTOS_CONFIG_DIR}")
elseif(DEFINED ENV{FREERTOS_CONFIG_DIR} AND EXISTS "$ENV{FREERTOS_CONFIG_DIR}/FreeRTOSConfig.h")
  set(ROBOHUB_FREERTOS_CONFIG_DIR_USED "$ENV{FREERTOS_CONFIG_DIR}")
else()
  set(ROBOHUB_FREERTOS_CONFIG_DIR_USED "${CMAKE_CURRENT_SOURCE_DIR}/config")
endif()
if(EXISTS "${ROBOHUB_FREERTOS_CONFIG_DIR_USED}/FreeRTOSConfig.h")
  message(STATUS "[${PROJECT_NAME}] FreeRTOSConfig.h from ${ROBOHUB_FREERTOS_CONFIG_DIR_USED}")
else()
  message(FATAL_ERROR
    "robohub-freertos: no FreeRTOSConfig.h. Put one in ${CMAKE_CURRENT_SOURCE_DIR}/config, "
    "or set FREERTOS_CONFIG_DIR to the board's.")
endif()

# ---------------------------------------------------------------------------
# The kernel
# ---------------------------------------------------------------------------
# Three ways it can come, in order: the platform already defined it, the
# platform named a checkout, and the module fetches the release it pins.
if(ROBOHUB_FREERTOS_PLATFORM_KERNEL OR TARGET FreeRTOS-Kernel)
  # The platform's kernel compiles where it is consumed, so the board's
  # FreeRTOSConfig.h goes on the kernel's interface for whoever links it.
  message(STATUS "[${PROJECT_NAME}] using the kernel the platform provided")
  target_include_directories(FreeRTOS-Kernel INTERFACE "${ROBOHUB_FREERTOS_CONFIG_DIR_USED}")
  set(ROBOHUB_FREERTOS_USES_PLATFORM_KERNEL ON)
else()
  set(ROBOHUB_FREERTOS_USES_PLATFORM_KERNEL OFF)

  # The upstream kernel's build file looks for a `freertos_config` target
  # carrying the configuration directory; it is made before the kernel is added.
  if(NOT TARGET freertos_config)
    add_library(freertos_config INTERFACE)
    target_include_directories(freertos_config INTERFACE "${ROBOHUB_FREERTOS_CONFIG_DIR_USED}")
  endif()

  set(ROBOHUB_FREERTOS_KERNEL_TAG "V11.1.0" CACHE STRING
      "the FreeRTOS-Kernel release to fetch")
  set(FREERTOS_PORT "${ROBOHUB_FREERTOS_PORT}" CACHE STRING "the FreeRTOS port to build" FORCE)
  set(FREERTOS_HEAP "${ROBOHUB_FREERTOS_HEAP}" CACHE STRING "the FreeRTOS heap to build" FORCE)

  set(ROBOHUB_FREERTOS_KERNEL_DIR "${ROBOHUB_FREERTOS_KERNEL_PATH}")
  if(NOT ROBOHUB_FREERTOS_KERNEL_DIR AND DEFINED ENV{FREERTOS_KERNEL_PATH})
    set(ROBOHUB_FREERTOS_KERNEL_DIR "$ENV{FREERTOS_KERNEL_PATH}")
  endif()
  if(NOT ROBOHUB_FREERTOS_KERNEL_DIR
     AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/FreeRTOS-Kernel/CMakeLists.txt")
    set(ROBOHUB_FREERTOS_KERNEL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/FreeRTOS-Kernel")
  endif()

  if(ROBOHUB_FREERTOS_KERNEL_DIR)
    message(STATUS "[${PROJECT_NAME}] FreeRTOS-Kernel checkout at ${ROBOHUB_FREERTOS_KERNEL_DIR}")
    add_subdirectory("${ROBOHUB_FREERTOS_KERNEL_DIR}" freertos-kernel)
  else()
    message(STATUS "[${PROJECT_NAME}] fetching FreeRTOS-Kernel ${ROBOHUB_FREERTOS_KERNEL_TAG}")
    include(FetchContent)
    FetchContent_Declare(FreeRTOS-Kernel
      GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
      GIT_TAG        ${ROBOHUB_FREERTOS_KERNEL_TAG}
      GIT_SHALLOW    TRUE
      GIT_PROGRESS   TRUE)
    FetchContent_MakeAvailable(FreeRTOS-Kernel)
    set(ROBOHUB_FREERTOS_KERNEL_DIR "${freertos-kernel_SOURCE_DIR}")
  endif()

  # The core's flags reach the kernel's real targets; the `FreeRTOS-Kernel`
  # alias, if there is one, cannot be given options.
  if(ROBOHUB_FREERTOS_ARCH_FLAGS)
    foreach(robohub_freertos_target freertos_kernel freertos_kernel_port)
      if(TARGET ${robohub_freertos_target})
        target_compile_options(${robohub_freertos_target} PRIVATE ${ROBOHUB_FREERTOS_ARCH_FLAGS})
      endif()
    endforeach()
  endif()
endif()

# ---------------------------------------------------------------------------
# Library target
# ---------------------------------------------------------------------------
# An interface library, not a static one. FreeRTOS's own CMake targets are
# interface libraries of sources — the kernel, the port and the heap compile
# wherever they are consumed — and a static library that linked them would
# compile them into itself and the program would compile them again, once per
# consumer. As an interface this module's two sources and the kernel's compile
# into the program that links it, and only there.
add_library(robohub-freertos INTERFACE)

target_sources(robohub-freertos INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/osal_freertos.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/freertos_hooks.c")

target_include_directories(robohub-freertos INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${ROBOHUB_COMMON_DIR}/include"
    "${ROBOHUB_COMMON_DIR}/include/robohub"
    "${ROBOHUB_FREERTOS_CONFIG_DIR_USED}"
)
if(ROBOHUB_FREERTOS_KERNEL_DIR)
  target_include_directories(robohub-freertos INTERFACE "${ROBOHUB_FREERTOS_KERNEL_DIR}/include")
endif()

target_compile_definitions(robohub-freertos INTERFACE RH_FREERTOS_KERNEL_PRESENT=1)
# The platform's hardware facts, as the configuration base reads them.
if(ROBOHUB_CPU_CLOCK_HZ)
  target_compile_definitions(robohub-freertos INTERFACE ROBOHUB_CPU_CLOCK_HZ=${ROBOHUB_CPU_CLOCK_HZ})
endif()
if(ROBOHUB_HEAP_SIZE)
  target_compile_definitions(robohub-freertos INTERFACE ROBOHUB_HEAP_SIZE=${ROBOHUB_HEAP_SIZE})
endif()

target_link_libraries(robohub-freertos INTERFACE robohub-common)
target_compile_features(robohub-freertos INTERFACE c_std_11)

if(ROBOHUB_FREERTOS_USES_PLATFORM_KERNEL)
  # The platform's kernel: the Raspberry Pi port, an STM32 port, an Xtensa one —
  # the names are the port's, and the platform defined them.
  target_link_libraries(robohub-freertos INTERFACE FreeRTOS-Kernel)
  if(TARGET FreeRTOS-Kernel-Heap4)
    target_link_libraries(robohub-freertos INTERFACE FreeRTOS-Kernel-Heap4)
  endif()
else()
  # The upstream kernel, plus the port's headers (portmacro.h), which FreeRTOS.h
  # includes next and which live on a target of their own.
  target_link_libraries(robohub-freertos INTERFACE freertos_kernel)
  if(TARGET freertos_kernel_port_headers)
    target_link_libraries(robohub-freertos INTERFACE freertos_kernel_port_headers)
  endif()
endif()

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
install(FILES module.yml DESTINATION share/robohub-freertos)
install(TARGETS robohub-freertos)
