#include "unity.h"
#include "common.h"

void setUp(void)
{
}

void tearDown(void)
{
}

/* mMsb / mLsb — byte extraction from a 16-bit word */
void test_mMsb_extracts_high_byte(void)
{
  TEST_ASSERT_EQUAL_UINT8(0xAB, mMsb(0xABCDU));
}

void test_mLsb_extracts_low_byte(void)
{
  TEST_ASSERT_EQUAL_UINT8(0xCD, mLsb(0xABCDU));
}

/* mMsw / mLsw — word extraction from a 32-bit value */
void test_mMsw_extracts_high_word(void)
{
  TEST_ASSERT_EQUAL_UINT16(0x1234U, mMsw(0x12345678UL));
}

void test_mLsw_extracts_low_word(void)
{
  TEST_ASSERT_EQUAL_UINT16(0x5678U, mLsw(0x12345678UL));
}

/* GetBitValue — read a single bit */
void test_GetBitValue_bit0_set(void)
{
  TEST_ASSERT_EQUAL_UINT8(1U, GetBitValue(0x01U, 0));
}

void test_GetBitValue_bit0_clear(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, GetBitValue(0x02U, 0));
}

void test_GetBitValue_bit3_set(void)
{
  TEST_ASSERT_EQUAL_UINT8(1U, GetBitValue(0x08U, 3));
}

/* SetBitValue / ClearBitValue — modify a single bit */
void test_SetBitValue_sets_bit2(void)
{
  unsigned int data = 0x00U;

  SetBitValue(data, 2);
  TEST_ASSERT_EQUAL_UINT(0x04U, data);
}

void test_ClearBitValue_clears_bit2(void)
{
  unsigned int data = 0xFFU;

  ClearBitValue(data, 2);
  TEST_ASSERT_EQUAL_UINT(0xFBU, data);
}

/* mHex2BCD / mBCD2Hex — decimal ↔ packed BCD */
void test_mHex2BCD_converts_99(void)
{
  TEST_ASSERT_EQUAL_UINT8(0x99U, mHex2BCD(99U));
}

void test_mHex2BCD_converts_0(void)
{
  TEST_ASSERT_EQUAL_UINT8(0x00U, mHex2BCD(0U));
}

void test_mBCD2Hex_converts_0x99(void)
{
  TEST_ASSERT_EQUAL_UINT8(99U, mBCD2Hex(0x99U));
}

void test_mBCD2Hex_converts_0x00(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, mBCD2Hex(0x00U));
}

/* mHighNibble / mLowNibble */
void test_mHighNibble_extracts_upper_4_bits(void)
{
  TEST_ASSERT_EQUAL_UINT8(0x0AU, mHighNibble(0xABU));
}

void test_mLowNibble_extracts_lower_4_bits(void)
{
  TEST_ASSERT_EQUAL_UINT8(0x0BU, mLowNibble(0xABU));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_mMsb_extracts_high_byte);
  RUN_TEST(test_mLsb_extracts_low_byte);
  RUN_TEST(test_mMsw_extracts_high_word);
  RUN_TEST(test_mLsw_extracts_low_word);
  RUN_TEST(test_GetBitValue_bit0_set);
  RUN_TEST(test_GetBitValue_bit0_clear);
  RUN_TEST(test_GetBitValue_bit3_set);
  RUN_TEST(test_SetBitValue_sets_bit2);
  RUN_TEST(test_ClearBitValue_clears_bit2);
  RUN_TEST(test_mHex2BCD_converts_99);
  RUN_TEST(test_mHex2BCD_converts_0);
  RUN_TEST(test_mBCD2Hex_converts_0x99);
  RUN_TEST(test_mBCD2Hex_converts_0x00);
  RUN_TEST(test_mHighNibble_extracts_upper_4_bits);
  RUN_TEST(test_mLowNibble_extracts_lower_4_bits);

  return UNITY_END();
}
