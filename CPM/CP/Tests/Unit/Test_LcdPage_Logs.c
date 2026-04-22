#include "unity.h"

#include <string.h>

#include "Domain/Lcd/LcdEngine.h"
#include "Domain/Lcd/LcdPage.h"
#include "Domain/Lcd/LcdLanguage.h"
#include "Domain/Lcd/LcdPageRegistry.h"
#include "Domain/Lcd/LcdServiceRegistry.h"
#include "Domain/Services/EventReportService.h"
#include "Domain/Services/MmiEventLogService.h"
#include "MockDisplayAdapter.h"

extern LcdPage_t LcdPage_Logs;
extern void LcdPage_Logs_Init(void *ctx,
                              const LcdServiceRegistry_t *services,
                              const LcdPageRegistry_t *pages);

static EventReportService_t s_eventReportService;
static MmiEventLogService_t s_eventLogService;
static MockDisplayAdapterCtx_t s_displayCtx;
static IDisplayPort_t s_displayPort;
static ISystemPort_t s_systemPort;
static LcdServiceRegistry_t s_services;
static LcdPageRegistry_t s_pages;
static LcdEngine_t s_engine;

static uint16_t SystemGetMainVoltage(void *ctx)
{
  (void) ctx;
  return 0U;
}

static uint8_t SystemGetTimeSource(void *ctx)
{
  (void) ctx;
  return 0U;
}

static uint8_t SystemGetLanguage(void *ctx)
{
  (void) ctx;
  return LANGUAGE_ENGLISH;
}

static void SystemSetLanguage(void *ctx, uint8_t lang)
{
  (void) ctx;
  (void) lang;
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

static void AssertRowPrefix(uint8_t row, const char *expected)
{
  TEST_ASSERT_EQUAL_MEMORY(expected,
                           &s_displayCtx.framebuffer[row][0],
                           strlen(expected));
}

void setUp(void)
{
  EventReportServiceInit(&s_eventReportService);
  MmiEventLogServiceInit(&s_eventLogService);
  MmiEventLogServiceBind(&s_eventLogService, &s_eventReportService);

  MockDisplayAdapterInit(&s_displayCtx);
  s_displayPort = MockDisplayAdapterCreatePort(&s_displayCtx);

  (void) memset(&s_systemPort, 0, sizeof(s_systemPort));
  s_systemPort.GetMainVoltage = SystemGetMainVoltage;
  s_systemPort.GetTimeSource = SystemGetTimeSource;
  s_systemPort.GetLanguage = SystemGetLanguage;
  s_systemPort.SetLanguage = SystemSetLanguage;

  (void) memset(&s_services, 0, sizeof(s_services));
  s_services.system = &s_systemPort;
  s_services.eventLogService = &s_eventLogService;

  (void) memset(&s_pages, 0, sizeof(s_pages));
  (void) memset(&s_engine, 0, sizeof(s_engine));
  LcdPage_Logs_Init(LcdPage_Logs.ctx, &s_services, &s_pages);
}

void tearDown(void)
{
}

void test_logs_page_shows_no_log_message_when_event_log_is_empty(void)
{
  LcdPage_Logs.OnEnter(LcdPage_Logs.ctx, &s_engine);
  LcdPage_Logs.OnDraw(LcdPage_Logs.ctx, &s_engine, &s_displayPort);

  AssertRowPrefix(0U, Lcd_GetNoLogStr(LANGUAGE_ENGLISH));
}

void test_logs_page_renders_ntcip_event_log_record_fields(void)
{
  static const uint8_t kValue[] = { 0x30U, 0x31U, 0x32U };

  AppendRecord(2U, 10U, 1735689605UL, 123U, &kValue[0], sizeof(kValue));

  LcdPage_Logs.OnEnter(LcdPage_Logs.ctx, &s_engine);
  LcdPage_Logs.OnDraw(LcdPage_Logs.ctx, &s_engine, &s_displayPort);

  AssertRowPrefix(0U, "<0000>");
  AssertRowPrefix(1U, "01/01 00:00:05.123");
  AssertRowPrefix(2U, "C:2 N:1 ID:10");
  AssertRowPrefix(3U, "V:30 31 32");
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_logs_page_shows_no_log_message_when_event_log_is_empty);
  RUN_TEST(test_logs_page_renders_ntcip_event_log_record_fields);
  return UNITY_END();
}
