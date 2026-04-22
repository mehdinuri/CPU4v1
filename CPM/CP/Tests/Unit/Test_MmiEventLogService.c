#include "unity.h"

#include <string.h>

#include "Domain/Services/EventReportService.h"
#include "Domain/Services/MmiEventLogService.h"

static EventReportService_t s_eventReportService;
static MmiEventLogService_t s_service;

void setUp(void)
{
  EventReportServiceInit(&s_eventReportService);
  MmiEventLogServiceInit(&s_service);
  MmiEventLogServiceBind(&s_service, &s_eventReportService);
}

void tearDown(void)
{
}

static void AppendRecord(uint8_t eventClass,
                         uint16_t eventId,
                         uint32_t eventTime,
                         uint16_t eventTimeMilliseconds,
                         const uint8_t *value,
                         uint8_t valueLength)
{
  uint16_t index = s_eventReportService.writeIndex;
  EventReportLogRecord_t *record = &s_eventReportService.logRecords[index];

  TEST_ASSERT_TRUE(valueLength <= EVENT_REPORT_EVENT_VALUE_MAX_LENGTH);

  (void) memset(record, 0, sizeof(*record));
  record->eventLogClass = eventClass;
  record->eventLogID = eventId;
  record->eventLogTime = eventTime;
  record->eventLogTimeMilliseconds = eventTimeMilliseconds;
  record->eventLogValueLength = valueLength;
  if ((value != NULL) && (valueLength > 0U))
  {
    (void) memcpy(&record->eventLogValue[0], value, valueLength);
  }

  s_eventReportService.writeIndex++;
  if (s_eventReportService.writeIndex >= EVENT_REPORT_MAX_EVENT_LOG_SIZE)
  {
    s_eventReportService.writeIndex = 0U;
  }

  if (s_eventReportService.count < EVENT_REPORT_MAX_EVENT_LOG_SIZE)
  {
    s_eventReportService.count++;
  }
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

void test_latest_index_and_read_record_helpers_follow_event_report_state(void)
{
  static const uint8_t kValue[] = { 0x30U, 0x31U, 0x32U };
  uint16_t latestIndex = 0U;
  MmiEventRecordV2_t record;

  AppendRecord(2U, 10U, 1000UL, 111U, &kValue[0], sizeof(kValue));
  AppendRecord(2U, 20U, 2000UL, 222U, NULL, 0U);

  TEST_ASSERT_TRUE(MmiEventLogServiceGetLatestIndex(&s_service, &latestIndex));
  TEST_ASSERT_EQUAL_UINT16(1U, latestIndex);

  TEST_ASSERT_TRUE(MmiEventLogServiceIsIndexValid(&s_service, 0U));
  TEST_ASSERT_FALSE(MmiEventLogServiceIsIndexValid(&s_service, 2U));
  TEST_ASSERT_TRUE(MmiEventLogServiceCanReadFromIndex(&s_service, 0U));
  TEST_ASSERT_FALSE(MmiEventLogServiceCanReadFromIndex(&s_service, 2U));
  TEST_ASSERT_TRUE(MmiEventLogServiceReadRecord(&s_service, 0U, &record));
  TEST_ASSERT_EQUAL_UINT16(0U, record.logIndex);
  TEST_ASSERT_EQUAL_UINT8(2U, record.eventClass);
  TEST_ASSERT_EQUAL_UINT8(1U, record.eventNumber);
  TEST_ASSERT_EQUAL_UINT16(10U, record.eventId);
  TEST_ASSERT_EQUAL_UINT32(1000UL, record.eventTime);
  TEST_ASSERT_EQUAL_UINT16(111U, record.eventTimeMilliseconds);
  TEST_ASSERT_EQUAL_UINT8(sizeof(kValue), record.valueLength);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&kValue[0], &record.value[0], sizeof(kValue));
}

void test_find_latest_by_event_id_returns_newest_matching_record(void)
{
  uint16_t latestDoorIndex = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;

  AppendRecord(2U, 64U, 1000UL, 0U, NULL, 0U);
  AppendRecord(3U, 65U, 2000UL, 0U, NULL, 0U);
  AppendRecord(2U, 64U, 3000UL, 0U, NULL, 0U);

  TEST_ASSERT_TRUE(MmiEventLogServiceFindLatestByEventId(&s_service,
                                                         64U,
                                                         &latestDoorIndex));
  TEST_ASSERT_EQUAL_UINT16(2U, latestDoorIndex);
  TEST_ASSERT_TRUE(MmiEventLogServiceFindLatestByEventId(&s_service,
                                                         99U,
                                                         &latestDoorIndex));
  TEST_ASSERT_EQUAL_UINT16(MMI_PROTOCOL_V2_EVENT_CURSOR_NONE, latestDoorIndex);
}

