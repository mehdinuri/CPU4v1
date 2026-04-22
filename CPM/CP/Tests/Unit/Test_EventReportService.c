/*
 * Tests/Unit/Test_EventReportService.c
 *
 * Unit tests for 1103 watch/report block encoding and millisecond timestamps.
 */
#include "unity.h"

#include <string.h>

#include "Domain/Intersection/GlobalTimeManagementService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/Mib1103v0352/BlockObjects.h"
#include "Domain/Services/EventReportService.h"
#include "MockRTCAdapter.h"

enum
{
  MOCK_TAG_COUNTER_A = 1,
  MOCK_TAG_COUNTER_B = 2,
  MOCK_TAG_STATUS_TEXT = 3
};

typedef struct
{
  uint32_t counterA;
  uint32_t counterB;
  uint8_t statusText[8];
  uint16_t statusTextLength;
} MockManagedState_t;

static const uint32_t kMockCounterAOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 99U, 1U
};
static const uint32_t kMockCounterAInstanceOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 99U, 1U, 0U
};
static const uint32_t kMockCounterBOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 99U, 2U
};
static const uint32_t kMockStatusTextOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 99U, 3U
};
static const uint32_t kWatchBlockValue1Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U, 4U, 1U, 4U, 1U
};
static const uint32_t kReportBlockValue1Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U, 4U, 1U, 4U, 1U
};

static MockManagedState_t s_mockState;
static IntersectionEngine_t s_engine;
static GlobalTimeManagementService_t s_timeService;
static MockRTCAdapterCtx_t s_rtcCtx;
static IRealtimeClockPort_t s_rtcPort;
static EventReportService_t s_service;
static NtcipObjectDirectory_t s_directory;
static NtcipContext_t s_context;

static void SetOid(NtcipOid_t *oid, const uint32_t *elements, uint8_t length)
{
  (void) memset(oid, 0, sizeof(*oid));
  oid->length = length;
  (void) memcpy(&oid->elements[0], elements, (size_t) length * sizeof(uint32_t));
}

static void SetRtc(uint8_t second, uint16_t milliseconds)
{
  RtcSnapshot_t snapshot = { 21U, 26U, 4U, 22U, 3U, 10U, 11U, second,
                             milliseconds };

  TEST_ASSERT_TRUE(RealtimeClockWriteSnapshot(&s_rtcPort, &snapshot));
}

static NtcipError_t GetMockObject(void *groupContext,
                                  const NtcipObjectDescriptor_t *descriptor,
                                  const uint32_t *indexes,
                                  uint8_t indexCount,
                                  const NtcipRequestContext_t *requestContext,
                                  NtcipValue_t *value)
{
  MockManagedState_t *state = (MockManagedState_t *) groupContext;

  (void) indexes;
  (void) indexCount;
  (void) requestContext;

  if ((state == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
    case MOCK_TAG_COUNTER_A:
      NtcipValueSetUnsigned32(value, state->counterA);
      return NTCIP_ERROR_OK;

    case MOCK_TAG_COUNTER_B:
      NtcipValueSetUnsigned32(value, state->counterB);
      return NTCIP_ERROR_OK;

    case MOCK_TAG_STATUS_TEXT:
      return NtcipValueSetOctetString(value,
                                      &state->statusText[0],
                                      state->statusTextLength);

    default:
      return NTCIP_ERROR_NOT_FOUND;
  }
}

static const NtcipObjectDescriptor_t kMockObjects[] =
{
  { kMockCounterAOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    MOCK_TAG_COUNTER_A, GetMockObject, NULL, NULL },
  { kMockCounterBOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    MOCK_TAG_COUNTER_B, GetMockObject, NULL, NULL },
  { kMockStatusTextOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OCTET_STRING,
    MOCK_TAG_STATUS_TEXT, GetMockObject, NULL, NULL }
};

void setUp(void)
{
  IntersectionConfig_t config;

  (void) memset(&s_mockState, 0, sizeof(s_mockState));
  s_mockState.counterA = 5U;
  s_mockState.counterB = 7U;
  s_mockState.statusText[0] = 'O';
  s_mockState.statusText[1] = 'K';
  s_mockState.statusTextLength = 2U;

  IntersectionEngineInit(&s_engine);
  IntersectionConfigInitDefaults(&config);
  config.globalTimeManagement.globalDaylightSaving = 2U;
  config.globalTimeManagement.controllerStandardTimeZoneSeconds = 0;
  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));

  MockRTCAdapterInit(&s_rtcCtx);
  s_rtcPort = MockRTCAdapterCreatePort(&s_rtcCtx);
  SetRtc(12U, 345U);

  GlobalTimeManagementServiceInit(&s_timeService);
  GlobalTimeManagementServiceBind(&s_timeService, &s_engine, &s_rtcPort);

  EventReportServiceInit(&s_service);
  EventReportServiceBindGlobalTimeManagementService(&s_service, &s_timeService);

  NtcipObjectDirectoryInit(&s_directory);
  NtcipContextInit(&s_context, NULL, &s_engine, NULL, NULL);
  NtcipContextBindEventReportService(&s_context, &s_service);
  TEST_ASSERT_TRUE(NtcipObjectDirectoryRegisterGroup(
    &s_directory,
    "test.mock",
    kMockObjects,
    (uint16_t) (sizeof(kMockObjects) / sizeof(kMockObjects[0])),
    &s_mockState));
  BlockObjectsRegister(&s_directory, &s_context);
  EventReportServiceBindObjectDirectory(&s_service, &s_directory);
}

