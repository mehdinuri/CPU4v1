/* App/Platform/STM32/Core/main_stm32.c
 *
 * Hardware adapter wiring.
 *
 * Owns the static adapter contexts, creates every port instance, and
 * exposes them through HardwarePorts.h. Called once from
 * MX_FREERTOS_Init() before the scheduler starts, after MX_GPIO_Init().
 *
 * Rule: one adapter instance per physical resource.
 *       No other translation unit may instantiate an adapter.
 */
#ifdef STM32H743xx
#include "stm32h7xx_hal.h"
#endif

#include <string.h>

#include "DomainServices.h"
#include "HardwarePorts.h"
#include "PersistencePorts.h"

#include "Adapters/STM32/HeaterAdapter.h"
#include "Adapters/STM32/PowerMonitorAdapter.h"
#include "Adapters/STM32/RelayAdapter.h"
#include "Adapters/STM32/DoorSensorAdapter.h"
#include "Adapters/STM32/CommLEDAdapter.h"
#include "Adapters/STM32/KeypadAdapter.h"
#include "Adapters/STM32/LCDAdapter.h"
#include "Adapters/STM32/RTCAdapter.h"
#include "Adapters/STM32/UnitAlarmAdapter.h"
#include "Adapters/STM32/UnitClockAdapter.h"
#include "Adapters/STM32/UnitInputAdapter.h"
#include "Adapters/STM32/FieldOutputCanAdapter.h"
#include "Adapters/STM32/ControlBusAdapter.h"
#include "Adapters/STM32/MmiCanAdapter.h"
#include "Adapters/STM32/CommsStatusAdapter.h"
#include "Adapters/STM32/ControllerModeControlAdapter.h"
#include "Adapters/STM32/FactoryResetAdapter.h"
#include "Adapters/STM32/MmuAdapter.h"
#include "Adapters/STM32/FieldInputCanAdapter.h"
#include "Adapters/STM32/CompositeModuleBusPort.h"
#include "Adapters/STM32/BrokenInputSettingsAdapter.h"
#include "Adapters/STM32/FlashStorageAdapter.h"
#include "Adapters/STM32/EepromStorageAdapter.h"
#include "Adapters/STM32/PersistenceAdapter.h"
#include "Adapters/STM32/UserAuthStoreAdapter.h"
#include "Adapters/STM32/ConfigRepositoryAdapter.h"
#include "Adapters/STM32/LogEventAdapter.h"
#include "Adapters/STM32/LogRepositoryAdapter.h"
#include "Adapters/STM32/ModemAdapter.h"
#include "Adapters/STM32/ModemConfigAdapter.h"
#include "Adapters/STM32/SerialAdapter.h"
#include "Adapters/STM32/UserSettingsAdapter.h"
#include "Adapters/STM32/GpsConfigAdapter.h"
#include "Adapters/STM32/GpsTimeSyncAdapter.h"
#include "Adapters/STM32/UBloxModemAdapter.h"
#include "Adapters/STM32/TelitModemAdapter.h"
#include "Adapters/STM32/QuectelModemAdapter.h"
#include "Adapters/STM32/QuectelNtcipModemAdapter.h"
#include "Adapters/STM32/UsrModemAdapter.h"
#include "Adapters/STM32/EthernetNtcipModemAdapter.h"
#include "Adapters/STM32/SystemResetAdapter.h"
#include "MCS.h"
#include "MSM.h"
#include "fdcan.h"
#include "usart.h"

/* ------------------------------------------------------------------
 * DMA RX buffer sizes — kept local to avoid including service headers.
 * Values must match the corresponding service-layer maximums.
 * ------------------------------------------------------------------ */
#define MODEM_RX_DMA_BUF_SIZE       1024U /* MCS_DATA_PACKET_MAX_LEN        */
#define INT_GPS_RX_DMA_BUF_SIZE     1024U /* GPS_COMM_MAX_RX_PACKET_LENGTH  */
#define AUX_SERIAL_RX_DMA_BUF_SIZE   256U /* UI_COMM_MAX_PACKET_LENGTH + 1  */
#define WIFI_RX_DMA_BUF_SIZE          256U

/* DMA RX buffers — must reside in D2 SRAM (non-cacheable) so DMA1/DMA2
 * can access them.  The .ram_d2_dma_buffers output section is placed in
 * RAM_D2_NON_CACHEABLE by the linker scripts. */
