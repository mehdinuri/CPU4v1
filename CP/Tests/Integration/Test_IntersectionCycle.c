/*
 * Tests/integration/Test_IntersectionCycle.c
 *
 * Integration test: full phase cycle through the ProgramCtx_t coordinator.
 *
 * Topology: 4-phase fixture (from Tests/Fixtures/TimingPlan_4Phase.h).
 *
 * Test scenario:
 *   1. Load config → ProgramInit → ProgramLoadConfig
 *   2. Inject transition rules: ALL_RED → Phase 0, Phase 0 → Phase 1
 *   3. Drive ProgramTick() for enough cycles to traverse Phase 0 → Phase 1
 *   4. Assert lamp states match expected colors at each stage
 */
#include "unity.h"
#include "Domain/Intersection/Program.h"
#include "MockSignalOutput.h"
#include "MockDetector.h"
#include "MockClock.h"
#include "MockSnmpNotifier.h"
#include "TimingPlan_4Phase.h"

/* --- fixture globals --------------------------------------------------- */

static MockSignalOutputCtx_t sigCtx;
static ISignalOutputPort_t sigPort;
static MockDetectorCtx_t detCtx;
static IDetectorInputPort_t detPort;
static MockClockCtx_t clkCtx;
static ISystemClockPort_t clkPort;
static MockSnmpNotifierCtx_t snmpCtx;
static ISnmpNotifierPort_t snmpPort;

static ProgramCtx_t prog;

/* Tick TICKS_PER_SECOND times = 1 logical second */
#define TICKS_PER_SECOND 10U

/* --- helpers ------------------------------------------------------------ */

static void tick_seconds(uint32_t n)
{
  uint32_t i, j;

  for (i = 0U; i < n; i++)
  {
    for (j = 0U; j < TICKS_PER_SECOND; j++)
    {
      ProgramTick(&prog);
    }
  }
}

/*
 * Build a minimal ProgramConfig_t with:
 *   - 4 phases, 6 SGs (from fixture)
 *   - 2 transitions in signal Program 0:
 *       T0: CTRL_STATE_ALL_RED → [always] → CMD_PHASE_START(0)
 *       T1: CTRL_STATE_PHASE   → [phase0.elapsed >= 12] → CMD_PHASE_START(1)
 *
 * Operand encoding:
 *   phase[0].elapsed = operand 0
 *   literal 12       = 128+12 = 140
 */
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
  cfg.SequenceCount = 0U;
  cfg.DetectorCount = 0U;
  cfg.activeSignalProgram = 0U;

  /* Build phases and signal groups from fixture */
  Fixture4P_BuildPhases(cfg.phases);
  Fixture4P_BuildSGs(cfg.signalGroups);

  /* ---------------------------------------------------------------
   * Transitions (signal Program 0)
   * --------------------------------------------------------------- */

  /* T0: ANY → Phase 0 start (unconditional — literal 1 == 1) */
  cfg.transitions[0][0].fromState = (uint8_t) CTRL_STATE_ALL_RED;
  cfg.transitions[0][0].ruleIndex = 0U;
  cfg.transitions[0][0].priority = 1U;
  cfg.transitions[0][0].param1 = 0U;        /* statement range start */
  cfg.transitions[0][0].param2 = 0U;        /* statement range end   */

  /* T1: PHASE → Phase 1 start when phase 0 elapsed >= 12 s */
  cfg.transitions[0][1].fromState = (uint8_t) CTRL_STATE_PHASE;
  cfg.transitions[0][1].ruleIndex = 1U;
  cfg.transitions[0][1].priority = 1U;
  cfg.transitions[0][1].param1 = 1U;        /* statement 1 */
  cfg.transitions[0][1].param2 = 1U;

  cfg.transitionCounts[0] = 2U;

  /* ---------------------------------------------------------------
   * Rules
   * --------------------------------------------------------------- */

  /* Rule 0: always true (1 == 1) */
  cfg.rules[0][0].operationStart = 0U;    /* Op 0; op 1 is zeroed sentinel */

  /* Rule 1: phase[0].elapsed >= 12 */
  cfg.rules[0][1].operationStart = 2U;    /* Op 2; op 3 is zeroed sentinel */

  /* ---------------------------------------------------------------
   * Operations
   * --------------------------------------------------------------- */

  /* Op 0: literal 1 == literal 1 → always true */
  cfg.operations[0][0].op = OPR_EQUAL;
  cfg.operations[0][0].operandA = 129U;     /* literal 1 = 128+1 */
  cfg.operations[0][0].operandB = 129U;
  /* Op 1: OPR_NONE sentinel (left zeroed by memset) */

  /* Op 2 (rule 1 start): phase[0].elapsed >= 12 */
  cfg.operations[0][2].op = OPR_GREATER_EQUAL;
  cfg.operations[0][2].operandA = 0U;      /* phase[0] elapsed */
  cfg.operations[0][2].operandB = 140U;    /* literal 12 = 128+12 */
  /* Op 3: OPR_NONE sentinel (left zeroed by memset) */

  /* T1 executes stmts[1..2]: first STOP the current phase, then START phase 1 */
  cfg.transitions[0][1].param2 = 2U;    /* override: param2=2 for 2-stmt range */

  /* ---------------------------------------------------------------
   * Statements
   * --------------------------------------------------------------- */

  /* Stmt 0: CMD_PHASE_START(0) */
  cfg.statements[0][0].cmd = CMD_PHASE_START;
  cfg.statements[0][0].param1 = 0U;

  /* Stmt 1: CMD_PHASE_STOP — closes all SGs and returns to ALL_RED briefly */
  cfg.statements[0][1].cmd = CMD_PHASE_STOP;
  cfg.statements[0][1].param1 = 0U;

  /* Stmt 2: CMD_PHASE_START(1) — opens phase 1 SGs in the same tick */
  cfg.statements[0][2].cmd = CMD_PHASE_START;
  cfg.statements[0][2].param1 = 1U;

  return cfg;
} /* build_config */

