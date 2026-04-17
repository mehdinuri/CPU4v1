/* Tests/Unit/Test_DomainSurface.c
 *
 * Covers the full surface of UnitAlarmService, FaultMonitorService,
 * SafetyDecisionService and PowerSupplyMonitor to push App/Domain
 * line coverage over the 80% plan target. These are small, focused
 * tests: each drives a single branch or fault-code path.
 */

#include <stdint.h>
#include <string.h>

#include "unity.h"

#include "Adapters/Mock/MockSafetyRelayAdapter.h"
#include "Adapters/Mock/MockUnitAlarmAdapter.h"
#include "Alarm/UnitAlarmService.h"
#include "FaultMonitor/FaultMonitorService.h"
#include "Malfunction/PowerSupplyMonitor.h"
#include "Malfunction/SafetyDecisionService.h"

static MockUnitAlarmAdapterCtx_t g_alarmCtx;
static IUnitAlarmPort_t g_alarmPort;

void setUp(void)
{
  MockUnitAlarmAdapterInit(&g_alarmCtx);
  g_alarmPort = MockUnitAlarmAdapterCreatePort(&g_alarmCtx);
}

void tearDown(void)
{
}

static void EmitCode(UnitAlarmService_t *service, FaultCode_t code)
{
  FaultEvent_t e;

  (void) memset(&e, 0, sizeof(e));
  e.code = code;
  e.severity = FAULT_SEVERITY_ERROR;
  UnitAlarmServiceOnFault(service, &e);
}

/* -------- UnitAlarmService -------- */

void test_unit_alarm_maps_conflict_and_status2_bits(void)
{
  UnitAlarmService_t svc;
  uint8_t status2 = 0U;

  UnitAlarmServiceInit(&svc, &g_alarmPort);
  EmitCode(&svc, FAULT_CODE_CONFLICT_GREEN_GREEN);
  EmitCode(&svc, FAULT_CODE_DUAL_INDICATION);
  EmitCode(&svc, FAULT_CODE_RED_FAIL);
  EmitCode(&svc, FAULT_CODE_CLEARANCE_SHORT);
  EmitCode(&svc, FAULT_CODE_MIN_YELLOW_SHORT);
  EmitCode(&svc, FAULT_CODE_DARK_CHANNEL);
  EmitCode(&svc, FAULT_CODE_SIGNAL_SEQUENCE);

  TEST_ASSERT_EQUAL_UINT8(1U,
                          UnitAlarmPortGetUnitAlarmStatus2(&g_alarmPort,
                                                           &status2));
  TEST_ASSERT_TRUE((status2 & UNIT_ALARM_STATUS2_CONFLICT) != 0U);
  TEST_ASSERT_TRUE((status2 & UNIT_ALARM_STATUS2_DUAL_INDICATION) != 0U);
  TEST_ASSERT_TRUE((status2 & UNIT_ALARM_STATUS2_RED_FAIL) != 0U);
  TEST_ASSERT_TRUE((status2 & UNIT_ALARM_STATUS2_CLEARANCE) != 0U);
  TEST_ASSERT_TRUE((status2 & UNIT_ALARM_STATUS2_MIN_YELLOW) != 0U);
  TEST_ASSERT_TRUE((status2 & UNIT_ALARM_STATUS2_DARK_CHANNEL) != 0U);
  TEST_ASSERT_TRUE((status2 & UNIT_ALARM_STATUS2_SIGNAL_SEQUENCE) != 0U);
}

void test_unit_alarm_maps_module_and_battery_to_status1(void)
{
  UnitAlarmService_t svc;
  uint8_t status1 = 0U;

  UnitAlarmServiceInit(&svc, &g_alarmPort);
  EmitCode(&svc, FAULT_CODE_MODULE_CP_MISSING);
  EmitCode(&svc, FAULT_CODE_MP_BATTERY_LOW);
  EmitCode(&svc, FAULT_CODE_MP_TEMPERATURE_HIGH);
  EmitCode(&svc, FAULT_CODE_MP_CONFIG_INVALID);
  EmitCode(&svc, FAULT_CODE_LAMP_OPEN_CIRCUIT);

  TEST_ASSERT_EQUAL_UINT8(1U,
                          UnitAlarmPortGetUnitAlarmStatus1(&g_alarmPort,
                                                           &status1));
  TEST_ASSERT_TRUE((status1 & UNIT_ALARM_STATUS1_MODULE_MISSING) != 0U);
  TEST_ASSERT_TRUE((status1 & UNIT_ALARM_STATUS1_BATTERY_LOW) != 0U);
  TEST_ASSERT_TRUE((status1 & UNIT_ALARM_STATUS1_TEMP_HIGH) != 0U);
  TEST_ASSERT_TRUE((status1 & UNIT_ALARM_STATUS1_CONFIG_INVALID) != 0U);
  TEST_ASSERT_TRUE((status1 & UNIT_ALARM_STATUS1_LAMP_FAULT) != 0U);
}

