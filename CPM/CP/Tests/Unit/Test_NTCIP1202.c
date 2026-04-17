/*
 * Tests/unit/Test_NTCIP1202.c
 *
 * Unit tests for App/Domain/NTCIP/NTCIP1202.c
 */
#include "unity.h"
#include "Domain/NTCIP/NTCIP1202.h"
#include "Domain/Intersection/SignalGroup.h"
#include "MockSignalOutput.h"
#include "MockDetector.h"
#include "MockClock.h"
#include "MockSnmpNotifier.h"
#include "TimingPlan_4Phase.h"

static MockSignalOutputCtx_t sigCtx;
static ISignalOutputPort_t sigPort;
static MockDetectorCtx_t detCtx;
static IDetectorInputPort_t detPort;
static MockClockCtx_t clkCtx;
static ISystemClockPort_t clkPort;
static MockSnmpNotifierCtx_t snmpCtx;
static ISnmpNotifierPort_t snmpPort;
static ProgramCtx_t prog;

void setUp(void)
{
  MockSignalOutput_Init(&sigCtx);
  sigPort = MockSignalOutput_Create(&sigCtx);
  MockDetector_Init(&detCtx);
  detPort = MockDetector_Create(&detCtx);
  MockClock_Init(&clkCtx);
  clkPort = MockClock_Create(&clkCtx);
  MockSnmpNotifier_Init(&snmpCtx);
  snmpPort = MockSnmpNotifier_Create(&snmpCtx);

  ProgramInit(&prog, &sigPort, &detPort, &clkPort, &snmpPort);

  ProgramConfig_t cfg;
  int i;

  for (i = 0; i < (int) sizeof(cfg); i++)
  {
    ((uint8_t *) &cfg)[i] = 0U;
  }

  cfg.phaseCount = FIXTURE_4P_PHASE_COUNT;
  cfg.signalGroupCount = FIXTURE_4P_SG_COUNT;
  cfg.DetectorCount = 2U;       /* Two Detectors for test */
  Fixture4P_BuildPhases(cfg.phases);
  Fixture4P_BuildSGs(cfg.signalGroups);
  /* Minimal Detector configs */
  cfg.Detectors[0].ownerSignalGroup = 0U;
  cfg.Detectors[1].ownerSignalGroup = 2U;
  ProgramLoadConfig(&prog, &cfg);
}

void tearDown(void)
{
}

/* --- Phase status ------------------------------------------------------- */

void Test_Phase_status_initially_zero(void)
{
  /* No phase active → status should have no ACTIVE bit */
  Ntcip1202PhaseStatus_t s = Ntcip1202_GetPhaseStatus(&prog, 0U);

  TEST_ASSERT_EQUAL_UINT8(0U, s & NTCIP1202_PHASE_STATUS_ACTIVE);
}

void Test_Phase_status_out_of_range_returns_zero(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, Ntcip1202_GetPhaseStatus(&prog, 99U));
}

/* --- Detector table ----------------------------------------------------- */

void test_get_Detector_initially_no_demand(void)
{
  Ntcip1202DetectorEntry_t d;
  bool ok = Ntcip1202_GetDetector(&prog, 0U, &d);

  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL_UINT8(0U, d.DetectorCallStatus);
  TEST_ASSERT_FALSE(d.DetectorFault);
}

void test_get_Detector_reflects_demand_after_tick(void)
{
  /* Inject demand on Detector 0 and tick */
  MockDetector_SetBusy(&detCtx, 0U);
  ProgramTick(&prog);     /* Detector_Tick accumulates demand */

  Ntcip1202DetectorEntry_t d;

  Ntcip1202_GetDetector(&prog, 0U, &d);
  TEST_ASSERT_GREATER_THAN_UINT8(0U, d.DetectorCallStatus);
}

void test_get_Detector_fault_after_broken_tick(void)
{
  MockDetector_SetBroken(&detCtx, 1U);
  ProgramTick(&prog);

  Ntcip1202DetectorEntry_t d;

  Ntcip1202_GetDetector(&prog, 1U, &d);
  TEST_ASSERT_TRUE(d.DetectorFault);
}

void test_get_Detector_out_of_range(void)
{
  Ntcip1202DetectorEntry_t d;

  TEST_ASSERT_FALSE(Ntcip1202_GetDetector(&prog, 99U, &d));
}

/* --- Alarm table -------------------------------------------------------- */

void test_no_alarms_initially(void)
{
  Ntcip1202AlarmEntry_t alarms[8];
  uint8_t n = Ntcip1202_GetActiveAlarms(&prog, alarms, 8U);

  TEST_ASSERT_EQUAL_UINT8(0U, n);
}

