# STM32 adapters — real HAL implementations of App/Ports/*
# Linked by the STM32 firmware executable only.

add_library(ssm_adapters_stm32 STATIC)

target_sources(ssm_adapters_stm32 PRIVATE
    GpioOutputAdapter.c
    GpioInputAdapter.c
    IwdgWatchdogAdapter.c
    HalTickAdapter.c
    FlashPersistenceAdapter.c
    AdcCurrentAdapter.c
    CanBusAdapter.c
    TimerAdapter.c
)

target_include_directories(ssm_adapters_stm32 PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Tasks
)

target_include_directories(ssm_adapters_stm32 PUBLIC
    ${CMAKE_SOURCE_DIR}/App
)

target_link_libraries(ssm_adapters_stm32 PUBLIC
    ssm_domain
    stm32cubemx
)

target_compile_options(ssm_adapters_stm32 PRIVATE
    -Wall -Wextra -Werror
)
