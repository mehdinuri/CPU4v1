/*
 * Tests/unit/Test_SignalGroup.c
 *
 * Unit tests for App/Domain/Intersection/SignalGroup.c
 */
#include "unity.h"
#include "Domain/Intersection/SignalGroup.h"
#include "MockSignalOutput.h"

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

static SignalGroupConfig_t make_vehicle_cfg(uint8_t outputIdx,
                                            uint8_t yellowDur)
{
  SignalGroupConfig_t cfg;
  int i;

  for (i = 0; i < SIGNAL_GROUPS_MAX; i++)
  {
    cfg.Conflicts[i].hasConflict = false;
    cfg.Conflicts[i].redClearanceInterval = 0U;
  }

  cfg.type = SG_TYPE_VEHICLE_MAINWAY;
  cfg.openingSignalIdx = 0U;
  cfg.closingSignalIdx = 0U;
  cfg.openingDuration = 0U;           /* No opening phase */
  cfg.yellowChangeInterval = yellowDur;
  cfg.pedestrianClearance = 0U;
  cfg.pedestrianWalk = 0U;
  cfg.flashSignalIdx = 0U;
  cfg.firstOutputIndex = outputIdx;
  cfg.criticalRedLampCount = 1U;

  return cfg;
}

/* Tick n times and return final state */
static SignalGroupState_t tick_n(SignalGroupRuntime_t *rt,
                                 const SignalGroupConfig_t *cfg,
                                 uint8_t sgIdx, uint8_t n)
{
  uint8_t i;

  for (i = 0U; i < n; i++)
  {
    SG_Tick(sgIdx, rt, cfg, &outPort);
  }

  return rt->state;
}

/* --- tests -------------------------------------------------------------- */

void test_reset_starts_closed(void)
{
  SignalGroupRuntime_t rt;

  SG_Reset(&rt);
  TEST_ASSERT_EQUAL_INT(SG_STATE_CLOSED, rt.state);
  TEST_ASSERT_EQUAL_UINT8(0U, rt.stateElapsedSeconds);
}

void test_is_closed_after_reset(void)
{
  SignalGroupRuntime_t rt;

  SG_Reset(&rt);
  TEST_ASSERT_TRUE(SG_IsClosed(&rt));
  TEST_ASSERT_FALSE(SG_IsOpen(&rt));
}

void test_open_with_no_opening_duration_goes_directly_to_open(void)
{
  SignalGroupConfig_t cfg = make_vehicle_cfg(0U, 3U);
  SignalGroupRuntime_t rt;

  SG_Reset(&rt);

  SG_Open(0U, &rt, &cfg, &outPort);
  /* openingDuration == 0, so should skip OPENING and go straight to OPEN */
  TEST_ASSERT_EQUAL_INT(SG_STATE_OPEN, rt.state);
  TEST_ASSERT_TRUE(SG_IsOpen(&rt));
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_GREEN, outCtx.lamps[0U]);
}

void test_open_with_opening_duration_enters_opening_state(void)
{
  SignalGroupConfig_t cfg = make_vehicle_cfg(0U, 3U);

  cfg.openingDuration = 2U;
  SignalGroupRuntime_t rt;

  SG_Reset(&rt);

  SG_Open(0U, &rt, &cfg, &outPort);
  TEST_ASSERT_EQUAL_INT(SG_STATE_OPENING, rt.state);
  TEST_ASSERT_FALSE(SG_IsOpen(&rt));
}

void test_opening_transitions_to_open_after_duration(void)
{
  SignalGroupConfig_t cfg = make_vehicle_cfg(0U, 3U);

  cfg.openingDuration = 2U;
  SignalGroupRuntime_t rt;

  SG_Reset(&rt);

  SG_Open(0U, &rt, &cfg, &outPort);
  TEST_ASSERT_EQUAL_INT(SG_STATE_OPENING, rt.state);

  /* After 2 ticks it should advance to OPEN */
  tick_n(&rt, &cfg, 0U, 2U);
  TEST_ASSERT_EQUAL_INT(SG_STATE_OPEN, rt.state);
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_GREEN, outCtx.lamps[0U]);
}