__attribute__((section(".ram_d2_dma_buffers"), aligned(32)))
static uint8_t s_modemRxDmaBuf[MODEM_RX_DMA_BUF_SIZE];

__attribute__((section(".ram_d2_dma_buffers"), aligned(32)))
static uint8_t s_internalGpsRxDmaBuf[INT_GPS_RX_DMA_BUF_SIZE];

__attribute__((section(".ram_d2_dma_buffers"), aligned(32)))
static uint8_t s_auxSerialRxDmaBuf[AUX_SERIAL_RX_DMA_BUF_SIZE];

__attribute__((section(".ram_d2_dma_buffers"), aligned(32)))
static uint8_t s_wifiRxDmaBuf[WIFI_RX_DMA_BUF_SIZE];

/* ------------------------------------------------------------------
 * Adapter contexts — one per hardware resource.
 * These are the sole owners of adapter state.
 * ------------------------------------------------------------------ */
static HeaterAdapterCtx_t s_heaterCtx;
static PowerMonitorAdapterCtx_t s_powerMonitorCtx;
static RelayAdapterCtx_t s_relayCtx;
static DoorSensorAdapterCtx_t s_doorCtx;
static CommLEDAdapterCtx_t s_commLEDCtx;
static KeypadAdapterCtx_t s_keypadCtx;
static LCDAdapterCtx_t s_lcdCtx;
static RTCAdapterCtx_t s_rtcCtx;
static UnitAlarmAdapterCtx_t s_unitAlarmCtx;
static UnitClockAdapterCtx_t s_unitClockCtx;
static UnitInputAdapterCtx_t s_unitInputCtx;
static FieldOutputCanAdapterCtx_t s_fieldOutputCanCtx;
static ControlBusAdapterCtx_t s_controlBusCtx;
static MmiCanAdapterCtx_t s_mmiCanCtx;
static CommsStatusAdapterCtx_t s_commsStatusCtx;
static ControllerModeControlAdapterCtx_t s_controllerModeCtx;
static FactoryResetAdapterCtx_t s_factoryResetCtx;
static MmuAdapterCtx_t s_mmuCtx;
static FieldInputCanAdapterCtx_t s_fieldInputCanCtx;
static CompositeModuleBusPortCtx_t s_compositeModuleBusCtx;
static BrokenInputSettingsAdapterCtx_t s_brokenInputSettingsCtx;
static FlashStorageAdapterCtx_t s_flashStorageCtx;
static EepromStorageAdapterCtx_t s_eepromStorageCtx;
static PersistenceAdapterCtx_t s_persistenceCtx;
static UserAuthStoreAdapterCtx_t s_userAuthStoreCtx;
static ConfigRepositoryAdapterCtx_t s_configRepositoryCtx;
static LogEventAdapterCtx_t s_logEventCtx;
static LogRepositoryAdapterCtx_t s_logRepositoryCtx;
static ModemConfigAdapterCtx_t s_modemConfigCtx;
static GpsConfigAdapterCtx_t s_gpsConfigCtx;
static GpsTimeSyncAdapterCtx_t s_gpsTimeSyncCtx;
static UserSettingsAdapterCtx_t s_userSettingsCtx;
static SystemResetAdapterCtx_t s_systemResetCtx;
static SerialAdapterCtx_t s_modemCtx;
static SerialAdapterCtx_t s_internalGpsCtx;
static SerialAdapterCtx_t s_auxSerialCtx;
static SerialAdapterCtx_t s_wifiCtx;

/* Modem driver adapter contexts — one per type; only one is active at runtime. */
static UBloxModemAdapterCtx_t s_ubloxModemCtx;
static TelitModemAdapterCtx_t s_telitModemCtx;
static QuectelModemAdapterCtx_t s_quectelModemCtx;
static QuectelNtcipModemAdapterCtx_t s_quectelNtcipModemCtx;
static UsrModemAdapterCtx_t s_usrModemCtx;
static EthernetNtcipModemAdapterCtx_t s_ethernetNtcipModemCtx;

/* ------------------------------------------------------------------
 * Port instances — declared extern in HardwarePorts.h.
 * ------------------------------------------------------------------ */
