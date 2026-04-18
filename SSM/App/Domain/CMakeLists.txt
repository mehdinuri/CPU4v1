# SSM Domain - pure C11, no HAL / FreeRTOS. Host + STM32 both consume.
# Domain sources are extracted from Tasks/ as logic is decoupled from the HAL.

add_library(ssm_domain INTERFACE)

target_include_directories(ssm_domain INTERFACE
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/Tasks/Inc
    ${CMAKE_SOURCE_DIR}/App
    ${CMAKE_SOURCE_DIR}/App/Domain
)

if(BUILD_TARGET STREQUAL "Host")
    target_compile_options(ssm_domain INTERFACE
        $<$<CONFIG:Debug>:-fprofile-arcs -ftest-coverage>
    )
endif()
