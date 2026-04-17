# Domain library — pure C11, NO HAL/FreeRTOS/LWIP includes allowed.
# All state lives in explicit Context_t structs; no global mutable state.
# Sources are added here as each module is implemented (Phase 3).

add_library(Domain STATIC)

target_sources(Domain PRIVATE
    Intersection/IntersectionConfig.c
    Intersection/ConfigurationService.c
    Intersection/IntersectionEngine.c
    Intersection/IntersectionActivationService.c
    Intersection/IntersectionController.c
    Intersection/IntersectionOutputDispatcher.c
    Intersection/Phase.c
    Intersection/SignalGroup.c
    Intersection/Conflict.c
    Intersection/TransitionRule.c
    Intersection/Detector.c
    Intersection/Sequence.c
    Intersection/Program.c
    Lcd/LcdEngine.c
    Lcd/LcdLanguage.c
    Lcd/LcdPage_Home.c
    Lcd/LcdPage_Login.c
    Lcd/LcdPage_Menu.c
    Lcd/LcdPage_Help.c
    Lcd/LcdPage_Logs.c
    Lcd/LcdPage_ConnectionLogs.c
    Lcd/LcdPage_Network.c
    Lcd/LcdPage_Settings.c
    Lcd/LcdPage_SettingsLanguage.c
    Lcd/LcdPage_SettingsDateTime.c
    Lcd/LcdPage_SettingsGps.c
    NTCIP/Core/NtcipTypes.c
    NTCIP/Core/NtcipObjectDirectory.c
    NTCIP/Core/NtcipDbTransactionService.c
    NTCIP/NTCIP1201.c
    NTCIP/NTCIP1202.c
    NTCIP/Mib1201v0315/GlobalConfigurationObjects.c
    NTCIP/Mib1201v0315/GlobalDbManagementObjects.c
    NTCIP/Mib1201v0315NewAuxIO/AuxIoV2Objects.c
    NTCIP/Mib1202v0335/PhaseObjects.c
    NTCIP/Mib1202v0335/PhaseStatusObjects.c
    NTCIP/Mib1202v0335/PhaseControlObjects.c
    NTCIP/Mib1202v0335/RingObjects.c
    NTCIP/Mib1202v0335/SequenceObjects.c
    NTCIP/Mib1202v0335/RingControlObjects.c
    NTCIP/Mib1202v0335/RingStatusObjects.c
    NTCIP/Mib1202v0335/CoordObjects.c
    NTCIP/Mib1202v0335/TimebaseObjects.c
    NTCIP/Mib1202v0335/ClockObjects.c
    NTCIP/Mib1202v0335/DetectorObjects.c
    NTCIP/Mib1202v0335/DetectorReportObjects.c
    NTCIP/Mib1202v0335/SpecialFunctionObjects.c
    NTCIP/Mib1202v0335/ChannelObjects.c
    NTCIP/Mib1202v0335/ChannelStatusObjects.c
    NTCIP/Mib1202v0335/OverlapObjects.c
    NTCIP/Mib1202v0335/OverlapStatusObjects.c
    NTCIP/Mib1202v0335/PreemptObjects.c
    NTCIP/Mib1202v0335/UnitObjects.c
    Intersection/DetectorReportService.c
)

# Domain headers are public so Ports/, Adapters/, Platform/ can include them
target_include_directories(Domain
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/..   # allows: #include "Domain/Intersection/Phase.h"
)

# Strict warnings enforced for all Domain code on all targets
target_compile_options(Domain PRIVATE
    -Wall -Wextra -Werror -Wpedantic
    -Wno-unused-parameter   # Allow stub implementations during development
)

# Coverage instrumentation forwarded from host-gcc.cmake
target_compile_options(Domain PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<STREQUAL:${BUILD_TARGET},Host>>:-fprofile-arcs -ftest-coverage>
)
