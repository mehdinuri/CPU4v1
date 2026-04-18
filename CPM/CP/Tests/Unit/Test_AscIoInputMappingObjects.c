/*
 * Tests/Unit/Test_AscIoInputMappingObjects.c
 *
 * Unit tests for canonical 1202 ascIOmapping input-side objects.
 */
#include "unity.h"

#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/NTCIP/NTCIP1202.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"
#include "MockConfigRepositoryAdapter.h"

#include <string.h>

static const uint32_t kAscIOmapMaxInputsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 2U, 0U
};
static const uint32_t kAscIOinputMapDeviceTypeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 4U, 1U, 3U, 1U, 1U
};
static const uint32_t kAscIOinputMapDevPinDescrOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 5U, 1U, 1U, 1U, 1U
};
static const uint32_t kAscIOinputMapDevPinStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 5U, 1U, 2U, 1U, 1U
};
static const uint32_t kAscIOmapMaxInputFunctionsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 9U, 1U, 0U
};
static const uint32_t kAscIOinputMaxFuncIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 9U, 2U, 1U, 2U, 34U
};
static const uint32_t kAscIOinputFunctionNameOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 9U, 2U, 1U, 3U, 34U
};

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_configurationService;
static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
static NtcipDbTransactionService_t s_dbTransactionService;
static NtcipContext_t s_ntcipContext;
static NtcipObjectDirectory_t s_directory;

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

void test_objects_report_input_map_capacity_and_default_unused_rows(void)
{
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOmapMaxInputsOid,
                                                13U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(64U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOinputMapDeviceTypeOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32((uint32_t) INTERSECTION_IO_MAP_DEVICE_UNUSED,
                           value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOinputMapDevPinDescrOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT16(0U, value.data.octetString.length);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOinputMapDevPinStatusOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);
}

void test_objects_report_standard_input_function_table(void)
{
  NtcipValue_t value;
  static const uint8_t kFunctionName[] = "pedestrianDetector";

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOmapMaxInputFunctionsOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(55U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOinputMaxFuncIndexOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(32U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kAscIOinputFunctionNameOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT16(sizeof(kFunctionName) - 1U,
                           value.data.octetString.length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(kFunctionName,
                                value.data.octetString.bytes,
                                sizeof(kFunctionName) - 1U);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_objects_report_input_map_capacity_and_default_unused_rows);
  RUN_TEST(test_objects_report_standard_input_function_table);

  return UNITY_END();
}
