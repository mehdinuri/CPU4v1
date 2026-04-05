# Domain library — pure C11, NO HAL/FreeRTOS/LWIP includes allowed.
# All state lives in explicit Context_t structs; no global mutable state.
# Sources are added here as each module is implemented (Phase 3).

add_library(Domain STATIC)

target_sources(Domain PRIVATE
    Intersection/Phase.c
    Intersection/SignalGroup.c
    Intersection/Conflict.c
    Intersection/TransitionRule.c
    Intersection/Detector.c
    Intersection/Sequence.c
    Intersection/Program.c
    NTCIP/NTCIP1201.c
    NTCIP/NTCIP1202.c
    NTCIP/OidRegistry.c
)

# Domain headers are public so Ports/, Adapters/, Platform/ can include them
target_include_directories(Domain
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/..   # allows: #include "Domain/Intersection/Phase.h"
)

# Strict warnings enforced for all Domain code on all targets
target_compile_options(Domain PRIVATE
    -Wall -Wextra -Werror -Wpedantic
    -Wno-unused-parameter   # Allow stub implementations during development
)

# Coverage instrumentation forwarded from host-gcc.cmake
target_compile_options(Domain PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<STREQUAL:${BUILD_TARGET},Host>>:-fprofile-arcs -ftest-coverage>
)
