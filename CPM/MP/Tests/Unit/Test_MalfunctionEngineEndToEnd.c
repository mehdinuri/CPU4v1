/* Tests/Unit/Test_MalfunctionEngineEndToEnd.c
 *
 * Integration-style smoke test for the MP Domain: stands up the
 * MalfunctionEngine against mocked ports, feeds a conflict scenario
 * through the field bus, and asserts the safety relay drops, the
 * FaultMonitorStatus lights up, and the UnitAlarm bits flip.
 */

#include <stdint.h>
#include <string.h>

#include "unity.h"

#include "Adapters/Mock/MockFieldBusAdapter.h"
#include "Adapters/Mock/MockSafetyRelayAdapter.h"
#include "Adapters/Mock/MockSystemMonitorAdapter.h"
#include "Adapters/Mock/MockUnitAlarmAdapter.h"
#include "Alarm/UnitAlarmService.h"
#include "FaultMonitor/FaultMonitorService.h"
#include "Intersection/ConfigurationService.h"
#include "Malfunction/MalfunctionEngine.h"
#include "Malfunction/SafetyDecisionService.h"

static MockFieldBusAdapterCtx_t g_fieldBus;
static MockSafetyRelayAdapterCtx_t g_relay;
static MockSystemMonitorAdapterCtx_t g_systemMonitor;
static MockUnitAlarmAdapterCtx_t g_alarm;

static IFieldBusPort_t g_fieldBusPort;
static ISafetyRelayPort_t g_relayPort;
static ISystemMonitorPort_t g_systemMonitorPort;
static IUnitAlarmPort_t g_alarmPort;

static ConfigurationService_t g_config;
static SafetyDecisionService_t g_safety;
static FaultMonitorService_t g_faultMonitor;
static UnitAlarmService_t g_unitAlarm;
static MalfunctionEngine_t g_engine;

static void SeedConflictConfig(void)
{
  IntersectionConfig_t cfg;
  ChannelOutputMapping_t mapping;

  (void) memset(&cfg, 0, sizeof(cfg));
  cfg.phaseCount = 2U;
  cfg.ringCount = 1U;
  cfg.barrierCount = 1U;

  /* Phase 1 and Phase 2 are not concurrent -> conflict. */
  cfg.phases[0].yellowChangeDs = 30U;
  cfg.phases[0].redClearDs = 20U;
  cfg.phases[0].concurrency.length = 0U;
  cfg.phases[1].yellowChangeDs = 30U;
  cfg.phases[1].redClearDs = 20U;
  cfg.phases[1].concurrency.length = 0U;

  /* Channel 0 controlled by phase 1 (vehicle); channel 1 by phase 2. */
  cfg.channels[0].controlSource = 1U;
  cfg.channels[0].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  cfg.channels[1].controlSource = 2U;
  cfg.channels[1].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;

  (void) ConfigurationServiceSetConfig(&g_config, &cfg);

  /* Output 0 -> channel 0 GREEN; output 1 -> channel 0 YELLOW;
   * output 2 -> channel 0 RED; likewise 3..5 for channel 1. */
  ChannelStateResolverInit(&mapping);
  mapping.outputs[0].channelIndex = 0U;
  mapping.outputs[0].color = CHANNEL_COLOR_GREEN;
  mapping.outputs[1].channelIndex = 0U;
  mapping.outputs[1].color = CHANNEL_COLOR_YELLOW;
  mapping.outputs[2].channelIndex = 0U;
  mapping.outputs[2].color = CHANNEL_COLOR_RED;
  mapping.outputs[3].channelIndex = 1U;
  mapping.outputs[3].color = CHANNEL_COLOR_GREEN;
  mapping.outputs[4].channelIndex = 1U;
  mapping.outputs[4].color = CHANNEL_COLOR_YELLOW;
  mapping.outputs[5].channelIndex = 1U;
  mapping.outputs[5].color = CHANNEL_COLOR_RED;

  (void) ConfigurationServiceSetOutputMapping(&g_config, &mapping);
  TEST_ASSERT_EQUAL_UINT8(1U, ConfigurationServiceValidate(&g_config));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          ConfigurationServiceChannelsConflict(&g_config, 0U,
                                                               1U));
} /* SeedConflictConfig */