void test_unit_alarm_maps_psm_to_status3(void)
{
  UnitAlarmService_t svc;
  uint8_t status3 = 0U;

  UnitAlarmServiceInit(&svc, &g_alarmPort);
  EmitCode(&svc, FAULT_CODE_PSM_LINE_VOLTAGE_LOW);
  EmitCode(&svc, FAULT_CODE_PSM_LINE_VOLTAGE_HIGH);
  EmitCode(&svc, FAULT_CODE_PSM_FREQUENCY_LOW);
  EmitCode(&svc, FAULT_CODE_PSM_FREQUENCY_HIGH);
  EmitCode(&svc, FAULT_CODE_PSM_RAIL_24V_FAIL);
  EmitCode(&svc, FAULT_CODE_PSM_RAIL_5V_FAIL);

  TEST_ASSERT_EQUAL_UINT8(1U,
                          UnitAlarmPortGetUnitAlarmStatus3(&g_alarmPort,
                                                           &status3));
  TEST_ASSERT_TRUE((status3 & UNIT_ALARM_STATUS3_LINE_V_LOW) != 0U);
  TEST_ASSERT_TRUE((status3 & UNIT_ALARM_STATUS3_LINE_V_HIGH) != 0U);
  TEST_ASSERT_TRUE((status3 & UNIT_ALARM_STATUS3_FREQ_LOW) != 0U);
  TEST_ASSERT_TRUE((status3 & UNIT_ALARM_STATUS3_FREQ_HIGH) != 0U);
  TEST_ASSERT_TRUE((status3 & UNIT_ALARM_STATUS3_RAIL_24V_FAIL) != 0U);
  TEST_ASSERT_TRUE((status3 & UNIT_ALARM_STATUS3_RAIL_5V_FAIL) != 0U);
}

void test_unit_alarm_maps_watchdog_and_relay_to_status4(void)
{
  UnitAlarmService_t svc;
  uint8_t status4 = 0U;

  UnitAlarmServiceInit(&svc, &g_alarmPort);
  EmitCode(&svc, FAULT_CODE_MP_WATCHDOG);
  EmitCode(&svc, FAULT_CODE_MP_RELAY_FEEDBACK_MISMATCH);

  TEST_ASSERT_EQUAL_UINT8(1U,
                          UnitAlarmPortGetUnitAlarmStatus4(&g_alarmPort,
                                                           &status4));
  TEST_ASSERT_TRUE((status4 & UNIT_ALARM_STATUS4_WATCHDOG) != 0U);
  TEST_ASSERT_TRUE((status4 & UNIT_ALARM_STATUS4_RELAY_MISMATCH) != 0U);
}

void test_unit_alarm_clear_resets_all_status_bytes(void)
{
  UnitAlarmService_t svc;
  uint8_t s1 = 0U;
  uint8_t s2 = 0U;
  uint8_t s3 = 0U;
  uint8_t s4 = 0U;

  UnitAlarmServiceInit(&svc, &g_alarmPort);
  EmitCode(&svc, FAULT_CODE_MODULE_CP_MISSING);
  EmitCode(&svc, FAULT_CODE_CONFLICT_GREEN_GREEN);
  EmitCode(&svc, FAULT_CODE_PSM_LINE_VOLTAGE_LOW);
  EmitCode(&svc, FAULT_CODE_MP_WATCHDOG);

  UnitAlarmServiceClear(&svc);

  (void) UnitAlarmPortGetUnitAlarmStatus1(&g_alarmPort, &s1);
  (void) UnitAlarmPortGetUnitAlarmStatus2(&g_alarmPort, &s2);
  (void) UnitAlarmPortGetUnitAlarmStatus3(&g_alarmPort, &s3);
  (void) UnitAlarmPortGetUnitAlarmStatus4(&g_alarmPort, &s4);
  TEST_ASSERT_EQUAL_UINT8(0U, s1);
  TEST_ASSERT_EQUAL_UINT8(0U, s2);
  TEST_ASSERT_EQUAL_UINT8(0U, s3);
  TEST_ASSERT_EQUAL_UINT8(0U, s4);
}

