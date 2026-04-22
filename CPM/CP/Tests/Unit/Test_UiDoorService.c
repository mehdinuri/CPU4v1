#include "unity.h"

#include <string.h>

#include "Adapters/Mock/MockDoorSensorAdapter.h"
#include "Domain/Services/EventReportService.h"
#include "Domain/Services/MmiEventLogService.h"
#include "Domain/Services/UiDoorService.h"

typedef struct
{
  EventReportService_t *eventReportService;
  uint8_t appendCount;
} TestUiDoorEventLoggerCtx_t;

static MockDoorSensorAdapterCtx_t s_doorCtx;
static IDoorSensorPort_t s_doorPort;
static EventReportService_t s_eventReportService;
static MmiEventLogService_t s_eventLogService;
static UiDoorService_t s_service;
static TestUiDoorEventLoggerCtx_t s_eventLoggerCtx;
static ILogEventPort_t s_eventPort;

static uint8_t AppendEvent(void *ctx,
                           uint8_t eventCode,
                           uint8_t eventParam,
                           uint16_t eventShortParam,
                           uint32_t eventLongParam)
{
  TestUiDoorEventLoggerCtx_t *eventLoggerCtx =
    (TestUiDoorEventLoggerCtx_t *) ctx;
  EventReportLogRecord_t *record;
  uint16_t eventId;
  uint16_t index;

  if ((eventLoggerCtx == NULL) || (eventLoggerCtx->eventReportService == NULL))
  {
    return 0U;
  }

  switch (eventCode)
  {
    case 64U:
      eventId = EVENT_REPORT_EVENT_ID_DOOR_OPEN;
      break;

    case 65U:
      eventId = EVENT_REPORT_EVENT_ID_DOOR_CLOSED;
      break;

    default:
      return 0U;
  }

  (void) eventParam;
  (void) eventShortParam;
  (void) eventLongParam;

  index = eventLoggerCtx->eventReportService->writeIndex;
  record = &eventLoggerCtx->eventReportService->logRecords[index];
  (void) memset(record, 0, sizeof(*record));
  record->eventLogClass = 2U;
  record->eventLogID = eventId;

  eventLoggerCtx->eventReportService->writeIndex++;
  if (eventLoggerCtx->eventReportService->writeIndex
      >= EVENT_REPORT_MAX_EVENT_LOG_SIZE)
  {
    eventLoggerCtx->eventReportService->writeIndex = 0U;
  }
  if (eventLoggerCtx->eventReportService->count < EVENT_REPORT_MAX_EVENT_LOG_SIZE)
  {
    eventLoggerCtx->eventReportService->count++;
  }

  eventLoggerCtx->appendCount++;
  return 1U;
}

static ILogEventPort_t CreateEventPort(TestUiDoorEventLoggerCtx_t *ctx)
{
  ILogEventPort_t port;

  port.ctx = ctx;
  port.Append = AppendEvent;
  return port;
}

void setUp(void)
{
  MockDoorSensorAdapterInit(&s_doorCtx);
  s_doorPort = MockDoorSensorAdapterCreatePort(&s_doorCtx);

  EventReportServiceInit(&s_eventReportService);
  MmiEventLogServiceInit(&s_eventLogService);
  MmiEventLogServiceBind(&s_eventLogService, &s_eventReportService);

  s_eventLoggerCtx.eventReportService = &s_eventReportService;
  s_eventLoggerCtx.appendCount = 0U;
  s_eventPort = CreateEventPort(&s_eventLoggerCtx);

  UiDoorServiceInit(&s_service);
  UiDoorServiceBind(&s_service,
                    &s_doorPort,
                    &s_eventPort,
                    &s_eventLogService);
}

void tearDown(void)
{
}

void test_initial_sample_sets_state_without_logging(void)
{
  TEST_ASSERT_TRUE(UiDoorServiceStep(&s_service, 0U));
  TEST_ASSERT_FALSE(UiDoorServiceIsOpen(&s_service));
  TEST_ASSERT_FALSE(UiDoorServiceConsumeChanged(&s_service));
  TEST_ASSERT_EQUAL_UINT32(1U, s_doorCtx.readCount);
  TEST_ASSERT_EQUAL_UINT8(0U, s_eventLoggerCtx.appendCount);
  TEST_ASSERT_EQUAL_UINT16(MMI_PROTOCOL_V2_EVENT_CURSOR_NONE,
                           UiDoorServiceGetLatestOpenLogIndex(&s_service));
  TEST_ASSERT_EQUAL_UINT16(MMI_PROTOCOL_V2_EVENT_CURSOR_NONE,
                           UiDoorServiceGetLatestCloseLogIndex(&s_service));
}

void test_service_polls_once_per_second_and_logs_open_close_transitions(void)
{
  TEST_ASSERT_TRUE(UiDoorServiceStep(&s_service, 0U));
  TEST_ASSERT_EQUAL_UINT32(1U, s_doorCtx.readCount);

  s_doorCtx.doorOpen = 1U;
  TEST_ASSERT_TRUE(UiDoorServiceStep(&s_service, 500U));
  TEST_ASSERT_EQUAL_UINT32(1U, s_doorCtx.readCount);
  TEST_ASSERT_FALSE(UiDoorServiceIsOpen(&s_service));
  TEST_ASSERT_FALSE(UiDoorServiceConsumeChanged(&s_service));

  TEST_ASSERT_TRUE(UiDoorServiceStep(&s_service, 1000U));
  TEST_ASSERT_EQUAL_UINT32(2U, s_doorCtx.readCount);
  TEST_ASSERT_TRUE(UiDoorServiceIsOpen(&s_service));
  TEST_ASSERT_TRUE(UiDoorServiceConsumeChanged(&s_service));
  TEST_ASSERT_FALSE(UiDoorServiceConsumeChanged(&s_service));
  TEST_ASSERT_EQUAL_UINT8(1U, s_eventLoggerCtx.appendCount);
  TEST_ASSERT_EQUAL_UINT32(1U, UiDoorServiceGetChangeSequence(&s_service));
  TEST_ASSERT_EQUAL_UINT16(0U,
                           UiDoorServiceGetLatestOpenLogIndex(&s_service));
  TEST_ASSERT_EQUAL_UINT16(MMI_PROTOCOL_V2_EVENT_CURSOR_NONE,
                           UiDoorServiceGetLatestCloseLogIndex(&s_service));

  TEST_ASSERT_TRUE(UiDoorServiceStep(&s_service, 1500U));
  TEST_ASSERT_EQUAL_UINT32(2U, s_doorCtx.readCount);

  s_doorCtx.doorOpen = 0U;
  TEST_ASSERT_TRUE(UiDoorServiceStep(&s_service, 2000U));
  TEST_ASSERT_EQUAL_UINT32(3U, s_doorCtx.readCount);
  TEST_ASSERT_FALSE(UiDoorServiceIsOpen(&s_service));
  TEST_ASSERT_TRUE(UiDoorServiceConsumeChanged(&s_service));
  TEST_ASSERT_EQUAL_UINT8(2U, s_eventLoggerCtx.appendCount);
  TEST_ASSERT_EQUAL_UINT32(2U, UiDoorServiceGetChangeSequence(&s_service));
  TEST_ASSERT_EQUAL_UINT16(0U,
                           UiDoorServiceGetLatestOpenLogIndex(&s_service));
  TEST_ASSERT_EQUAL_UINT16(1U,
                           UiDoorServiceGetLatestCloseLogIndex(&s_service));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_initial_sample_sets_state_without_logging);
  RUN_TEST(test_service_polls_once_per_second_and_logs_open_close_transitions);
  return UNITY_END();
}
