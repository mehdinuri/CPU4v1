#include "unity.h"

#include <string.h>

#include "Domain/Services/MmiEventLogService.h"
#include "Adapters/Mock/MockLogRepositoryAdapter.h"

typedef struct
{
  uint8_t bSeconds;
  uint8_t bMinutes;
  uint8_t bHours;
  uint8_t bMonthDay;
  uint8_t bMonth;
  uint16_t sYear;

  struct
  {
    uint8_t bEvent;
    uint8_t bParam;
    uint16_t sParam;
    uint32_t lParam;
  } SEvent;
} __attribute__((packed)) TestMmiLogStorageRecord_t;

static MockLogRepositoryAdapterCtx_t s_logCtx;
static ILogRepositoryPort_t s_logPort;
static MmiEventLogService_t s_service;

void setUp(void)
{
  MockLogRepositoryAdapterInit(&s_logCtx);
  s_logPort = MockLogRepositoryAdapterCreatePort(&s_logCtx);
  MmiEventLogServiceInit(&s_service);
  MmiEventLogServiceBind(&s_service, &s_logPort);
}

void tearDown(void)
{
}

static void AppendRecord(uint8_t seconds,
                         uint8_t minutes,
                         uint8_t hours,
                         uint8_t day,
                         uint8_t month,
                         uint16_t year,
                         uint8_t eventCode,
                         uint8_t eventParam,
                         uint16_t eventShortParam,
                         uint32_t eventLongParam)
{
  TestMmiLogStorageRecord_t record;

  (void) memset(&record, 0, sizeof(record));
  record.bSeconds = seconds;
  record.bMinutes = minutes;
  record.bHours = hours;
  record.bMonthDay = day;
  record.bMonth = month;
  record.sYear = year;
  record.SEvent.bEvent = eventCode;
  record.SEvent.bParam = eventParam;
  record.SEvent.sParam = eventShortParam;
  record.SEvent.lParam = eventLongParam;

  TEST_ASSERT_TRUE(LogRepositoryAppend(&s_logPort,
                                       &record,
                                       sizeof(record),
                                       NULL));
}

void test_cursor_returns_none_when_no_logs_exist(void)
{
  uint8_t payload[8];
  uint16_t payloadLength = 0U;
  uint16_t cursor = 0U;

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiEventLogServiceRead(&s_service,
                                                 MMI_PROTOCOL_V2_EVENT_RESOURCE_CURSOR,
                                                 NULL,
                                                 0U,
                                                 &payload[0],
                                                 sizeof(payload),
                                                 &payloadLength));
  TEST_ASSERT_EQUAL_UINT16(sizeof(cursor), payloadLength);
  (void) memcpy(&cursor, &payload[0], sizeof(cursor));
  TEST_ASSERT_EQUAL_UINT16(MMI_PROTOCOL_V2_EVENT_CURSOR_NONE, cursor);
}

void test_latest_index_and_read_record_helpers_follow_repository_state(void)
{
  uint16_t latestIndex = 0U;
  MmiEventRecordV2_t record;

  AppendRecord(1U, 2U, 3U, 4U, 5U, 2026U, 10U, 11U, 12U, 13UL);
  AppendRecord(6U, 7U, 8U, 9U, 10U, 2027U, 20U, 21U, 22U, 23UL);

  TEST_ASSERT_TRUE(MmiEventLogServiceGetLatestIndex(&s_service, &latestIndex));
  TEST_ASSERT_EQUAL_UINT16(1U, latestIndex);

  TEST_ASSERT_TRUE(MmiEventLogServiceCanReadFromIndex(&s_service, 0U));
  TEST_ASSERT_FALSE(MmiEventLogServiceCanReadFromIndex(&s_service, 2U));
  TEST_ASSERT_TRUE(MmiEventLogServiceReadRecord(&s_service, 0U, &record));
  TEST_ASSERT_EQUAL_UINT16(0U, record.logIndex);
  TEST_ASSERT_EQUAL_UINT8(10U, record.eventCode);
  TEST_ASSERT_EQUAL_UINT32(13UL, record.eventLongParam);
}

void test_find_latest_by_event_code_returns_newest_matching_record(void)
{
  uint16_t latestDoorIndex = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;

  AppendRecord(1U, 2U, 3U, 4U, 5U, 2026U, 64U, 0U, 0U, 0UL);
  AppendRecord(6U, 7U, 8U, 9U, 10U, 2027U, 65U, 0U, 0U, 0UL);
  AppendRecord(11U, 12U, 13U, 14U, 15U, 2028U, 64U, 0U, 0U, 0UL);

  TEST_ASSERT_TRUE(MmiEventLogServiceFindLatestByEventCode(&s_service,
                                                           64U,
                                                           &latestDoorIndex));
  TEST_ASSERT_EQUAL_UINT16(2U, latestDoorIndex);
  TEST_ASSERT_TRUE(MmiEventLogServiceFindLatestByEventCode(&s_service,
                                                           99U,
                                                           &latestDoorIndex));
  TEST_ASSERT_EQUAL_UINT16(MMI_PROTOCOL_V2_EVENT_CURSOR_NONE, latestDoorIndex);
}