void tearDown(void)
{
}

void test_watch_and_report_blocks_encode_open_type_wrapped_values(void)
{
  EventReportConfiguration_t *config = EventReportServiceGetCandidateConfig(
    &s_service);
  NtcipOctetString_t blockValue;
  NtcipValue_t managedValue;
  static const uint8_t kExpectedWatchValue[] =
  {
    0x03U, 0x02U, 0x01U, 0x05U,
    0x03U, 0x02U, 0x01U, 0x07U
  };
  static const uint8_t kExpectedReportValue[] =
  {
    0x03U, 0x02U, 0x01U, 0x07U,
    0x04U, 0x04U, 0x02U, 'O', 'K'
  };

  config->watchBlockRows[0].watchBlockStatus = EVENT_REPORT_ROW_STATUS_ACTIVE;
  config->watchObjectRows[0].watchStatus = EVENT_REPORT_ROW_STATUS_ACTIVE;
  SetOid(&config->watchObjectRows[0].watchOid, &kMockCounterAOid[0], 12U);
  config->watchObjectRows[1].watchStatus = EVENT_REPORT_ROW_STATUS_ACTIVE;
  SetOid(&config->watchObjectRows[1].watchOid, &kMockCounterBOid[0], 12U);

  config->reportBlockRows[0].reportBlockStatus = EVENT_REPORT_ROW_STATUS_ACTIVE;
  config->reportObjectRows[0].reportStatus = EVENT_REPORT_ROW_STATUS_ACTIVE;
  SetOid(&config->reportObjectRows[0].reportOid, &kMockCounterBOid[0], 12U);
  config->reportObjectRows[1].reportStatus = EVENT_REPORT_ROW_STATUS_ACTIVE;
  SetOid(&config->reportObjectRows[1].reportOid, &kMockStatusTextOid[0], 12U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                &kMockCounterAInstanceOid[0],
                                                13U,
                                                NULL,
                                                &managedValue));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          EventReportServiceValidateWatchObjectOid(
                            &s_service,
                            &config->watchObjectRows[0].watchOid));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          EventReportServiceValidateReportObjectOid(
                            &s_service,
                            &config->reportObjectRows[1].reportOid));

  TEST_ASSERT_TRUE(EventReportServiceReadWatchBlockValue(&s_service,
                                                         1U,
                                                         &blockValue));
  TEST_ASSERT_EQUAL_UINT16(sizeof(kExpectedWatchValue), blockValue.length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&kExpectedWatchValue[0],
                                &blockValue.bytes[0],
                                sizeof(kExpectedWatchValue));

  TEST_ASSERT_TRUE(EventReportServiceReadReportBlockValue(&s_service,
                                                          1U,
                                                          &blockValue));
  TEST_ASSERT_EQUAL_UINT16(sizeof(kExpectedReportValue), blockValue.length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&kExpectedReportValue[0],
                                &blockValue.bytes[0],
                                sizeof(kExpectedReportValue));
}

