/*
 * Tests/Unit/Test_OutputVerify.c
 *
 * Exercises the post-write verify debouncer — mismatch accumulation, the
 * per-channel reset on match, threshold latching, and the sticky fault
 * flag.
 */
#include <string.h>
#include "unity.h"
#include "Domain/OutputVerify.h"

static OutputVerifyState_t state;
static SignalOutputImage_t cmd;
static SignalOutputImage_t obs;

void setUp(void)
{
  OutputVerify_Reset(&state);
  memset(&cmd, 0, sizeof(cmd));
  memset(&obs, 0, sizeof(obs));
}

void tearDown(void)
{
}

void test_reset_clears_state(void)
{
  memset(&state, 0xAB, sizeof(state));
  OutputVerify_Reset(&state);

  TEST_ASSERT_EQUAL_UINT8(0U, state.faultActive);
  for (uint8_t i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    TEST_ASSERT_EQUAL_UINT8(0U, state.mismatchCount[i]);
  }
}

void test_matching_images_produce_no_fault(void)
{
  cmd.channels[0] = 1U;
  obs.channels[0] = 1U;

  for (uint8_t i = 0U; i < 20U; i++)
  {
    OutputVerify_Step(&state, &cmd, &obs);
  }

  TEST_ASSERT_EQUAL_UINT8(0U, state.faultActive);
  TEST_ASSERT_EQUAL_UINT8(0U, state.mismatchCount[0]);
}

void test_single_mismatch_below_threshold_does_not_trip(void)
{
  cmd.channels[3] = 1U;
  /* obs.channels[3] stays 0 — mismatch */

  for (uint8_t i = 0U; i < (OUTPUT_VERIFY_FAULT_THRESHOLD - 1U); i++)
  {
    OutputVerify_Step(&state, &cmd, &obs);
  }

  TEST_ASSERT_EQUAL_UINT8(0U, state.faultActive);
  TEST_ASSERT_EQUAL_UINT8(OUTPUT_VERIFY_FAULT_THRESHOLD - 1U,
                          state.mismatchCount[3]);
}

void test_persistent_mismatch_latches_fault(void)
{
  cmd.channels[5] = 1U;
  /* obs.channels[5] stays 0 — mismatch */

  for (uint8_t i = 0U; i < OUTPUT_VERIFY_FAULT_THRESHOLD; i++)
  {
    OutputVerify_Step(&state, &cmd, &obs);
  }

  TEST_ASSERT_EQUAL_UINT8(1U, state.faultActive);
}

void test_match_resets_per_channel_counter(void)
{
  cmd.channels[1] = 1U;
  obs.channels[1] = 0U;

  /* 3 mismatches */
  for (uint8_t i = 0U; i < 3U; i++)
  {
    OutputVerify_Step(&state, &cmd, &obs);
  }

  TEST_ASSERT_EQUAL_UINT8(3U, state.mismatchCount[1]);

  /* Now matching — counter should clear */
  obs.channels[1] = 1U;
  OutputVerify_Step(&state, &cmd, &obs);
  TEST_ASSERT_EQUAL_UINT8(0U, state.mismatchCount[1]);
  TEST_ASSERT_EQUAL_UINT8(0U, state.faultActive);
}

void test_fault_is_sticky_even_if_channel_recovers(void)
{
  cmd.channels[7] = 1U;
  /* 5 mismatches — trip */
  for (uint8_t i = 0U; i < OUTPUT_VERIFY_FAULT_THRESHOLD; i++)
  {
    OutputVerify_Step(&state, &cmd, &obs);
  }

  TEST_ASSERT_EQUAL_UINT8(1U, state.faultActive);

  /* Channel recovers — counter clears, but fault stays latched */
  obs.channels[7] = 1U;
  OutputVerify_Step(&state, &cmd, &obs);
  TEST_ASSERT_EQUAL_UINT8(0U, state.mismatchCount[7]);
  TEST_ASSERT_EQUAL_UINT8(1U, state.faultActive);
}

void test_fault_cleared_only_by_reset(void)
{
  cmd.channels[2] = 1U;
  for (uint8_t i = 0U; i < OUTPUT_VERIFY_FAULT_THRESHOLD; i++)
  {
    OutputVerify_Step(&state, &cmd, &obs);
  }

  TEST_ASSERT_EQUAL_UINT8(1U, state.faultActive);

  OutputVerify_Reset(&state);
  TEST_ASSERT_EQUAL_UINT8(0U, state.faultActive);
}

void test_any_of_twelve_channels_can_trip(void)
{
  /* Mismatch only on channel 11 */
  cmd.channels[11] = 1U;

  for (uint8_t i = 0U; i < OUTPUT_VERIFY_FAULT_THRESHOLD; i++)
  {
    OutputVerify_Step(&state, &cmd, &obs);
  }

  TEST_ASSERT_EQUAL_UINT8(1U, state.faultActive);
  TEST_ASSERT_GREATER_OR_EQUAL_UINT8(OUTPUT_VERIFY_FAULT_THRESHOLD,
                                     state.mismatchCount[11]);
}

void test_counter_saturates_at_ff(void)
{
  cmd.channels[0] = 1U;
  for (uint16_t i = 0U; i < 300U; i++)
  {
    OutputVerify_Step(&state, &cmd, &obs);
  }

  TEST_ASSERT_EQUAL_UINT8(0xFFU, state.mismatchCount[0]);
}

void test_normalises_nonzero_channel_values(void)
{
  cmd.channels[4] = 7U;     /* any nonzero = commanded on */
  obs.channels[4] = 99U;    /* any nonzero = observed on */

  for (uint8_t i = 0U; i < 10U; i++)
  {
    OutputVerify_Step(&state, &cmd, &obs);
  }

  TEST_ASSERT_EQUAL_UINT8(0U, state.faultActive);
  TEST_ASSERT_EQUAL_UINT8(0U, state.mismatchCount[4]);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_reset_clears_state);
  RUN_TEST(test_matching_images_produce_no_fault);
  RUN_TEST(test_single_mismatch_below_threshold_does_not_trip);
  RUN_TEST(test_persistent_mismatch_latches_fault);
  RUN_TEST(test_match_resets_per_channel_counter);
  RUN_TEST(test_fault_is_sticky_even_if_channel_recovers);
  RUN_TEST(test_fault_cleared_only_by_reset);
  RUN_TEST(test_any_of_twelve_channels_can_trip);
  RUN_TEST(test_counter_saturates_at_ff);
  RUN_TEST(test_normalises_nonzero_channel_values);

  return UNITY_END();
}
