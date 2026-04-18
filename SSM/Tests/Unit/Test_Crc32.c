/*
 * Tests/Unit/Test_Crc32.c
 *
 * Known-vector tests for Domain/Crc32. Values sourced from the CRC-32/ISO-HDLC
 * entry in reveng.sourceforge.io/crc-catalogue/all.htm.
 */
#include <string.h>
#include "unity.h"
#include "Domain/Crc32.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_empty_input_is_zero(void)
{
  TEST_ASSERT_EQUAL_UINT32(0x00000000U, Crc32_Compute("", 0U));
}

void test_single_zero_byte(void)
{
  uint8_t data = 0x00U;

  TEST_ASSERT_EQUAL_UINT32(0xD202EF8DU, Crc32_Compute(&data, 1U));
}

void test_single_letter_a(void)
{
  TEST_ASSERT_EQUAL_UINT32(0xE8B7BE43U, Crc32_Compute("a", 1U));
}

void test_canonical_string(void)
{
  /* The standard "123456789" test vector. */
  TEST_ASSERT_EQUAL_UINT32(0xCBF43926U, Crc32_Compute("123456789", 9U));
}

void test_erased_flash_pattern(void)
{
  /* A freshly-erased flash page is all-0xFF. The CRC of such a buffer
   * should never accidentally match a valid record's CRC — sanity check
   * that our algorithm produces something distinctive for this case.
   */
  uint8_t erased[8];

  memset(erased, 0xFFU, sizeof(erased));
  TEST_ASSERT_NOT_EQUAL(0x00000000U, Crc32_Compute(erased, sizeof(erased)));
}

void test_single_bit_flip_changes_crc(void)
{
  uint8_t a[4] = { 0x01U, 0x02U, 0x03U, 0x04U };
  uint8_t b[4] = { 0x01U, 0x02U, 0x03U, 0x05U };

  uint32_t lCrcA = Crc32_Compute(a, 4U);
  uint32_t lCrcB = Crc32_Compute(b, 4U);

  TEST_ASSERT_NOT_EQUAL(lCrcA, lCrcB);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_empty_input_is_zero);
  RUN_TEST(test_single_zero_byte);
  RUN_TEST(test_single_letter_a);
  RUN_TEST(test_canonical_string);
  RUN_TEST(test_erased_flash_pattern);
  RUN_TEST(test_single_bit_flip_changes_crc);

  return UNITY_END();
}
