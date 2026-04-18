# PSM_Domain library — pure C11, NO HAL/FreeRTOS includes allowed.
# All functions take explicit parameters; no global mutable state.

add_library(PSM_Domain STATIC)

target_sources(PSM_Domain PRIVATE
    Measurement.c
    MeasurementService.c
)

# Public include: callers use #include "Domain/Measurement.h"
# Also exposes App/Config/ (EepromMap.h) and App/Ports/ transitively.
target_include_directories(PSM_Domain
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Strict warnings enforced for all Domain code
target_compile_options(PSM_Domain PRIVATE
    -Wall -Wextra -Werror -Wpedantic
    -Wno-unused-parameter
)

# Coverage instrumentation — only for Host Debug builds
target_compile_options(PSM_Domain PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<STREQUAL:${BUILD_TARGET},Host>>:-fprofile-arcs -ftest-coverage>
)
