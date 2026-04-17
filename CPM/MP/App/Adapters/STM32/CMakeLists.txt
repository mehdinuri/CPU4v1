# STM32 hardware adapters. Host tests substitute Mock adapters via
# App/Adapters/Mock/. Compiled only for BUILD_TARGET=STM32.

add_library(mp_adapters_stm32 STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/ControlBusAdapter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/EventLogAdapter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FieldBusAdapter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/PersistenceAdapter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/RTCAdapter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/SafetyRelayAdapter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/StatusLEDAdapter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/SystemMonitorAdapter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/UnitAlarmAdapter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/WatchdogAdapter.c
)

target_include_directories(mp_adapters_stm32 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/App
    ${CMAKE_SOURCE_DIR}/App/Domain
    ${CMAKE_SOURCE_DIR}/../Libs
)

target_link_libraries(mp_adapters_stm32 PUBLIC
    mp_domain
    stm32cubemx
)
