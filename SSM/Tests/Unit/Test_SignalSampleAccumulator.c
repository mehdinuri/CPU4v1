/*
 * Tests/Unit/Test_SignalSampleAccumulator.c
 *
 * Covers the on/off sample accumulator domain service. Uses
 * MockSignalInputAdapter to drive the input side end-to-end.
 */
#include <string.h>
#include "unity.h"
#include "Domain/SignalSampleAccumulator.h"
#include "Adapters/Mock/MockSignalInputAdapter.h"

static tSSignalSampleAccumulator acc;

void setUp(void)
{
  SignalSampleAccumulator_Reset(&acc);
}

void tearDown(void)
{
}

void test_reset_zeros_all_counters(void)
{
  /* Spam some data first. */
  memset(&acc, 0x55, sizeof(acc));
  SignalSampleAccumulator_Reset(&acc);

  for (uint8_t i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    TEST_ASSERT_EQUAL_UINT8(0U, acc.aOnCntr[i]);
    TEST_ASSERT_EQUAL_UINT8(0U, acc.aOffCntr[i]);
  }
}

void test_observe_accumulates_on_counts(void)
{
  tSSignalInputSnapshot snap;

  memset(&snap, 0, sizeof(snap));
  snap.aChannels[3] = 1U;

  SignalSampleAccumulator_Observe(&acc, &snap);
  SignalSampleAccumulator_Observe(&acc, &snap);
  SignalSampleAccumulator_Observe(&acc, &snap);

  TEST_ASSERT_EQUAL_UINT8(3U, acc.aOnCntr[3]);
  TEST_ASSERT_EQUAL_UINT8(0U, acc.aOffCntr[3]);
  TEST_ASSERT_EQUAL_UINT8(0U, acc.aOnCntr[4]);
  TEST_ASSERT_EQUAL_UINT8(3U, acc.aOffCntr[4]);
}

void test_counters_saturate_at_ff(void)
{
  tSSignalInputSnapshot snap;

  memset(&snap, 0, sizeof(snap));
  snap.aChannels[0] = 1U;

  for (uint16_t i = 0U; i < 300U; i++)
  {
    SignalSampleAccumulator_Observe(&acc, &snap);
  }

  TEST_ASSERT_EQUAL_UINT8(0xFFU, acc.aOnCntr[0]);
  TEST_ASSERT_EQUAL_UINT8(0xFFU, acc.aOffCntr[1]);
}

void test_summary_uses_strict_majority(void)
{
  tSSignalOutputImage image;

  acc.aOnCntr[0] = 5U; acc.aOffCntr[0] = 3U;      /* on > off → 1 */
  acc.aOnCntr[1] = 3U; acc.aOffCntr[1] = 5U;      /* off > on → 0 */
  acc.aOnCntr[2] = 4U; acc.aOffCntr[2] = 4U;      /* tie → 0 */

  SignalSampleAccumulator_Summary(&acc, &image);

  TEST_ASSERT_EQUAL_UINT8(1U, image.aChannels[0]);
  TEST_ASSERT_EQUAL_UINT8(0U, image.aChannels[1]);
  TEST_ASSERT_EQUAL_UINT8(0U, image.aChannels[2]);
}

void test_observe_via_mock_input_port(void)
{
  tSMockSignalInputAdapterCtx mockCtx;
  ISignalInputPort_t port;
  tSSignalInputSnapshot active;
  tSSignalInputSnapshot quiet;
  tSSignalOutputImage image;

  MockSignalInputAdapter_Init(&mockCtx);
  port = MockSignalInputAdapter_CreatePort(&mockCtx);

  memset(&active, 0, sizeof(active));
  active.aChannels[0] = 1U;
  memset(&quiet, 0, sizeof(quiet));

  /* Four "active" samples + one "quiet" → on=4, off=1 → majority → active */
  for (uint8_t i = 0U; i < 4U; i++)
  {
    MockSignalInputAdapter_SetSnapshot(&mockCtx, &active);
    tSSignalInputSnapshot captured;

    SignalInput_Sample(&port, &captured);
    SignalSampleAccumulator_Observe(&acc, &captured);
  }

  MockSignalInputAdapter_SetSnapshot(&mockCtx, &quiet);
  tSSignalInputSnapshot captured;

  SignalInput_Sample(&port, &captured);
  SignalSampleAccumulator_Observe(&acc, &captured);

  SignalSampleAccumulator_Summary(&acc, &image);
  TEST_ASSERT_EQUAL_UINT32(5U, mockCtx.lSampleCount);
  TEST_ASSERT_EQUAL_UINT8(1U, image.aChannels[0]);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_reset_zeros_all_counters);
  RUN_TEST(test_observe_accumulates_on_counts);
  RUN_TEST(test_counters_saturate_at_ff);
  RUN_TEST(test_summary_uses_strict_majority);
  RUN_TEST(test_observe_via_mock_input_port);

  return UNITY_END();
}