void test_event_step_uses_watch_block_compare_and_true_rtc_milliseconds(void)
{
  EventReportConfiguration_t *config = EventReportServiceGetCandidateConfig(
    &s_service);
  EventReportLogRecord_t record;
  NtcipOctetString_t pendingTrap;
  uint16_t latestIndex = 0U;
  uint32_t expectedTime = 0U;
  uint16_t expectedMilliseconds = 0U;
  static const uint8_t kExpectedLogValue[] =
  {
    0x04U, 0x04U, 0x03U, 0x02U, 0x01U, 0x09U
  };

  config->classes[4].eventClassLimit = 8U;
  config->trapControl = 1U;
  config->watchBlockRows[0].watchBlockStatus = EVENT_REPORT_ROW_STATUS_ACTIVE;
  config->watchObjectRows[0].watchStatus = EVENT_REPORT_ROW_STATUS_ACTIVE;
  SetOid(&config->watchObjectRows[0].watchOid, &kMockCounterAOid[0], 12U);

  config->reportBlockRows[0].reportBlockStatus = EVENT_REPORT_ROW_STATUS_ACTIVE;
  config->reportObjectRows[0].reportStatus = EVENT_REPORT_ROW_STATUS_ACTIVE;
  SetOid(&config->reportObjectRows[0].reportOid, &kMockCounterBOid[0], 12U);

  config->configs[7].eventConfigClass = 5U;
  config->configs[7].eventConfigMode = EVENT_REPORT_MODE_ON_CHANGE;
  config->configs[7].eventConfigAction = EVENT_REPORT_ACTION_LOG;
  SetOid(&config->configs[7].eventConfigCompareOid, &kWatchBlockValue1Oid[0],
         16U);
  SetOid(&config->configs[7].eventConfigLogOid, &kReportBlockValue1Oid[0], 16U);
  config->trapRows[7][0].trapDestEnable = 1U;

  EventReportServiceRefreshWorkingConfig(&s_service);
  TEST_ASSERT_EQUAL_UINT8(EVENT_REPORT_STATUS_LOG,
                          config->configs[7].eventConfigStatus);

  EventReportServicePrime(&s_service);

  s_mockState.counterA = 6U;
  s_mockState.counterB = 9U;
  SetRtc(18U, 678U);
  TEST_ASSERT_TRUE(GlobalTimeManagementServiceGetGlobalTimeWithMilliseconds(
    &s_timeService,
    &expectedTime,
    &expectedMilliseconds));

  EventReportServiceStep(&s_service, 1234U);

  latestIndex = EventReportServiceGetLatestLogIndex(&s_service);
  TEST_ASSERT_TRUE(latestIndex != 0xFFFFU);
  TEST_ASSERT_TRUE(EventReportServiceReadLogRecord(&s_service,
                                                   latestIndex,
                                                   &record));
  TEST_ASSERT_EQUAL_UINT8(5U, record.eventLogClass);
  TEST_ASSERT_EQUAL_UINT16(8U, record.eventLogID);
  TEST_ASSERT_EQUAL_UINT32(expectedTime, record.eventLogTime);
  TEST_ASSERT_EQUAL_UINT16(expectedMilliseconds, record.eventLogTimeMilliseconds);
  TEST_ASSERT_EQUAL_UINT8(sizeof(kExpectedLogValue), record.eventLogValueLength);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&kExpectedLogValue[0],
                                &record.eventLogValue[0],
                                sizeof(kExpectedLogValue));
  TEST_ASSERT_TRUE(EventReportServiceCopyPendingTrap(&s_service, &pendingTrap));
  TEST_ASSERT_TRUE(pendingTrap.length > 2U);
  TEST_ASSERT_EQUAL_UINT8(1U, pendingTrap.bytes[0]);
  TEST_ASSERT_EQUAL_UINT8(1U, pendingTrap.bytes[1]);
  TEST_ASSERT_EQUAL_UINT8(EVENT_REPORT_TRAP_LINK_PENDING,
                          config->trapMgmtRows[0].trapMgmtLinkStateStatus);

  EventReportServiceAcknowledgeTrapDispatch(&s_service, 1U);
  TEST_ASSERT_FALSE(EventReportServiceCopyPendingTrap(&s_service, &pendingTrap));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_watch_and_report_blocks_encode_open_type_wrapped_values);
  RUN_TEST(test_event_step_uses_watch_block_compare_and_true_rtc_milliseconds);
  return UNITY_END();
}
