/* App/Domain/Malfunction/MalfunctionEngine.c */

#include "Malfunction/MalfunctionEngine.h"

#include <stddef.h>
#include <string.h>

#define MALFUNCTION_ENGINE_SELF_MONITOR_SAMPLE_TICKS 100U
#define MALFUNCTION_ENGINE_SELF_MONITOR_CONSECUTIVE_SAMPLES 3U
#define MALFUNCTION_ENGINE_BATTERY_LOW_THRESHOLD_MV 2800U
#define MALFUNCTION_ENGINE_BATTERY_RECOVERY_THRESHOLD_MV 3000U
#define MALFUNCTION_ENGINE_TEMPERATURE_HIGH_THRESHOLD_C 40
#define MALFUNCTION_ENGINE_TEMPERATURE_RECOVERY_THRESHOLD_C 38

static uint8_t RequiredSsmHealthy(const ChannelOutputMapping_t *mapping,
                                  const FieldBusSnapshot_t *snapshot)
{
  uint16_t requiredSsmMask = 0U;
  uint16_t healthySsmMask = 0U;
  uint32_t outputIndex;

  if ((mapping == NULL) || (snapshot == NULL))
  {
    return 0U;
  }

  for (outputIndex = 0U; outputIndex < MP_SIGNAL_OUTPUT_COUNT_MAX; outputIndex++)
  {
    const OutputChannelMap_t *entry = &mapping->outputs[outputIndex];
    uint8_t ssmIndex;

    if ((entry->channelIndex >= MP_CHANNEL_COUNT_MAX)
        || (entry->color == CHANNEL_COLOR_NONE))
    {
      continue;
    }

    ssmIndex = (uint8_t) (outputIndex / FIELD_BUS_SSM_OUTPUTS_PER_MODULE);
    if (ssmIndex >= FIELD_BUS_SSM_COUNT)
    {
      continue;
    }

    requiredSsmMask |= (uint16_t) (1U << ssmIndex);
    if (snapshot->ssm[ssmIndex].alive != 0U)
    {
      healthySsmMask |= (uint16_t) (1U << ssmIndex);
    }
  }

  if (requiredSsmMask == 0U)
  {
    return 0U;
  }

  return (uint8_t) (healthySsmMask == requiredSsmMask);
}

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

}

static void EmitSelfMonitorFault(const FaultEmit_t *emit,
                                 FaultCode_t code,
                                 uint32_t timestampTicks,
                                 uint32_t param)
{
  FaultEvent_t event;

  (void) memset(&event, 0, sizeof(event));
  event.code = code;
  event.severity = FAULT_SEVERITY_ERROR;
  event.timestampTicks = timestampTicks;
  event.param = param;
  FaultEmitPublish(emit, &event);
}

