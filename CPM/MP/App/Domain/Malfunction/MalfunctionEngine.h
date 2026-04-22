/* App/Domain/Malfunction/MalfunctionEngine.h
 *
 * 10 ms orchestrator for the TS2 MMU behaviour. Owns every monitor
 * context. Per tick: pulls a field-bus snapshot, derives commanded
 * and measured channel-state images, runs all monitors, and forwards
 * emitted faults into the safety-decision service, fault-monitor
 * surface, and unit alarm service.
 *
 * The engine has no HAL or RTOS dependencies; a platform task calls
 * MalfunctionEngineTick() on a 10 ms period.
 */
#ifndef MALFUNCTION_ENGINE_H
#define MALFUNCTION_ENGINE_H

#include <stdint.h>

#include "Alarm/UnitAlarmService.h"
#include "FaultMonitor/FaultMonitorService.h"
#include "Intersection/ChannelStateResolver.h"
#include "Intersection/ConfigurationService.h"
#include "Malfunction/ClearanceMonitor.h"
#include "Malfunction/ConflictMonitor.h"
#include "Malfunction/DarkChannelMonitor.h"
#include "Malfunction/DualIndicationMonitor.h"
#include "Malfunction/LampFaultMonitor.h"
#include "Malfunction/MinYellowMonitor.h"
#include "Malfunction/ModuleHealthMonitor.h"
#include "Malfunction/PowerSupplyMonitor.h"
#include "Malfunction/RedFailMonitor.h"
#include "Malfunction/SafetyDecisionService.h"
#include "Malfunction/SignalSequenceMonitor.h"
#include "Ports/IFieldBusPort.h"
#include "Ports/ISystemMonitorPort.h"

typedef struct
{
  /* Collaborators (not owned) */
  const ConfigurationService_t *config;
  IFieldBusPort_t *fieldBus;
  ISystemMonitorPort_t *systemMonitor;
  SafetyDecisionService_t *safety;
  FaultMonitorService_t *faultMonitor;
  UnitAlarmService_t *unitAlarm;

  /* Owned monitor contexts */
  ConflictMonitor_t conflict;
  DualIndicationMonitor_t dualIndication;
  DarkChannelMonitor_t darkChannel;
  RedFailMonitor_t redFail;
  MinYellowMonitor_t minYellow;
  ClearanceMonitor_t clearance;
  SignalSequenceMonitor_t signalSequence;
  LampFaultMonitor_t lampFault;
  PowerSupplyMonitor_t powerSupply;
  ModuleHealthMonitor_t moduleHealth;

  /* Tick counter drives fault timestamps. */
  uint32_t tickCount;
  uint32_t faultsEmittedLastTick;
  uint32_t totalFaultsEmitted;
  uint8_t batteryLowActive;
  uint8_t temperatureHighActive;
  uint8_t batteryLowSampleCount;
  uint8_t batteryRecoverySampleCount;
  uint8_t temperatureHighSampleCount;
  uint8_t temperatureRecoverySampleCount;
} MalfunctionEngine_t;

void MalfunctionEngineInit(MalfunctionEngine_t *engine,
                           const ConfigurationService_t *config,
                           IFieldBusPort_t *fieldBus,
                           ISystemMonitorPort_t *systemMonitor,
                           SafetyDecisionService_t *safety,
                           FaultMonitorService_t *faultMonitor,
                           UnitAlarmService_t *unitAlarm);
void MalfunctionEngineTick(MalfunctionEngine_t *engine);
uint32_t MalfunctionEngineGetTick(const MalfunctionEngine_t *engine);
uint32_t MalfunctionEngineGetTotalFaults(const MalfunctionEngine_t *engine);

#endif /* MALFUNCTION_ENGINE_H */
