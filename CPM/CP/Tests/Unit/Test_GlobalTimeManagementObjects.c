/*
 * Tests/Unit/Test_GlobalTimeManagementObjects.c
 *
 * Unit tests for canonical 1201 globalTimeManagement object routing.
 */
#include "unity.h"

#include "Domain/Intersection/GlobalTimeManagementService.h"
#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"
#include "MockConfigRepositoryAdapter.h"
#include "MockRTCAdapter.h"

#include <string.h>

static const uint32_t kDbCreateTransactionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 1U, 0U
};
static const uint32_t kDbTransactionIdOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 4U, 0U
};
static const uint32_t kGlobalTimeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 1U, 0U
};
static const uint32_t kControllerLocalTimeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 6U, 0U
};
static const uint32_t kDayPlanActionNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 5U, 1U, 5U, 1U, 1U
};
static const uint32_t kValidTimebaseActionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 3U, 1U, 1U, 3U
};
static const uint32_t kInvalidActionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 1U, 1U, 0U
};

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_configurationService;
static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
static NtcipDbTransactionService_t s_dbTransactionService;
static NtcipContext_t s_ntcipContext;
static NtcipObjectDirectory_t s_directory;
static MockRTCAdapterCtx_t s_rtcCtx;
static IRealtimeClockPort_t s_rtcPort;
static GlobalTimeManagementService_t s_globalTimeManagementService;

static void ReloadEngine(void)
{
  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(
    &s_engine,
    ConfigurationServiceGetActiveConfig(&s_configurationService)));
}

static void StartTransaction(NtcipRequestContext_t *request)
{
  NtcipValue_t value;

  memset(request, 0, sizeof(*request));
  request->sessionKey = 0x1234U;

  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     request,
                                                     &value));

  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbTransactionIdOid,
                                                     13U,
                                                     request,
                                                     &value));
  request->transactionIdValid = 1U;
  request->transactionId = 1U;
}

static void VerifyAndCommit(NtcipRequestContext_t *request)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_VERIFY);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     request,
                                                     &value));
  ReloadEngine();
}

static void SetRtc(uint8_t century,
                   uint8_t year,
                   uint8_t month,
                   uint8_t date,
                   uint8_t weekDay,
                   uint8_t hour,
                   uint8_t minute,
                   uint8_t second)
{
  RtcSnapshot_t snapshot = {
    century, year, month, date, weekDay, hour, minute, second
  };

  TEST_ASSERT_TRUE(RealtimeClockWriteSnapshot(&s_rtcPort, &snapshot));
}

void setUp(void)
{
  MockConfigRepositoryAdapterInit(&s_repoCtx);
  s_repoPort = MockConfigRepositoryAdapterCreatePort(&s_repoCtx);
  ConfigurationServiceInit(&s_configurationService, &s_repoPort);
  IntersectionEngineInit(&s_engine);
  IntersectionControllerInit(&s_controller);
  NtcipDbTransactionServiceInit(&s_dbTransactionService, &s_configurationService);
  NtcipContextInit(&s_ntcipContext,
                   &s_configurationService,
                   &s_engine,
                   &s_controller,
                   &s_dbTransactionService);
  MockRTCAdapterInit(&s_rtcCtx);
  s_rtcPort = MockRTCAdapterCreatePort(&s_rtcCtx);
  GlobalTimeManagementServiceInit(&s_globalTimeManagementService);
  GlobalTimeManagementServiceBind(&s_globalTimeManagementService,
                                  &s_engine,
                                  &s_rtcPort);
  NtcipContextBindGlobalTimeManagementService(&s_ntcipContext,
                                              &s_globalTimeManagementService);
  NtcipObjectDirectoryInit(&s_directory);
  Ntcip1201RegisterObjects(&s_directory, &s_ntcipContext);
  ReloadEngine();
  SetRtc(21U, 26U, 4U, 18U, 7U, 9U, 20U, 0U);
}

void tearDown(void)
{
}

void test_objects_report_global_and_local_time(void)
{
  IntersectionGlobalTimeManagementConfig_t globalTimeManagement;
  NtcipValue_t value;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_configurationService));
  TEST_ASSERT_TRUE(ConfigurationServiceGetCandidateGlobalTimeManagementConfig(
    &s_configurationService,
    &globalTimeManagement));
  globalTimeManagement.controllerStandardTimeZoneSeconds = 3600;
  globalTimeManagement.globalDaylightSaving = 2U;
  TEST_ASSERT_TRUE(ConfigurationServiceSetGlobalTimeManagementConfig(
    &s_configurationService,
    &globalTimeManagement));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_configurationService));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_configurationService));
  ReloadEngine();
  GlobalTimeManagementServiceReset(&s_globalTimeManagementService);

  TEST_ASSERT_TRUE(GlobalTimeManagementServiceSetGlobalTime(
    &s_globalTimeManagementService,
    3600U));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kGlobalTimeOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(3600U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kControllerLocalTimeOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(7200U, value.data.unsigned32);
}

void test_day_plan_action_oid_rejects_invalid_reference_and_accepts_timebase_action_oid(
  void)
{
  NtcipRequestContext_t request;
  NtcipValue_t value;

  StartTransaction(&request);

  NtcipValueSetObjectId(&value,
                        kInvalidActionOid,
                        (uint8_t) (sizeof(kInvalidActionOid) / sizeof(uint32_t)));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_GEN_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kDayPlanActionNumberOid,
                                                    17U,
                                                    &request,
                                                    &value));

  NtcipValueSetObjectId(&value,
                        kValidTimebaseActionOid,
                        (uint8_t) (sizeof(kValidTimebaseActionOid)
                                   / sizeof(uint32_t)));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kDayPlanActionNumberOid,
                                                    17U,
                                                    &request,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDayPlanActionNumberOid,
                                                     17U,
                                                     &request,
                                                     &value));

  VerifyAndCommit(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kDayPlanActionNumberOid,
                                                17U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT8(NTCIP_VALUE_TYPE_OBJECT_ID, value.type);
  TEST_ASSERT_EQUAL_UINT8(15U, value.data.objectId.length);
  TEST_ASSERT_EQUAL_UINT32(kValidTimebaseActionOid[14],
                           value.data.objectId.elements[14]);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_objects_report_global_and_local_time);
  RUN_TEST(
    test_day_plan_action_oid_rejects_invalid_reference_and_accepts_timebase_action_oid);
  return UNITY_END();
}
