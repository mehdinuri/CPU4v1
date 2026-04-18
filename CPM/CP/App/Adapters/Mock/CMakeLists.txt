# Mock adapters for host-side unit and integration tests.
# Implement port interfaces using in-memory state; no hardware required.
# Sources are added here as each mock is implemented (Phase 4).

add_library(Mock_Adapters STATIC)

target_sources(Mock_Adapters PRIVATE
    MockSignalOutput.c
    MockDetector.c
    MockClock.c
    MockSnmpNotifier.c
    MockPersistenceAdapter.c
    MockConfigRepositoryAdapter.c
    MockLogRepositoryAdapter.c
    MockRTCAdapter.c
    MockUnitAlarmAdapter.c
    MockDisplayAdapter.c
    MockUserInputAdapter.c
    MockHeaterAdapter.c
    MockPowerMonitorAdapter.c
    MockDoorSensorAdapter.c
    MockStatusLEDAdapter.c
    MockRelayAdapter.c
    MockSerialAdapter.c
    MockModemAdapter.c
)

target_include_directories(Mock_Adapters
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/App/Ports
)

target_link_libraries(Mock_Adapters PUBLIC Domain)

target_compile_options(Mock_Adapters PRIVATE
    -Wall -Wextra -Werror
    $<$<AND:$<CONFIG:Debug>,$<STREQUAL:${BUILD_TARGET},Host>>:-fprofile-arcs -ftest-coverage>
)
