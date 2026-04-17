/*
 * Tests/Unit/Test_UnitAlarmPort.c
 *
 * Unit tests for the application-owned unit alarm port contract.
 */
#include "unity.h"

#include "MockUnitAlarmAdapter.h"

static MockUnitAlarmAdapterCtx_t s_ctx;
static IUnitAlarmPort_t s_port;

void setUp(void)
{
  MockUnitAlarmAdapterInit(&s_ctx);
  s_port = MockUnitAlarmAdapterCreatePort(&s_ctx);
}

void tearDown(void)
{
}

void test_unit_alarm_port_defaults_to_one_alarm_group_and_zero_status(void)
{
  uint8_t value = 0xFFU;

  TEST_ASSERT_TRUE(UnitAlarmPortGetMaxAlarmGroups(&s_port, &value));
  TEST_ASSERT_EQUAL_UINT8(1U, value);

  TEST_ASSERT_TRUE(UnitAlarmPortGetAlarmGroupState(&s_port, 0U, &value));
  TEST_ASSERT_EQUAL_UINT8(0U, value);

  TEST_ASSERT_TRUE(UnitAlarmPortGetUnitAlarmStatus1(&s_port, &value));
  TEST_ASSERT_EQUAL_UINT8(0U, value);

  TEST_ASSERT_TRUE(UnitAlarmPortGetUnitAlarmStatus2(&s_port, &value));
  TEST_ASSERT_EQUAL_UINT8(0U, value);

  TEST_ASSERT_TRUE(UnitAlarmPortGetUnitAlarmStatus3(&s_port, &value));
  TEST_ASSERT_EQUAL_UINT8(0U, value);

  TEST_ASSERT_TRUE(UnitAlarmPortGetUnitAlarmStatus4(&s_port, &value));
  TEST_ASSERT_EQUAL_UINT8(0U, value);
}

void test_unit_alarm_port_roundtrips_group_state_and_alarm_status(void)
{
  uint8_t value = 0U;

  TEST_ASSERT_TRUE(UnitAlarmPortSetMaxAlarmGroups(&s_port, 1U));
  TEST_ASSERT_TRUE(UnitAlarmPortSetAlarmGroupState(&s_port, 0U, 0x5AU));
  TEST_ASSERT_TRUE(UnitAlarmPortSetUnitAlarmStatus1(&s_port, 0x84U));
  TEST_ASSERT_TRUE(UnitAlarmPortSetUnitAlarmStatus2(&s_port, 0x83U));
  TEST_ASSERT_TRUE(UnitAlarmPortSetUnitAlarmStatus3(&s_port, 0x03U));
  TEST_ASSERT_TRUE(UnitAlarmPortSetUnitAlarmStatus4(&s_port, 0x21U));

  TEST_ASSERT_TRUE(UnitAlarmPortGetAlarmGroupState(&s_port, 0U, &value));
  TEST_ASSERT_EQUAL_UINT8(0x5AU, value);

  TEST_ASSERT_TRUE(UnitAlarmPortGetUnitAlarmStatus1(&s_port, &value));
  TEST_ASSERT_EQUAL_UINT8(0x84U, value);

  TEST_ASSERT_TRUE(UnitAlarmPortGetUnitAlarmStatus2(&s_port, &value));
  TEST_ASSERT_EQUAL_UINT8(0x83U, value);

  TEST_ASSERT_TRUE(UnitAlarmPortGetUnitAlarmStatus3(&s_port, &value));
  TEST_ASSERT_EQUAL_UINT8(0x03U, value);

  TEST_ASSERT_TRUE(UnitAlarmPortGetUnitAlarmStatus4(&s_port, &value));
  TEST_ASSERT_EQUAL_UINT8(0x21U, value);
}

void test_unit_alarm_status2_acknowledge_clears_only_power_restart_bit(void)
{
  uint8_t value = 0U;

  TEST_ASSERT_TRUE(UnitAlarmPortSetUnitAlarmStatus2(&s_port, 0x83U));

  TEST_ASSERT_TRUE(UnitAlarmPortGetUnitAlarmStatus2(&s_port, &value));
  TEST_ASSERT_EQUAL_UINT8(0x83U, value);

  UnitAlarmPortAcknowledgeUnitAlarmStatus2Read(&s_port);

  TEST_ASSERT_TRUE(UnitAlarmPortGetUnitAlarmStatus2(&s_port, &value));
  TEST_ASSERT_EQUAL_UINT8(0x82U, value);
}

void test_unit_alarm_port_rejects_zero_group_count_and_out_of_range_group(void)
{
  uint8_t value = 0U;

  TEST_ASSERT_FALSE(UnitAlarmPortSetMaxAlarmGroups(&s_port, 0U));
  TEST_ASSERT_FALSE(UnitAlarmPortSetAlarmGroupState(&s_port, 1U, 0x01U));
  TEST_ASSERT_FALSE(UnitAlarmPortGetAlarmGroupState(&s_port, 1U, &value));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_unit_alarm_port_defaults_to_one_alarm_group_and_zero_status);
  RUN_TEST(test_unit_alarm_port_roundtrips_group_state_and_alarm_status);
  RUN_TEST(test_unit_alarm_status2_acknowledge_clears_only_power_restart_bit);
  RUN_TEST(
    test_unit_alarm_port_rejects_zero_group_count_and_out_of_range_group);

  return UNITY_END();
}