/* --- setUp / tearDown --------------------------------------------------- */

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

void test_initial_state_is_dark(void)
{
  TEST_ASSERT_EQUAL_INT(CTRL_STATE_DARK, ProgramGetState(&prog));
}

void test_request_all_red_transitions_to_all_red(void)
{
  ProgramRequestState(&prog, CTRL_STATE_ALL_RED);
  ProgramTick(&prog);
  TEST_ASSERT_EQUAL_INT(CTRL_STATE_ALL_RED, ProgramGetState(&prog));
}

void test_all_red_to_phase0_via_TransitionRule(void)
{
  ProgramRequestState(&prog, CTRL_STATE_ALL_RED);
  ProgramTick(&prog);
  TEST_ASSERT_EQUAL_INT(CTRL_STATE_ALL_RED, ProgramGetState(&prog));

  /* One more tick — the always-true transition should fire and start phase 0 */
  ProgramTick(&prog);
  TEST_ASSERT_EQUAL_INT(CTRL_STATE_PHASE, ProgramGetState(&prog));
  TEST_ASSERT_EQUAL_UINT8(0U, prog.runtime.activePhase);
}

void Test_Phase0_sgs_show_green(void)
{
  ProgramRequestState(&prog, CTRL_STATE_ALL_RED);
  ProgramTick(&prog);
  ProgramTick(&prog);     /* Fires T0: start phase 0 */

  TEST_ASSERT_EQUAL_INT(CTRL_STATE_PHASE, ProgramGetState(&prog));

  /* SG 0 output (firstOutputIndex=0) and SG 1 output (firstOutputIndex=3) = green */
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_GREEN, sigCtx.lamps[0U]);
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_GREEN, sigCtx.lamps[3U]);

  /* EW SGs (SG 2 firstOutput=6, SG 3 firstOutput=9) remain red */
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_RED, sigCtx.lamps[6U]);
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_RED, sigCtx.lamps[9U]);
}

void Test_Phase_transitions_to_phase1_after_min_elapsed(void)
{
  ProgramRequestState(&prog, CTRL_STATE_ALL_RED);
  ProgramTick(&prog);
  ProgramTick(&prog);     /* Start phase 0 */

  /* Phase 0 minGreenTime=10, maxGreenTime=60.
   * Rule T1 fires when phase[0].elapsed >= 12.
   * Tick 12 seconds worth (each second = TICKS_PER_SECOND ticks). */
  tick_seconds(12U);

  TEST_ASSERT_EQUAL_INT(CTRL_STATE_PHASE, ProgramGetState(&prog));
  /* After transition, active phase should be 1 */
  TEST_ASSERT_EQUAL_UINT8(1U, prog.runtime.activePhase);
}

void Test_Phase1_sgs_show_green(void)
{
  ProgramRequestState(&prog, CTRL_STATE_ALL_RED);
  ProgramTick(&prog);
  ProgramTick(&prog);
  tick_seconds(12U);

  /* SG 2 (firstOutput=6) and SG 3 (firstOutput=9) should now be green */
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_GREEN, sigCtx.lamps[6U]);
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_GREEN, sigCtx.lamps[9U]);
  /* NS SGs should be red (closed) */
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_RED, sigCtx.lamps[0U]);
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_RED, sigCtx.lamps[3U]);
}

void test_request_flash_overrides_phase(void)
{
  ProgramRequestState(&prog, CTRL_STATE_ALL_RED);
  ProgramTick(&prog);
  ProgramTick(&prog);

  ProgramRequestState(&prog, CTRL_STATE_FLASH);
  ProgramTick(&prog);

  TEST_ASSERT_EQUAL_INT(CTRL_STATE_FLASH, ProgramGetState(&prog));
}

void test_startup_snmp_trap_emitted(void)
{
  /* ProgramInit sends SNMP_TRAP_CONTROLLER_STARTUP */
  TEST_ASSERT_TRUE(MockSnmpNotifier_HasTrap(&snmpCtx,
                                            SNMP_TRAP_CONTROLLER_STARTUP));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_initial_state_is_dark);
  RUN_TEST(test_request_all_red_transitions_to_all_red);
  RUN_TEST(test_all_red_to_phase0_via_TransitionRule);
  RUN_TEST(Test_Phase0_sgs_show_green);
  RUN_TEST(Test_Phase_transitions_to_phase1_after_min_elapsed);
  RUN_TEST(Test_Phase1_sgs_show_green);
  RUN_TEST(test_request_flash_overrides_phase);
  RUN_TEST(test_startup_snmp_trap_emitted);

  return UNITY_END();
}