/* -------- FaultMonitorService -------- */

void test_fault_monitor_service_trace_records_events(void)
{
  FaultMonitorService_t svc;
  FaultEvent_t event;
  const FaultMonitorTrace_t *trace;

  FaultMonitorServiceInit(&svc);
  (void) memset(&event, 0, sizeof(event));
  event.code = FAULT_CODE_CONFLICT_YELLOW_GREEN;
  event.source = (uint16_t) ((1U << 8U) | 2U);
  event.timestampTicks = 42U;
  FaultMonitorServiceOnFault(&svc, &event);

  trace = FaultMonitorServiceGetTrace(&svc);
  TEST_ASSERT_NOT_NULL(trace);
  TEST_ASSERT_EQUAL_UINT32(1U, FaultMonitorTraceSize(trace));
  const FaultEvent_t *stored = FaultMonitorTraceAt(trace, 0U);

  TEST_ASSERT_NOT_NULL(stored);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_CONFLICT_YELLOW_GREEN, stored->code);
}

void test_fault_monitor_service_aggregates_global_flags(void)
{
  FaultMonitorService_t svc;
  FaultEvent_t event;
  FaultMonitorStatus_t snapshot;

  FaultMonitorServiceInit(&svc);

  (void) memset(&event, 0, sizeof(event));
  event.code = FAULT_CODE_PSM_LINE_VOLTAGE_LOW;
  FaultMonitorServiceOnFault(&svc, &event);
  event.code = FAULT_CODE_PSM_RAIL_24V_FAIL;
  FaultMonitorServiceOnFault(&svc, &event);
  event.code = FAULT_CODE_PSM_RAIL_5V_FAIL;
  FaultMonitorServiceOnFault(&svc, &event);
  event.code = FAULT_CODE_MODULE_CP_MISSING;
  FaultMonitorServiceOnFault(&svc, &event);
  event.code = FAULT_CODE_MODULE_PSM_MISSING;
  FaultMonitorServiceOnFault(&svc, &event);
  event.code = FAULT_CODE_MODULE_SSM_MISSING;
  FaultMonitorServiceOnFault(&svc, &event);
  event.code = FAULT_CODE_MP_WATCHDOG;
  FaultMonitorServiceOnFault(&svc, &event);
  event.code = FAULT_CODE_MP_RELAY_FEEDBACK_MISMATCH;
  FaultMonitorServiceOnFault(&svc, &event);
  event.code = FAULT_CODE_MP_CONFIG_INVALID;
  FaultMonitorServiceOnFault(&svc, &event);

  FaultMonitorServiceRead(&svc, &snapshot);
  TEST_ASSERT_EQUAL_UINT8(1U, snapshot.global.acLineFault);
  TEST_ASSERT_EQUAL_UINT8(1U, snapshot.global.rail24VFault);
  TEST_ASSERT_EQUAL_UINT8(1U, snapshot.global.rail5VFault);
  TEST_ASSERT_EQUAL_UINT8(1U, snapshot.global.cpMissing);
  TEST_ASSERT_EQUAL_UINT8(1U, snapshot.global.psmMissing);
  TEST_ASSERT_EQUAL_UINT8(1U, snapshot.global.ssmMissing);
  TEST_ASSERT_EQUAL_UINT8(1U, snapshot.global.watchdog);
  TEST_ASSERT_EQUAL_UINT8(1U, snapshot.global.relayFeedbackMismatch);
  TEST_ASSERT_EQUAL_UINT8(1U, snapshot.global.configInvalid);
}

void test_fault_monitor_service_trace_at_out_of_range_returns_null(void)
{
  FaultMonitorService_t svc;

  FaultMonitorServiceInit(&svc);
  const FaultMonitorTrace_t *trace = FaultMonitorServiceGetTrace(&svc);

  TEST_ASSERT_NOT_NULL(trace);
  TEST_ASSERT_NULL(FaultMonitorTraceAt(trace, 0U));
}

/* -------- SafetyDecisionService -------- */

void test_safety_decision_flash_action_for_power_supply(void)
{
  MockSafetyRelayAdapterCtx_t relayCtx;
  ISafetyRelayPort_t relay;
  SafetyDecisionService_t svc;
  FaultEvent_t event;

  MockSafetyRelayAdapterInit(&relayCtx);
  relay = MockSafetyRelayAdapterCreatePort(&relayCtx);
  SafetyDecisionServiceInit(&svc, &relay);

  (void) memset(&event, 0, sizeof(event));
  event.code = FAULT_CODE_PSM_LINE_VOLTAGE_LOW;
  event.severity = FAULT_SEVERITY_CRITICAL;
  SafetyDecisionServiceOnFault(&svc, &event);

  TEST_ASSERT_EQUAL(SAFETY_ACTION_FLASH,
                    SafetyDecisionServiceGetLatchedAction(&svc));
}

