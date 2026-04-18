/*
 * Tests/Unit/Test_AscIoOutputMappingObjects.c
 *
 * Unit tests for canonical 1202 ascIOmapping output-side objects.
 */
#include "unity.h"

#include "Domain/Intersection/IntersectionOutputDispatcher.h"
#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/NTCIP/NTCIP1202.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"
#include "MockConfigRepositoryAdapter.h"

#include <string.h>

static const uint32_t kDbCreateTransactionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 1U, 0U
};
static const uint32_t kDbTransactionIdOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 4U, 0U
};
static const uint32_t kAscIOmaxMapsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 1U, 1U, 0U
};
static const uint32_t kAscIOactiveMapOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 1U, 2U, 0U
};
static const uint32_t kAscIOmapDescriptionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 8U, 1U, 1U, 1U
};
static const uint32_t kAscIOoutputMapDeviceTypeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 2U, 1U, 1U
};
static const uint32_t kAscIOoutputMapDevicePinOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 6U, 1U, 1U
};
static const uint32_t kAscIOoutputMapFunctionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 9U, 1U, 1U
};
static const uint32_t kAscIOoutputMapFunctionIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 11U, 1U, 1U
};
static const uint32_t kAscIOoutputMapDevPinDescrOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 7U, 1U, 1U, 1U, 1U
};
static const uint32_t kAscIOoutputMapDevPinStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 7U, 1U, 2U, 1U, 1U
};
static const uint32_t kAscIOmapMaxOutputFunctionsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 10U, 1U, 0U
};
static const uint32_t kAscIOoutputMaxFuncIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 10U, 2U, 1U, 2U, 8U
};
static const uint32_t kAscIOoutputFunctionNameOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 10U, 2U, 1U, 3U, 8U
};

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_configurationService;
static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
static IntersectionOutputDispatcher_t s_dispatcher;
static NtcipDbTransactionService_t s_dbTransactionService;
static NtcipContext_t s_ntcipContext;
static NtcipObjectDirectory_t s_directory;

static void StartTransaction(NtcipRequestContext_t *request)
{
  NtcipValue_t value;

  memset(request, 0, sizeof(*request));
  request->sessionKey = 0xA510U;

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

static void SetUnsigned32Object(const uint32_t *oid,
                                uint8_t oidLength,
                                NtcipRequestContext_t *request,
                                uint32_t data)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, data);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     oid,
                                                     oidLength,
                                                     request,
                                                     &value));
}

void setUp(void)
{
  IntersectionConfig_t config;

  MockConfigRepositoryAdapterInit(&s_repoCtx);
  s_repoPort = MockConfigRepositoryAdapterCreatePort(&s_repoCtx);
  ConfigurationServiceInit(&s_configurationService, &s_repoPort);
  IntersectionEngineInit(&s_engine);
  IntersectionConfigInitDefaults(&config);
  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionControllerInit(&s_controller);
  IntersectionOutputDispatcherInit(&s_dispatcher);
  s_controller.outputDispatcher = &s_dispatcher;
  NtcipDbTransactionServiceInit(&s_dbTransactionService, &s_configurationService);
  NtcipContextInit(&s_ntcipContext,
                   &s_configurationService,
                   &s_engine,
                   &s_controller,
                   &s_dbTransactionService);
  NtcipObjectDirectoryInit(&s_directory);
  Ntcip1201RegisterObjects(&s_directory, &s_ntcipContext);
  Ntcip1202RegisterObjects(&s_directory, &s_ntcipContext);
}

void tearDown(void)
{
}

void test_objects_persist_output_map_rows_and_report_live_pin_status(void)
{
  NtcipRequestContext_t request;
  NtcipValue_t value;
  static const uint8_t kDescription[] = "Main output map";
  static const uint8_t kPinName[] = "pinC1-2";
  static const uint8_t kFunctionName[] = "channelGreen";

  StartTransaction(&request);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetOctetString(&value,
                                                 kDescription,
                                                 sizeof(kDescription) - 1U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kAscIOmapDescriptionOid,
                                                     15U,
                                                     &request,
                                                     &value));
  SetUnsigned32Object(kAscIOoutputMapDeviceTypeOid, 16U, &request,
                      (uint32_t) INTERSECTION_IO_MAP_DEVICE_FIO);
  SetUnsigned32Object(kAscIOoutputMapDevicePinOid, 16U, &request, 1U);
  SetUnsigned32Object(kAscIOoutputMapFunctionOid, 16U, &request,
                      (uint32_t) INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_GREEN);
  SetUnsigned32Object(kAscIOoutputMapFunctionIndexOid, 16U, &request, 1U);
  VerifyAndCommit(&request);

  s_dispatcher.lastAppliedImage.channels[0] = OUTPUT_DRIVER_ASPECT_FLASH_GREEN;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOmaxMapsOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOactiveMapOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOmapDescriptionOid,
                                                15U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT16(sizeof(kDescription) - 1U,
                           value.data.octetString.length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(kDescription,
                                value.data.octetString.bytes,
                                sizeof(kDescription) - 1U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOoutputMapDeviceTypeOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32((uint32_t) INTERSECTION_IO_MAP_DEVICE_FIO,
                           value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOoutputMapDevPinDescrOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT16(sizeof(kPinName) - 1U, value.data.octetString.length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(kPinName,
                                value.data.octetString.bytes,
                                sizeof(kPinName) - 1U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOoutputMapDevPinStatusOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  s_dispatcher.lastAppliedImage.channels[0] = OUTPUT_DRIVER_ASPECT_RED;
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOoutputMapDevPinStatusOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOmapMaxOutputFunctionsOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(29U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOoutputMaxFuncIndexOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32((uint32_t) INTERSECTION_CHANNEL_COUNT_MAX,
                           value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOoutputFunctionNameOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT16(sizeof(kFunctionName) - 1U,
                           value.data.octetString.length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(kFunctionName,
                                value.data.octetString.bytes,
                                sizeof(kFunctionName) - 1U);
}

void test_objects_reject_invalid_values_and_obey_remote_write_lock(void)
{
  NtcipRequestContext_t request;
  NtcipValue_t value;
  static const uint8_t kDescription[] = "LOCKED";

  StartTransaction(&request);

  NtcipValueSetUnsigned32(&value, 2U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_BAD_VALUE,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kAscIOactiveMapOid,
                                                    14U,
                                                    &request,
                                                    &value));

  NtcipValueSetUnsigned32(
    &value,
    (uint32_t) INTERSECTION_IO_MAP_OUTPUT_FUNCTION_LOGIC_OUTPUT);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_BAD_VALUE,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kAscIOoutputMapFunctionOid,
                                                    16U,
                                                    &request,
                                                    &value));

  TEST_ASSERT_TRUE(IntersectionEngineSetUnitControl(&s_engine, 0x02U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetOctetString(&value,
                                                 kDescription,
                                                 sizeof(kDescription) - 1U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_ACCESS,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kAscIOmapDescriptionOid,
                                                    15U,
                                                    &request,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_ACCESS,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kAscIOmapDescriptionOid,
                                                     15U,
                                                     &request,
                                                     &value));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_objects_persist_output_map_rows_and_report_live_pin_status);
  RUN_TEST(test_objects_reject_invalid_values_and_obey_remote_write_lock);
  return UNITY_END();
}
