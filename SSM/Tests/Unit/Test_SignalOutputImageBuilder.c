/*
 * Tests/Unit/Test_SignalOutputImageBuilder.c
 *
 * Unit tests for App/Domain/SignalOutputImageBuilder.c.
 * Uses MockSignalOutputAdapter only to demonstrate the end-to-end shape;
 * the builder itself is pure and can be tested without the port.
 */
#include <string.h>
#include "unity.h"
#include "Domain/SignalOutputImageBuilder.h"
#include "Adapters/Mock/MockSignalOutputAdapter.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static void MakeInputs(SignalOutputBuildInputs_t *in,
                       uint8_t flash,
                       uint8_t sync)
{
  memset(in, 0, sizeof(*in));
  in->flashActive = flash;
  in->flashSyncActive = sync;
}

void test_run_mode_copies_run_channels(void)
{
  SignalOutputBuildInputs_t in;
  SignalOutputImage_t out;

  MakeInputs(&in, 0U, 0U);
  in.runChannels[0] = 1U;
  in.runChannels[3] = 1U;
  in.runChannels[6] = 1U;
  in.runChannels[9] = 1U;

  SignalOutputImageBuilder_Build(&in, &out);

  TEST_ASSERT_EQUAL_UINT8(1U, out.channels[0]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.channels[3]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.channels[6]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.channels[9]);
  TEST_ASSERT_EQUAL_UINT8(0U, out.channels[1]);
  TEST_ASSERT_EQUAL_UINT8(0U, out.channels[10]);
}

void test_run_mode_passes_through_conflicting_signals(void)
{
  SignalOutputBuildInputs_t in;
  SignalOutputImage_t out;

  /* Force conflicting Red+Green on group 1 (channels 0 and 2) */
  MakeInputs(&in, 0U, 0U);
  in.runChannels[0] = 1U;
  in.runChannels[2] = 1U;

  SignalOutputImageBuilder_Build(&in, &out);

  /* Both should be preserved as 1 */
  TEST_ASSERT_EQUAL_UINT8(1U, out.channels[0]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.channels[2]);
  TEST_ASSERT_EQUAL_UINT8(0U, out.channels[1]);
}

void test_flash_active_sync_off_forces_all_dark(void)
{
  SignalOutputBuildInputs_t in;
  SignalOutputImage_t out;

  /* flash channels would otherwise be ON; sync off should zero them */
  MakeInputs(&in, 1U, 0U);
  in.flashChannels[0] = 1U;
  in.flashChannels[3] = 1U;
  in.runChannels[0] = 1U;

  SignalOutputImageBuilder_Build(&in, &out);

  for (uint8_t i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    TEST_ASSERT_EQUAL_UINT8(0U, out.channels[i]);
  }
}

void test_flash_active_sync_on_uses_flash_channels(void)
{
  SignalOutputBuildInputs_t in;
  SignalOutputImage_t out;

  MakeInputs(&in, 1U, 1U);
  in.flashChannels[2] = 1U;
  in.flashChannels[5] = 1U;
  in.flashChannels[8] = 1U;
  in.flashChannels[11] = 1U;

  SignalOutputImageBuilder_Build(&in, &out);

  TEST_ASSERT_EQUAL_UINT8(1U, out.channels[2]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.channels[5]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.channels[8]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.channels[11]);
  TEST_ASSERT_EQUAL_UINT8(0U, out.channels[0]);
  TEST_ASSERT_EQUAL_UINT8(0U, out.channels[10]);
}

void test_port_wiring_with_mock_records_apply(void)
{
  MockSignalOutputAdapterCtx_t mockCtx;
  ISignalOutputPort_t port;
  SignalOutputBuildInputs_t in;
  SignalOutputImage_t image;

  MockSignalOutputAdapter_Init(&mockCtx);
  port = MockSignalOutputAdapter_CreatePort(&mockCtx);

  memset(&in, 0, sizeof(in));
  in.runChannels[5] = 1U;
  SignalOutputImageBuilder_Build(&in, &image);

  SignalOutput_Apply(&port, &image);
  SignalOutput_Apply(&port, &image);

  TEST_ASSERT_EQUAL_UINT32(2U, mockCtx.applyCount);
  TEST_ASSERT_EQUAL_UINT8(1U, mockCtx.lastImage.channels[5]);
  TEST_ASSERT_EQUAL_UINT8(0U, mockCtx.lastImage.channels[0]);
}

void test_port_wiring_with_mock_records_all_off(void)
{
  MockSignalOutputAdapterCtx_t mockCtx;
  ISignalOutputPort_t port;

  MockSignalOutputAdapter_Init(&mockCtx);
  port = MockSignalOutputAdapter_CreatePort(&mockCtx);

  SignalOutput_AllOff(&port);

  TEST_ASSERT_EQUAL_UINT32(1U, mockCtx.allOffCount);
  TEST_ASSERT_EQUAL_UINT32(0U, mockCtx.applyCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_run_mode_copies_run_channels);
  RUN_TEST(test_run_mode_passes_through_conflicting_signals);
  RUN_TEST(test_flash_active_sync_off_forces_all_dark);
  RUN_TEST(test_flash_active_sync_on_uses_flash_channels);
  RUN_TEST(test_port_wiring_with_mock_records_apply);
  RUN_TEST(test_port_wiring_with_mock_records_all_off);

  return UNITY_END();
}
