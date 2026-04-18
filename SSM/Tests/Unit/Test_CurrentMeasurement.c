/*
 * Tests/Unit/Test_CurrentMeasurement.c
 *
 * Tests for App/Domain/CurrentMeasurement.c — the 10-bit-per-channel
 * pack. Drives the port via MockCurrentMeasurementAdapter too, to verify
 * the adapter's GetLatest feeds into the pack correctly.
 */
#include <string.h>
#include "unity.h"
#include "Domain/CurrentMeasurement.h"
#include "Adapters/Mock/MockCurrentMeasurementAdapter.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static tSCurrentMeasurementSnapshot MakeSnap(uint16_t a,
                                             uint16_t b,
                                             uint16_t c,
                                             uint16_t d)
{
  tSCurrentMeasurementSnapshot s;

  s.aCurrents_mA[0] = a;
  s.aCurrents_mA[1] = b;
  s.aCurrents_mA[2] = c;
  s.aCurrents_mA[3] = d;
  s.bStatus = 0U;
  s.lSeqNo = 0U;

  return s;
}

void test_pack_zero_currents_gives_zero_bytes(void)
{
  tSCurrentMeasurementSnapshot snap = MakeSnap(0U, 0U, 0U, 0U);
  tSCurrentMeasurementWire wire;

  CurrentMeasurement_Pack(&snap, &wire);

  TEST_ASSERT_EQUAL_UINT8(0U, wire.aCurLow[0]);
  TEST_ASSERT_EQUAL_UINT8(0U, wire.aCurLow[1]);
  TEST_ASSERT_EQUAL_UINT8(0U, wire.aCurLow[2]);
  TEST_ASSERT_EQUAL_UINT8(0U, wire.aCurLow[3]);
  TEST_ASSERT_EQUAL_UINT8(0U, wire.bCurHighBitsPacked);
}

void test_pack_low_bits_preserved(void)
{
  tSCurrentMeasurementSnapshot snap = MakeSnap(0x00FFU,
                                               0x00ABU,
                                               0x0055U,
                                               0x000FU);
  tSCurrentMeasurementWire wire;

  CurrentMeasurement_Pack(&snap, &wire);

  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.aCurLow[0]);
  TEST_ASSERT_EQUAL_UINT8(0xABU, wire.aCurLow[1]);
  TEST_ASSERT_EQUAL_UINT8(0x55U, wire.aCurLow[2]);
  TEST_ASSERT_EQUAL_UINT8(0x0FU, wire.aCurLow[3]);
  TEST_ASSERT_EQUAL_UINT8(0U,    wire.bCurHighBitsPacked);
}

void test_pack_high_bits_go_to_correct_fields(void)
{
  /* Channel 0 = 0x100 → high2=1 (bits 0..1);
   * Channel 1 = 0x200 → high2=2 (bits 2..3);
   * Channel 2 = 0x300 → high2=3 (bits 4..5);
   * Channel 3 = 0x000 → high2=0 (bits 6..7)
   * Expected: 0b00_11_10_01 = 0x39
   */
  tSCurrentMeasurementSnapshot snap = MakeSnap(0x100U, 0x200U, 0x300U, 0x000U);
  tSCurrentMeasurementWire wire;

  CurrentMeasurement_Pack(&snap, &wire);

  TEST_ASSERT_EQUAL_UINT8(0x00U, wire.aCurLow[0]);
  TEST_ASSERT_EQUAL_UINT8(0x00U, wire.aCurLow[1]);
  TEST_ASSERT_EQUAL_UINT8(0x00U, wire.aCurLow[2]);
  TEST_ASSERT_EQUAL_UINT8(0x00U, wire.aCurLow[3]);
  TEST_ASSERT_EQUAL_UINT8(0x39U, wire.bCurHighBitsPacked);
}