static void TickSystemMonitor(MalfunctionEngine_t *engine, const FaultEmit_t *emit)
{
  uint16_t batteryMilliVolts;
  int16_t temperatureDegC;

  if ((engine == NULL) || (emit == NULL) || (engine->systemMonitor == NULL))
  {
    return;
  }

  if ((engine->tickCount % MALFUNCTION_ENGINE_SELF_MONITOR_SAMPLE_TICKS) != 0U)
  {
    return;
  }

  if (SystemMonitorGetBatteryVoltageMilliVolts(engine->systemMonitor,
                                               &batteryMilliVolts) != 0U)
  {
    if (engine->batteryLowActive == 0U)
    {
      engine->batteryRecoverySampleCount = 0U;
      if (batteryMilliVolts < MALFUNCTION_ENGINE_BATTERY_LOW_THRESHOLD_MV)
      {
        if (engine->batteryLowSampleCount < UINT8_MAX)
        {
          engine->batteryLowSampleCount++;
        }
      }
      else
      {
        engine->batteryLowSampleCount = 0U;
      }

      if (engine->batteryLowSampleCount
          >= MALFUNCTION_ENGINE_SELF_MONITOR_CONSECUTIVE_SAMPLES)
      {
        EmitSelfMonitorFault(emit,
                             FAULT_CODE_MP_BATTERY_LOW,
                             engine->tickCount,
                             batteryMilliVolts);
        engine->batteryLowActive = 1U;
        engine->batteryLowSampleCount = 0U;
      }
    }
    else
    {
      engine->batteryLowSampleCount = 0U;
      if (batteryMilliVolts > MALFUNCTION_ENGINE_BATTERY_RECOVERY_THRESHOLD_MV)
      {
        if (engine->batteryRecoverySampleCount < UINT8_MAX)
        {
          engine->batteryRecoverySampleCount++;
        }
      }
      else
      {
        engine->batteryRecoverySampleCount = 0U;
      }

      if (engine->batteryRecoverySampleCount
          >= MALFUNCTION_ENGINE_SELF_MONITOR_CONSECUTIVE_SAMPLES)
      {
        engine->batteryLowActive = 0U;
        engine->batteryRecoverySampleCount = 0U;
      }
    }
  }

  if (SystemMonitorGetThermistorDegC(engine->systemMonitor,
                                     &temperatureDegC) != 0U)
  {
    if (engine->temperatureHighActive == 0U)
    {
      engine->temperatureRecoverySampleCount = 0U;
      if (temperatureDegC > MALFUNCTION_ENGINE_TEMPERATURE_HIGH_THRESHOLD_C)
      {
        if (engine->temperatureHighSampleCount < UINT8_MAX)
        {
          engine->temperatureHighSampleCount++;
        }
      }
      else
      {
        engine->temperatureHighSampleCount = 0U;
      }

      if (engine->temperatureHighSampleCount
          >= MALFUNCTION_ENGINE_SELF_MONITOR_CONSECUTIVE_SAMPLES)
      {
        EmitSelfMonitorFault(emit,
                             FAULT_CODE_MP_TEMPERATURE_HIGH,
                             engine->tickCount,
                             (uint32_t) temperatureDegC);
        engine->temperatureHighActive = 1U;
        engine->temperatureHighSampleCount = 0U;
      }
    }
    else
    {
      engine->temperatureHighSampleCount = 0U;
      if (temperatureDegC < MALFUNCTION_ENGINE_TEMPERATURE_RECOVERY_THRESHOLD_C)
      {
        if (engine->temperatureRecoverySampleCount < UINT8_MAX)
        {
          engine->temperatureRecoverySampleCount++;
        }
      }
      else
      {
        engine->temperatureRecoverySampleCount = 0U;
      }

      if (engine->temperatureRecoverySampleCount
          >= MALFUNCTION_ENGINE_SELF_MONITOR_CONSECUTIVE_SAMPLES)
      {
        engine->temperatureHighActive = 0U;
        engine->temperatureRecoverySampleCount = 0U;
      }
    }
  }
}

void MalfunctionEngineInit(MalfunctionEngine_t *engine,
                           const ConfigurationService_t *config,
                           IFieldBusPort_t *fieldBus,
                           ISystemMonitorPort_t *systemMonitor,
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
  engine->systemMonitor = systemMonitor;
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

  if (engine->safety != NULL)
  {
    SafetyDecisionServiceSetRequiredSsmHealthy(engine->safety, 0U);
  }

  ModuleHealthMonitorTick(&engine->moduleHealth,
                          &snapshot,
                          engine->tickCount,
                          &emit);

  PowerSupplyMonitorTick(&engine->powerSupply,
                         &snapshot,
                         engine->tickCount,
                         &emit);
  TickSystemMonitor(engine, &emit);

  /* Field-bus checks that require config must wait for a valid one. */
  if ((engine->config == NULL)
      || (ConfigurationServiceGetState(engine->config) != CONFIG_STATE_VALID))
  {
    return;
  }

  const ChannelOutputMapping_t *mapping =
    ConfigurationServiceGetMapping(engine->config);

  if (engine->safety != NULL)
  {
    SafetyDecisionServiceSetRequiredSsmHealthy(engine->safety,
                                               RequiredSsmHealthy(mapping,
                                                                  &snapshot));
  }

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
