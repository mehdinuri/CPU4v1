/*
 * Tests/unit/Test_Conflict.c
 *
 * Unit tests for App/Domain/Intersection/Conflict.c
 */
#include "unity.h"
#include "Domain/Intersection/Conflict.h"
#include "Domain/Intersection/SignalGroup.h"
#include "MockSnmpNotifier.h"
#include "ConflictMatrix_6sg.h"

static SignalGroupConfig_t sgCfg[FIXTURE_6SG_COUNT];
static SignalGroupRuntime_t sgRt[FIXTURE_6SG_COUNT];
static MockSnmpNotifierCtx_t snmpCtx;
static ISnmpNotifierPort_t snmpPort;

void setUp(void)
{
  Fixture6SG_Build(sgCfg);
  uint8_t i;

  for (i = 0U; i < FIXTURE_6SG_COUNT; i++)
  {
    SG_Reset(&sgRt[i]);
  }

  MockSnmpNotifier_Init(&snmpCtx);
  snmpPort = MockSnmpNotifier_Create(&snmpCtx);
}

void tearDown(void)
{
}

/* --- tests -------------------------------------------------------------- */

void test_no_Conflict_when_all_closed(void)
{
  ConflictType_t result = Conflict_Check(sgCfg,
                                         sgRt,
                                         FIXTURE_6SG_COUNT,
                                         &snmpPort);

  TEST_ASSERT_EQUAL_INT(CONFLICT_NONE, result);
  TEST_ASSERT_EQUAL_UINT32(0U, snmpCtx.count);
}

void test_no_Conflict_non_Conflicting_pair_both_green(void)
{
  /* SG 0 and SG 1 are NS — not in Conflict with each other */
  sgRt[0].state = SG_STATE_OPEN;
  sgRt[1].state = SG_STATE_OPEN;

  ConflictType_t result = Conflict_Check(sgCfg,
                                         sgRt,
                                         FIXTURE_6SG_COUNT,
                                         &snmpPort);

  TEST_ASSERT_EQUAL_INT(CONFLICT_NONE, result);
}

void test_green_green_Conflict_detected(void)
{
  /* SG 0 (NS) + SG 2 (EW) — Conflicting pair, both green */
  sgRt[0].state = SG_STATE_OPEN;
  sgRt[2].state = SG_STATE_OPEN;

  ConflictType_t result = Conflict_Check(sgCfg,
                                         sgRt,
                                         FIXTURE_6SG_COUNT,
                                         &snmpPort);

  TEST_ASSERT_EQUAL_INT(CONFLICT_GREEN_GREEN, result);
}

void test_green_green_Conflict_emits_snmp_trap(void)
{
  sgRt[0].state = SG_STATE_OPEN;
  sgRt[2].state = SG_STATE_OPEN;

  Conflict_Check(sgCfg, sgRt, FIXTURE_6SG_COUNT, &snmpPort);

  TEST_ASSERT_TRUE(MockSnmpNotifier_HasTrap(&snmpCtx,
                                            SNMP_TRAP_CONFLICT_FAULT));
}

void test_yellow_green_Conflict_detected(void)
{
  /* SG 0 yellow (CLOSING), SG 2 green (OPEN) */
  sgRt[0].state = SG_STATE_CLOSING;    /* yellow */
  sgRt[2].state = SG_STATE_OPEN;       /* green  */

  ConflictType_t result = Conflict_Check(sgCfg,
                                         sgRt,
                                         FIXTURE_6SG_COUNT,
                                         &snmpPort);

  TEST_ASSERT_EQUAL_INT(CONFLICT_YELLOW_GREEN, result);
}

void test_no_Conflict_when_one_closed(void)
{
  /* SG 0 open, SG 2 closed — should not Conflict */
  sgRt[0].state = SG_STATE_OPEN;
  sgRt[2].state = SG_STATE_CLOSED;

  ConflictType_t result = Conflict_Check(sgCfg,
                                         sgRt,
                                         FIXTURE_6SG_COUNT,
                                         &snmpPort);

  TEST_ASSERT_EQUAL_INT(CONFLICT_NONE, result);
}

void Test_Conflict_exists_returns_true_for_configured_pair(void)
{
  TEST_ASSERT_TRUE(Conflict_Exists(&sgCfg[0], 2U));
  TEST_ASSERT_TRUE(Conflict_Exists(&sgCfg[0], 3U));
}

void Test_Conflict_exists_returns_false_for_non_Conflicting_pair(void)
{
  /* SG 0 and SG 1 are not in Conflict */
  TEST_ASSERT_FALSE(Conflict_Exists(&sgCfg[0], 1U));
}

void test_clearance_seconds_zero_for_no_Conflict(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, Conflict_GetClearanceSeconds(&sgCfg[0], 1U));
}

void test_opening_state_counts_as_active(void)
{
  /* SG in OPENING (approaching green) + Conflicting SG in OPEN → should flag */
  sgRt[0].state = SG_STATE_OPENING;
  sgRt[2].state = SG_STATE_OPEN;

  ConflictType_t result = Conflict_Check(sgCfg,
                                         sgRt,
                                         FIXTURE_6SG_COUNT,
                                         &snmpPort);

  TEST_ASSERT_NOT_EQUAL(CONFLICT_NONE, result);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_no_Conflict_when_all_closed);
  RUN_TEST(test_no_Conflict_non_Conflicting_pair_both_green);
  RUN_TEST(test_green_green_Conflict_detected);
  RUN_TEST(test_green_green_Conflict_emits_snmp_trap);
  RUN_TEST(test_yellow_green_Conflict_detected);
  RUN_TEST(test_no_Conflict_when_one_closed);
  RUN_TEST(Test_Conflict_exists_returns_true_for_configured_pair);
  RUN_TEST(Test_Conflict_exists_returns_false_for_non_Conflicting_pair);
  RUN_TEST(test_clearance_seconds_zero_for_no_Conflict);
  RUN_TEST(test_opening_state_counts_as_active);

  return UNITY_END();
}