IHeaterPort_t g_heaterPort;
IPowerMonitorPort_t g_powerMonitorPort;
IRelayPort_t g_relayPort;
IDoorSensorPort_t g_doorPort;
IStatusLEDPort_t g_commLEDPort;
IUserInputPort_t g_keypadPort;
IDisplayPort_t g_lcdPort;
IRealtimeClockPort_t g_rtcPort;
IUnitAlarmPort_t g_unitAlarmPort;
IUnitClockPort_t g_unitClockPort;
IUnitInputPort_t g_unitInputPort;
IOutputDriverPort_t g_outputDriverPort;
IMmuPort_t g_mmuPort;
IModuleBusPort_t g_moduleBusPort;
IPersistencePort_t g_persistencePort;
IConfigRepositoryPort_t g_configRepositoryPort;
ILogRepositoryPort_t g_logRepositoryPort;
ISerialPort_t g_modemPort;              /* UART4  — GPRS modem           */
ISerialPort_t g_internalGpsPort;        /* UART8  — internal GPS (on PCB) */
ISerialPort_t g_auxSerialPort;          /* USART2 — external GPS or UI   */
ISerialPort_t g_wifiPort;               /* UART5  — WiFi placeholder     */

IModemPort_t g_modemDriverPort;         /* active modem driver           */

static IFlashStoragePort_t s_flashStoragePort;
static IEepromStoragePort_t s_eepromStoragePort;
static IModuleBusPort_t s_fieldInputModuleBusPort;
static IModuleBusPort_t s_internalModuleBusPort;
static IControlBusPort_t s_controlBusPort;
static ICommsStatusPort_t s_commsStatusPort;
static IControllerModeControlPort_t s_controllerModePort;
static IFactoryResetPort_t s_factoryResetPort;
static IUserAuthStorePort_t s_userAuthStorePort;
static IBrokenInputSettingsPort_t s_brokenInputSettingsPort;
static IModemConfigPort_t s_modemConfigPort;
static IGpsPort_t s_gpsConfigPort;
static IGpsTimeSyncPort_t s_gpsTimeSyncPort;
static IUserSettingsPort_t s_userSettingsPort;
static ILogEventPort_t s_logEventPort;

ConfigurationService_t g_configurationService;
CpMpLinkService_t g_cpMpLinkService;
IntersectionEngine_t g_intersectionEngine;
IntersectionActivationService_t g_intersectionActivationService;
IntersectionController_t g_intersectionController;
DetectorReportService_t g_detectorReportService;
GlobalTimeManagementService_t g_globalTimeManagementService;
IntersectionOutputDispatcher_t g_intersectionOutputDispatcher;
UserAuthService_t g_userAuthService;
UiPowerService_t g_uiPowerService;
UiLanguageService_t g_uiLanguageService;
UiCommsIdentityService_t g_uiCommsIdentityService;
UiDoorService_t g_uiDoorService;
RelayControlService_t g_relayControlService;
OutputTestService_t g_outputTestService;
MmiEventLogService_t g_mmiEventLogService;
MmiLocalSettingsService_t g_mmiLocalSettingsService;
MmiMaintenanceService_t g_mmiMaintenanceService;
MmiService_t g_mmiService;
MmiSnapshotCache_t g_mmiSnapshotCache;
ISystemResetPort_t g_systemResetPort;

/* ------------------------------------------------------------------
 * Private API
 * ------------------------------------------------------------------ */
