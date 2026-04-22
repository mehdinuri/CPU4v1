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

static CurrentMeasurementSnapshot_t MakeSnap(uint16_t a,
                                             uint16_t b,
                                             uint16_t c,
                                             uint16_t d)
{
  CurrentMeasurementSnapshot_t s;

  s.currentsMa[0] = a;
  s.currentsMa[1] = b;
  s.currentsMa[2] = c;
  s.currentsMa[3] = d;
  s.status = 0U;
  s.seqNo = 0U;

  return s;
}

void test_pack_zero_currents_gives_zero_bytes(void)
{
  CurrentMeasurementSnapshot_t snap = MakeSnap(0U, 0U, 0U, 0U);
  CurrentMeasurementWire_t wire;

  CurrentMeasurement_Pack(&snap, &wire);

  TEST_ASSERT_EQUAL_UINT8(0U, wire.curLow[0]);
  TEST_ASSERT_EQUAL_UINT8(0U, wire.curLow[1]);
  TEST_ASSERT_EQUAL_UINT8(0U, wire.curLow[2]);
  TEST_ASSERT_EQUAL_UINT8(0U, wire.curLow[3]);
  TEST_ASSERT_EQUAL_UINT8(0U, wire.curHighBitsPacked);
}

void test_pack_low_bits_preserved(void)
{
  CurrentMeasurementSnapshot_t snap = MakeSnap(0x00FFU,
                                               0x00ABU,
                                               0x0055U,
                                               0x000FU);
  CurrentMeasurementWire_t wire;

  CurrentMeasurement_Pack(&snap, &wire);

  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.curLow[0]);
  TEST_ASSERT_EQUAL_UINT8(0xABU, wire.curLow[1]);
  TEST_ASSERT_EQUAL_UINT8(0x55U, wire.curLow[2]);
  TEST_ASSERT_EQUAL_UINT8(0x0FU, wire.curLow[3]);
  TEST_ASSERT_EQUAL_UINT8(0U,    wire.curHighBitsPacked);
}

void test_pack_high_bits_go_to_correct_fields(void)
{
  /* Channel 0 = 0x100 → high2=1 (bits 0..1);
   * Channel 1 = 0x200 → high2=2 (bits 2..3);
   * Channel 2 = 0x300 → high2=3 (bits 4..5);
   * Channel 3 = 0x000 → high2=0 (bits 6..7)
   * Expected: 0b00_11_10_01 = 0x39
   */
  CurrentMeasurementSnapshot_t snap = MakeSnap(0x100U, 0x200U, 0x300U, 0x000U);
  CurrentMeasurementWire_t wire;

  CurrentMeasurement_Pack(&snap, &wire);

  TEST_ASSERT_EQUAL_UINT8(0x00U, wire.curLow[0]);
  TEST_ASSERT_EQUAL_UINT8(0x00U, wire.curLow[1]);
  TEST_ASSERT_EQUAL_UINT8(0x00U, wire.curLow[2]);
  TEST_ASSERT_EQUAL_UINT8(0x00U, wire.curLow[3]);
  TEST_ASSERT_EQUAL_UINT8(0x39U, wire.curHighBitsPacked);
}

void test_pack_ten_bit_max_preserved(void)
{
  CurrentMeasurementSnapshot_t snap = MakeSnap(0x3FFU, 0x3FFU, 0x3FFU, 0x3FFU);
  CurrentMeasurementWire_t wire;

  CurrentMeasurement_Pack(&snap, &wire);

  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.curLow[0]);
  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.curLow[1]);
  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.curLow[2]);
  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.curLow[3]);
  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.curHighBitsPacked);
}

void test_pack_saturates_above_ten_bits(void)
{
  /* 0xFFFF → should clamp to 0x3FF for all channels */
  CurrentMeasurementSnapshot_t snap = MakeSnap(0xFFFFU,
                                               0x1000U,
                                               0x0400U,
                                               0x0500U);
  CurrentMeasurementWire_t wire;

  CurrentMeasurement_Pack(&snap, &wire);

  for (uint8_t i = 0U; i < CURRENT_CHANNEL_COUNT; i++)
  {
    TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.curLow[i]);
  }

  TEST_ASSERT_EQUAL_UINT8(0xFFU, wire.curHighBitsPacked);
}

void test_pack_from_port_matches_get_latest_snapshot(void)
{
  MockCurrentMeasurementAdapterCtx_t mockCtx;
  ICurrentMeasurementPort_t port;
  CurrentMeasurementSnapshot_t canned = MakeSnap(150U, 280U, 512U, 1023U);
  CurrentMeasurementSnapshot_t got;
  CurrentMeasurementWire_t wire;

  MockCurrentMeasurementAdapter_Init(&mockCtx);
  port = MockCurrentMeasurementAdapter_CreatePort(&mockCtx);
  MockCurrentMeasurementAdapter_SetSnapshot(&mockCtx, &canned);

  CurrentMeasurement_GetLatest(&port, &got);
  CurrentMeasurement_Pack(&got, &wire);

  TEST_ASSERT_EQUAL_UINT32(1U, mockCtx.getLatestCount);
  TEST_ASSERT_EQUAL_UINT8(150U & 0xFFU, wire.curLow[0]);
  TEST_ASSERT_EQUAL_UINT8(280U & 0xFFU, wire.curLow[1]);
  TEST_ASSERT_EQUAL_UINT8(512U & 0xFFU, wire.curLow[2]);
  TEST_ASSERT_EQUAL_UINT8(1023U & 0xFFU, wire.curLow[3]);

  /* high bits: ch0=0(150), ch1=1(280>>8=1), ch2=2(512>>8=2), ch3=3(1023>>8=3)
   * packed = 0b11_10_01_00 = 0xE4 */
  TEST_ASSERT_EQUAL_UINT8(0xE4U, wire.curHighBitsPacked);
}

void test_port_snapshot_preserves_status_bits(void)
{
  MockCurrentMeasurementAdapterCtx_t mockCtx;
  ICurrentMeasurementPort_t port;
  CurrentMeasurementSnapshot_t canned = MakeSnap(1U, 2U, 3U, 4U);
  CurrentMeasurementSnapshot_t got;

  canned.status = CURRENT_MEASUREMENT_STATUS_SATURATED;

  MockCurrentMeasurementAdapter_Init(&mockCtx);
  port = MockCurrentMeasurementAdapter_CreatePort(&mockCtx);
  MockCurrentMeasurementAdapter_SetSnapshot(&mockCtx, &canned);

  CurrentMeasurement_GetLatest(&port, &got);

  TEST_ASSERT_EQUAL_UINT8(CURRENT_MEASUREMENT_STATUS_SATURATED, got.status);
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