static void SeedSnapshotGreenGreen(FieldBusSnapshot_t *snapshot)
{
  (void) memset(snapshot, 0, sizeof(*snapshot));
  snapshot->cpu.cpAlive = 1U;

  /* All SSMs alive, and outputs 0 (ch0 GREEN) + 3 (ch1 GREEN) both
   * report voltage presence. SSM 0 carries outputs 0..11. */
  snapshot->ssm[0].alive = 1U;
  snapshot->ssm[0].voltagePresenceBits = (uint16_t) ((1U << 0U) | (1U << 3U));

  uint8_t i;

  for (i = 1U; i < FIELD_BUS_SSM_COUNT; i++)
  {
    snapshot->ssm[i].alive = 1U;
  }

  for (i = 0U; i < FIELD_BUS_PSM_COUNT; i++)
  {
    snapshot->psm[i].alive = 1U;
    snapshot->psm[i].lineVoltageAdcCounts = 302U; /* mid-range */
    snapshot->psm[i].lineFrequencyRaw = 100U;
    snapshot->psm[i].rail24V1AdcCounts = 100U;
    snapshot->psm[i].rail24V2AdcCounts = 100U;
    snapshot->psm[i].rail5V1AdcCounts = 100U;
    snapshot->psm[i].rail5V2AdcCounts = 100U;
  }
}

static uint32_t CountEventsInRange(FaultMonitorService_t *service,
                                   uint32_t firstSequence,
                                   uint32_t lastSequence,
                                   FaultCode_t code,
                                   uint32_t expectedParam)
{
  FaultEvent_t event;
  uint32_t count = 0U;
  uint32_t sequence;

  for (sequence = firstSequence; sequence <= lastSequence; sequence++)
  {
    if ((FaultMonitorServiceGetEventBySequence(service, sequence, &event) != 0U)
        && (event.code == code) && (event.param == expectedParam))
    {
      count++;
    }
  }

  return count;
}

void setUp(void)
{
  MockFieldBusAdapterInit(&g_fieldBus);
  MockSafetyRelayAdapterInit(&g_relay);
  MockSystemMonitorAdapterInit(&g_systemMonitor);
  MockUnitAlarmAdapterInit(&g_alarm);

  g_fieldBusPort = MockFieldBusAdapterCreatePort(&g_fieldBus);
  g_relayPort = MockSafetyRelayAdapterCreatePort(&g_relay);
  g_systemMonitorPort = MockSystemMonitorAdapterCreatePort(&g_systemMonitor);
  g_alarmPort = MockUnitAlarmAdapterCreatePort(&g_alarm);

  ConfigurationServiceInit(&g_config, NULL);
  SafetyDecisionServiceInit(&g_safety, &g_relayPort);
  FaultMonitorServiceInit(&g_faultMonitor);
  UnitAlarmServiceInit(&g_unitAlarm, &g_alarmPort);

  MalfunctionEngineInit(&g_engine,
                        &g_config,
                        &g_fieldBusPort,
                        &g_systemMonitorPort,
                        &g_safety,
                        &g_faultMonitor,
                        &g_unitAlarm);

  /* Pretend relay starts closed at power-up. */
  (void) SafetyRelaySetState(&g_relayPort, SAFETY_RELAY_STATE_CLOSED);
  g_relay.transitionCount = 0U;
}

void tearDown(void)
{
}

void test_engine_idles_when_no_config(void)
{
  FieldBusSnapshot_t snapshot;

  SeedSnapshotGreenGreen(&snapshot);
  MockFieldBusAdapterSetSnapshot(&g_fieldBus, &snapshot);

  MalfunctionEngineTick(&g_engine);

  /* No config yet -> no per-channel monitors run, so no conflict
   * fault; safety relay should still be closed. */
  TEST_ASSERT_EQUAL(SAFETY_ACTION_NONE,
                    SafetyDecisionServiceGetLatchedAction(&g_safety));
}

