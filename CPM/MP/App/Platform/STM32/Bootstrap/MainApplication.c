/* App/Platform/STM32/Bootstrap/MainApplication.c */

#include "MainApplication.h"

#include <stddef.h>

#include "Adapters/STM32/ControlBusAdapter.h"
#include "Adapters/STM32/EventLogAdapter.h"
#include "Adapters/STM32/FieldBusAdapter.h"
#include "Adapters/STM32/PersistenceAdapter.h"
#include "Adapters/STM32/RTCAdapter.h"
#include "Adapters/STM32/SafetyRelayAdapter.h"
#include "Adapters/STM32/StatusLEDAdapter.h"
#include "Adapters/STM32/SystemMonitorAdapter.h"
#include "Adapters/STM32/UnitAlarmAdapter.h"
#include "Adapters/STM32/WatchdogAdapter.h"
#include "Bootstrap/DomainServices.h"
#include "Bootstrap/HardwarePorts.h"
#include "Platform/STM32/Tasks/Tasks.h"

#include "fdcan.h"

/* Global port handles (declared extern in HardwarePorts.h) */
IControlBusPort_t g_controlBusPort;
IFieldBusPort_t g_fieldBusPort;
ISafetyRelayPort_t g_safetyRelayPort;
IStatusLEDPort_t g_statusLEDPort;
IEventLogPort_t g_eventLogPort;
IPersistencePort_t g_persistencePort;
IRealtimeClockPort_t g_rtcPort;
ISystemMonitorPort_t g_systemMonitorPort;
IWatchdogPort_t g_watchdogPort;
IUnitAlarmPort_t g_unitAlarmPort;

/* Global domain service instances */
ConfigurationService_t g_configurationService;
CpMpLinkService_t g_cpMpLinkService;
SafetyDecisionService_t g_safetyDecisionService;
FaultMonitorService_t g_faultMonitorService;
UnitAlarmService_t g_unitAlarmService;
MalfunctionEngine_t g_malfunctionEngine;

/* Static adapter contexts - one per hardware resource. */
static ControlBusAdapterCtx_t s_controlBusCtx;
static FieldBusAdapterCtx_t s_fieldBusCtx;
static SafetyRelayAdapterCtx_t s_safetyRelayCtx;
static StatusLEDAdapterCtx_t s_statusLEDCtx;
static EventLogAdapterCtx_t s_eventLogCtx;
static PersistenceAdapterCtx_t s_persistenceCtx;
static RTCAdapterCtx_t s_rtcCtx;
static SystemMonitorAdapterCtx_t s_systemMonitorCtx;
static WatchdogAdapterCtx_t s_watchdogCtx;
static UnitAlarmAdapterCtx_t s_unitAlarmCtx;

FieldBusAdapterCtx_t *MainApplicationGetFieldBusAdapter(void)
{
  return &s_fieldBusCtx;
}

ControlBusAdapterCtx_t *MainApplicationGetControlBusAdapter(void)
{
  return &s_controlBusCtx;
}

void MainApplication_Init(void)
{
  /* 1. Hardware adapters -------------------------------------------- */
  ControlBusAdapterInit(&s_controlBusCtx, &hfdcan2);
  FieldBusAdapterInit(&s_fieldBusCtx, &hfdcan1);
  SafetyRelayAdapterInit(&s_safetyRelayCtx);
  SafetyRelayAdapterSetTopology(&s_safetyRelayCtx,
                                SAFETY_RELAY_TOPOLOGY_LEGACY_ACTIVE_HIGH_CLOSE);
  StatusLEDAdapterInit(&s_statusLEDCtx);
  EventLogAdapterInit(&s_eventLogCtx);
  PersistenceAdapterInit(&s_persistenceCtx);
  RTCAdapterInit(&s_rtcCtx);
  SystemMonitorAdapterInit(&s_systemMonitorCtx);
  WatchdogAdapterInit(&s_watchdogCtx);
  UnitAlarmAdapterInit(&s_unitAlarmCtx);

  /* 2. Create port instances over those adapters -------------------- */
  g_controlBusPort = ControlBusAdapterCreatePort(&s_controlBusCtx);
  g_fieldBusPort = FieldBusAdapterCreatePort(&s_fieldBusCtx);
  g_safetyRelayPort = SafetyRelayAdapterCreatePort(&s_safetyRelayCtx);
  g_statusLEDPort = StatusLEDAdapterCreatePort(&s_statusLEDCtx);
  g_eventLogPort = EventLogAdapterCreatePort(&s_eventLogCtx);
  g_persistencePort = PersistenceAdapterCreatePort(&s_persistenceCtx);
  g_rtcPort = RTCAdapterCreatePort(&s_rtcCtx);
  g_systemMonitorPort = SystemMonitorAdapterCreatePort(&s_systemMonitorCtx);
  g_watchdogPort = WatchdogAdapterCreatePort(&s_watchdogCtx);
  g_unitAlarmPort = UnitAlarmAdapterCreatePort(&s_unitAlarmCtx);

  /* 3. Domain services --------------------------------------------- */
  ConfigurationServiceInit(&g_configurationService, &g_persistencePort);
  SafetyDecisionServiceInit(&g_safetyDecisionService, &g_safetyRelayPort);
  SafetyDecisionServiceReset(&g_safetyDecisionService);
  FaultMonitorServiceInit(&g_faultMonitorService);
  UnitAlarmServiceInit(&g_unitAlarmService, &g_unitAlarmPort);
  MalfunctionEngineInit(&g_malfunctionEngine,
                        &g_configurationService,
                        &g_fieldBusPort,
                        &g_eventLogPort,
                        &g_safetyDecisionService,
                        &g_faultMonitorService,
                        &g_unitAlarmService);
  CpMpLinkServiceInit(&g_cpMpLinkService,
                      &g_controlBusPort,
                      &g_configurationService,
                      &g_safetyDecisionService,
                      &g_faultMonitorService);

  CANStart(&hfdcan1);
  CANStart(&hfdcan2);

  /* 4. Application tasks ------------------------------------------- */
  Tasks_Start();
} /* MainApplication_Init */
