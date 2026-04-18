/*
 * Tests/Unit/Test_SignalOutput.c
 *
 * Unit tests for App/Domain/SignalOutput.c
 * Covers the on/off-counter majority predicate.
 */
#include "unity.h"
#include "Domain/SignalOutput.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_is_active_on_greater_than_off(void)
{
  TEST_ASSERT_EQUAL_UINT8(1U, SignalOutput_IsActive(5U, 3U));
}

void test_is_active_off_greater_than_on(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, SignalOutput_IsActive(3U, 5U));
}

void test_is_active_equal_counts(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, SignalOutput_IsActive(4U, 4U));
}

void test_is_active_both_zero(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, SignalOutput_IsActive(0U, 0U));
}

void test_is_active_on_only(void)
{
  TEST_ASSERT_EQUAL_UINT8(1U, SignalOutput_IsActive(1U, 0U));
}

void test_is_active_off_only(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, SignalOutput_IsActive(0U, 1U));
}

void test_is_active_max_on(void)
{
  TEST_ASSERT_EQUAL_UINT8(1U, SignalOutput_IsActive(255U, 0U));
}

void test_is_active_max_off(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, SignalOutput_IsActive(0U, 255U));
}

void test_is_active_max_equal(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, SignalOutput_IsActive(255U, 255U));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_is_active_on_greater_than_off);
  RUN_TEST(test_is_active_off_greater_than_on);
  RUN_TEST(test_is_active_equal_counts);
  RUN_TEST(test_is_active_both_zero);
  RUN_TEST(test_is_active_on_only);
  RUN_TEST(test_is_active_off_only);
  RUN_TEST(test_is_active_max_on);
  RUN_TEST(test_is_active_max_off);
  RUN_TEST(test_is_active_max_equal);

  return UNITY_END();
}
