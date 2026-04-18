# ssm_domain library — pure C11, NO HAL/FreeRTOS includes allowed.
# All functions take explicit parameters; no global mutable state.

add_library(ssm_domain STATIC)

target_sources(ssm_domain PRIVATE
    SignalOutput.c
    SignalOutputImageBuilder.c
    SignalFlashConfig.c
    CurrentMeasurement.c
    SignalSampleAccumulator.c
    Crc32.c
    FlashSyncWatchdog.c
    OutputVerify.c
    VoltageCurrentFrame.c
)

# Public include: callers use #include "Domain/SignalOutput.h"
target_include_directories(ssm_domain
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Strict warnings enforced for all Domain code
target_compile_options(ssm_domain PRIVATE
    -Wall -Wextra -Werror -Wpedantic
    -Wno-unused-parameter
)

# Coverage instrumentation — only for Host Debug builds
target_compile_options(ssm_domain PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<STREQUAL:${BUILD_TARGET},Host>>:-fprofile-arcs -ftest-coverage>
)