void test_conflict_drops_safety_relay(void)
{
  FieldBusSnapshot_t snapshot;
  uint32_t i;
  SafetyRelayState_t relayState;

  SeedConflictConfig();
  SeedSnapshotGreenGreen(&snapshot);
  MockFieldBusAdapterSetSnapshot(&g_fieldBus, &snapshot);

  /* Conflict dwell threshold is 400 ms (40 ticks). Tick 50 times to
   * leave headroom for the first-pass init of some monitors. */
  for (i = 0U; i < 50U; i++)
  {
    MalfunctionEngineTick(&g_engine);
  }

  TEST_ASSERT_EQUAL(SAFETY_ACTION_DARK,
                    SafetyDecisionServiceGetLatchedAction(&g_safety));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          SafetyRelayGetActualState(&g_relayPort, &relayState));
  TEST_ASSERT_EQUAL(SAFETY_RELAY_STATE_OPEN, relayState);

  FaultMonitorStatus_t status;

  FaultMonitorServiceRead(&g_faultMonitor, &status);
  TEST_ASSERT_EQUAL_UINT8(1U, status.channels[1].conflict);

  uint8_t alarm2 = 0U;

  TEST_ASSERT_EQUAL_UINT8(1U,
                          UnitAlarmPortGetUnitAlarmStatus2(&g_alarmPort,
                                                           &alarm2));
  TEST_ASSERT_TRUE((alarm2 & UNIT_ALARM_STATUS2_CONFLICT) != 0U);

}

void test_module_missing_flags_alarm(void)
{
  FieldBusSnapshot_t snapshot;
  uint32_t i;
  SafetyRelayState_t relayState;

  SeedConflictConfig();
  SeedSnapshotGreenGreen(&snapshot);

  /* Kill SSM 3 - no alive pulse. */
  snapshot.ssm[3].alive = 0U;
  MockFieldBusAdapterSetSnapshot(&g_fieldBus, &snapshot);

  for (i = 0U; i < 20U; i++)
  {
    MalfunctionEngineTick(&g_engine);
  }

  uint8_t alarm1 = 0U;

  TEST_ASSERT_EQUAL_UINT8(1U,
                          UnitAlarmPortGetUnitAlarmStatus1(&g_alarmPort,
                                                           &alarm1));
  TEST_ASSERT_TRUE((alarm1 & UNIT_ALARM_STATUS1_MODULE_MISSING) != 0U);
  TEST_ASSERT_EQUAL(SAFETY_ACTION_DARK,
                    SafetyDecisionServiceGetLatchedAction(&g_safety));
  TEST_ASSERT_EQUAL_UINT8(1U, SafetyRelayGetActualState(&g_relayPort, &relayState));
  TEST_ASSERT_EQUAL(SAFETY_RELAY_STATE_OPEN, relayState);
}

void test_battery_low_emits_fault_after_three_samples(void)
{
  FieldBusSnapshot_t snapshot;
  uint8_t alarm1 = 0U;
  uint32_t totalFaultsBefore;
  uint32_t totalFaultsAfter;
  uint32_t i;

  SeedConflictConfig();
  SeedSnapshotGreenGreen(&snapshot);
  MockFieldBusAdapterSetSnapshot(&g_fieldBus, &snapshot);
  g_systemMonitor.batteryVoltageMilliVolts = 2700U;
  totalFaultsBefore = FaultMonitorServiceGetTotalFaults(&g_faultMonitor);

  for (i = 0U; i < 300U; i++)
  {
    MalfunctionEngineTick(&g_engine);
  }

  totalFaultsAfter = FaultMonitorServiceGetTotalFaults(&g_faultMonitor);
  TEST_ASSERT_TRUE(totalFaultsAfter > totalFaultsBefore);
  TEST_ASSERT_GREATER_THAN_UINT32(0U,
                                  CountEventsInRange(&g_faultMonitor,
                                                     totalFaultsBefore + 1U,
                                                     totalFaultsAfter,
                                                     FAULT_CODE_MP_BATTERY_LOW,
                                                     2700U));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          UnitAlarmPortGetUnitAlarmStatus1(&g_alarmPort,
                                                           &alarm1));
  TEST_ASSERT_TRUE((alarm1 & UNIT_ALARM_STATUS1_BATTERY_LOW) != 0U);
}

