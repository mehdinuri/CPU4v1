/*
 * Tests/integration/Test_Conflict_safety.c
 *
 * Integration test: Conflict detection → ALL_RED safety fallback.
 *
 * Scenario:
 *   1. Start phase 0 (NS green: SG 0, SG 1).
 *   2. Forcibly set Conflicting SG 2 (EW) to SG_STATE_OPEN in runtime,
 *      simulating a runaway condition or config error.
 *   3. The next ProgramTick() must detect the Conflict and:
 *      a) Switch controller state to CTRL_STATE_ALL_RED.
 *      b) Emit SNMP_TRAP_CONFLICT_FAULT.
 */
#include "unity.h"
#include "Domain/Intersection/Program.h"
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

/* Minimal config that starts phase 0 on first tick from ALL_RED */
static ProgramConfig_t build_config(void)
{
  ProgramConfig_t cfg;
  int i;

  for (i = 0; i < (int) sizeof(cfg); i++)
  {
    ((uint8_t *) &cfg)[i] = 0U;
  }

  cfg.phaseCount = FIXTURE_4P_PHASE_COUNT;
  cfg.signalGroupCount = FIXTURE_4P_SG_COUNT;
  cfg.activeSignalProgram = 0U;

  Fixture4P_BuildPhases(cfg.phases);
  Fixture4P_BuildSGs(cfg.signalGroups);

  /* One transition: ALL_RED → phase 0, always true */
  cfg.transitions[0][0].fromState = (uint8_t) CTRL_STATE_ALL_RED;
  cfg.transitions[0][0].ruleIndex = 0U;
  cfg.transitions[0][0].priority = 1U;
  cfg.transitions[0][0].param1 = 0U;
  cfg.transitions[0][0].param2 = 0U;
  cfg.transitionCounts[0] = 1U;

  cfg.rules[0][0].operationStart = 0U;

  /* Op 0: 1 == 1 (always true) */
  cfg.operations[0][0].op = OPR_EQUAL;
  cfg.operations[0][0].operandA = 129U;
  cfg.operations[0][0].operandB = 129U;
  cfg.operations[0][1].op = OPR_NONE;

  /* Stmt 0: CMD_PHASE_START(0) */
  cfg.statements[0][0].cmd = CMD_PHASE_START;
  cfg.statements[0][0].param1 = 0U;

  return cfg;
}

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

  ProgramConfig_t cfg = build_config();

  ProgramLoadConfig(&prog, &cfg);
}

void tearDown(void)
{
}

/* --- tests -------------------------------------------------------------- */

void Test_Conflict_triggers_all_red(void)
{
  /* Get into PHASE state with phase 0 active */
  ProgramRequestState(&prog, CTRL_STATE_ALL_RED);
  ProgramTick(&prog);     /* Apply ALL_RED */
  ProgramTick(&prog);     /* Fires T0 → start phase 0 */
  TEST_ASSERT_EQUAL_INT(CTRL_STATE_PHASE, ProgramGetState(&prog));

  /* Inject Conflict: forcibly open SG 2 (Conflicts with SG 0, SG 1) */
  prog.runtime.signalGroups[2].state = SG_STATE_OPEN;

  /* Next tick must detect Conflict and fall to ALL_RED */
  ProgramTick(&prog);

  TEST_ASSERT_EQUAL_INT(CTRL_STATE_ALL_RED, ProgramGetState(&prog));
}

void Test_Conflict_emits_snmp_trap(void)
{
  ProgramRequestState(&prog, CTRL_STATE_ALL_RED);
  ProgramTick(&prog);
  ProgramTick(&prog);

  /* Clear any startup traps so we can count Conflict traps precisely */
  MockSnmpNotifier_Init(&snmpCtx);
  snmpPort = MockSnmpNotifier_Create(&snmpCtx);
  prog.snmpNotifier = &snmpPort;

  prog.runtime.signalGroups[2].state = SG_STATE_OPEN;
  ProgramTick(&prog);

  TEST_ASSERT_TRUE(MockSnmpNotifier_HasTrap(&snmpCtx,
                                            SNMP_TRAP_CONFLICT_FAULT));
}

void Test_Conflict_sets_all_lamps_to_red(void)
{
  ProgramRequestState(&prog, CTRL_STATE_ALL_RED);
  ProgramTick(&prog);
  ProgramTick(&prog);

  prog.runtime.signalGroups[2].state = SG_STATE_OPEN;
  ProgramTick(&prog);

  /* All signal group first outputs must be RED */
  uint8_t sg;

  for (sg = 0U; sg < FIXTURE_4P_SG_COUNT; sg++)
  {
    uint8_t outIdx = prog.config.signalGroups[sg].firstOutputIndex;

    TEST_ASSERT_EQUAL_MESSAGE(SIGNAL_COLOR_RED, sigCtx.lamps[outIdx],
                              "Lamp not red after Conflict safety fallback");
  }
}

void test_no_Conflict_no_fallback(void)
{
  /* Proper phase 0 operation — SG 2/3 remain closed */
  ProgramRequestState(&prog, CTRL_STATE_ALL_RED);
  ProgramTick(&prog);
  ProgramTick(&prog);

  TEST_ASSERT_EQUAL_INT(CTRL_STATE_PHASE, ProgramGetState(&prog));

  /* Multiple ticks without injecting Conflicts */
  uint8_t i;

  for (i = 0U; i < 5U; i++)
  {
    ProgramTick(&prog);
  }

  /* Should still be in PHASE, not ALL_RED */
  TEST_ASSERT_EQUAL_INT(CTRL_STATE_PHASE, ProgramGetState(&prog));
  TEST_ASSERT_FALSE(MockSnmpNotifier_HasTrap(&snmpCtx,
                                             SNMP_TRAP_CONFLICT_FAULT));
}

void test_yellow_green_Conflict_also_triggers_fallback(void)
{
  ProgramRequestState(&prog, CTRL_STATE_ALL_RED);
  ProgramTick(&prog);
  ProgramTick(&prog);

  /* SG 2 is yellow (CLOSING), SG 0 is green — that's a yellow-green Conflict */
  prog.runtime.signalGroups[2].state = SG_STATE_CLOSING;

  ProgramTick(&prog);

  TEST_ASSERT_EQUAL_INT(CTRL_STATE_ALL_RED, ProgramGetState(&prog));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(Test_Conflict_triggers_all_red);
  RUN_TEST(Test_Conflict_emits_snmp_trap);
  RUN_TEST(Test_Conflict_sets_all_lamps_to_red);
  RUN_TEST(test_no_Conflict_no_fallback);
  RUN_TEST(test_yellow_green_Conflict_also_triggers_fallback);

  return UNITY_END();
}