static void WireModemDriver(void)
{
  switch (ModemAdapterLoadModuleType(&s_eepromStoragePort))
  {
      case MCS_MODULE_TYPE_UBLOX:
      {
        UBloxModemAdapterInit(&s_ubloxModemCtx);
        g_modemDriverPort = UBloxModemAdapterCreatePort(&s_ubloxModemCtx);
        break;
      }

      case MCS_MODULE_TYPE_TELIT:
      {
        TelitModemAdapterInit(&s_telitModemCtx);
        g_modemDriverPort = TelitModemAdapterCreatePort(&s_telitModemCtx);
        break;
      }

      case MCS_MODULE_TYPE_QUECTEL:
      {
        QuectelModemAdapterInit(&s_quectelModemCtx);
        g_modemDriverPort = QuectelModemAdapterCreatePort(&s_quectelModemCtx);
        break;
      }

      case MCS_MODULE_TYPE_QUECTEL_NTCIP:
      {
        QuectelNtcipModemAdapterInit(&s_quectelNtcipModemCtx);
        g_modemDriverPort =
          QuectelNtcipModemAdapterCreatePort(&s_quectelNtcipModemCtx);
        break;
      }

      case MCS_MODULE_TYPE_USR:
      {
        UsrModemAdapterInit(&s_usrModemCtx);
        g_modemDriverPort = UsrModemAdapterCreatePort(&s_usrModemCtx);
        break;
      }

      case MCS_MODULE_TYPE_ETH_NTCIP:
      default:
      {
        EthernetNtcipModemAdapterInit(&s_ethernetNtcipModemCtx);
        g_modemDriverPort =
          EthernetNtcipModemAdapterCreatePort(&s_ethernetNtcipModemCtx);
        break;
      }
  } /* switch */
} /* WireModemDriver */

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */
void MainApplication_Init(void)
{
  HeaterAdapterInit(&s_heaterCtx);
  g_heaterPort = HeaterAdapterCreatePort(&s_heaterCtx);

  UiPowerServiceInit(&g_uiPowerService);
  PowerMonitorAdapterInit(&s_powerMonitorCtx, &g_uiPowerService);
  g_powerMonitorPort = PowerMonitorAdapterCreatePort(&s_powerMonitorCtx);

  RelayAdapterInit(&s_relayCtx);
  g_relayPort = RelayAdapterCreatePort(&s_relayCtx);

  DoorSensorAdapterInit(&s_doorCtx);
  g_doorPort = DoorSensorAdapterCreatePort(&s_doorCtx);

  CommLEDAdapterInit(&s_commLEDCtx);
  g_commLEDPort = CommLEDAdapterCreatePort(&s_commLEDCtx);

  KeypadAdapterInit(&s_keypadCtx);
  g_keypadPort = KeypadAdapterCreatePort(&s_keypadCtx);

  LCDAdapterInit(&s_lcdCtx);
  g_lcdPort = LCDAdapterCreatePort(&s_lcdCtx);

  RTCAdapterInit(&s_rtcCtx);
  g_rtcPort = RTCAdapterCreatePort(&s_rtcCtx);

  UnitAlarmAdapterInit(&s_unitAlarmCtx);
  g_unitAlarmPort = UnitAlarmAdapterCreatePort(&s_unitAlarmCtx);

  UnitInputAdapterInit(&s_unitInputCtx);
  g_unitInputPort = UnitInputAdapterCreatePort(&s_unitInputCtx);

  FlashStorageAdapterInit(&s_flashStorageCtx);
  s_flashStoragePort = FlashStorageAdapterCreatePort(&s_flashStorageCtx);

  EepromStorageAdapterInit(&s_eepromStorageCtx);
  s_eepromStoragePort = EepromStorageAdapterCreatePort(&s_eepromStorageCtx);

  MSMInit(&s_flashStoragePort, &s_eepromStoragePort);

  PersistenceAdapterInit(&s_persistenceCtx);
  g_persistencePort = PersistenceAdapterCreatePort(&s_persistenceCtx);
  UiLanguageServiceInit(&g_uiLanguageService);
  UiLanguageServiceBind(&g_uiLanguageService, &g_persistencePort);
  (void) UiLanguageServiceLoad(&g_uiLanguageService);
  UserAuthStoreAdapterInit(&s_userAuthStoreCtx, &g_persistencePort);
  s_userAuthStorePort = UserAuthStoreAdapterCreatePort(&s_userAuthStoreCtx);
  BrokenInputSettingsAdapterInit(&s_brokenInputSettingsCtx);
  s_brokenInputSettingsPort =
    BrokenInputSettingsAdapterCreatePort(&s_brokenInputSettingsCtx);
  ModemConfigAdapterInit(&s_modemConfigCtx);
  s_modemConfigPort = ModemConfigAdapterCreatePort(&s_modemConfigCtx);
  GpsConfigAdapterInit(&s_gpsConfigCtx);
  s_gpsConfigPort = GpsConfigAdapterCreatePort(&s_gpsConfigCtx);
  GpsTimeSyncAdapterInit(&s_gpsTimeSyncCtx);
  s_gpsTimeSyncPort = GpsTimeSyncAdapterCreatePort(&s_gpsTimeSyncCtx);
  UserSettingsAdapterInit(&s_userSettingsCtx);
  s_userSettingsPort = UserSettingsAdapterCreatePort(&s_userSettingsCtx);
  CommsStatusAdapterInit(&s_commsStatusCtx);
  s_commsStatusPort = CommsStatusAdapterCreatePort(&s_commsStatusCtx);
  ControllerModeControlAdapterInit(&s_controllerModeCtx);
  s_controllerModePort =
    ControllerModeControlAdapterCreatePort(&s_controllerModeCtx);
  FactoryResetAdapterInit(&s_factoryResetCtx);
  s_factoryResetPort = FactoryResetAdapterCreatePort(&s_factoryResetCtx);
  SystemResetAdapterInit(&s_systemResetCtx);
  g_systemResetPort = SystemResetAdapterCreatePort(&s_systemResetCtx);

  ConfigRepositoryAdapterInit(&s_configRepositoryCtx, &g_persistencePort);
  g_configRepositoryPort =
    ConfigRepositoryAdapterCreatePort(&s_configRepositoryCtx);
  UserAuthServiceInit(&g_userAuthService);
  UserAuthServiceBind(&g_userAuthService, &s_userAuthStorePort);

  ConfigurationServiceInit(&g_configurationService, &g_configRepositoryPort);
  UiCommsIdentityServiceInit(&g_uiCommsIdentityService);
  UiCommsIdentityServiceBind(&g_uiCommsIdentityService, &s_commsStatusPort);
  UiDoorServiceInit(&g_uiDoorService);
  RelayControlServiceInit(&g_relayControlService);
  OutputTestServiceInit(&g_outputTestService);
  UnitClockAdapterInit(&s_unitClockCtx, &g_configurationService);
  g_unitClockPort = UnitClockAdapterCreatePort(&s_unitClockCtx);
  IntersectionEngineInit(&g_intersectionEngine);
  IntersectionActivationServiceInit(&g_intersectionActivationService);
  (void) IntersectionActivationServiceLoadCommittedLivePlan(
    &g_intersectionActivationService,
    ConfigurationServiceGetActiveConfig(&g_configurationService),
    ConfigurationServiceGetActiveSetId(&g_configurationService));
  (void) IntersectionEngineLoadConfig(&g_intersectionEngine,
                                      ConfigurationServiceGetActiveConfig(
                                        &g_configurationService));
  FieldInputCanAdapterInit(&s_fieldInputCanCtx,
                           ConfigurationServiceGetActiveSetId(
                             &g_configurationService),
                           &g_uiPowerService);
  s_fieldInputModuleBusPort =
    FieldInputCanAdapterCreatePort(&s_fieldInputCanCtx);
  (void) memset(&s_internalModuleBusPort, 0, sizeof(s_internalModuleBusPort));
  CompositeModuleBusPortInit(&s_compositeModuleBusCtx,
                             &s_fieldInputModuleBusPort,
                             &s_internalModuleBusPort);
  g_moduleBusPort = CompositeModuleBusPortCreatePort(&s_compositeModuleBusCtx);
  FieldOutputCanAdapterInit(&s_fieldOutputCanCtx,
                            &g_configurationService,
                            &g_rtcPort,
                            ConfigurationServiceGetActiveSetId(
                              &g_configurationService));
  g_outputDriverPort = FieldOutputCanAdapterCreatePort(&s_fieldOutputCanCtx);
  ControlBusAdapterInit(&s_controlBusCtx, &hfdcan2);
  s_controlBusPort = ControlBusAdapterCreatePort(&s_controlBusCtx);

  MmuAdapterInit(&s_mmuCtx);
  MmuAdapterBindRelayPort(&s_mmuCtx, &g_relayPort);
  MmuAdapterBindRelayControlService(&s_mmuCtx, &g_relayControlService);
  MmuAdapterSetRelayTopology(&s_mmuCtx,
                             MMU_RELAY_TOPOLOGY_LEGACY_ACTIVE_HIGH_CLOSE);
  g_mmuPort = MmuAdapterCreatePort(&s_mmuCtx);

  IntersectionOutputDispatcherInit(&g_intersectionOutputDispatcher);
  IntersectionOutputDispatcherBind(&g_intersectionOutputDispatcher,
                                   &g_intersectionEngine,
                                   &g_mmuPort,
                                   &g_outputDriverPort);
  IntersectionOutputDispatcherBindOutputTestService(
    &g_intersectionOutputDispatcher,
    &g_outputTestService);
  IntersectionControllerInit(&g_intersectionController);
  IntersectionControllerBind(&g_intersectionController,
                             &g_intersectionEngine,
                             &g_intersectionOutputDispatcher,
                             &g_intersectionActivationService,
                             &g_moduleBusPort,
                             &g_unitInputPort,
                             &g_mmuPort);
  IntersectionControllerSetExpectedModuleBusConfigEpoch(
    &g_intersectionController,
    ConfigurationServiceGetActiveSetId(&g_configurationService));
  CpMpLinkServiceInit(&g_cpMpLinkService,
                      &s_controlBusPort,
                      &g_configurationService,
                      &g_intersectionController);
  DetectorReportServiceInit(&g_detectorReportService);
  DetectorReportServiceBind(&g_detectorReportService,
                            &g_intersectionEngine,
                            &g_intersectionController,
                            &g_rtcPort);
  GlobalTimeManagementServiceInit(&g_globalTimeManagementService);
  GlobalTimeManagementServiceBind(&g_globalTimeManagementService,
                                  &g_intersectionEngine,
                                  &g_rtcPort);
  MmiServiceInit(&g_mmiService);
  MmiServiceBind(&g_mmiService,
                 &g_configurationService,
                 &g_intersectionEngine,
                 &g_intersectionController,
                 &g_detectorReportService,
                 &g_globalTimeManagementService,
                 &g_cpMpLinkService);
  MmiServiceBindUiPowerService(&g_mmiService, &g_uiPowerService);
  MmiServiceBindUiCommsIdentityService(&g_mmiService,
                                       &g_uiCommsIdentityService);
  MmiServiceBindUiDoorService(&g_mmiService, &g_uiDoorService);
  MmiServiceBindRelayControlService(&g_mmiService, &g_relayControlService);
  MmiServiceBindOutputTestService(&g_mmiService, &g_outputTestService);
  MmiSnapshotCacheInit(&g_mmiSnapshotCache);
  MmiSnapshotCacheBind(&g_mmiSnapshotCache,
                       &g_configurationService,
                       &g_intersectionEngine,
                       &g_intersectionController,
                       &g_detectorReportService,
                       &g_globalTimeManagementService,
                       &g_cpMpLinkService);
  MmiSnapshotCacheBindUiPowerService(&g_mmiSnapshotCache, &g_uiPowerService);
  MmiSnapshotCacheBindUiCommsIdentityService(&g_mmiSnapshotCache,
                                             &g_uiCommsIdentityService);
  MmiSnapshotCacheBindUiDoorService(&g_mmiSnapshotCache, &g_uiDoorService);
  MmiSnapshotCacheBindRelayControlService(&g_mmiSnapshotCache,
                                          &g_relayControlService);
  MmiSnapshotCacheBindOutputTestService(&g_mmiSnapshotCache,
                                        &g_outputTestService);
  MmiLocalSettingsServiceInit(&g_mmiLocalSettingsService);
  MmiLocalSettingsServiceBind(&g_mmiLocalSettingsService,
                              &s_modemConfigPort,
                              &s_gpsConfigPort,
                              &g_rtcPort,
                              &s_gpsTimeSyncPort,
                              &s_userSettingsPort,
                              &s_brokenInputSettingsPort,
                              &g_configurationService,
                              &g_globalTimeManagementService,
                              &g_userAuthService);
  MmiMaintenanceServiceInit(&g_mmiMaintenanceService);
  MmiMaintenanceServiceBind(&g_mmiMaintenanceService,
                            &s_controllerModePort,
                            &g_moduleBusPort,
                            &g_mmiLocalSettingsService,
                            &s_factoryResetPort);
  MmiMaintenanceServiceBindRelayControlService(&g_mmiMaintenanceService,
                                               &g_relayControlService);
  MmiMaintenanceServiceBindOutputTestService(&g_mmiMaintenanceService,
                                             &g_outputTestService);
  MmiCanAdapterInit(&s_mmiCanCtx,
                    &hfdcan1,
                    &g_mmiService,
                    &g_mmiSnapshotCache);
  MmiCanAdapterBindActivationService(&s_mmiCanCtx,
                                     &g_intersectionActivationService);
  MmiCanAdapterBindDetectorReportService(&s_mmiCanCtx,
                                         &g_detectorReportService);
  MmiCanAdapterBindGlobalTimeManagementService(&s_mmiCanCtx,
                                               &g_globalTimeManagementService);
  MmiCanAdapterBindDoorSensorPort(&s_mmiCanCtx, &g_doorPort);
  MmiCanAdapterBindHeaterPort(&s_mmiCanCtx, &g_heaterPort);
  MmiCanAdapterBindPowerMonitorPort(&s_mmiCanCtx, &g_powerMonitorPort);
  MmiCanAdapterBindUnitAlarmPort(&s_mmiCanCtx, &g_unitAlarmPort);
  MmiCanAdapterBindUnitClockPort(&s_mmiCanCtx, &g_unitClockPort);
  MmiCanAdapterBindCpMpLinkService(&s_mmiCanCtx, &g_cpMpLinkService);
  MmiCanAdapterBindLocalSettingsService(&s_mmiCanCtx,
                                        &g_mmiLocalSettingsService);
  MmiCanAdapterBindMaintenanceService(&s_mmiCanCtx,
                                      &g_mmiMaintenanceService);
  (void) IntersectionControllerStep(&g_intersectionController);
  DetectorReportServiceStep(&g_detectorReportService);
  GlobalTimeManagementServiceStep(&g_globalTimeManagementService);
  (void) MmiSnapshotCacheRefresh(&g_mmiSnapshotCache);

  LogRepositoryAdapterInit(&s_logRepositoryCtx, &s_eepromStoragePort);
  g_logRepositoryPort = LogRepositoryAdapterCreatePort(&s_logRepositoryCtx);
  LogEventAdapterInit(&s_logEventCtx);
  s_logEventPort = LogEventAdapterCreatePort(&s_logEventCtx);
  MmiEventLogServiceInit(&g_mmiEventLogService);
  MmiEventLogServiceBind(&g_mmiEventLogService, &g_logRepositoryPort);
  UiDoorServiceBind(&g_uiDoorService,
                    &g_doorPort,
                    &s_logEventPort,
                    &g_mmiEventLogService);
  UiDoorServiceRefreshLatestLogIndices(&g_uiDoorService);
  MmiCanAdapterBindEventLogService(&s_mmiCanCtx, &g_mmiEventLogService);

  /* Serial adapters — init order matches UART numbering for clarity.
   * SerialAdapterInit arms the first ReceiveToIdle_DMA and creates
   * the TX-done binary semaphore; both are safe before the scheduler
   * starts (osKernelInitialize has already been called by this point). */
  SerialAdapterInit(&s_modemCtx, &huart4, &hdma_uart4_rx,
                    s_modemRxDmaBuf, MODEM_RX_DMA_BUF_SIZE);
  SerialAdapterRegisterHalCallbacks(&huart4, &s_modemCtx);
  g_modemPort = SerialAdapterCreatePort(&s_modemCtx);

  SerialAdapterInit(&s_internalGpsCtx, &huart8, &hdma_uart8_rx,
                    s_internalGpsRxDmaBuf, INT_GPS_RX_DMA_BUF_SIZE);
  SerialAdapterRegisterHalCallbacks(&huart8, &s_internalGpsCtx);
  g_internalGpsPort = SerialAdapterCreatePort(&s_internalGpsCtx);

  SerialAdapterInit(&s_auxSerialCtx, &huart2, &hdma_usart2_rx,
                    s_auxSerialRxDmaBuf, AUX_SERIAL_RX_DMA_BUF_SIZE);
  SerialAdapterRegisterHalCallbacks(&huart2, &s_auxSerialCtx);
  g_auxSerialPort = SerialAdapterCreatePort(&s_auxSerialCtx);

  SerialAdapterInit(&s_wifiCtx, &huart5, &hdma_uart5_rx,
                    s_wifiRxDmaBuf, WIFI_RX_DMA_BUF_SIZE);
  SerialAdapterRegisterHalCallbacks(&huart5, &s_wifiCtx);
  g_wifiPort = SerialAdapterCreatePort(&s_wifiCtx);

  WireModemDriver();
} /* MainApplication_Init */
ControlBusAdapterCtx_t *MainApplicationGetControlBusAdapter(void)
{
  return &s_controlBusCtx;
}