void test_pack_ten_bit_max_preserved(void)
{
  tSCurrentMeasurementSnapshot snap = MakeSnap(0x3FFU, 0x3FFU, 0x3FFU, 0x3FFU);
  tSCurrentMeasurementWire wire;

  CurrentMeasurement_Pack(&snap, &wire);

  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.aCurLow[0]);
  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.aCurLow[1]);
  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.aCurLow[2]);
  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.aCurLow[3]);
  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.bCurHighBitsPacked);
}

void test_pack_saturates_above_ten_bits(void)
{
  /* 0xFFFF → should clamp to 0x3FF for all channels */
  tSCurrentMeasurementSnapshot snap = MakeSnap(0xFFFFU,
                                               0x1000U,
                                               0x0400U,
                                               0x0500U);
  tSCurrentMeasurementWire wire;

  CurrentMeasurement_Pack(&snap, &wire);

  for (uint8_t i = 0U; i < CURRENT_CHANNEL_COUNT; i++)
  {
    TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.aCurLow[i]);
  }

  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.bCurHighBitsPacked);
}

void test_pack_from_port_matches_get_latest_snapshot(void)
{
  tSMockCurrentMeasurementAdapterCtx mockCtx;
  ICurrentMeasurementPort_t port;
  tSCurrentMeasurementSnapshot canned = MakeSnap(150U, 280U, 512U, 1023U);
  tSCurrentMeasurementSnapshot got;
  tSCurrentMeasurementWire wire;

  MockCurrentMeasurementAdapter_Init(&mockCtx);
  port = MockCurrentMeasurementAdapter_CreatePort(&mockCtx);
  MockCurrentMeasurementAdapter_SetSnapshot(&mockCtx, &canned);

  CurrentMeasurement_GetLatest(&port, &got);
  CurrentMeasurement_Pack(&got, &wire);

  TEST_ASSERT_EQUAL_UINT32(1U, mockCtx.lGetLatestCount);
  TEST_ASSERT_EQUAL_UINT8(150U & 0xFFU, wire.aCurLow[0]);
  TEST_ASSERT_EQUAL_UINT8(280U & 0xFFU, wire.aCurLow[1]);
  TEST_ASSERT_EQUAL_UINT8(512U & 0xFFU, wire.aCurLow[2]);
  TEST_ASSERT_EQUAL_UINT8(1023U & 0xFFU, wire.aCurLow[3]);

  /* high bits: ch0=0(150), ch1=1(280>>8=1), ch2=2(512>>8=2), ch3=3(1023>>8=3)
   * packed = 0b11_10_01_00 = 0xE4 */
  TEST_ASSERT_EQUAL_UINT8(0xE4U, wire.bCurHighBitsPacked);
}

void test_port_snapshot_preserves_status_bits(void)
{
  tSMockCurrentMeasurementAdapterCtx mockCtx;
  ICurrentMeasurementPort_t port;
  tSCurrentMeasurementSnapshot canned = MakeSnap(1U, 2U, 3U, 4U);
  tSCurrentMeasurementSnapshot got;

  canned.bStatus = CURRENT_MEASUREMENT_STATUS_SATURATED;

  MockCurrentMeasurementAdapter_Init(&mockCtx);
  port = MockCurrentMeasurementAdapter_CreatePort(&mockCtx);
  MockCurrentMeasurementAdapter_SetSnapshot(&mockCtx, &canned);

  CurrentMeasurement_GetLatest(&port, &got);

  TEST_ASSERT_EQUAL_UINT8(CURRENT_MEASUREMENT_STATUS_SATURATED, got.bStatus);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_pack_zero_currents_gives_zero_bytes);
  RUN_TEST(test_pack_low_bits_preserved);
  RUN_TEST(test_pack_high_bits_go_to_correct_fields);
  RUN_TEST(test_pack_ten_bit_max_preserved);
  RUN_TEST(test_pack_saturates_above_ten_bits);
  RUN_TEST(test_pack_from_port_matches_get_latest_snapshot);
  RUN_TEST(test_port_snapshot_preserves_status_bits);

  return UNITY_END();
}
