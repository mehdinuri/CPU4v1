/*
 * Tests/unit/Test_Sequence.c
 *
 * Unit tests for App/Domain/Intersection/Sequence.c
 */
#include "unity.h"
#include "Domain/Intersection/Sequence.h"
#include "MockSignalOutput.h"
#include <string.h>

static MockSignalOutputCtx_t outCtx;
static ISignalOutputPort_t outPort;

void setUp(void)
{
  MockSignalOutput_Init(&outCtx);
  outPort = MockSignalOutput_Create(&outCtx);
}

void tearDown(void)
{
}

/* --- helpers ----------------------------------------------------------- */

/* Build a 3-step Sequence: SG0 and SG1 alternate.
 *   Step 0 (3 s): SG0=green(1), SG1=red(0)
 *   Step 1 (2 s): SG0=red(0),   SG1=green(1)
 *   Step 2 (1 s): SG0=red(0),   SG1=red(0)
 *
 * Signal packed: 4 bits per SG, SG0 in low nibble of byte 0, SG1 in high nibble.
 */
static SequenceConfig_t make_seq(void)
{
  SequenceConfig_t cfg;

  memset(&cfg, 0, sizeof(cfg));
  cfg.stepCount = 3U;
  cfg.stepDurations[0] = 3U;
  cfg.stepDurations[1] = 2U;
  cfg.stepDurations[2] = 1U;

  /* Step 0: SG0=1(green), SG1=0(red) → byte0 = 0x01 */
  cfg.stepSignals[0][0] = 0x01U;
  /* Step 1: SG0=0(red), SG1=1(green) → byte0 = 0x10 */
  cfg.stepSignals[1][0] = 0x10U;
  /* Step 2: both red → byte0 = 0x00 */
  cfg.stepSignals[2][0] = 0x00U;

  return cfg;
}

static SignalGroupConfig_t sgCfgs[2];

static void make_sg_cfgs(void)
{
  memset(sgCfgs, 0, sizeof(sgCfgs));
  sgCfgs[0].firstOutputIndex = 0U;
  sgCfgs[1].firstOutputIndex = 3U;
}

/* --- tests -------------------------------------------------------------- */

void test_reset_zeros_runtime(void)
{
  SequenceRuntime_t rt;

  rt.currentStep = 5U;
  rt.stepElapsedSeconds = 3U;
  rt.loopCount = 2U;

  Sequence_Reset(&rt);

  TEST_ASSERT_EQUAL_UINT8(0U, rt.currentStep);
  TEST_ASSERT_EQUAL_UINT8(0U, rt.stepElapsedSeconds);
  TEST_ASSERT_EQUAL_UINT8(0U, rt.loopCount);
}

void test_apply_step0_sets_sg0_green(void)
{
  SequenceConfig_t cfg = make_seq();

  make_sg_cfgs();

  Sequence_ApplyStep(0U, &cfg, sgCfgs, 2U, &outPort);

  TEST_ASSERT_EQUAL(SIGNAL_COLOR_GREEN, outCtx.lamps[0U]);   /* SG0 output at idx 0 */
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_RED,   outCtx.lamps[3U]);   /* SG1 output at idx 3 */
}

void test_apply_step1_sets_sg1_green(void)
{
  SequenceConfig_t cfg = make_seq();

  make_sg_cfgs();

  Sequence_ApplyStep(1U, &cfg, sgCfgs, 2U, &outPort);

  TEST_ASSERT_EQUAL(SIGNAL_COLOR_RED,   outCtx.lamps[0U]);
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_GREEN, outCtx.lamps[3U]);
}

void test_empty_Sequence_returns_done_immediately(void)
{
  SequenceConfig_t cfg;
  SequenceRuntime_t rt;

  memset(&cfg, 0, sizeof(cfg));
  Sequence_Reset(&rt);
  cfg.stepCount = 0U;

  bool done = Sequence_Tick(0U, &rt, &cfg, sgCfgs, 2U, &outPort);

  TEST_ASSERT_TRUE(done);
}

void test_tick_returns_false_before_step_expires(void)
{
  SequenceConfig_t cfg = make_seq();
  SequenceRuntime_t rt;

  make_sg_cfgs();
  Sequence_Reset(&rt);

  /* Step 0 lasts 3 seconds — 2 ticks should not complete it */
  bool done = Sequence_Tick(0U, &rt, &cfg, sgCfgs, 2U, &outPort);

  TEST_ASSERT_FALSE(done);
  done = Sequence_Tick(0U, &rt, &cfg, sgCfgs, 2U, &outPort);
  TEST_ASSERT_FALSE(done);
}

void test_tick_advances_step_after_duration(void)
{
  SequenceConfig_t cfg = make_seq();
  SequenceRuntime_t rt;

  make_sg_cfgs();
  Sequence_Reset(&rt);

  /* Tick through step 0 (3 s) */
  uint8_t i;

  for (i = 0U; i < 3U; i++)
  {
    Sequence_Tick(0U, &rt, &cfg, sgCfgs, 2U, &outPort);
  }

  TEST_ASSERT_EQUAL_UINT8(1U, rt.currentStep);
}

void test_tick_completes_full_loop_and_returns_done(void)
{
  SequenceConfig_t cfg = make_seq();
  SequenceRuntime_t rt;

  make_sg_cfgs();
  Sequence_Reset(&rt);

  /* Total duration: 3+2+1 = 6 ticks */
  bool done = false;
  uint8_t i;

  for (i = 0U; i < 6U; i++)
  {
    done = Sequence_Tick(0U, &rt, &cfg, sgCfgs, 2U, &outPort);
  }

  TEST_ASSERT_TRUE(done);
  TEST_ASSERT_EQUAL_UINT8(1U, rt.loopCount);
}

void test_out_of_range_step_in_apply_step_does_not_crash(void)
{
  SequenceConfig_t cfg = make_seq();

  make_sg_cfgs();
  /* Step 99 is out of range — should be a no-op, not crash */
  Sequence_ApplyStep(99U, &cfg, sgCfgs, 2U, &outPort);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_reset_zeros_runtime);
  RUN_TEST(test_apply_step0_sets_sg0_green);
  RUN_TEST(test_apply_step1_sets_sg1_green);
  RUN_TEST(test_empty_Sequence_returns_done_immediately);
  RUN_TEST(test_tick_returns_false_before_step_expires);
  RUN_TEST(test_tick_advances_step_after_duration);
  RUN_TEST(test_tick_completes_full_loop_and_returns_done);
  RUN_TEST(test_out_of_range_step_in_apply_step_does_not_crash);

  return UNITY_END();
}
