/* Tests/Unit/Test_Monitors.c
 *
 * Focused unit tests for each malfunction monitor. Each test builds
 * only the inputs that monitor needs, asserts the expected
 * FaultEvent_t is emitted (or not), and exercises the dwell-threshold
 * reset paths for branch coverage.
 *
 * The shared FaultCollector is a trivial FaultEmit_t sink that records
 * the first fault emitted; tests then assert on its contents.
 */

#include <stdint.h>
#include <string.h>

#include "unity.h"

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

typedef struct
{
  uint32_t count;
  FaultEvent_t last;
} FaultCollector_t;

static void CollectorOnFault(void *ctx, const FaultEvent_t *event)
{
  FaultCollector_t *c = (FaultCollector_t *) ctx;

  c->count++;
  c->last = *event;
}

static FaultCollector_t g_collector;
static FaultEmit_t g_emit;

void setUp(void)
{
  (void) memset(&g_collector, 0, sizeof(g_collector));
  g_emit.fn = CollectorOnFault;
  g_emit.ctx = &g_collector;
}

void tearDown(void)
{
}

/* ---------- ConflictMonitor ---------- */

static void SeedTwoChannelConfig(ConfigurationService_t *svc)
{
  IntersectionConfig_t cfg;
  ChannelOutputMapping_t mapping;

  (void) memset(&cfg, 0, sizeof(cfg));
  cfg.phaseCount = 2U;
  cfg.ringCount = 1U;
  cfg.phases[0].yellowChangeDs = 30U;
  cfg.phases[0].redClearDs = 20U;
  cfg.phases[1].yellowChangeDs = 30U;
  cfg.phases[1].redClearDs = 20U;
  cfg.channels[0].controlSource = 1U;
  cfg.channels[0].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  cfg.channels[1].controlSource = 2U;
  cfg.channels[1].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;

  ConfigurationServiceInit(svc, NULL);
  (void) ConfigurationServiceSetConfig(svc, &cfg);
  ChannelStateResolverInit(&mapping);
  (void) ConfigurationServiceSetOutputMapping(svc, &mapping);
  (void) ConfigurationServiceValidate(svc);
}

static void SeedOverlapMonitorConfig(ConfigurationService_t *svc)
{
  IntersectionConfig_t cfg;
  ChannelOutputMapping_t mapping;

  (void) memset(&cfg, 0, sizeof(cfg));
  cfg.phaseCount = 4U;
  cfg.ringCount = 1U;
  cfg.phases[0].concurrency.length = 1U;
  cfg.phases[0].concurrency.values[0] = 3U;
  cfg.channels[0].controlSource = 1U;
  cfg.channels[0].controlType = INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP;
  cfg.channels[1].controlSource = 2U;
  cfg.channels[1].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  cfg.overlaps[0].includedPhases.length = 1U;
  cfg.overlaps[0].includedPhases.values[0] = 3U;
  cfg.overlaps[0].trailYellowDs = 4U;
  cfg.overlaps[0].trailRedDs = 6U;

  ConfigurationServiceInit(svc, NULL);
  (void) ConfigurationServiceSetConfig(svc, &cfg);
  ChannelStateResolverInit(&mapping);
  (void) ConfigurationServiceSetOutputMapping(svc, &mapping);
  (void) ConfigurationServiceValidate(svc);
}

void test_conflict_monitor_green_green_fires_after_dwell(void)
{
  ConfigurationService_t svc;
  ConflictMonitor_t monitor;
  ChannelStateImage_t image;
  uint32_t i;

  SeedTwoChannelConfig(&svc);
  ConflictMonitorInit(&monitor, 0U);
  (void) memset(&image, 0, sizeof(image));
  image.channels[0].green = 1U;
  image.channels[1].green = 1U;

  for (i = 0U; i < (CONFLICT_DWELL_TICKS_DEFAULT - 1U); i++)
  {
    ConflictMonitorTick(&monitor, &svc, &image, i, &g_emit);
  }

  TEST_ASSERT_EQUAL_UINT32(0U, g_collector.count);

  ConflictMonitorTick(&monitor, &svc, &image, 100U, &g_emit);
  TEST_ASSERT_EQUAL_UINT32(1U, g_collector.count);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_CONFLICT_GREEN_GREEN,
                           g_collector.last.code);
}

