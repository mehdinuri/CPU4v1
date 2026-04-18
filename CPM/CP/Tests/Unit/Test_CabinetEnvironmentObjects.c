/*
 * Tests/Unit/Test_CabinetEnvironmentObjects.c
 *
 * Unit tests for canonical 1202 cabinetEnvironment object routing.
 */
#include "unity.h"

#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/NTCIP/NTCIP1202.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"
#include "MockConfigRepositoryAdapter.h"
#include "MockDoorSensorAdapter.h"
#include "MockHeaterAdapter.h"
#include "MockPowerMonitorAdapter.h"

#include <string.h>

static const uint32_t kDbCreateTransactionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 1U, 0U
};
static const uint32_t kDbTransactionIdOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 4U, 0U
};
static const uint32_t kDoorOnStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 2U, 1U, 5U, 1U, 1U
};
static const uint32_t kHeaterOnStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 2U, 1U, 5U, 3U, 1U
};
static const uint32_t kFanErrorStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 2U, 1U, 6U, 2U, 1U
};
static const uint32_t kTempCurrentReadingOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 4U, 1U, 3U, 1U
};
static const uint32_t kHumidityCurrentReadingOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 6U, 1U, 3U, 1U
};
static const uint32_t kPowerSourceOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 7U, 0U
};
static const uint32_t kLineVoltsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 8U, 0U
};
static const uint32_t kFanTypeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 2U, 1U, 2U, 2U, 1U
};
static const uint32_t kFanDescriptionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 2U, 1U, 4U, 2U, 1U
};
static const uint32_t kTempHighThresholdOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 4U, 1U, 4U, 1U
};
static const uint32_t kTempLowThresholdOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 4U, 1U, 5U, 1U
};
static const uint32_t kHumidityThresholdOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 6U, 1U, 4U, 1U
};
static const uint32_t kAtccLedModeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 9U, 0U
};

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_configurationService;
static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
static NtcipDbTransactionService_t s_dbTransactionService;
static NtcipContext_t s_ntcipContext;
static NtcipObjectDirectory_t s_directory;
static MockDoorSensorAdapterCtx_t s_doorCtx;
static IDoorSensorPort_t s_doorPort;
static MockHeaterAdapterCtx_t s_heaterCtx;
static IHeaterPort_t s_heaterPort;
static MockPowerMonitorAdapterCtx_t s_powerCtx;
static IPowerMonitorPort_t s_powerPort;

static void StartTransaction(NtcipRequestContext_t *request)
{
  NtcipValue_t value;

  memset(request, 0, sizeof(*request));
  request->sessionKey = 0xCA51U;

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
  MockDoorSensorAdapterInit(&s_doorCtx);
  s_doorPort = MockDoorSensorAdapterCreatePort(&s_doorCtx);
  MockHeaterAdapterInit(&s_heaterCtx);
  s_heaterPort = MockHeaterAdapterCreatePort(&s_heaterCtx);
  MockPowerMonitorAdapterInit(&s_powerCtx);
  s_powerPort = MockPowerMonitorAdapterCreatePort(&s_powerCtx);
  NtcipContextBindDoorSensorPort(&s_ntcipContext, &s_doorPort);
  NtcipContextBindHeaterPort(&s_ntcipContext, &s_heaterPort);
  NtcipContextBindPowerMonitorPort(&s_ntcipContext, &s_powerPort);
  NtcipObjectDirectoryInit(&s_directory);
  Ntcip1201RegisterObjects(&s_directory, &s_ntcipContext);
  Ntcip1202RegisterObjects(&s_directory, &s_ntcipContext);
}

void tearDown(void)
{
}

void test_objects_report_live_cabinet_status_and_missing_sensor_defaults(void)
{
  NtcipValue_t value;

  s_doorCtx.doorOpen = 1U;
  s_heaterCtx.heaterOn = 1U;
  s_powerCtx.primarySource = 3U;
  s_powerCtx.lineVoltageTenthsVrms = 2315U;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kDoorOnStatusOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kHeaterOnStatusOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kFanErrorStatusOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(4U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kTempCurrentReadingOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_INT32(-128, value.data.signed32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kHumidityCurrentReadingOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(101U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPowerSourceOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(3U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kLineVoltsOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(2315U, value.data.unsigned32);
}

void test_writable_cabinet_environment_objects_commit(void)
{
  NtcipRequestContext_t request;
  NtcipValue_t value;
  static const uint8_t kFanNorth[] = "FAN NORTH";

  StartTransaction(&request);

  NtcipValueSetUnsigned32(&value, 5U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kFanTypeOid,
                                                    16U,
                                                    &request,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kFanTypeOid,
                                                     16U,
                                                     &request,
                                                     &value));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetOctetString(&value,
                                                 kFanNorth,
                                                 sizeof(kFanNorth) - 1U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kFanDescriptionOid,
                                                     16U,
                                                     &request,
                                                     &value));

  NtcipValueSetSigned32(&value, 40);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kTempHighThresholdOid,
                                                     15U,
                                                     &request,
                                                     &value));

  NtcipValueSetSigned32(&value, -10);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kTempLowThresholdOid,
                                                     15U,
                                                     &request,
                                                     &value));

  NtcipValueSetUnsigned32(&value, 85U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kHumidityThresholdOid,
                                                     15U,
                                                     &request,
                                                     &value));

  NtcipValueSetUnsigned32(&value, 2U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kAtccLedModeOid,
                                                     13U,
                                                     &request,
                                                     &value));

  VerifyAndCommit(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kFanTypeOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(5U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kFanDescriptionOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT8(NTCIP_VALUE_TYPE_OCTET_STRING, value.type);
  TEST_ASSERT_EQUAL_UINT16(sizeof(kFanNorth) - 1U,
                           value.data.octetString.length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(kFanNorth,
                                value.data.octetString.bytes,
                                sizeof(kFanNorth) - 1U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kTempHighThresholdOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_INT32(40, value.data.signed32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kTempLowThresholdOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_INT32(-10, value.data.signed32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kHumidityThresholdOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(85U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAtccLedModeOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(2U, value.data.unsigned32);
}

void test_temp_threshold_set_test_rejects_low_above_high(void)
{
  NtcipRequestContext_t request;
  NtcipValue_t value;

  StartTransaction(&request);

  NtcipValueSetSigned32(&value, 1);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_BAD_VALUE,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kTempLowThresholdOid,
                                                    15U,
                                                    &request,
                                                    &value));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_objects_report_live_cabinet_status_and_missing_sensor_defaults);
  RUN_TEST(test_writable_cabinet_environment_objects_commit);
  RUN_TEST(test_temp_threshold_set_test_rejects_low_above_high);
  return UNITY_END();
}