void test_temperature_high_rearms_after_recovery(void)
{
  FieldBusSnapshot_t snapshot;
  uint32_t totalFaultsBefore;
  uint32_t totalFaultsAfter;
  uint32_t i;

  SeedConflictConfig();
  SeedSnapshotGreenGreen(&snapshot);
  MockFieldBusAdapterSetSnapshot(&g_fieldBus, &snapshot);
  g_systemMonitor.thermistorDegC = 41;
  totalFaultsBefore = FaultMonitorServiceGetTotalFaults(&g_faultMonitor);

  for (i = 0U; i < 300U; i++)
  {
    MalfunctionEngineTick(&g_engine);
  }

  g_systemMonitor.thermistorDegC = 37;
  for (i = 0U; i < 300U; i++)
  {
    MalfunctionEngineTick(&g_engine);
  }

  g_systemMonitor.thermistorDegC = 42;
  for (i = 0U; i < 300U; i++)
  {
    MalfunctionEngineTick(&g_engine);
  }

  totalFaultsAfter = FaultMonitorServiceGetTotalFaults(&g_faultMonitor);
  TEST_ASSERT_TRUE(totalFaultsAfter > (totalFaultsBefore + 1U));
  TEST_ASSERT_EQUAL_UINT32(1U,
                           CountEventsInRange(&g_faultMonitor,
                                              totalFaultsBefore + 1U,
                                              totalFaultsAfter,
                                              FAULT_CODE_MP_TEMPERATURE_HIGH,
                                              41U));
  TEST_ASSERT_EQUAL_UINT32(1U,
                           CountEventsInRange(&g_faultMonitor,
                                              totalFaultsBefore + 1U,
                                              totalFaultsAfter,
                                              FAULT_CODE_MP_TEMPERATURE_HIGH,
                                              42U));
}

void test_read_and_ack_clears_deltas(void)
{
  FieldBusSnapshot_t snapshot;
  FaultMonitorStatus_t status;
  uint32_t i;

  SeedConflictConfig();
  SeedSnapshotGreenGreen(&snapshot);
  MockFieldBusAdapterSetSnapshot(&g_fieldBus, &snapshot);

  for (i = 0U; i < 50U; i++)
  {
    MalfunctionEngineTick(&g_engine);
  }

  FaultMonitorServiceReadAndAck(&g_faultMonitor, &status);
  TEST_ASSERT_EQUAL_UINT8(1U, status.channels[1].conflict);

  /* A second read-and-ack with no new faults reports clear. */
  FaultMonitorServiceReadAndAck(&g_faultMonitor, &status);
  TEST_ASSERT_EQUAL_UINT8(0U, status.channels[1].conflict);

  /* But the cumulative snapshot still reports the fault. */
  FaultMonitorServiceRead(&g_faultMonitor, &status);
  TEST_ASSERT_EQUAL_UINT8(1U, status.channels[1].conflict);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_engine_idles_when_no_config);
  RUN_TEST(test_conflict_drops_safety_relay);
  RUN_TEST(test_module_missing_flags_alarm);
  RUN_TEST(test_battery_low_emits_fault_after_three_samples);
  RUN_TEST(test_temperature_high_rearms_after_recovery);
  RUN_TEST(test_read_and_ack_clears_deltas);

  return UNITY_END();
}
