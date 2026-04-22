/*
 * Tests/Unit/Test_RTCPort.c
 *
 * Unit tests for the HAL-free RTC port contract using the mock adapter.
 */
#include "unity.h"

#include "MockRTCAdapter.h"

static MockRTCAdapterCtx_t s_ctx;
static IRealtimeClockPort_t s_port;

void setUp(void)
{
  MockRTCAdapterInit(&s_ctx);
  s_port = MockRTCAdapterCreatePort(&s_ctx);
}

void tearDown(void)
{
}

void test_rtc_port_snapshot_round_trip(void)
{
  RtcSnapshot_t written = { 21U, 24U, 4U, 8U, 2U, 13U, 14U, 15U, 678U };
  RtcSnapshot_t read = { 0 };

  TEST_ASSERT_TRUE(RealtimeClockWriteSnapshot(&s_port, &written));
  TEST_ASSERT_TRUE(RealtimeClockReadSnapshot(&s_port, &read));

  TEST_ASSERT_EQUAL_UINT8(written.Century, read.Century);
  TEST_ASSERT_EQUAL_UINT8(written.Year, read.Year);
  TEST_ASSERT_EQUAL_UINT8(written.Month, read.Month);
  TEST_ASSERT_EQUAL_UINT8(written.Date, read.Date);
  TEST_ASSERT_EQUAL_UINT8(written.WeekDay, read.WeekDay);
  TEST_ASSERT_EQUAL_UINT8(written.Hours, read.Hours);
  TEST_ASSERT_EQUAL_UINT8(written.Minutes, read.Minutes);
  TEST_ASSERT_EQUAL_UINT8(written.Seconds, read.Seconds);
  TEST_ASSERT_EQUAL_UINT16(written.Milliseconds, read.Milliseconds);
}

void test_rtc_port_metadata_round_trip(void)
{
  uint32_t value = 0U;

  TEST_ASSERT_TRUE(
    RealtimeClockWriteMetadata(&s_port, RTC_META_INIT_SIGNATURE, 0x2014U));
  TEST_ASSERT_TRUE(
    RealtimeClockWriteMetadata(&s_port, RTC_META_CENTURY, 21U));

  TEST_ASSERT_TRUE(
    RealtimeClockReadMetadata(&s_port, RTC_META_INIT_SIGNATURE, &value));
  TEST_ASSERT_EQUAL_HEX32(0x2014U, value);

  TEST_ASSERT_TRUE(RealtimeClockReadMetadata(&s_port, RTC_META_CENTURY,
                                             &value));
  TEST_ASSERT_EQUAL_UINT32(21U, value);
}

void test_rtc_port_dst_adjustment_updates_hour(void)
{
  RtcSnapshot_t snapshot = { 21U, 24U, 4U, 8U, 2U, 10U, 0U, 0U };

  TEST_ASSERT_TRUE(RealtimeClockWriteSnapshot(&s_port, &snapshot));
  TEST_ASSERT_TRUE(
    RealtimeClockApplyDstAdjustment(&s_port, RTC_DST_ADD_ONE_HOUR));
  TEST_ASSERT_EQUAL_UINT8(11U, s_ctx.snapshot.Hours);

  TEST_ASSERT_TRUE(
    RealtimeClockApplyDstAdjustment(&s_port, RTC_DST_SUBTRACT_ONE_HOUR));
  TEST_ASSERT_EQUAL_UINT8(10U, s_ctx.snapshot.Hours);
  TEST_ASSERT_EQUAL_UINT32(2U, s_ctx.adjustmentCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_rtc_port_snapshot_round_trip);
  RUN_TEST(test_rtc_port_metadata_round_trip);
  RUN_TEST(test_rtc_port_dst_adjustment_updates_hour);

  return UNITY_END();
}