void test_conflict_monitor_resets_when_cleared(void)
{
  ConfigurationService_t svc;
  ConflictMonitor_t monitor;
  ChannelStateImage_t image;
  uint32_t i;

  SeedTwoChannelConfig(&svc);
  ConflictMonitorInit(&monitor, 0U);
  (void) memset(&image, 0, sizeof(image));
  image.channels[0].green = 1U;
  image.channels[1].yellow = 1U;

  for (i = 0U; i < 20U; i++)
  {
    ConflictMonitorTick(&monitor, &svc, &image, i, &g_emit);
  }

  image.channels[1].yellow = 0U;
  image.channels[1].red = 1U;

  for (i = 0U; i < CONFLICT_DWELL_TICKS_DEFAULT; i++)
  {
    ConflictMonitorTick(&monitor, &svc, &image, i + 100U, &g_emit);
  }

  TEST_ASSERT_EQUAL_UINT32(0U, g_collector.count);
}

/* ---------- DualIndicationMonitor ---------- */

void test_dual_indication_fires_on_persistent_overlap(void)
{
  DualIndicationMonitor_t monitor;
  ChannelStateImage_t image;
  uint32_t i;

  DualIndicationMonitorInit(&monitor, 0U);
  (void) memset(&image, 0, sizeof(image));
  image.channels[3].red = 1U;
  image.channels[3].green = 1U;

  for (i = 0U; i < DUAL_INDICATION_DWELL_TICKS_DEFAULT; i++)
  {
    DualIndicationMonitorTick(&monitor, &image, i, &g_emit);
  }

  TEST_ASSERT_EQUAL_UINT32(1U, g_collector.count);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_DUAL_INDICATION, g_collector.last.code);
  TEST_ASSERT_EQUAL_UINT16(3U, g_collector.last.source);
}

/* ---------- DarkChannelMonitor ---------- */

void test_dark_channel_fires_when_commanded_but_no_aspect(void)
{
  DarkChannelMonitor_t monitor;
  ChannelStateImage_t commanded;
  ChannelStateImage_t measured;
  uint32_t i;

  DarkChannelMonitorInit(&monitor, 0U);
  (void) memset(&commanded, 0, sizeof(commanded));
  (void) memset(&measured, 0, sizeof(measured));
  commanded.channels[2].red = 1U;

  for (i = 0U; i < DARK_CHANNEL_DWELL_TICKS_DEFAULT; i++)
  {
    DarkChannelMonitorTick(&monitor, &commanded, &measured, i, &g_emit);
  }

  TEST_ASSERT_EQUAL_UINT32(1U, g_collector.count);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_DARK_CHANNEL, g_collector.last.code);
}

/* ---------- RedFailMonitor ---------- */

void test_red_fail_fires_when_red_absent(void)
{
  RedFailMonitor_t monitor;
  ChannelStateImage_t commanded;
  ChannelStateImage_t measured;
  uint32_t i;

  RedFailMonitorInit(&monitor, 0U);
  (void) memset(&commanded, 0, sizeof(commanded));
  (void) memset(&measured, 0, sizeof(measured));
  commanded.channels[4].red = 1U;
  /* measured stays all-zero -> red missing */

  for (i = 0U; i < RED_FAIL_DWELL_TICKS_DEFAULT; i++)
  {
    RedFailMonitorTick(&monitor, &commanded, &measured, i, &g_emit);
  }

  TEST_ASSERT_EQUAL_UINT32(1U, g_collector.count);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_RED_FAIL, g_collector.last.code);
}

/* ---------- SignalSequenceMonitor ---------- */

