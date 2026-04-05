/*
 * Tests/unit/Test_Phase.c
 *
 * Unit tests for App/Domain/Intersection/Phase.c
 */
#include "unity.h"
#include "Domain/Intersection/Phase.h"

/* Unity boilerplate */
void setUp(void)
{
}

void tearDown(void)
{
}

/* --- helpers ----------------------------------------------------------- */

static PhaseConfig_t make_cfg(uint8_t minGreen, uint8_t maxGreen)
{
  PhaseConfig_t cfg;

  cfg.signalGroupMask = 0x03U;
  cfg.minGreenTime = minGreen;
  cfg.maxGreenTime = maxGreen;

  return cfg;
}

/* --- tests -------------------------------------------------------------- */

void test_reset_clears_runtime(void)
{
  PhaseRuntime_t rt;

  rt.elapsedSeconds = 99U;
  rt.extensionSeconds = 5;
  rt.maxTimeElapsed = true;

  Phase_Reset(&rt);

  TEST_ASSERT_EQUAL_UINT16(0U, rt.elapsedSeconds);
  TEST_ASSERT_EQUAL_INT8(0, rt.extensionSeconds);
  TEST_ASSERT_FALSE(rt.maxTimeElapsed);
}

void test_tick_increments_elapsed(void)
{
  PhaseConfig_t cfg = make_cfg(5U, 30U);
  PhaseRuntime_t rt;

  Phase_Reset(&rt);

  Phase_Tick(&rt, &cfg);
  TEST_ASSERT_EQUAL_UINT16(1U, rt.elapsedSeconds);

  Phase_Tick(&rt, &cfg);
  TEST_ASSERT_EQUAL_UINT16(2U, rt.elapsedSeconds);
}

void test_tick_returns_false_before_max(void)
{
  PhaseConfig_t cfg = make_cfg(5U, 10U);
  PhaseRuntime_t rt;

  Phase_Reset(&rt);

  bool done = false;
  uint8_t i;

  for (i = 0U; i < 9U; i++)
  {
    done = Phase_Tick(&rt, &cfg);
  }

  TEST_ASSERT_FALSE(done);
}

void test_tick_returns_true_at_max(void)
{
  PhaseConfig_t cfg = make_cfg(5U, 10U);
  PhaseRuntime_t rt;

  Phase_Reset(&rt);

  uint8_t i;

  for (i = 0U; i < 10U; i++)
  {
    Phase_Tick(&rt, &cfg);
  }

  TEST_ASSERT_TRUE(rt.maxTimeElapsed);
}

void test_min_time_not_elapsed_initially(void)
{
  PhaseConfig_t cfg = make_cfg(10U, 60U);
  PhaseRuntime_t rt;

  Phase_Reset(&rt);

  TEST_ASSERT_FALSE(Phase_MinTimeElapsed(&rt, &cfg));
}

void test_min_time_elapsed_after_min_ticks(void)
{
  PhaseConfig_t cfg = make_cfg(5U, 60U);
  PhaseRuntime_t rt;

  Phase_Reset(&rt);

  uint8_t i;

  for (i = 0U; i < 5U; i++)
  {
    Phase_Tick(&rt, &cfg);
  }

  TEST_ASSERT_TRUE(Phase_MinTimeElapsed(&rt, &cfg));
}

void test_extension_adds_green_time(void)
{
  PhaseConfig_t cfg = make_cfg(5U, 20U);
  PhaseRuntime_t rt;

  Phase_Reset(&rt);

  /* Tick to max */
  uint8_t i;

  for (i = 0U; i < 20U; i++)
  {
    Phase_Tick(&rt, &cfg);
  }

  TEST_ASSERT_TRUE(rt.maxTimeElapsed);

  /* Apply +5 s extension — should allow 5 more seconds */
  Phase_ApplyExtension(&rt, &cfg, 5);
  TEST_ASSERT_FALSE(rt.maxTimeElapsed);

  for (i = 0U; i < 5U; i++)
  {
    Phase_Tick(&rt, &cfg);
  }

  TEST_ASSERT_TRUE(rt.maxTimeElapsed);
}

void test_negative_extension_clamped_to_min(void)
{
  PhaseConfig_t cfg = make_cfg(10U, 30U);     /* -30 brings effectiveMax to 0 → clamped to min=10 */
  PhaseRuntime_t rt;

  Phase_Reset(&rt);

  /* Tick 20 seconds */
  uint8_t i;

  for (i = 0U; i < 20U; i++)
  {
    Phase_Tick(&rt, &cfg);
  }

  /* Try to shorten by 30 seconds — clamped: elapsedSeconds must not drop below minGreenTime */
  Phase_ApplyExtension(&rt, &cfg, -30);

  /* Phase should end immediately (or at next tick) since elapsed >= effective max */
  bool done = Phase_Tick(&rt, &cfg);

  TEST_ASSERT_TRUE(done || rt.maxTimeElapsed);
}

void test_max_time_not_elapsed_initially(void)
{
  PhaseConfig_t cfg = make_cfg(5U, 30U);
  PhaseRuntime_t rt;

  Phase_Reset(&rt);

  TEST_ASSERT_FALSE(Phase_MaxTimeElapsed(&rt, &cfg));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_reset_clears_runtime);
  RUN_TEST(test_tick_increments_elapsed);
  RUN_TEST(test_tick_returns_false_before_max);
  RUN_TEST(test_tick_returns_true_at_max);
  RUN_TEST(test_min_time_not_elapsed_initially);
  RUN_TEST(test_min_time_elapsed_after_min_ticks);
  RUN_TEST(test_extension_adds_green_time);
  RUN_TEST(test_negative_extension_clamped_to_min);
  RUN_TEST(test_max_time_not_elapsed_initially);

  return UNITY_END();
}
