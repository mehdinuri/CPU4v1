# STM32 platform layer: bootstrap and thin RTOS/task wrappers around the
# current domain and adapter services.

add_library(Platform_STM32 STATIC)

target_sources(Platform_STM32 PRIVATE
    Bootstrap/main_stm32.c
    Services/Persistence/MLM.c
    Services/Persistence/MSM.c
    Services/Persistence/SettingsStorage.c
    ../../Adapters/STM32/LCDAdapter.c
    Services/Time/gps.c
    Services/Time/LrlfDetectTimeStore.c
    Services/Time/SystemStartTimeStore.c
    Services/Time/TimeSourceState.c
    Services/Time/time.c
    Services/Comms/MCS.c
    Services/Comms/PPPOSAsynch.c
    Services/UI/lcd.c
    Services/UI/ui.c
    Core/SystemRuntime.c
    Core/DWT.c
    Core/FaultHandler.c
    Core/TimerCapture.c
    Core/usb.c
    Core/lcdDrv.c
    Tasks/FieldCanTxTask.c
    Tasks/IntersectionControlTask.c
    Tasks/Maintenance.c
)

target_include_directories(Platform_STM32
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/Bootstrap
            ${CMAKE_CURRENT_SOURCE_DIR}/Core
            ${CMAKE_CURRENT_SOURCE_DIR}/Tasks
            ${CMAKE_CURRENT_SOURCE_DIR}/Services/Persistence
            ${CMAKE_CURRENT_SOURCE_DIR}/Services/Time
            ${CMAKE_CURRENT_SOURCE_DIR}/Services/Comms
            ${CMAKE_CURRENT_SOURCE_DIR}/Services/UI
    PRIVATE ${CMAKE_SOURCE_DIR}/App/Ports
            ${CMAKE_SOURCE_DIR}/App/Domain
            ${CMAKE_SOURCE_DIR}/App/Domain/Intersection
            ${CMAKE_SOURCE_DIR}/App/Adapters/STM32
)

target_link_libraries(Platform_STM32
    PRIVATE Domain Adapters_STM32 stm32cubemx
)