void test_signal_sequence_flags_illegal_green_to_red(void)
{
  SignalSequenceMonitor_t monitor;
  ChannelStateImage_t image;

  SignalSequenceMonitorInit(&monitor);
  (void) memset(&image, 0, sizeof(image));
  image.channels[0].green = 1U;
  SignalSequenceMonitorTick(&monitor, &image, 1U, &g_emit);

  (void) memset(&image, 0, sizeof(image));
  image.channels[0].red = 1U;
  SignalSequenceMonitorTick(&monitor, &image, 2U, &g_emit);

  TEST_ASSERT_EQUAL_UINT32(1U, g_collector.count);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_SIGNAL_SEQUENCE, g_collector.last.code);
}

void test_signal_sequence_allows_red_to_green(void)
{
  SignalSequenceMonitor_t monitor;
  ChannelStateImage_t image;

  SignalSequenceMonitorInit(&monitor);
  (void) memset(&image, 0, sizeof(image));
  image.channels[0].red = 1U;
  SignalSequenceMonitorTick(&monitor, &image, 1U, &g_emit);

  (void) memset(&image, 0, sizeof(image));
  image.channels[0].green = 1U;
  SignalSequenceMonitorTick(&monitor, &image, 2U, &g_emit);

  TEST_ASSERT_EQUAL_UINT32(0U, g_collector.count);
}

/* ---------- MinYellowMonitor ---------- */

void test_min_yellow_fires_when_too_short(void)
{
  ConfigurationService_t svc;
  MinYellowMonitor_t monitor;
  ChannelStateImage_t image;

  SeedTwoChannelConfig(&svc);
  MinYellowMonitorInit(&monitor);

  (void) memset(&image, 0, sizeof(image));
  image.channels[0].yellow = 1U;
  /* Only 10 ticks of yellow -> way under 30 decis * 10 = 300 ticks. */
  for (uint32_t i = 0U; i < 10U; i++)
  {
    MinYellowMonitorTick(&monitor, &svc, &image, i, &g_emit);
  }

  (void) memset(&image, 0, sizeof(image));
  image.channels[0].red = 1U;
  MinYellowMonitorTick(&monitor, &svc, &image, 20U, &g_emit);

  TEST_ASSERT_EQUAL_UINT32(1U, g_collector.count);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_MIN_YELLOW_SHORT, g_collector.last.code);
}

void test_configuration_service_derives_overlap_monitoring_profile(void)
{
  ConfigurationService_t svc;

  SeedOverlapMonitorConfig(&svc);

  TEST_ASSERT_EQUAL_UINT8(4U, ConfigurationServiceGetChannelMinYellowDs(&svc, 0U));
  TEST_ASSERT_EQUAL_UINT8(6U, ConfigurationServiceGetChannelRedClearDs(&svc, 0U));
  TEST_ASSERT_TRUE(ConfigurationServiceChannelsConflict(&svc, 0U, 1U));
}

/* ---------- ClearanceMonitor ---------- */

void test_clearance_short_fires_when_conflict_enters_too_early(void)
{
  ConfigurationService_t svc;
  ClearanceMonitor_t monitor;
  ChannelStateImage_t image;

  SeedTwoChannelConfig(&svc);
  ClearanceMonitorInit(&monitor);

  /* First tick: initialise lastColor[]. */
  (void) memset(&image, 0, sizeof(image));
  image.channels[0].yellow = 1U;
  image.channels[1].red = 1U;
  ClearanceMonitorTick(&monitor, &svc, &image, 1U, &g_emit);

  /* Channel 0 goes Y -> R, channel 1 is still red. Starts the
   * clearance countdown on channel 0. */
  (void) memset(&image, 0, sizeof(image));
  image.channels[0].red = 1U;
  image.channels[1].red = 1U;
  ClearanceMonitorTick(&monitor, &svc, &image, 2U, &g_emit);

  /* Channel 1 prematurely becomes green while ch0 is still in
   * clearance window. */
  (void) memset(&image, 0, sizeof(image));
  image.channels[0].red = 1U;
  image.channels[1].green = 1U;
  ClearanceMonitorTick(&monitor, &svc, &image, 3U, &g_emit);

  TEST_ASSERT_GREATER_THAN_UINT32(0U, g_collector.count);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_CLEARANCE_SHORT, g_collector.last.code);
}

