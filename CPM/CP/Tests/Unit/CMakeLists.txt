# Unit tests — pure Domain logic with mock adapters.
# Tests are enabled here as each Domain module is ported and its types defined.

# TODO: Uncomment as each domain module is implemented on this branch:
# Add_Unity_Test(Test_Phase            Test_Phase.c)
# Add_Unity_Test(Test_SignalGroup      Test_SignalGroup.c)
# Add_Unity_Test(Test_Detector         Test_Detector.c)
# Add_Unity_Test(Test_Conflict         Test_Conflict.c)
# Add_Unity_Test(Test_TransitionRule   Test_TransitionRule.c)
# Add_Unity_Test(Test_Sequence         Test_Sequence.c)
# Add_Unity_Test(Test_NTCIP1201        Test_NTCIP1201.c)
# Add_Unity_Test(Test_NTCIP1202        Test_NTCIP1202.c)

Add_Unity_Test(Test_ConfigurationService  Test_ConfigurationService.c)
Add_Unity_Test(Test_IntersectionActivationService Test_IntersectionActivationService.c)
Add_Unity_Test(Test_DetectorReportService Test_DetectorReportService.c)
Add_Unity_Test(Test_DetectorReportObjects Test_DetectorReportObjects.c)
Add_Unity_Test(Test_GlobalTimeManagementService Test_GlobalTimeManagementService.c)
Add_Unity_Test(Test_GlobalTimeManagementObjects Test_GlobalTimeManagementObjects.c)
Add_Unity_Test(Test_MmiService Test_MmiService.c)
Add_Unity_Test(Test_MmiSnapshotCache Test_MmiSnapshotCache.c)
Add_Unity_Test(Test_MmiLocalSettingsService Test_MmiLocalSettingsService.c)
Add_Unity_Test(Test_MmiEventLogService Test_MmiEventLogService.c)
Add_Unity_Test(Test_EventReportService Test_EventReportService.c)
Add_Unity_Test(Test_MmiMaintenanceService Test_MmiMaintenanceService.c)
Add_Unity_Test(Test_UiDoorService Test_UiDoorService.c)
Add_Unity_Test(Test_RelayControlService Test_RelayControlService.c)
Add_Unity_Test(Test_UserAuthService Test_UserAuthService.c)
Add_Unity_Test(Test_LcdEventText Test_LcdEventText.c)
Add_Unity_Test(Test_LcdPage_Logs Test_LcdPage_Logs.c)
Add_Unity_Test(Test_CabinetEnvironmentObjects Test_CabinetEnvironmentObjects.c)
Add_Unity_Test(Test_AscIoInputMappingObjects Test_AscIoInputMappingObjects.c)
Add_Unity_Test(Test_AscIoOutputMappingObjects Test_AscIoOutputMappingObjects.c)
Add_Unity_Test(Test_IntersectionEngine Test_IntersectionEngine.c)
Add_Unity_Test(Test_IntersectionController Test_IntersectionController.c)
Add_Unity_Test(Test_CpMpLinkService Test_CpMpLinkService.c)
Add_Unity_Test(Test_IntersectionOutputDispatcher Test_IntersectionOutputDispatcher.c)
Add_Unity_Test(Test_MmuAdapter
               Test_MmuAdapter.c
               ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/MmuAdapter.c)
target_include_directories(Test_MmuAdapter PRIVATE
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32
    ${CMAKE_SOURCE_DIR}/App/Domain
    ${CMAKE_SOURCE_DIR}/App/Ports
)
Add_Unity_Test(Test_PhaseObjects Test_PhaseObjects.c)
Add_Unity_Test(Test_ClockObjects Test_ClockObjects.c)
Add_Unity_Test(Test_NtcipDbTransactionService Test_NtcipDbTransactionService.c)
Add_Unity_Test(Test_NtcipObjectDirectory Test_NtcipObjectDirectory.c)
Add_Unity_Test(Test_SnmpSecurityObjects Test_SnmpSecurityObjects.c)
Add_Unity_Test(Test_LWIPSNMPAdapter
               Test_LWIPSNMPAdapter.c
               ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/LWIPSNMPAdapter.c)
target_include_directories(Test_LWIPSNMPAdapter PRIVATE
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32
    ${CMAKE_SOURCE_DIR}/App/Domain
    ${CMAKE_SOURCE_DIR}/App/Domain/Intersection
)
Add_Unity_Test(Test_FieldBusTxAdapter
               Test_FieldBusTxAdapter.c
               ${CMAKE_SOURCE_DIR}/App/Adapters/STM32/FieldBusTxAdapter.c)
target_include_directories(Test_FieldBusTxAdapter PRIVATE
    ${CMAKE_SOURCE_DIR}/App/Adapters/STM32
    ${CMAKE_SOURCE_DIR}/App/Domain
    ${CMAKE_SOURCE_DIR}/App/Ports
)
Add_Unity_Test(Test_PersistencePort  Test_PersistencePort.c)
Add_Unity_Test(Test_LogRepository    Test_LogRepository.c)
Add_Unity_Test(Test_RTCPort          Test_RTCPort.c)
Add_Unity_Test(Test_UnitAlarmPort    Test_UnitAlarmPort.c)
Add_Unity_Test(Test_DisplayPort      Test_DisplayPort.c)
Add_Unity_Test(Test_UserInputPort    Test_UserInputPort.c)
Add_Unity_Test(Test_ModemPort        Test_ModemPort.c)