void test_safety_decision_error_severity_drops_when_configured(void)
{
  MockSafetyRelayAdapterCtx_t relayCtx;
  ISafetyRelayPort_t relay;
  SafetyDecisionService_t svc;
  FaultEvent_t event;

  MockSafetyRelayAdapterInit(&relayCtx);
  relay = MockSafetyRelayAdapterCreatePort(&relayCtx);
  SafetyDecisionServiceInit(&svc, &relay);
  SafetyDecisionServiceConfigure(&svc, 1U);

  (void) memset(&event, 0, sizeof(event));
  event.code = FAULT_CODE_CONFLICT_YELLOW_YELLOW;
  event.severity = FAULT_SEVERITY_ERROR;
  SafetyDecisionServiceOnFault(&svc, &event);

  TEST_ASSERT_EQUAL(SAFETY_ACTION_DARK,
                    SafetyDecisionServiceGetLatchedAction(&svc));
}

void test_safety_decision_reset_restores_relay(void)
{
  MockSafetyRelayAdapterCtx_t relayCtx;
  ISafetyRelayPort_t relay;
  SafetyDecisionService_t svc;
  FaultEvent_t event;
  SafetyRelayState_t state;

  MockSafetyRelayAdapterInit(&relayCtx);
  relay = MockSafetyRelayAdapterCreatePort(&relayCtx);
  SafetyDecisionServiceInit(&svc, &relay);

  event.code = FAULT_CODE_CONFLICT_GREEN_GREEN;
  event.severity = FAULT_SEVERITY_CRITICAL;
  event.source = 0U;
  event.timestampTicks = 0U;
  event.param = 0U;
  SafetyDecisionServiceOnFault(&svc, &event);

  TEST_ASSERT_EQUAL_UINT8(1U, SafetyRelayGetActualState(&relay, &state));
  TEST_ASSERT_EQUAL(SAFETY_RELAY_STATE_OPEN, state);

  SafetyDecisionServiceReset(&svc);
  TEST_ASSERT_EQUAL(SAFETY_ACTION_NONE,
                    SafetyDecisionServiceGetLatchedAction(&svc));
  TEST_ASSERT_EQUAL_UINT8(1U, SafetyRelayGetActualState(&relay, &state));
  TEST_ASSERT_EQUAL(SAFETY_RELAY_STATE_CLOSED, state);
}

/* -------- PowerSupplyMonitor -------- */

typedef struct
{
  uint32_t count;
  FaultEvent_t last;
} Collector_t;

static void OnFault(void *ctx, const FaultEvent_t *event)
{
  Collector_t *c = (Collector_t *) ctx;

  c->count++;
  c->last = *event;
}

void test_power_supply_fires_on_high_voltage(void)
{
  PowerSupplyMonitor_t monitor;
  FieldBusSnapshot_t snapshot;
  Collector_t collector = { 0 };
  FaultEmit_t emit = { .fn = OnFault, .ctx = &collector };

  PowerSupplyMonitorInit(&monitor, NULL);
  (void) memset(&snapshot, 0, sizeof(snapshot));
  snapshot.psm[0].alive = 1U;
  snapshot.psm[0].lineVoltageAdcCounts = 400U; /* above 364 upper */
  snapshot.psm[0].lineFrequencyRaw = 100U;
  snapshot.psm[0].rail24V1AdcCounts = 100U;
  snapshot.psm[0].rail24V2AdcCounts = 100U;
  snapshot.psm[0].rail5V1AdcCounts = 100U;
  snapshot.psm[0].rail5V2AdcCounts = 100U;

  PowerSupplyMonitorTick(&monitor, &snapshot, 1U, &emit);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_PSM_LINE_VOLTAGE_HIGH,
                           collector.last.code);
}

