# PSM_Platform_STM32 — hardware adapter wiring + concrete adapters.
# Compiled only for BUILD_TARGET=STM32.

add_library(PSM_Platform_STM32 STATIC
    Bootstrap/main_stm32.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/IndicatorLEDAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/SignalInputAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/EepromAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/CANTxAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/VoltageSensorAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/FrequencySensorAdapter.c
)

# Bootstrap/ is PUBLIC so #include "HardwarePorts.h" works in tasks.
# App/ is PRIVATE so Ports/ and Adapters/STM32/ headers resolve.
# Tasks/Inc is PRIVATE for storage.h and can_msg_sender.h used by adapters.
target_include_directories(PSM_Platform_STM32
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Bootstrap
    PRIVATE ${CMAKE_SOURCE_DIR}/App
    PRIVATE ${CMAKE_SOURCE_DIR}/Tasks/Inc
)

# Link stm32cubemx to get HAL include paths (Core/Inc, Drivers/*, etc.)
# needed by adapter .c files that include gpio.h, fdcan.h, etc.
target_link_libraries(PSM_Platform_STM32 PRIVATE stm32cubemx)

target_compile_options(PSM_Platform_STM32 PRIVATE
    -Wall -Wextra -Werror -Wpedantic
    -Wno-unused-parameter
)
