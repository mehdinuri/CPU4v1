/*
 * Tests/Unit/Test_SignalCardIdentity.c
 *
 * Validates the accepted SSM address range and its command-bit mapping.
 */
#include "unity.h"
#include "Domain/SignalCardIdentity.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_card_ids_zero_through_seven_are_valid(void)
{
  for (uint8_t i = 0U; i < SIGNAL_CARD_ID_VALID_COUNT; i++)
  {
    TEST_ASSERT_EQUAL_UINT8(1U, SignalCardIdentity_IsValid(i));
  }
}

void test_card_ids_eight_and_above_are_invalid(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, SignalCardIdentity_IsValid(8U));
  TEST_ASSERT_EQUAL_UINT8(0U, SignalCardIdentity_IsValid(15U));
}

void test_command_bank_splits_the_eight_valid_cards_into_two_halves(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, SignalCardIdentity_CommandBank(0U));
  TEST_ASSERT_EQUAL_UINT8(0U, SignalCardIdentity_CommandBank(3U));
  TEST_ASSERT_EQUAL_UINT8(1U, SignalCardIdentity_CommandBank(4U));
  TEST_ASSERT_EQUAL_UINT8(1U, SignalCardIdentity_CommandBank(7U));
}

void test_invalid_card_id_yields_invalid_bank(void)
{
  TEST_ASSERT_EQUAL_UINT8(0xFFU, SignalCardIdentity_CommandBank(9U));
}

void test_output_bit_base_maps_each_bank_locally_in_steps_of_twelve(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, SignalCardIdentity_OutputBitBase(0U));
  TEST_ASSERT_EQUAL_UINT8(12U, SignalCardIdentity_OutputBitBase(1U));
  TEST_ASSERT_EQUAL_UINT8(24U, SignalCardIdentity_OutputBitBase(2U));
  TEST_ASSERT_EQUAL_UINT8(36U, SignalCardIdentity_OutputBitBase(3U));
  TEST_ASSERT_EQUAL_UINT8(0U, SignalCardIdentity_OutputBitBase(4U));
  TEST_ASSERT_EQUAL_UINT8(12U, SignalCardIdentity_OutputBitBase(5U));
  TEST_ASSERT_EQUAL_UINT8(24U, SignalCardIdentity_OutputBitBase(6U));
  TEST_ASSERT_EQUAL_UINT8(36U, SignalCardIdentity_OutputBitBase(7U));
}

void test_invalid_card_id_yields_invalid_output_bit_base(void)
{
  TEST_ASSERT_EQUAL_UINT8(0xFFU, SignalCardIdentity_OutputBitBase(12U));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_card_ids_zero_through_seven_are_valid);
  RUN_TEST(test_card_ids_eight_and_above_are_invalid);
  RUN_TEST(test_command_bank_splits_the_eight_valid_cards_into_two_halves);
  RUN_TEST(test_invalid_card_id_yields_invalid_bank);
  RUN_TEST(test_output_bit_base_maps_each_bank_locally_in_steps_of_twelve);
  RUN_TEST(test_invalid_card_id_yields_invalid_output_bit_base);

  return UNITY_END();
}
