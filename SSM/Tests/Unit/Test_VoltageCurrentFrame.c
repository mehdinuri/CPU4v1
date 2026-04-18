/*
 * Tests/Unit/Test_VoltageCurrentFrame.c
 *
 * Pins the exact byte-layout of the outgoing CAN telemetry frame. These
 * tests are the contract between SSM and CP/PSM/MP receivers — changes
 * that break them also break the legacy wire format.
 */
#include <string.h>
#include "unity.h"
#include "Domain/VoltageCurrentFrame.h"

static tSVoltageCurrentFrameInputs in;
static uint8_t bytes[VOLTAGE_CURRENT_FRAME_BYTES];

void setUp(void)
{
  memset(&in, 0, sizeof(in));
  memset(bytes, 0xABU, sizeof(bytes));      /* poison — must be overwritten */
}

void tearDown(void)
{
}

void test_all_zeros_produces_all_zero_bytes(void)
{
  VoltageCurrentFrame_Encode(&in, bytes);

  for (uint8_t i = 0U; i < VOLTAGE_CURRENT_FRAME_BYTES; i++)
  {
    TEST_ASSERT_EQUAL_UINT8(0U, bytes[i]);
  }
}

void test_channel_0_voltage_sets_bit_0_of_byte_0(void)
{
  in.SVoltageImage.aChannels[0] = 1U;
  VoltageCurrentFrame_Encode(&in, bytes);

  TEST_ASSERT_EQUAL_UINT8(0x01U, bytes[0]);
  TEST_ASSERT_EQUAL_UINT8(0x00U, bytes[1]);
}

void test_channel_7_voltage_sets_bit_7_of_byte_0(void)
{
  in.SVoltageImage.aChannels[7] = 1U;
  VoltageCurrentFrame_Encode(&in, bytes);

  TEST_ASSERT_EQUAL_UINT8(0x80U, bytes[0]);
  TEST_ASSERT_EQUAL_UINT8(0x00U, bytes[1]);
}

void test_channel_8_voltage_sets_bit_0_of_byte_1(void)
{
  in.SVoltageImage.aChannels[8] = 1U;
  VoltageCurrentFrame_Encode(&in, bytes);

  TEST_ASSERT_EQUAL_UINT8(0x00U, bytes[0]);
  TEST_ASSERT_EQUAL_UINT8(0x01U, bytes[1]);
}

void test_channel_11_voltage_sets_bit_3_of_byte_1(void)
{
  in.SVoltageImage.aChannels[11] = 1U;
  VoltageCurrentFrame_Encode(&in, bytes);

  TEST_ASSERT_EQUAL_UINT8(0x00U, bytes[0]);
  TEST_ASSERT_EQUAL_UINT8(0x08U, bytes[1]);
  /* Bits 4..7 of byte 1 are always zero (unused channels). */
  TEST_ASSERT_EQUAL_UINT8(0x00U, bytes[1] & 0xF0U);
}

void test_all_12_voltage_channels_high(void)
{
  for (uint8_t i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    in.SVoltageImage.aChannels[i] = 1U;
  }

  VoltageCurrentFrame_Encode(&in, bytes);

  TEST_ASSERT_EQUAL_UINT8(0xFFU, bytes[0]);
  TEST_ASSERT_EQUAL_UINT8(0x0FU, bytes[1]);
}

void test_nonzero_voltage_values_normalise_to_single_bit(void)
{
  /* aChannels stores 1 by convention but any nonzero value should
   * still just set the bit. */
  in.SVoltageImage.aChannels[2] = 0xFFU;
  VoltageCurrentFrame_Encode(&in, bytes);

  TEST_ASSERT_EQUAL_UINT8(0x04U, bytes[0]);
}

void test_current_low_bytes_pass_through(void)
{
  in.SCurrentWire.aCurLow[0] = 0x11U;
  in.SCurrentWire.aCurLow[1] = 0x22U;
  in.SCurrentWire.aCurLow[2] = 0x33U;
  in.SCurrentWire.aCurLow[3] = 0x44U;
  VoltageCurrentFrame_Encode(&in, bytes);

  TEST_ASSERT_EQUAL_UINT8(0x11U, bytes[2]);
  TEST_ASSERT_EQUAL_UINT8(0x22U, bytes[3]);
  TEST_ASSERT_EQUAL_UINT8(0x33U, bytes[4]);
  TEST_ASSERT_EQUAL_UINT8(0x44U, bytes[5]);
}

void test_current_high_bits_pass_through(void)
{
  in.SCurrentWire.bCurHighBitsPacked = 0xE4U;    /* 0b11100100 */
  VoltageCurrentFrame_Encode(&in, bytes);

  TEST_ASSERT_EQUAL_UINT8(0xE4U, bytes[6]);
}

void test_byte_7_status_passthrough(void)
{
  in.bStatus = 0xA5U;
  VoltageCurrentFrame_Encode(&in, bytes);

  TEST_ASSERT_EQUAL_UINT8(0xA5U, bytes[7]);
}

void test_voltage_independent_of_current_fields(void)
{
  /* Setting current fields should not touch voltage bytes. */
  in.SCurrentWire.aCurLow[0] = 0xFFU;
  in.SCurrentWire.aCurLow[1] = 0xFFU;
  in.SCurrentWire.aCurLow[2] = 0xFFU;
  in.SCurrentWire.aCurLow[3] = 0xFFU;
  in.SCurrentWire.bCurHighBitsPacked = 0xFFU;

  VoltageCurrentFrame_Encode(&in, bytes);

  TEST_ASSERT_EQUAL_UINT8(0x00U, bytes[0]);
  TEST_ASSERT_EQUAL_UINT8(0x00U, bytes[1]);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_all_zeros_produces_all_zero_bytes);
  RUN_TEST(test_channel_0_voltage_sets_bit_0_of_byte_0);
  RUN_TEST(test_channel_7_voltage_sets_bit_7_of_byte_0);
  RUN_TEST(test_channel_8_voltage_sets_bit_0_of_byte_1);
  RUN_TEST(test_channel_11_voltage_sets_bit_3_of_byte_1);
  RUN_TEST(test_all_12_voltage_channels_high);
  RUN_TEST(test_nonzero_voltage_values_normalise_to_single_bit);
  RUN_TEST(test_current_low_bytes_pass_through);
  RUN_TEST(test_current_high_bits_pass_through);
  RUN_TEST(test_byte_7_status_passthrough);
  RUN_TEST(test_voltage_independent_of_current_fields);

  return UNITY_END();
}
