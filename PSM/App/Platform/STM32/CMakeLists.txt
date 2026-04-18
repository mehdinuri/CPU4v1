# PSM_Platform_STM32 — hardware adapter wiring + concrete adapters.
# Compiled only for BUILD_TARGET=STM32.

add_library(PSM_Platform_STM32 STATIC
    Bootstrap/main_stm32.c
    Tasks/Measurement.c
    Tasks/CanMsgParser.c
    Tasks/CanMsgSender.c
    Tasks/Storage.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/IndicatorLEDAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/SignalInputAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/EepromAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/CANTxAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/CANRxAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/VoltageSensorAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/FrequencySensorAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/FrequencyCaptureAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/WatchdogAdapter.c
)

# Bootstrap/ and Tasks/ are PUBLIC so the main executable (Core/Src/*) and
# adapter sources can resolve headers like HardwarePorts.h, Measurement.h,
# CanMsgParser.h, Storage.h.
# App/ is PRIVATE so Ports/ and Adapters/STM32/ headers resolve.
target_include_directories(PSM_Platform_STM32
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Bootstrap
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Tasks
    PRIVATE ${CMAKE_SOURCE_DIR}/App
)

# Link stm32cubemx to get HAL include paths (Core/Inc, Drivers/*, etc.)
# needed by adapter .c files that include gpio.h, fdcan.h, etc.
# Link PSM_Domain so Tasks/*.c (now compiled into this library) can resolve
# MeasurementService_* and other domain symbols at link time.
target_link_libraries(PSM_Platform_STM32 PRIVATE stm32cubemx PSM_Domain)

target_compile_options(PSM_Platform_STM32 PRIVATE
    -Wall -Wextra -Werror -Wpedantic
    -Wno-unused-parameter
)
