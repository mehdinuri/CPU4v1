/* App/Domain/Malfunction/MalfunctionEngine.c */

#include "Malfunction/MalfunctionEngine.h"

#include <stddef.h>
#include <string.h>

static void OnEmit(void *ctx, const FaultEvent_t *event)
{
  MalfunctionEngine_t *engine = (MalfunctionEngine_t *) ctx;

  if ((engine == NULL) || (event == NULL))
  {
    return;
  }

  engine->faultsEmittedLastTick++;
  engine->totalFaultsEmitted++;

  SafetyDecisionServiceOnFault(engine->safety, event);
  FaultMonitorServiceOnFault(engine->faultMonitor, event);
  UnitAlarmServiceOnFault(engine->unitAlarm, event);

  if (engine->eventLog != NULL)
  {
    EventLogRecord_t record;

    (void) memset(&record, 0, sizeof(record));
    record.eventCode = (uint16_t) event->code;
    record.source = event->source;
    record.timestampSeconds = event->timestampTicks / 100U;
    record.params[0] = (uint8_t) (event->param & 0xFFU);
    record.params[1] = (uint8_t) ((event->param >> 8U) & 0xFFU);
    record.params[2] = (uint8_t) ((event->param >> 16U) & 0xFFU);
    record.params[3] = (uint8_t) ((event->param >> 24U) & 0xFFU);
    record.params[4] = (uint8_t) event->severity;
    (void) EventLogAppend(engine->eventLog, &record);
  }
}

void MalfunctionEngineInit(MalfunctionEngine_t *engine,
                           const ConfigurationService_t *config,
                           IFieldBusPort_t *fieldBus,
                           IEventLogPort_t *eventLog,
                           SafetyDecisionService_t *safety,
                           FaultMonitorService_t *faultMonitor,
                           UnitAlarmService_t *unitAlarm)
{
  if (engine == NULL)
  {
    return;
  }

  (void) memset(engine, 0, sizeof(*engine));
  engine->config = config;
  engine->fieldBus = fieldBus;
  engine->eventLog = eventLog;
  engine->safety = safety;
  engine->faultMonitor = faultMonitor;
  engine->unitAlarm = unitAlarm;

  ConflictMonitorInit(&engine->conflict, 0U);
  DualIndicationMonitorInit(&engine->dualIndication, 0U);
  DarkChannelMonitorInit(&engine->darkChannel, 0U);
  RedFailMonitorInit(&engine->redFail, 0U);
  MinYellowMonitorInit(&engine->minYellow);
  ClearanceMonitorInit(&engine->clearance);
  SignalSequenceMonitorInit(&engine->signalSequence);
  LampFaultMonitorInit(&engine->lampFault, 0U);
  PowerSupplyMonitorInit(&engine->powerSupply, NULL);
  ModuleHealthMonitorInit(&engine->moduleHealth);
}

void MalfunctionEngineTick(MalfunctionEngine_t *engine)
{
  FieldBusSnapshot_t snapshot;
  ChannelStateImage_t commanded;
  ChannelStateImage_t measured;
  FaultEmit_t emit;

  if (engine == NULL)
  {
    return;
  }

  engine->tickCount++;
  engine->faultsEmittedLastTick = 0U;

  if (engine->fieldBus == NULL)
  {
    return;
  }

  if (FieldBusReadSnapshot(engine->fieldBus, &snapshot) == 0U)
  {
    return;
  }

  emit.fn = OnEmit;
  emit.ctx = engine;

  ModuleHealthMonitorTick(&engine->moduleHealth,
                          &snapshot,
                          engine->tickCount,
                          &emit);

  PowerSupplyMonitorTick(&engine->powerSupply,
                         &snapshot,
                         engine->tickCount,
                         &emit);

  /* Field-bus checks that require config must wait for a valid one. */
  if ((engine->config == NULL)
      || (ConfigurationServiceGetState(engine->config) != CONFIG_STATE_VALID))
  {
    return;
  }

  const ChannelOutputMapping_t *mapping =
    ConfigurationServiceGetMapping(engine->config);

  (void) ChannelStateResolverResolveCommanded(mapping,
                                              &snapshot.cpu.
                                              commandedLoadSwitches,
                                              &commanded);
  (void) ChannelStateResolverResolveMeasured(mapping,
                                             snapshot.ssm,
                                             FIELD_BUS_SSM_COUNT,
                                             &measured);

  ConflictMonitorTick(&engine->conflict,
                      engine->config,
                      &measured,
                      engine->tickCount,
                      &emit);
  DualIndicationMonitorTick(&engine->dualIndication,
                            &measured,
                            engine->tickCount,
                            &emit);
  DarkChannelMonitorTick(&engine->darkChannel,
                         &commanded,
                         &measured,
                         engine->tickCount,
                         &emit);
  RedFailMonitorTick(&engine->redFail,
                     &commanded,
                     &measured,
                     engine->tickCount,
                     &emit);
  MinYellowMonitorTick(&engine->minYellow,
                       engine->config,
                       &measured,
                       engine->tickCount,
                       &emit);
  ClearanceMonitorTick(&engine->clearance,
                       engine->config,
                       &measured,
                       engine->tickCount,
                       &emit);
  SignalSequenceMonitorTick(&engine->signalSequence,
                            &measured,
                            engine->tickCount,
                            &emit);
  LampFaultMonitorTick(&engine->lampFault,
                       &snapshot.cpu.commandedLoadSwitches,
                       snapshot.ssm,
                       FIELD_BUS_SSM_COUNT,
                       engine->tickCount,
                       &emit);
} /* MalfunctionEngineTick */

uint32_t MalfunctionEngineGetTick(const MalfunctionEngine_t *engine)
{
  return (engine != NULL) ? engine->tickCount : 0U;
}

uint32_t MalfunctionEngineGetTotalFaults(const MalfunctionEngine_t *engine)
{
  return (engine != NULL) ? engine->totalFaultsEmitted : 0U;
}