/* ---------- LampFaultMonitor ---------- */

void test_lamp_open_fires_when_commanded_but_no_voltage(void)
{
  LampFaultMonitor_t monitor;
  FieldBusLoadSwitchImage_t commanded;
  FieldBusSsmTelemetry_t ssm[FIELD_BUS_SSM_COUNT];
  uint32_t i;

  LampFaultMonitorInit(&monitor, 0U);
  (void) memset(&commanded, 0, sizeof(commanded));
  commanded.bits[0] = 0x01U; /* output 0 commanded on */
  (void) memset(ssm, 0, sizeof(ssm));
  ssm[0].alive = 1U;
  ssm[0].voltagePresenceBits = 0U;

  for (i = 0U; i < LAMP_FAULT_DWELL_TICKS_DEFAULT; i++)
  {
    LampFaultMonitorTick(&monitor, &commanded, ssm, FIELD_BUS_SSM_COUNT,
                         i, &g_emit);
  }

  TEST_ASSERT_EQUAL_UINT32(1U, g_collector.count);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_LAMP_OPEN_CIRCUIT, g_collector.last.code);
}

void test_lamp_driven_externally_fires_when_voltage_present_but_not_commanded(
  void)
{
  LampFaultMonitor_t monitor;
  FieldBusLoadSwitchImage_t commanded;
  FieldBusSsmTelemetry_t ssm[FIELD_BUS_SSM_COUNT];
  uint32_t i;

  LampFaultMonitorInit(&monitor, 0U);
  (void) memset(&commanded, 0, sizeof(commanded));
  (void) memset(ssm, 0, sizeof(ssm));
  ssm[0].alive = 1U;
  ssm[0].voltagePresenceBits = 0x01U;

  for (i = 0U; i < LAMP_FAULT_DWELL_TICKS_DEFAULT; i++)
  {
    LampFaultMonitorTick(&monitor, &commanded, ssm, FIELD_BUS_SSM_COUNT,
                         i, &g_emit);
  }

  TEST_ASSERT_EQUAL_UINT32(1U, g_collector.count);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_LAMP_DRIVEN_EXTERNALLY,
                           g_collector.last.code);
}

/* ---------- PowerSupplyMonitor ---------- */

void test_power_supply_fires_when_line_voltage_low(void)
{
  PowerSupplyMonitor_t monitor;
  FieldBusSnapshot_t snapshot;

  PowerSupplyMonitorInit(&monitor, NULL);
  (void) memset(&snapshot, 0, sizeof(snapshot));
  snapshot.psm[0].alive = 1U;
  snapshot.psm[0].lineVoltageAdcCounts = 100U; /* below 241 */
  snapshot.psm[0].lineFrequencyRaw = 100U;
  snapshot.psm[0].rail24V1AdcCounts = 100U;
  snapshot.psm[0].rail24V2AdcCounts = 100U;
  snapshot.psm[0].rail5V1AdcCounts = 100U;
  snapshot.psm[0].rail5V2AdcCounts = 100U;

  PowerSupplyMonitorTick(&monitor, &snapshot, 1U, &g_emit);

  TEST_ASSERT_EQUAL_UINT32(1U, g_collector.count);
  TEST_ASSERT_EQUAL_UINT16(FAULT_CODE_PSM_LINE_VOLTAGE_LOW,
                           g_collector.last.code);
}

