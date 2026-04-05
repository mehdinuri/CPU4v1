/*
 * Tests/unit/Test_NTCIP1201.c
 *
 * Unit tests for App/Domain/NTCIP/NTCIP1201.c and OidRegistry.c
 */
#include "unity.h"
#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/NTCIP/OidRegistry.h"
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

  /* Build a minimal config using the 4-phase fixture */
  ProgramConfig_t cfg;
  int i;

  for (i = 0; i < (int) sizeof(cfg); i++)
  {
    ((uint8_t *) &cfg)[i] = 0U;
  }

  cfg.phaseCount = FIXTURE_4P_PHASE_COUNT;
  cfg.signalGroupCount = FIXTURE_4P_SG_COUNT;
  Fixture4P_BuildPhases(cfg.phases);
  Fixture4P_BuildSGs(cfg.signalGroups);
  ProgramLoadConfig(&prog, &cfg);
}

void tearDown(void)
{
}

/* --- NTCIP 1201 GET phase ------------------------------------------------ */

void test_get_phase_returns_min_max_green(void)
{
  Ntcip1201PhaseEntry_t e;
  bool ok = Ntcip1201_GetPhase(&prog, 0U, &e);

  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL_UINT8(10U, e.phaseMinGreenTime);
  TEST_ASSERT_EQUAL_UINT8(60U, e.phaseMaxGreenTime);
}

void test_get_phase_returns_1based_phase_number(void)
{
  Ntcip1201PhaseEntry_t e;

  Ntcip1201_GetPhase(&prog, 0U, &e);
  TEST_ASSERT_EQUAL_UINT8(1U, e.phaseNumber);

  Ntcip1201_GetPhase(&prog, 3U, &e);
  TEST_ASSERT_EQUAL_UINT8(4U, e.phaseNumber);
}

void test_get_phase_out_of_range_returns_false(void)
{
  Ntcip1201PhaseEntry_t e;

  TEST_ASSERT_FALSE(Ntcip1201_GetPhase(&prog, 99U, &e));
}

void test_get_phase_null_out_returns_false(void)
{
  TEST_ASSERT_FALSE(Ntcip1201_GetPhase(&prog, 0U, (void *) 0));
}

/* --- NTCIP 1201 SET phase ------------------------------------------------ */

void test_set_phase_min_green_valid(void)
{
  bool ok = Ntcip1201_SetPhaseMinGreen(&prog, 0U, 15U);

  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL_UINT8(15U, prog.config.phases[0].minGreenTime);
}

void test_set_phase_min_green_above_max_rejected(void)
{
  /* maxGreen = 60, try to set minGreen = 70 */
  TEST_ASSERT_FALSE(Ntcip1201_SetPhaseMinGreen(&prog, 0U, 70U));
  /* config unchanged */
  TEST_ASSERT_EQUAL_UINT8(10U, prog.config.phases[0].minGreenTime);
}

void test_set_phase_max_green_valid(void)
{
  TEST_ASSERT_TRUE(Ntcip1201_SetPhaseMaxGreen(&prog, 0U, 90U));
  TEST_ASSERT_EQUAL_UINT8(90U, prog.config.phases[0].maxGreenTime);
}

void test_set_phase_max_green_below_min_rejected(void)
{
  /* minGreen = 10, try to set maxGreen = 5 */
  TEST_ASSERT_FALSE(Ntcip1201_SetPhaseMaxGreen(&prog, 0U, 5U));
}

void test_set_yellow_change_interval(void)
{
  TEST_ASSERT_TRUE(Ntcip1201_SetPhaseYellowChange(&prog, 0U, 4U));
  TEST_ASSERT_EQUAL_UINT8(4U, prog.config.signalGroups[0].yellowChangeInterval);
}

void test_set_yellow_change_out_of_range_sg_rejected(void)
{
  TEST_ASSERT_FALSE(Ntcip1201_SetPhaseYellowChange(&prog, 99U, 4U));
}

/* --- Unit status -------------------------------------------------------- */

void test_get_unit_status_max_phases(void)
{
  Ntcip1201UnitStatus_t s = Ntcip1201_GetUnitStatus(&prog);

  TEST_ASSERT_EQUAL_UINT8(FIXTURE_4P_PHASE_COUNT, s.maxPhases);
}

