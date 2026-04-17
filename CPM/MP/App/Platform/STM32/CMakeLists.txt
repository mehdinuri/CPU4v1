# MP Platform layer: bootstrap + FreeRTOS task bodies. STM32-only.

add_library(mp_platform_stm32 STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Bootstrap/MainApplication.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Tasks/Tasks.c
)

target_include_directories(mp_platform_stm32 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/App
    ${CMAKE_SOURCE_DIR}/App/Domain
    ${CMAKE_SOURCE_DIR}/../Libs
)

target_link_libraries(mp_platform_stm32 PUBLIC
    mp_domain
    mp_adapters_stm32
    stm32cubemx
)