void test_power_supply_does_not_refire_while_latched(void)
{
  PowerSupplyMonitor_t monitor;
  FieldBusSnapshot_t snapshot;

  PowerSupplyMonitorInit(&monitor, NULL);
  (void) memset(&snapshot, 0, sizeof(snapshot));
  snapshot.psm[0].alive = 1U;
  snapshot.psm[0].lineVoltageAdcCounts = 100U;
  snapshot.psm[0].lineFrequencyRaw = 100U;
  snapshot.psm[0].rail24V1AdcCounts = 100U;
  snapshot.psm[0].rail24V2AdcCounts = 100U;
  snapshot.psm[0].rail5V1AdcCounts = 100U;
  snapshot.psm[0].rail5V2AdcCounts = 100U;

  for (uint32_t i = 0U; i < 5U; i++)
  {
    PowerSupplyMonitorTick(&monitor, &snapshot, i, &g_emit);
  }

  TEST_ASSERT_EQUAL_UINT32(1U, g_collector.count);
}

/* ---------- ModuleHealthMonitor ---------- */

void test_module_health_flags_missing_ssm(void)
{
  ModuleHealthMonitor_t monitor;
  FieldBusSnapshot_t snapshot;
  uint32_t i;

  ModuleHealthMonitorInit(&monitor);
  (void) memset(&snapshot, 0, sizeof(snapshot));
  /* everything missing -> should eventually flag CP, SSMs, PSMs. */

  for (i = 0U; i < MODULE_HEALTH_SSM_THRESHOLD_DEFAULT + 2U; i++)
  {
    ModuleHealthMonitorTick(&monitor, &snapshot, i, &g_emit);
  }

  /* Expect at least one SSM-missing fault among the emitted. */
  TEST_ASSERT_GREATER_THAN_UINT32(0U, g_collector.count);
}

/* ---------- SafetyDecisionService ---------- */

void test_safety_decision_drops_relay_on_critical_conflict(void)
{
  SafetyDecisionService_t svc;
  FaultEvent_t event;

  SafetyDecisionServiceInit(&svc, NULL);

  event.code = FAULT_CODE_CONFLICT_GREEN_GREEN;
  event.severity = FAULT_SEVERITY_CRITICAL;
  event.source = 0U;
  event.timestampTicks = 1U;
  event.param = 0U;
  SafetyDecisionServiceOnFault(&svc, &event);

  TEST_ASSERT_EQUAL(SAFETY_ACTION_DARK,
                    SafetyDecisionServiceGetLatchedAction(&svc));
}

void test_safety_decision_ignores_warning_severity(void)
{
  SafetyDecisionService_t svc;
  FaultEvent_t event;

  SafetyDecisionServiceInit(&svc, NULL);

  event.code = FAULT_CODE_LAMP_OPEN_CIRCUIT;
  event.severity = FAULT_SEVERITY_WARNING;
  event.source = 0U;
  event.timestampTicks = 1U;
  event.param = 0U;
  SafetyDecisionServiceOnFault(&svc, &event);

  TEST_ASSERT_EQUAL(SAFETY_ACTION_NONE,
                    SafetyDecisionServiceGetLatchedAction(&svc));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_conflict_monitor_green_green_fires_after_dwell);
  RUN_TEST(test_conflict_monitor_resets_when_cleared);
  RUN_TEST(test_dual_indication_fires_on_persistent_overlap);
  RUN_TEST(test_dark_channel_fires_when_commanded_but_no_aspect);
  RUN_TEST(test_red_fail_fires_when_red_absent);
  RUN_TEST(test_signal_sequence_flags_illegal_green_to_red);
  RUN_TEST(test_signal_sequence_allows_red_to_green);
  RUN_TEST(test_min_yellow_fires_when_too_short);
  RUN_TEST(test_configuration_service_derives_overlap_monitoring_profile);
  RUN_TEST(test_clearance_short_fires_when_conflict_enters_too_early);
  RUN_TEST(test_lamp_open_fires_when_commanded_but_no_voltage);
  RUN_TEST(
    test_lamp_driven_externally_fires_when_voltage_present_but_not_commanded);
  RUN_TEST(test_power_supply_fires_when_line_voltage_low);
  RUN_TEST(test_power_supply_does_not_refire_while_latched);
  RUN_TEST(test_module_health_flags_missing_ssm);
  RUN_TEST(test_safety_decision_drops_relay_on_critical_conflict);
  RUN_TEST(test_safety_decision_ignores_warning_severity);

  return UNITY_END();
}
