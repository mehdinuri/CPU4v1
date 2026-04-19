# MP Domain - pure C11, no HAL / FreeRTOS. Host + STM32 both consume.

add_library(mp_domain STATIC
    # Intersection
    ${CMAKE_CURRENT_SOURCE_DIR}/Intersection/ChannelStateResolver.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Intersection/CpMpLinkService.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Intersection/ConfigurationService.c
    # Malfunction monitors + services
    ${CMAKE_CURRENT_SOURCE_DIR}/Malfunction/ClearanceMonitor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Malfunction/ConflictMonitor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Malfunction/DarkChannelMonitor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Malfunction/DualIndicationMonitor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Malfunction/LampFaultMonitor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Malfunction/MalfunctionEngine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Malfunction/MinYellowMonitor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Malfunction/ModuleHealthMonitor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Malfunction/PowerSupplyMonitor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Malfunction/RedFailMonitor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Malfunction/SafetyDecisionService.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Malfunction/SignalSequenceMonitor.c
    # Fault monitor (TS2 Port-1 surface)
    ${CMAKE_CURRENT_SOURCE_DIR}/FaultMonitor/FaultMonitorService.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FaultMonitor/FaultMonitorStatus.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FaultMonitor/FaultMonitorTrace.c
    # Alarm
    ${CMAKE_CURRENT_SOURCE_DIR}/Alarm/UnitAlarmService.c
    # Shared intersection types (vendored from CP)
    ${CMAKE_SOURCE_DIR}/../Libs/Intersection/IntersectionConfig.c
)

target_include_directories(mp_domain PUBLIC
    ${CMAKE_SOURCE_DIR}/App
    ${CMAKE_SOURCE_DIR}/App/Domain
    ${CMAKE_SOURCE_DIR}/../Libs
)

target_compile_features(mp_domain PUBLIC c_std_11)
target_compile_options(mp_domain PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-O0 -g3>
)

if(BUILD_TARGET STREQUAL "Host")
    target_compile_options(mp_domain PRIVATE
        $<$<CONFIG:Debug>:-fprofile-arcs -ftest-coverage>
    )
    target_link_options(mp_domain PUBLIC
        $<$<CONFIG:Debug>:-fprofile-arcs -ftest-coverage>
    )
endif()
