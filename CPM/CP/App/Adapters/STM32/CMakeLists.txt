# STM32H743 concrete port adapters.
# Each adapter implements exactly one port interface and may use FreeRTOS
# primitives internally, but never exposes them through the port interface.

add_library(Adapters_STM32 STATIC)

target_sources(Adapters_STM32 PRIVATE
    ControlBusAdapter.c
    FieldCanQueueTx.c
    FieldBusTxAdapter.c
    MmiCanAdapter.c
    CommsStatusAdapter.c
    ControllerModeControlAdapter.c
    FactoryResetAdapter.c
    SystemResetAdapter.c
    ShadowOutputDriverAdapter.c
    MmuAdapter.c
    FieldBusRxAdapter.c
    RTCAdapter.c
    UnitAlarmAdapter.c
    UnitClockAdapter.c
    LWIPSNMPAdapter.c
    LWIPSNMPBridge.c
    LWIPSNMPRootMibs.c
    SnmpSecurityAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Core/flash.c
    FlashStorageAdapter.c
    EepromStorageAdapter.c
    PersistenceAdapter.c
    UserAuthStoreAdapter.c
    ConfigRepositoryAdapter.c
    LogEventAdapter.c
    LogRepositoryAdapter.c
    KeypadAdapter.c
    UnitInputAdapter.c
    BrokenInputSettingsAdapter.c
    GpsConfigAdapter.c
    GpsTimeSyncAdapter.c
    HeaterAdapter.c
    PowerMonitorAdapter.c
    DoorSensorAdapter.c
    CommLEDAdapter.c
    RelayAdapter.c
    ModemAdapter.c
    ModemConfigAdapter.c
    SerialAdapter.c
    UserSettingsAdapter.c
    QuectelModemAdapter.c
    EthernetModemAdapter.c
)

target_include_directories(Adapters_STM32
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/App/Ports
            ${CMAKE_SOURCE_DIR}/App/Domain
            ${CMAKE_SOURCE_DIR}/App/Domain/Intersection
            ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Bootstrap
            ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Core
            ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Tasks
            ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Services/Persistence
            ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Services/Time
            ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Services/Comms
            ${CMAKE_SOURCE_DIR}/App/Platform/STM32/Services/UI
)

target_link_libraries(Adapters_STM32
    PUBLIC  Domain
    PRIVATE stm32cubemx
)