void Test_Detector_fault_appears_in_alarm_table(void)
{
  MockDetector_SetBroken(&detCtx, 0U);
  ProgramTick(&prog);    /* Sets rt.isBroken */

  Ntcip1202AlarmEntry_t alarms[8];
  uint8_t n = Ntcip1202_GetActiveAlarms(&prog, alarms, 8U);

  TEST_ASSERT_GREATER_THAN_UINT8(0U, n);
  TEST_ASSERT_EQUAL_INT(NTCIP1202_ALARM_DETECTOR_FAILURE, alarms[0].alarmType);
  TEST_ASSERT_EQUAL_UINT8(0U, alarms[0].objectIndex);
}

void test_lamp_fault_appears_in_alarm_table(void)
{
  /* Inject a critical red lamp fault via SG_UpdateFaults */
  ISnmpNotifierPort_t notifier = MockSnmpNotifier_Create(&snmpCtx);

  SG_UpdateFaults(0U, &prog.runtime.signalGroups[0],
                  &prog.config.signalGroups[0],
                  true, false, false, &notifier);

  Ntcip1202AlarmEntry_t alarms[8];
  uint8_t n = Ntcip1202_GetActiveAlarms(&prog, alarms, 8U);

  TEST_ASSERT_GREATER_THAN_UINT8(0U, n);

  bool found = false;
  uint8_t i;

  for (i = 0U; i < n; i++)
  {
    if ((alarms[i].alarmType == NTCIP1202_ALARM_LAMP_FAILURE)
        && (alarms[i].objectIndex == 0U) )
    {
      found = true;
    }
  }

  TEST_ASSERT_TRUE(found);
}

/* --- System date/time --------------------------------------------------- */

void test_get_datetime_at_epoch_zero(void)
{
  /* epoch=0 → 1970-01-01 00:00:00 */
  clkCtx.epoch = 0U;
  Ntcip1202DateTime_t dt = Ntcip1202_GetDateTime(&prog);

  TEST_ASSERT_EQUAL_UINT16(1970U, dt.year);
  TEST_ASSERT_EQUAL_UINT8(1U,     dt.month);
  TEST_ASSERT_EQUAL_UINT8(1U,     dt.day);
  TEST_ASSERT_EQUAL_UINT8(0U,     dt.hour);
}

void test_get_datetime_known_epoch(void)
{
  /* 2025-01-01 00:00:00 UTC ≈ epoch 1735689600 */
  clkCtx.epoch = 1735689600U;
  Ntcip1202DateTime_t dt = Ntcip1202_GetDateTime(&prog);

  TEST_ASSERT_EQUAL_UINT16(2025U, dt.year);
  TEST_ASSERT_EQUAL_UINT8(1U,     dt.month);
  TEST_ASSERT_EQUAL_UINT8(1U,     dt.day);
}

void test_set_datetime_roundtrip(void)
{
  Ntcip1202DateTime_t dtIn;

  dtIn.year = 2026U;
  dtIn.month = 3U;
  dtIn.day = 27U;
  dtIn.hour = 12U;
  dtIn.minute = 30U;
  dtIn.second = 0U;

  bool ok = Ntcip1202_SetDateTime(&prog, &dtIn);

  TEST_ASSERT_TRUE(ok);

  Ntcip1202DateTime_t dtOut = Ntcip1202_GetDateTime(&prog);

  TEST_ASSERT_EQUAL_UINT16(2026U, dtOut.year);
  TEST_ASSERT_EQUAL_UINT8(3U,     dtOut.month);
  TEST_ASSERT_EQUAL_UINT8(27U,    dtOut.day);
  TEST_ASSERT_EQUAL_UINT8(12U,    dtOut.hour);
}

void test_set_datetime_invalid_month_rejected(void)
{
  Ntcip1202DateTime_t dt = { 2026U, 13U, 1U, 0U, 0U, 0U };

  TEST_ASSERT_FALSE(Ntcip1202_SetDateTime(&prog, &dt));
}

void test_set_datetime_invalid_hour_rejected(void)
{
  Ntcip1202DateTime_t dt = { 2026U, 1U, 1U, 25U, 0U, 0U };

  TEST_ASSERT_FALSE(Ntcip1202_SetDateTime(&prog, &dt));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(Test_Phase_status_initially_zero);
  RUN_TEST(Test_Phase_status_out_of_range_returns_zero);
  RUN_TEST(test_get_Detector_initially_no_demand);
  RUN_TEST(test_get_Detector_reflects_demand_after_tick);
  RUN_TEST(test_get_Detector_fault_after_broken_tick);
  RUN_TEST(test_get_Detector_out_of_range);
  RUN_TEST(test_no_alarms_initially);
  RUN_TEST(Test_Detector_fault_appears_in_alarm_table);
  RUN_TEST(test_lamp_fault_appears_in_alarm_table);
  RUN_TEST(test_get_datetime_at_epoch_zero);
  RUN_TEST(test_get_datetime_known_epoch);
  RUN_TEST(test_set_datetime_roundtrip);
  RUN_TEST(test_set_datetime_invalid_month_rejected);
  RUN_TEST(test_set_datetime_invalid_hour_rejected);

  return UNITY_END();
}
