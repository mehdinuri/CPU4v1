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

static void MakeInputs(tSSignalOutputBuildInputs *pIn,
                       uint8_t bFlash,
                       uint8_t bSync)
{
  memset(pIn, 0, sizeof(*pIn));
  pIn->bFlashActive = bFlash;
  pIn->bFlashSyncActive = bSync;
}

void test_run_mode_copies_run_channels(void)
{
  tSSignalOutputBuildInputs in;
  tSSignalOutputImage out;

  MakeInputs(&in, 0U, 0U);
  in.aRunChannels[0] = 1U;
  in.aRunChannels[3] = 1U;
  in.aRunChannels[6] = 1U;
  in.aRunChannels[9] = 1U;

  SignalOutputImageBuilder_Build(&in, &out);

  TEST_ASSERT_EQUAL_UINT8(1U, out.aChannels[0]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.aChannels[3]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.aChannels[6]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.aChannels[9]);
  TEST_ASSERT_EQUAL_UINT8(0U, out.aChannels[1]);
  TEST_ASSERT_EQUAL_UINT8(0U, out.aChannels[10]);
}

void test_run_mode_passes_through_conflicting_signals(void)
{
  tSSignalOutputBuildInputs in;
  tSSignalOutputImage out;

  /* Force conflicting Red+Green on group 1 (channels 0 and 2) */
  MakeInputs(&in, 0U, 0U);
  in.aRunChannels[0] = 1U;
  in.aRunChannels[2] = 1U;

  SignalOutputImageBuilder_Build(&in, &out);

  /* Both should be preserved as 1 */
  TEST_ASSERT_EQUAL_UINT8(1U, out.aChannels[0]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.aChannels[2]);
  TEST_ASSERT_EQUAL_UINT8(0U, out.aChannels[1]);
}

void test_flash_active_sync_off_forces_all_dark(void)
{
  tSSignalOutputBuildInputs in;
  tSSignalOutputImage out;

  /* flash channels would otherwise be ON; sync off should zero them */
  MakeInputs(&in, 1U, 0U);
  in.aFlashChannels[0] = 1U;
  in.aFlashChannels[3] = 1U;
  in.aRunChannels[0] = 1U;

  SignalOutputImageBuilder_Build(&in, &out);

  for (uint8_t i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    TEST_ASSERT_EQUAL_UINT8(0U, out.aChannels[i]);
  }
}

void test_flash_active_sync_on_uses_flash_channels(void)
{
  tSSignalOutputBuildInputs in;
  tSSignalOutputImage out;

  MakeInputs(&in, 1U, 1U);
  in.aFlashChannels[2] = 1U;
  in.aFlashChannels[5] = 1U;
  in.aFlashChannels[8] = 1U;
  in.aFlashChannels[11] = 1U;

  SignalOutputImageBuilder_Build(&in, &out);

  TEST_ASSERT_EQUAL_UINT8(1U, out.aChannels[2]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.aChannels[5]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.aChannels[8]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.aChannels[11]);
  TEST_ASSERT_EQUAL_UINT8(0U, out.aChannels[0]);
  TEST_ASSERT_EQUAL_UINT8(0U, out.aChannels[10]);
}

void test_port_wiring_with_mock_records_apply(void)
{
  tSMockSignalOutputAdapterCtx mockCtx;
  ISignalOutputPort_t port;
  tSSignalOutputBuildInputs in;
  tSSignalOutputImage image;

  MockSignalOutputAdapter_Init(&mockCtx);
  port = MockSignalOutputAdapter_CreatePort(&mockCtx);

  memset(&in, 0, sizeof(in));
  in.aRunChannels[5] = 1U;
  SignalOutputImageBuilder_Build(&in, &image);

  SignalOutput_Apply(&port, &image);
  SignalOutput_Apply(&port, &image);

  TEST_ASSERT_EQUAL_UINT32(2U, mockCtx.lApplyCount);
  TEST_ASSERT_EQUAL_UINT8(1U, mockCtx.SLastImage.aChannels[5]);
  TEST_ASSERT_EQUAL_UINT8(0U, mockCtx.SLastImage.aChannels[0]);
}

void test_port_wiring_with_mock_records_all_off(void)
{
  tSMockSignalOutputAdapterCtx mockCtx;
  ISignalOutputPort_t port;

  MockSignalOutputAdapter_Init(&mockCtx);
  port = MockSignalOutputAdapter_CreatePort(&mockCtx);

  SignalOutput_AllOff(&port);

  TEST_ASSERT_EQUAL_UINT32(1U, mockCtx.lAllOffCount);
  TEST_ASSERT_EQUAL_UINT32(0U, mockCtx.lApplyCount);
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