void test_page_returns_requested_records_and_caps_payload_to_four_records(void)
{
  static const uint8_t kValue0[] = { 0x01U, 0x02U };
  static const uint8_t kValue1[] = { 0xA0U };
  static const uint8_t kValue2[] = { 0xB1U, 0xB2U, 0xB3U };
  static const uint8_t kValue3[] = { 0xC4U };
  static const uint8_t kValue4[] = { 0xD5U };
  MmiEventPageRequestV2_t request = { 0U, 6U, 0U };
  uint8_t payload[512];
  uint16_t payloadLength = 0U;
  MmiEventPageHeaderV2_t header;
  MmiEventRecordV2_t record0;
  MmiEventRecordV2_t record1;
  MmiEventRecordV2_t record2;
  MmiEventRecordV2_t record3;

  AppendRecord(2U, 10U, 1000UL, 10U, &kValue0[0], sizeof(kValue0));
  AppendRecord(2U, 11U, 1100UL, 11U, &kValue1[0], sizeof(kValue1));
  AppendRecord(3U, 20U, 2000UL, 20U, &kValue2[0], sizeof(kValue2));
  AppendRecord(2U, 12U, 1200UL, 12U, &kValue3[0], sizeof(kValue3));
  AppendRecord(4U, 30U, 3000UL, 30U, &kValue4[0], sizeof(kValue4));

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiEventLogServiceRead(&s_service,
                                                 MMI_PROTOCOL_V2_EVENT_RESOURCE_PAGE,
                                                 (const uint8_t *) &request,
                                                 sizeof(request),
                                                 &payload[0],
                                                 sizeof(payload),
                                                 &payloadLength));
  TEST_ASSERT_EQUAL_UINT16(sizeof(header)
                           + (uint16_t) (4U * sizeof(MmiEventRecordV2_t)),
                           payloadLength);

  (void) memcpy(&header, &payload[0], sizeof(header));
  TEST_ASSERT_EQUAL_UINT16(0U, header.startIndex);
  TEST_ASSERT_EQUAL_UINT8(4U, header.count);
  TEST_ASSERT_EQUAL_UINT8(1U, header.moreAvailable);

  (void) memcpy(&record0, &payload[sizeof(header)], sizeof(record0));
  (void) memcpy(&record1,
                &payload[sizeof(header) + sizeof(record0)],
                sizeof(record1));
  (void) memcpy(&record2,
                &payload[sizeof(header) + sizeof(record0) + sizeof(record1)],
                sizeof(record2));
  (void) memcpy(&record3,
                &payload[sizeof(header)
                         + sizeof(record0)
                         + sizeof(record1)
                         + sizeof(record2)],
                sizeof(record3));

  TEST_ASSERT_EQUAL_UINT16(0U, record0.logIndex);
  TEST_ASSERT_EQUAL_UINT8(2U, record0.eventClass);
  TEST_ASSERT_EQUAL_UINT8(1U, record0.eventNumber);
  TEST_ASSERT_EQUAL_UINT16(10U, record0.eventId);
  TEST_ASSERT_EQUAL_UINT32(1000UL, record0.eventTime);
  TEST_ASSERT_EQUAL_UINT16(10U, record0.eventTimeMilliseconds);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&kValue0[0], &record0.value[0], sizeof(kValue0));

  TEST_ASSERT_EQUAL_UINT16(1U, record1.logIndex);
  TEST_ASSERT_EQUAL_UINT8(2U, record1.eventClass);
  TEST_ASSERT_EQUAL_UINT8(2U, record1.eventNumber);
  TEST_ASSERT_EQUAL_UINT16(11U, record1.eventId);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&kValue1[0], &record1.value[0], sizeof(kValue1));

  TEST_ASSERT_EQUAL_UINT16(2U, record2.logIndex);
  TEST_ASSERT_EQUAL_UINT8(3U, record2.eventClass);
  TEST_ASSERT_EQUAL_UINT8(1U, record2.eventNumber);
  TEST_ASSERT_EQUAL_UINT16(20U, record2.eventId);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&kValue2[0], &record2.value[0], sizeof(kValue2));

  TEST_ASSERT_EQUAL_UINT16(3U, record3.logIndex);
  TEST_ASSERT_EQUAL_UINT8(2U, record3.eventClass);
  TEST_ASSERT_EQUAL_UINT8(3U, record3.eventNumber);
  TEST_ASSERT_EQUAL_UINT16(12U, record3.eventId);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&kValue3[0], &record3.value[0], sizeof(kValue3));
}

void test_page_rejects_bad_index_and_zero_max_count(void)
{
  MmiEventPageRequestV2_t zeroCountRequest = { 0U, 0U, 0U };
  MmiEventPageRequestV2_t badIndexRequest = { 1U, 1U, 0U };
  uint8_t payload[64];
  uint16_t payloadLength = 0U;

  AppendRecord(2U, 10U, 1000UL, 0U, NULL, 0U);

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
  RUN_TEST(test_latest_index_and_read_record_helpers_follow_event_report_state);
  RUN_TEST(test_find_latest_by_event_id_returns_newest_matching_record);
  RUN_TEST(test_page_returns_requested_records_and_caps_payload_to_four_records);
  RUN_TEST(test_page_rejects_bad_index_and_zero_max_count);
  return UNITY_END();
}
