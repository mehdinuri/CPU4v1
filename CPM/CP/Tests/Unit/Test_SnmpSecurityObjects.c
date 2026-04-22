/* Tests/Unit/Test_SnmpSecurityObjects.c
 *
 * Focused coverage for release-policy access control on NTCIP 1103 security
 * objects and the Teknotel vendor-private SNMPv3 credential rotation objects.
 */
#include "unity.h"

#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NTCIP1103.h"
#include "Domain/NTCIP/MibVendor59748/UnitObjects.h"
#include "Domain/NTCIP/NtcipContext.h"
#include "Domain/Services/EventReportService.h"
#include "MockConfigRepositoryAdapter.h"
#include "MockSnmpSecurityAdapter.h"

static const uint32_t kCommunityNameUserRow1Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 5U, 3U, 1U, 2U, 1U
};
static const uint32_t kUnitSnmpV3ActiveUsernameOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 3U, 2U, 0U
};
static const uint32_t kUnitSnmpV3NewAuthPassphraseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 3U, 3U, 0U
};
static const uint32_t kUnitSnmpV3ApplyOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 3U, 5U, 0U
};

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_configService;
static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
static NtcipDbTransactionService_t s_dbService;
static EventReportService_t s_eventReportService;
static MockSnmpSecurityAdapterCtx_t s_snmpSecurityCtx;
static ISnmpSecurityPort_t s_snmpSecurityPort;
static NtcipObjectDirectory_t s_directory;
static NtcipContext_t s_context;

static void BeginTransaction(NtcipRequestContext_t *request,
                             uint8_t transactionId)
{
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipDbTransactionServiceSetCreateTransactionValue(
                          &s_dbService,
                          request,
                          (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipDbTransactionServiceSetTransactionIdValue(
                          &s_dbService,
                          request,
                          transactionId));
  request->transactionIdValid = 1U;
  request->transactionId = transactionId;
}

static void SetOctetString(const uint32_t *oid,
                           uint8_t oidLength,
                           const NtcipRequestContext_t *request,
                           const char *text)
{
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetCString(&value, text));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     oid,
                                                     oidLength,
                                                     request,
                                                     &value));
}

static void SetUnsigned32(const uint32_t *oid,
                          uint8_t oidLength,
                          const NtcipRequestContext_t *request,
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
  MockConfigRepositoryAdapterInit(&s_repoCtx);
  s_repoPort = MockConfigRepositoryAdapterCreatePort(&s_repoCtx);
  ConfigurationServiceInit(&s_configService, &s_repoPort);
  IntersectionEngineInit(&s_engine);
  IntersectionControllerInit(&s_controller);
  NtcipDbTransactionServiceInit(&s_dbService, &s_configService);
  EventReportServiceInit(&s_eventReportService);
  MockSnmpSecurityAdapterInit(&s_snmpSecurityCtx);
  s_snmpSecurityPort = MockSnmpSecurityAdapterCreatePort(&s_snmpSecurityCtx);

  NtcipContextInit(&s_context,
                   &s_configService,
                   &s_engine,
                   &s_controller,
                   &s_dbService);
  NtcipContextBindEventReportService(&s_context, &s_eventReportService);
  NtcipContextBindSnmpSecurityPort(&s_context, &s_snmpSecurityPort);

  NtcipObjectDirectoryInit(&s_directory);
  Ntcip1103RegisterObjects(&s_directory, &s_context);
  TeknotelUnitObjectsRegister(&s_directory, &s_context);
}

void tearDown(void)
{
}

void test_release_policy_hides_community_name_rows_from_v2c(void)
{
  NtcipRequestContext_t request = NTCIP_REQUEST_CONTEXT_INIT(0x1111U, 0U, 0U);
  NtcipValue_t value;

  s_snmpSecurityCtx.strictReleasePolicy = 1U;
  request.authModel = (uint8_t) NTCIP_AUTH_MODEL_SNMP_V2C;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_ACCESS,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kCommunityNameUserRow1Oid,
                                                15U,
                                                &request,
                                                &value));

  request.authModel = (uint8_t) NTCIP_AUTH_MODEL_LOCAL;
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kCommunityNameUserRow1Oid,
                                                15U,
                                                &request,
                                                &value));
  TEST_ASSERT_EQUAL_UINT8(6U, value.data.octetString.length);
}

void test_release_policy_blocks_v2c_security_writes(void)
{
  NtcipRequestContext_t request = NTCIP_REQUEST_CONTEXT_INIT(0x2222U, 0U, 0U);
  NtcipValue_t value;

  s_snmpSecurityCtx.strictReleasePolicy = 1U;
  request.authModel = (uint8_t) NTCIP_AUTH_MODEL_SNMP_V2C;
  BeginTransaction(&request, 11U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetCString(&value, "private2"));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_ACCESS,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kCommunityNameUserRow1Oid,
                                                    15U,
                                                    &request,
                                                    &value));
}

void test_vendor_v3_apply_reuses_single_passphrase(void)
{
  NtcipRequestContext_t request = NTCIP_REQUEST_CONTEXT_INIT(0x3333U, 0U, 0U);
  NtcipValue_t value;

  s_snmpSecurityCtx.strictReleasePolicy = 1U;
  request.authModel = (uint8_t) NTCIP_AUTH_MODEL_SNMP_V3;
  request.securityLevel = (uint8_t) NTCIP_SECURITY_LEVEL_AUTH_PRIV;
  BeginTransaction(&request, 12U);

  SetOctetString(kUnitSnmpV3ActiveUsernameOid,
                 13U,
                 &request,
                 "operator1");
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitSnmpV3ActiveUsernameOid,
                                                13U,
                                                &request,
                                                &value));
  TEST_ASSERT_EQUAL_UINT8(9U, value.data.octetString.length);

  SetOctetString(kUnitSnmpV3NewAuthPassphraseOid,
                 13U,
                 &request,
                 "NEWPASS123");
  SetUnsigned32(kUnitSnmpV3ApplyOid, 13U, &request, 1U);

  TEST_ASSERT_EQUAL_UINT32(1U, s_snmpSecurityCtx.setV3CredentialsCount);
  TEST_ASSERT_EQUAL_STRING("operator1", &s_snmpSecurityCtx.username[0]);
  TEST_ASSERT_EQUAL_STRING("NEWPASS123", &s_snmpSecurityCtx.authPassphrase[0]);
  TEST_ASSERT_EQUAL_STRING("NEWPASS123", &s_snmpSecurityCtx.privPassphrase[0]);
}

void test_release_policy_blocks_v2c_vendor_security_writes(void)
{
  NtcipRequestContext_t request = NTCIP_REQUEST_CONTEXT_INIT(0x4444U, 0U, 0U);
  NtcipValue_t value;

  s_snmpSecurityCtx.strictReleasePolicy = 1U;
  request.authModel = (uint8_t) NTCIP_AUTH_MODEL_SNMP_V2C;
  BeginTransaction(&request, 13U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetCString(&value, "operator2"));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_ACCESS,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kUnitSnmpV3ActiveUsernameOid,
                                                    13U,
                                                    &request,
                                                    &value));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_release_policy_hides_community_name_rows_from_v2c);
  RUN_TEST(test_release_policy_blocks_v2c_security_writes);
  RUN_TEST(test_vendor_v3_apply_reuses_single_passphrase);
  RUN_TEST(test_release_policy_blocks_v2c_vendor_security_writes);
  return UNITY_END();
}
