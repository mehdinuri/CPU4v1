/*
 * Tests/unit/Test_Detector.c
 *
 * Unit tests for App/Domain/Intersection/Detector.c
 */
#include "unity.h"
#include "Domain/Intersection/Detector.h"
#include "MockDetector.h"
#include "MockSnmpNotifier.h"

static MockDetectorCtx_t detCtx;
static IDetectorInputPort_t detPort;
static MockSnmpNotifierCtx_t snmpCtx;
static ISnmpNotifierPort_t snmpPort;

void setUp(void)
{
  MockDetector_Init(&detCtx);
  detPort = MockDetector_Create(&detCtx);
  MockSnmpNotifier_Init(&snmpCtx);
  snmpPort = MockSnmpNotifier_Create(&snmpCtx);
}

void tearDown(void)
{
}

static DetectorConfig_t make_cfg(void)
{
  DetectorConfig_t cfg;

  cfg.ownerSignalGroup = 0U;
  cfg.greenExtensionPerDemand = 2U;
  cfg.redTimeIfBroken = 10U;
  cfg.fallbackPhaseIfBroken = 0U;
  cfg.activeLevelHigh = true;

  return cfg;
}

/* --- tests -------------------------------------------------------------- */

void test_reset_clears_all_fields(void)
{
  DetectorRuntime_t rt;

  rt.demandCountInPeriod = 5U;
  rt.occupancyTimeMs = 500U;
  rt.isBroken = true;

  Detector_ResetPeriod(&rt);

  TEST_ASSERT_EQUAL_UINT8(0U, rt.demandCountInPeriod);
  TEST_ASSERT_EQUAL_UINT16(0U, rt.occupancyTimeMs);
  /* isBroken is NOT reset by ResetPeriod — cross-period state */
}

void test_tick_empty_Detector_no_demand(void)
{
  DetectorConfig_t cfg = make_cfg();
  DetectorRuntime_t rt;

  Detector_ResetPeriod(&rt);
  rt.isBroken = false;

  /* Detector returns EMPTY */
  Detector_Tick(0U, &rt, &cfg, &detPort, &snmpPort);

  TEST_ASSERT_EQUAL_UINT8(0U, rt.demandCountInPeriod);
  TEST_ASSERT_FALSE(Detector_HasDemand(&rt));
}

void test_tick_busy_Detector_increments_demand(void)
{
  DetectorConfig_t cfg = make_cfg();
  DetectorRuntime_t rt;

  Detector_ResetPeriod(&rt);
  rt.isBroken = false;

  MockDetector_SetBusy(&detCtx, 0U);

  Detector_Tick(0U, &rt, &cfg, &detPort, &snmpPort);

  TEST_ASSERT_TRUE(Detector_HasDemand(&rt));
  TEST_ASSERT_GREATER_THAN_UINT8(0U, rt.demandCountInPeriod);
}

void test_tick_busy_accumulates_occupancy(void)
{
  DetectorConfig_t cfg = make_cfg();
  DetectorRuntime_t rt;

  Detector_ResetPeriod(&rt);
  rt.isBroken = false;

  MockDetector_SetBusy(&detCtx, 0U);

  Detector_Tick(0U, &rt, &cfg, &detPort, &snmpPort);
  Detector_Tick(0U, &rt, &cfg, &detPort, &snmpPort);

  TEST_ASSERT_GREATER_THAN_UINT16(0U, rt.occupancyTimeMs);
}

void test_broken_Detector_sets_is_broken(void)
{
  DetectorConfig_t cfg = make_cfg();
  DetectorRuntime_t rt;

  Detector_ResetPeriod(&rt);
  rt.isBroken = false;
  rt.brokenDurationMs = 0U;

  MockDetector_SetBroken(&detCtx, 0U);

  Detector_Tick(0U, &rt, &cfg, &detPort, &snmpPort);

  TEST_ASSERT_TRUE(rt.isBroken);
}

void test_broken_Detector_emits_snmp_trap_on_first_tick(void)
{
  DetectorConfig_t cfg = make_cfg();
  DetectorRuntime_t rt;

  Detector_ResetPeriod(&rt);
  rt.isBroken = false;
  rt.brokenDurationMs = 0U;

  MockDetector_SetBroken(&detCtx, 0U);

  Detector_Tick(0U, &rt, &cfg, &detPort, &snmpPort);

  TEST_ASSERT_TRUE(MockSnmpNotifier_HasTrap(&snmpCtx,
                                            SNMP_TRAP_DETECTOR_FAILURE));
}

void test_broken_Detector_no_second_trap_while_still_broken(void)
{
  DetectorConfig_t cfg = make_cfg();
  DetectorRuntime_t rt;

  Detector_ResetPeriod(&rt);
  rt.isBroken = false;
  rt.brokenDurationMs = 0U;

  MockDetector_SetBroken(&detCtx, 0U);

  Detector_Tick(0U, &rt, &cfg, &detPort, &snmpPort);
  uint32_t trapsBefore = snmpCtx.count;

  /* Second tick while still broken should not emit another trap */
  Detector_Tick(0U, &rt, &cfg, &detPort, &snmpPort);
  TEST_ASSERT_EQUAL_UINT32(trapsBefore, snmpCtx.count);
}

void test_broken_Detector_does_not_accumulate_occupancy(void)
{
  DetectorConfig_t cfg = make_cfg();
  DetectorRuntime_t rt;

  Detector_ResetPeriod(&rt);
  rt.isBroken = false;
  rt.brokenDurationMs = 0U;

  MockDetector_SetBroken(&detCtx, 0U);

  Detector_Tick(0U, &rt, &cfg, &detPort, &snmpPort);

  TEST_ASSERT_EQUAL_UINT16(0U, rt.occupancyTimeMs);
}

void test_gap_ms_returns_max_when_no_demand(void)
{
  DetectorRuntime_t rt;

  Detector_ResetPeriod(&rt);
  rt.isBroken = false;

  TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, Detector_GapMs(&rt));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_reset_clears_all_fields);
  RUN_TEST(test_tick_empty_Detector_no_demand);
  RUN_TEST(test_tick_busy_Detector_increments_demand);
  RUN_TEST(test_tick_busy_accumulates_occupancy);
  RUN_TEST(test_broken_Detector_sets_is_broken);
  RUN_TEST(test_broken_Detector_emits_snmp_trap_on_first_tick);
  RUN_TEST(test_broken_Detector_no_second_trap_while_still_broken);
  RUN_TEST(test_broken_Detector_does_not_accumulate_occupancy);
  RUN_TEST(test_gap_ms_returns_max_when_no_demand);

  return UNITY_END();
}
