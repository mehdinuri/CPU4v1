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
#include "Adapters/STM32/SignalCardAdapter.h"
#include "Adapters/STM32/MmuAdapter.h"
#include "Adapters/STM32/FieldInputCanAdapter.h"
#include "Adapters/STM32/CompositeModuleBusPort.h"
#include "Adapters/STM32/ModuleBusAdapter.h"
#include "Adapters/STM32/FlashStorageAdapter.h"
#include "Adapters/STM32/EepromStorageAdapter.h"
#include "Adapters/STM32/PersistenceAdapter.h"
#include "Adapters/STM32/ConfigRepositoryAdapter.h"
#include "Adapters/STM32/LogRepositoryAdapter.h"
#include "Adapters/STM32/ModemAdapter.h"
#include "Adapters/STM32/SerialAdapter.h"
#include "Adapters/STM32/UBloxModemAdapter.h"
#include "Adapters/STM32/TelitModemAdapter.h"
#include "Adapters/STM32/QuectelModemAdapter.h"
#include "Adapters/STM32/QuectelNtcipModemAdapter.h"
#include "Adapters/STM32/UsrModemAdapter.h"
#include "Adapters/STM32/EthernetNtcipModemAdapter.h"
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
static SignalCardAdapterCtx_t s_signalCardCtx;
static MmuAdapterCtx_t s_mmuCtx;
static FieldInputCanAdapterCtx_t s_fieldInputCanCtx;
static ModuleBusAdapterCtx_t s_moduleBusCtx;
static CompositeModuleBusPortCtx_t s_compositeModuleBusCtx;
static FlashStorageAdapterCtx_t s_flashStorageCtx;
static EepromStorageAdapterCtx_t s_eepromStorageCtx;
static PersistenceAdapterCtx_t s_persistenceCtx;
static ConfigRepositoryAdapterCtx_t s_configRepositoryCtx;
static LogRepositoryAdapterCtx_t s_logRepositoryCtx;
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

ConfigurationService_t g_configurationService;
IntersectionEngine_t g_intersectionEngine;
IntersectionActivationService_t g_intersectionActivationService;
IntersectionController_t g_intersectionController;
DetectorReportService_t g_detectorReportService;
GlobalTimeManagementService_t g_globalTimeManagementService;
IntersectionOutputDispatcher_t g_intersectionOutputDispatcher;

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

  PowerMonitorAdapterInit(&s_powerMonitorCtx);
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

  ConfigRepositoryAdapterInit(&s_configRepositoryCtx, &g_persistencePort);
  g_configRepositoryPort =
    ConfigRepositoryAdapterCreatePort(&s_configRepositoryCtx);

  ConfigurationServiceInit(&g_configurationService, &g_configRepositoryPort);
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
                             &g_configurationService));
  s_fieldInputModuleBusPort =
    FieldInputCanAdapterCreatePort(&s_fieldInputCanCtx);
  ModuleBusAdapterInit(&s_moduleBusCtx,
                       &hfdcan2,
                       ConfigurationServiceGetActiveSetId(
                         &g_configurationService));
  s_internalModuleBusPort = ModuleBusAdapterCreatePort(&s_moduleBusCtx);
  CompositeModuleBusPortInit(&s_compositeModuleBusCtx,
                             &s_fieldInputModuleBusPort,
                             &s_internalModuleBusPort);
  g_moduleBusPort = CompositeModuleBusPortCreatePort(&s_compositeModuleBusCtx);
  SignalCardAdapterInit(&s_signalCardCtx,
                        &hfdcan2,
                        ConfigurationServiceGetActiveSetId(
                          &g_configurationService));
  g_outputDriverPort = SignalCardAdapterCreatePort(&s_signalCardCtx);

  MmuAdapterInit(&s_mmuCtx);
  g_mmuPort = MmuAdapterCreatePort(&s_mmuCtx);

  IntersectionOutputDispatcherInit(&g_intersectionOutputDispatcher);
  IntersectionOutputDispatcherBind(&g_intersectionOutputDispatcher,
                                   &g_intersectionEngine,
                                   &g_mmuPort,
                                   &g_outputDriverPort);
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
  DetectorReportServiceInit(&g_detectorReportService);
  DetectorReportServiceBind(&g_detectorReportService,
                            &g_intersectionEngine,
                            &g_intersectionController,
                            &g_rtcPort);
  GlobalTimeManagementServiceInit(&g_globalTimeManagementService);
  GlobalTimeManagementServiceBind(&g_globalTimeManagementService,
                                  &g_intersectionEngine,
                                  &g_rtcPort);
  (void) IntersectionControllerStep(&g_intersectionController);
  DetectorReportServiceStep(&g_detectorReportService);
  GlobalTimeManagementServiceStep(&g_globalTimeManagementService);

  LogRepositoryAdapterInit(&s_logRepositoryCtx, &s_eepromStoragePort);
  g_logRepositoryPort = LogRepositoryAdapterCreatePort(&s_logRepositoryCtx);

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