void test_close_with_no_pedestrian_clearance_goes_to_closing(void)
{
  SignalGroupConfig_t cfg = make_vehicle_cfg(0U, 3U);
  SignalGroupRuntime_t rt;

  SG_Reset(&rt);
  SG_Open(0U, &rt, &cfg, &outPort);

  SG_Close(0U, &rt, &cfg, &outPort);
  /* No pedestrianClearance (green flash), so should enter CLOSING immediately */
  TEST_ASSERT_EQUAL_INT(SG_STATE_CLOSING, rt.state);
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_YELLOW, outCtx.lamps[0U]);
}

void test_closing_transitions_to_closed_after_yellow_duration(void)
{
  SignalGroupConfig_t cfg = make_vehicle_cfg(0U, 3U);
  SignalGroupRuntime_t rt;

  SG_Reset(&rt);
  SG_Open(0U, &rt, &cfg, &outPort);
  SG_Close(0U, &rt, &cfg, &outPort);

  /* Tick through the 3-second yellow */
  tick_n(&rt, &cfg, 0U, 3U);
  TEST_ASSERT_EQUAL_INT(SG_STATE_CLOSED, rt.state);
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_RED, outCtx.lamps[0U]);
  TEST_ASSERT_TRUE(SG_IsClosed(&rt));
}

void test_green_flash_before_yellow_when_ped_clearance_nonzero(void)
{
  SignalGroupConfig_t cfg = make_vehicle_cfg(0U, 3U);

  cfg.pedestrianClearance = 4U;     /* 4 s green flash */
  SignalGroupRuntime_t rt;

  SG_Reset(&rt);
  SG_Open(0U, &rt, &cfg, &outPort);
  SG_Close(0U, &rt, &cfg, &outPort);

  TEST_ASSERT_EQUAL_INT(SG_STATE_GREEN_FLASH, rt.state);
  TEST_ASSERT_EQUAL(SIGNAL_COLOR_FLASH, outCtx.lamps[0U]);

  /* After 4 ticks, transitions to CLOSING */
  tick_n(&rt, &cfg, 0U, 4U);
  TEST_ASSERT_EQUAL_INT(SG_STATE_CLOSING, rt.state);
}

void test_lamp_color_green_when_open(void)
{
  SignalGroupConfig_t cfg = make_vehicle_cfg(5U, 3U);
  SignalGroupRuntime_t rt;

  SG_Reset(&rt);
  SG_Open(0U, &rt, &cfg, &outPort);

  TEST_ASSERT_EQUAL(SIGNAL_COLOR_GREEN, outCtx.lamps[5U]);
}

void test_flush_called_on_open(void)
{
  SignalGroupConfig_t cfg = make_vehicle_cfg(0U, 3U);
  SignalGroupRuntime_t rt;

  SG_Reset(&rt);

  uint32_t before = outCtx.flushCount;

  SG_Open(0U, &rt, &cfg, &outPort);
  TEST_ASSERT_GREATER_THAN_UINT32(before, outCtx.flushCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_reset_starts_closed);
  RUN_TEST(test_is_closed_after_reset);
  RUN_TEST(test_open_with_no_opening_duration_goes_directly_to_open);
  RUN_TEST(test_open_with_opening_duration_enters_opening_state);
  RUN_TEST(test_opening_transitions_to_open_after_duration);
  RUN_TEST(test_close_with_no_pedestrian_clearance_goes_to_closing);
  RUN_TEST(test_closing_transitions_to_closed_after_yellow_duration);
  RUN_TEST(test_green_flash_before_yellow_when_ped_clearance_nonzero);
  RUN_TEST(test_lamp_color_green_when_open);
  RUN_TEST(test_flush_called_on_open);

  return UNITY_END();
}
