# Unit tests for MP utility code.
# common.h is a pure-C header with no HAL dependencies — safe to test on x86-64.
Add_Unity_Test(test_common
    ${CMAKE_CURRENT_SOURCE_DIR}/test_common.c
)

# --- Hexagonal architecture tests (App/Ports + App/Adapters/Mock) ---
Add_Unity_Test(Test_MockSafetyRelayAdapter
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_MockSafetyRelayAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockSafetyRelayAdapter.c
)

# --- Shared intersection types (linked against mp_domain library) ---
Add_Unity_Domain_Test(Test_SharedIntersectionTypes
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_SharedIntersectionTypes.c
)

# --- Domain integration (engine + monitors + services end-to-end) ---
Add_Unity_Domain_Test(Test_MalfunctionEngineEndToEnd
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_MalfunctionEngineEndToEnd.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockFieldBusAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockSafetyRelayAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockSystemMonitorAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockUnitAlarmAdapter.c
)

# --- Per-monitor focused unit tests ---
Add_Unity_Domain_Test(Test_Monitors
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_Monitors.c
)

# --- Domain surface: UnitAlarm, FaultMonitor, Safety, PowerSupply --
Add_Unity_Domain_Test(Test_DomainSurface
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_DomainSurface.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockSafetyRelayAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockUnitAlarmAdapter.c
)

Add_Unity_Domain_Test(Test_CpMpLinkService
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_CpMpLinkService.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockControlBusAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockSafetyRelayAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockUnitAlarmAdapter.c
)

# --- All ports + mock adapters (compile + link sanity) ---
Add_Unity_Test(Test_AllPortsAndMocks
    ${CMAKE_CURRENT_SOURCE_DIR}/Test_AllPortsAndMocks.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockControlBusAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockFieldBusAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockPersistenceAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockRealtimeClockAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockSafetyRelayAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockStatusLEDAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockSystemMonitorAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockUnitAlarmAdapter.c
    ${CMAKE_SOURCE_DIR}/App/Adapters/Mock/MockWatchdogAdapter.c
)