void test_set_control_mode_flash(void)
{
  bool ok = Ntcip1201_SetControlMode(&prog, 4U);   /* 4 = FLASH */

  TEST_ASSERT_TRUE(ok);
  /* One tick to apply the state change */
  ProgramTick(&prog);
  TEST_ASSERT_EQUAL_INT(CTRL_STATE_FLASH, ProgramGetState(&prog));
}

void test_set_control_mode_unknown_rejected(void)
{
  TEST_ASSERT_FALSE(Ntcip1201_SetControlMode(&prog, 99U));
}

/* --- Phase status ------------------------------------------------------- */

void Test_Phase_status_not_active_initially(void)
{
  Ntcip1201PhaseStatus_t s;

  TEST_ASSERT_TRUE(Ntcip1201_GetPhaseStatus(&prog, 0U, &s));
  TEST_ASSERT_FALSE(s.isActive);
}

/* --- OID registry ------------------------------------------------------- */

void test_oid_get_max_phases(void)
{
  OidValue_t v;
  OidResult_t r = OidRegistry_Get(&prog, OID_MAX_PHASES, 0U, &v);

  TEST_ASSERT_EQUAL_INT(OID_RESULT_OK, r);
  TEST_ASSERT_EQUAL_UINT32(PHASES_MAX, v.intVal);
}

void test_oid_get_phase_min_green(void)
{
  OidValue_t v;
  OidResult_t r = OidRegistry_Get(&prog, OID_PHASE_MIN_GREEN_n, 0U, &v);

  TEST_ASSERT_EQUAL_INT(OID_RESULT_OK, r);
  TEST_ASSERT_EQUAL_UINT32(10U, v.intVal);
}

void test_oid_set_phase_min_green(void)
{
  OidValue_t v;

  v.intVal = 12U;
  OidResult_t r = OidRegistry_Set(&prog, OID_PHASE_MIN_GREEN_n, 0U, &v);

  TEST_ASSERT_EQUAL_INT(OID_RESULT_OK, r);
  TEST_ASSERT_EQUAL_UINT8(12U, prog.config.phases[0].minGreenTime);
}

void test_oid_set_read_only_object_returns_error(void)
{
  OidValue_t v;

  v.intVal = 5U;
  OidResult_t r = OidRegistry_Set(&prog, OID_MAX_PHASES, 0U, &v);

  TEST_ASSERT_EQUAL_INT(OID_RESULT_READ_ONLY, r);
}

void test_oid_get_unknown_returns_not_found(void)
{
  OidValue_t v;
  OidResult_t r = OidRegistry_Get(&prog, OID_UNKNOWN, 0U, &v);

  TEST_ASSERT_EQUAL_INT(OID_RESULT_NOT_FOUND, r);
}

void test_oid_get_phase_out_of_range(void)
{
  OidValue_t v;
  OidResult_t r = OidRegistry_Get(&prog, OID_PHASE_MIN_GREEN_n, 99U, &v);

  TEST_ASSERT_EQUAL_INT(OID_RESULT_RANGE_ERR, r);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_get_phase_returns_min_max_green);
  RUN_TEST(test_get_phase_returns_1based_phase_number);
  RUN_TEST(test_get_phase_out_of_range_returns_false);
  RUN_TEST(test_get_phase_null_out_returns_false);
  RUN_TEST(test_set_phase_min_green_valid);
  RUN_TEST(test_set_phase_min_green_above_max_rejected);
  RUN_TEST(test_set_phase_max_green_valid);
  RUN_TEST(test_set_phase_max_green_below_min_rejected);
  RUN_TEST(test_set_yellow_change_interval);
  RUN_TEST(test_set_yellow_change_out_of_range_sg_rejected);
  RUN_TEST(test_get_unit_status_max_phases);
  RUN_TEST(test_set_control_mode_flash);
  RUN_TEST(test_set_control_mode_unknown_rejected);
  RUN_TEST(Test_Phase_status_not_active_initially);
  RUN_TEST(test_oid_get_max_phases);
  RUN_TEST(test_oid_get_phase_min_green);
  RUN_TEST(test_oid_set_phase_min_green);
  RUN_TEST(test_oid_set_read_only_object_returns_error);
  RUN_TEST(test_oid_get_unknown_returns_not_found);
  RUN_TEST(test_oid_get_phase_out_of_range);

  return UNITY_END();
}