void test_page_returns_requested_records_and_more_available_flag(void)
{
  MmiEventPageRequestV2_t request = { 0U, 2U, 0U };
  uint8_t payload[128];
  uint16_t payloadLength = 0U;
  MmiEventPageHeaderV2_t header;
  MmiEventRecordV2_t record0;
  MmiEventRecordV2_t record1;

  AppendRecord(1U, 2U, 3U, 4U, 5U, 2026U, 10U, 11U, 12U, 13UL);
  AppendRecord(6U, 7U, 8U, 9U, 10U, 2027U, 20U, 21U, 22U, 23UL);
  AppendRecord(11U, 12U, 13U, 14U, 15U, 2028U, 30U, 31U, 32U, 33UL);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiEventLogServiceRead(&s_service,
                                                 MMI_PROTOCOL_V2_EVENT_RESOURCE_PAGE,
                                                 (const uint8_t *) &request,
                                                 sizeof(request),
                                                 &payload[0],
                                                 sizeof(payload),
                                                 &payloadLength));
  TEST_ASSERT_TRUE(payloadLength >= (sizeof(header)
                                     + sizeof(record0)
                                     + sizeof(record1)));

  (void) memcpy(&header, &payload[0], sizeof(header));
  TEST_ASSERT_EQUAL_UINT16(0U, header.startIndex);
  TEST_ASSERT_EQUAL_UINT8(2U, header.count);
  TEST_ASSERT_EQUAL_UINT8(1U, header.moreAvailable);

  (void) memcpy(&record0, &payload[sizeof(header)], sizeof(record0));
  (void) memcpy(&record1, &payload[sizeof(header) + sizeof(record0)],
                sizeof(record1));

  TEST_ASSERT_EQUAL_UINT16(0U, record0.logIndex);
  TEST_ASSERT_EQUAL_UINT8(1U, record0.second);
  TEST_ASSERT_EQUAL_UINT8(2U, record0.minute);
  TEST_ASSERT_EQUAL_UINT8(3U, record0.hour);
  TEST_ASSERT_EQUAL_UINT16(2026U, record0.year);
  TEST_ASSERT_EQUAL_UINT8(10U, record0.eventCode);
  TEST_ASSERT_EQUAL_UINT32(13UL, record0.eventLongParam);

  TEST_ASSERT_EQUAL_UINT16(1U, record1.logIndex);
  TEST_ASSERT_EQUAL_UINT8(6U, record1.second);
  TEST_ASSERT_EQUAL_UINT8(7U, record1.minute);
  TEST_ASSERT_EQUAL_UINT8(8U, record1.hour);
  TEST_ASSERT_EQUAL_UINT16(2027U, record1.year);
  TEST_ASSERT_EQUAL_UINT8(20U, record1.eventCode);
  TEST_ASSERT_EQUAL_UINT32(23UL, record1.eventLongParam);
}

void test_page_rejects_bad_index_and_zero_max_count(void)
{
  MmiEventPageRequestV2_t zeroCountRequest = { 0U, 0U, 0U };
  MmiEventPageRequestV2_t badIndexRequest = { 1U, 1U, 0U };
  uint8_t payload[64];
  uint16_t payloadLength = 0U;

  AppendRecord(1U, 2U, 3U, 4U, 5U, 2026U, 10U, 11U, 12U, 13UL);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_INVALID_VALUE,
                          MmiEventLogServiceRead(&s_service,
                                                 MMI_PROTOCOL_V2_EVENT_RESOURCE_PAGE,
                                                 (const uint8_t *) &zeroCountRequest,
                                                 sizeof(zeroCountRequest),
                                                 &payload[0],
                                                 sizeof(payload),
                                                 &payloadLength));

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_BAD_INDEX,
                          MmiEventLogServiceRead(&s_service,
                                                 MMI_PROTOCOL_V2_EVENT_RESOURCE_PAGE,
                                                 (const uint8_t *) &badIndexRequest,
                                                 sizeof(badIndexRequest),
                                                 &payload[0],
                                                 sizeof(payload),
                                                 &payloadLength));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_cursor_returns_none_when_no_logs_exist);
  RUN_TEST(test_latest_index_and_read_record_helpers_follow_repository_state);
  RUN_TEST(test_find_latest_by_event_code_returns_newest_matching_record);
  RUN_TEST(test_page_returns_requested_records_and_more_available_flag);
  RUN_TEST(test_page_rejects_bad_index_and_zero_max_count);
  return UNITY_END();
}