void test_power_supply_fires_on_rail_24v_fail(void)
{
  PowerSupplyMonitor_t monitor;
  FieldBusSnapshot_t snapshot;
  Collector_t collector = { 0 };
  FaultEmit_t emit = { .fn = OnFault, .ctx = &collector };

  PowerSupplyMonitorInit(&monitor, NULL);
  (void) memset(&snapshot, 0, sizeof(snapshot));
  snapshot.psm[0].alive = 1U;
  snapshot.psm[0].lineVoltageAdcCounts = 300U;
  snapshot.psm[0].lineFrequencyRaw = 100U;
  snapshot.psm[0].rail24V1AdcCounts = 5U; /* below 20 */
  snapshot.psm[0].rail24V2AdcCounts = 100U;
  snapshot.psm[0].rail5V1AdcCounts = 100U;
  snapshot.psm[0].rail5V2AdcCounts = 100U;

  PowerSupplyMonitorTick(&monitor, &snapshot, 1U, &emit);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_PSM_RAIL_24V_FAIL,
                           collector.last.code);
}

void test_power_supply_fires_on_rail_5v_fail(void)
{
  PowerSupplyMonitor_t monitor;
  FieldBusSnapshot_t snapshot;
  Collector_t collector = { 0 };
  FaultEmit_t emit = { .fn = OnFault, .ctx = &collector };

  PowerSupplyMonitorInit(&monitor, NULL);
  (void) memset(&snapshot, 0, sizeof(snapshot));
  snapshot.psm[0].alive = 1U;
  snapshot.psm[0].lineVoltageAdcCounts = 300U;
  snapshot.psm[0].lineFrequencyRaw = 100U;
  snapshot.psm[0].rail24V1AdcCounts = 100U;
  snapshot.psm[0].rail24V2AdcCounts = 100U;
  snapshot.psm[0].rail5V1AdcCounts = 1U;
  snapshot.psm[0].rail5V2AdcCounts = 100U;

  PowerSupplyMonitorTick(&monitor, &snapshot, 1U, &emit);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_PSM_RAIL_5V_FAIL, collector.last.code);
}

void test_power_supply_clears_latched_after_recovery(void)
{
  PowerSupplyMonitor_t monitor;
  FieldBusSnapshot_t snapshot;
  Collector_t collector = { 0 };
  FaultEmit_t emit = { .fn = OnFault, .ctx = &collector };

  PowerSupplyMonitorInit(&monitor, NULL);
  (void) memset(&snapshot, 0, sizeof(snapshot));
  snapshot.psm[0].alive = 1U;
  snapshot.psm[0].lineVoltageAdcCounts = 100U;
  snapshot.psm[0].lineFrequencyRaw = 100U;
  snapshot.psm[0].rail24V1AdcCounts = 100U;
  snapshot.psm[0].rail24V2AdcCounts = 100U;
  snapshot.psm[0].rail5V1AdcCounts = 100U;
  snapshot.psm[0].rail5V2AdcCounts = 100U;
  PowerSupplyMonitorTick(&monitor, &snapshot, 1U, &emit);
  TEST_ASSERT_EQUAL_UINT32(1U, collector.count);

  /* Recover into hysteresis band. */
  snapshot.psm[0].lineVoltageAdcCounts = 300U;
  PowerSupplyMonitorTick(&monitor, &snapshot, 2U, &emit);

  /* Re-trigger must fire again (latch was cleared). */
  snapshot.psm[0].lineVoltageAdcCounts = 50U;
  PowerSupplyMonitorTick(&monitor, &snapshot, 3U, &emit);
  TEST_ASSERT_EQUAL_UINT32(2U, collector.count);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_unit_alarm_maps_conflict_and_status2_bits);
  RUN_TEST(test_unit_alarm_maps_module_and_battery_to_status1);
  RUN_TEST(test_unit_alarm_maps_psm_to_status3);
  RUN_TEST(test_unit_alarm_maps_watchdog_and_relay_to_status4);
  RUN_TEST(test_unit_alarm_clear_resets_all_status_bytes);
  RUN_TEST(test_fault_monitor_service_trace_records_events);
  RUN_TEST(test_fault_monitor_service_aggregates_global_flags);
  RUN_TEST(test_fault_monitor_service_trace_at_out_of_range_returns_null);
  RUN_TEST(test_safety_decision_flash_action_for_power_supply);
  RUN_TEST(test_safety_decision_error_severity_drops_when_configured);
  RUN_TEST(test_safety_decision_reset_restores_relay);
  RUN_TEST(test_power_supply_fires_on_high_voltage);
  RUN_TEST(test_power_supply_fires_on_rail_24v_fail);
  RUN_TEST(test_power_supply_fires_on_rail_5v_fail);
  RUN_TEST(test_power_supply_clears_latched_after_recovery);

  return UNITY_END();
}
