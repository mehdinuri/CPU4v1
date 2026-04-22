/*
 * Tests/Unit/Test_CanRxBackpressure.c
 *
 * Exercises the consecutive-drop counter used by can_msg_parser to promote
 * silent CAN Rx loss into the sticky EVENT_FLAGS_MAINTENANCE_CAN_RX_OVERRUN
 * flag: reset, success-resets-streak, below/at/above threshold, idempotent
 * overrun reporting, saturation, and NULL-safety on every entry point.
 */
#include "unity.h"
#include "Domain/CanRxBackpressure.h"

#define THRESHOLD 8U

static CanRxBackpressure_t state;

void setUp(void)
{
  CanRxBackpressure_Reset(&state);
}

void tearDown(void)
{
}

void test_reset_zeroes_counter(void)
{
  state.consecutiveDrops = 12345U;
  CanRxBackpressure_Reset(&state);

  TEST_ASSERT_EQUAL_UINT32(0U, state.consecutiveDrops);
}

void test_success_on_fresh_state_stays_zero(void)
{
  CanRxBackpressure_RecordSuccess(&state);

  TEST_ASSERT_EQUAL_UINT32(0U, state.consecutiveDrops);
}

void test_single_drop_below_threshold_does_not_escalate(void)
{
  uint8_t escalated = CanRxBackpressure_RecordDrop(&state, THRESHOLD);

  TEST_ASSERT_EQUAL_UINT8(0U, escalated);
  TEST_ASSERT_EQUAL_UINT32(1U, state.consecutiveDrops);
}

void test_drops_up_to_threshold_minus_one_do_not_escalate(void)
{
  uint8_t escalated = 0U;

  for (uint32_t i = 0U; i < (THRESHOLD - 1U); i++)
  {
    escalated = CanRxBackpressure_RecordDrop(&state, THRESHOLD);
    TEST_ASSERT_EQUAL_UINT8(0U, escalated);
  }

  TEST_ASSERT_EQUAL_UINT32(THRESHOLD - 1U, state.consecutiveDrops);
}

void test_threshold_crossing_escalates(void)
{
  for (uint32_t i = 0U; i < (THRESHOLD - 1U); i++)
  {
    (void) CanRxBackpressure_RecordDrop(&state, THRESHOLD);
  }

  /* The drop that takes us to == THRESHOLD must escalate. */
  uint8_t escalated = CanRxBackpressure_RecordDrop(&state, THRESHOLD);

  TEST_ASSERT_EQUAL_UINT8(1U, escalated);
  TEST_ASSERT_EQUAL_UINT32(THRESHOLD, state.consecutiveDrops);
}

void test_further_drops_remain_escalated_idempotently(void)
{
  /* Drive well past the threshold. */
  for (uint32_t i = 0U; i < (THRESHOLD * 3U); i++)
  {
    uint8_t escalated = CanRxBackpressure_RecordDrop(&state, THRESHOLD);

    if (i >= (THRESHOLD - 1U))
    {
      TEST_ASSERT_EQUAL_UINT8(1U, escalated);
    }
    else
    {
      TEST_ASSERT_EQUAL_UINT8(0U, escalated);
    }
  }
}

void test_success_clears_streak_even_after_escalation(void)
{
  for (uint32_t i = 0U; i < (THRESHOLD * 2U); i++)
  {
    (void) CanRxBackpressure_RecordDrop(&state, THRESHOLD);
  }

  TEST_ASSERT_GREATER_OR_EQUAL_UINT32(THRESHOLD, state.consecutiveDrops);

  CanRxBackpressure_RecordSuccess(&state);
  TEST_ASSERT_EQUAL_UINT32(0U, state.consecutiveDrops);

  /* Next drop starts the streak again from 1 — not from residual value. */
  uint8_t escalated = CanRxBackpressure_RecordDrop(&state, THRESHOLD);

  TEST_ASSERT_EQUAL_UINT8(0U, escalated);
  TEST_ASSERT_EQUAL_UINT32(1U, state.consecutiveDrops);
}

void test_threshold_of_one_escalates_on_first_drop(void)
{
  uint8_t escalated = CanRxBackpressure_RecordDrop(&state, 1U);

  TEST_ASSERT_EQUAL_UINT8(1U, escalated);
  TEST_ASSERT_EQUAL_UINT32(1U, state.consecutiveDrops);
}

void test_threshold_of_zero_escalates_immediately(void)
{
  /* Degenerate but well-defined: anything >= 0 is escalation. */
  uint8_t escalated = CanRxBackpressure_RecordDrop(&state, 0U);

  TEST_ASSERT_EQUAL_UINT8(1U, escalated);
}

void test_counter_saturates_at_uint32_max(void)
{
  state.consecutiveDrops = 0xFFFFFFFFU;

  uint8_t escalated = CanRxBackpressure_RecordDrop(&state, THRESHOLD);

  /* Must stay saturated, not wrap back below threshold. */
  TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFU, state.consecutiveDrops);
  TEST_ASSERT_EQUAL_UINT8(1U, escalated);
}

void test_null_state_is_safe(void)
{
  /* None of these should fault or segfault. */
  CanRxBackpressure_Reset(NULL);
  CanRxBackpressure_RecordSuccess(NULL);
  uint8_t escalated = CanRxBackpressure_RecordDrop(NULL, THRESHOLD);

  TEST_ASSERT_EQUAL_UINT8(0U, escalated);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_reset_zeroes_counter);
  RUN_TEST(test_success_on_fresh_state_stays_zero);
  RUN_TEST(test_single_drop_below_threshold_does_not_escalate);
  RUN_TEST(test_drops_up_to_threshold_minus_one_do_not_escalate);
  RUN_TEST(test_threshold_crossing_escalates);
  RUN_TEST(test_further_drops_remain_escalated_idempotently);
  RUN_TEST(test_success_clears_streak_even_after_escalation);
  RUN_TEST(test_threshold_of_one_escalates_on_first_drop);
  RUN_TEST(test_threshold_of_zero_escalates_immediately);
  RUN_TEST(test_counter_saturates_at_uint32_max);
  RUN_TEST(test_null_state_is_safe);

  return UNITY_END();
}
