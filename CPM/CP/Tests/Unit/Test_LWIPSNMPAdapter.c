/*
 * Tests/Unit/Test_LWIPSNMPAdapter.c
 *
 * Unit tests for the lwIP-facing adapter around managed OID detection and
 * stable transaction context reconstruction.
 */
#include "unity.h"

#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Domain/Services/EventReportService.h"
#include "LWIPSNMPAdapter.h"
#include "MockConfigRepositoryAdapter.h"
#include "MockSnmpSecurityAdapter.h"

static const uint32_t kDbCreateTransactionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 1U, 0U
};
static const uint32_t kDbTransactionIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 4U, 0U
};
static const uint32_t kPhaseMinimumGreenOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 4U, 1U
};
static const uint32_t kPhaseMinimumGreenInvalidRowOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 4U, 99U
};
static const uint32_t kUnknownOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 99U, 1U, 0U
};
static const uint32_t kEventSourcePowerOnCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 1U, 0U
};

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static MockSnmpSecurityAdapterCtx_t s_snmpSecurityCtx;
static ISnmpSecurityPort_t s_snmpSecurityPort;
static ConfigurationService_t s_configService;
static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
static EventReportService_t s_eventReportService;
static LWIPSNMPAdapterCtx_t s_adapter;

static void SetUnsigned32(const uint32_t *oid,
                          uint8_t oidLength,
                          NtcipRequestContext_t *request,
                          uint32_t value32)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, value32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        LWIPSNMPAdapterSetValue(&s_adapter,
                                                oid,
                                                oidLength,
                                                request,
                                                &value));
}

void setUp(void)
{
  MockConfigRepositoryAdapterInit(&s_repoCtx);
  s_repoPort = MockConfigRepositoryAdapterCreatePort(&s_repoCtx);
  MockSnmpSecurityAdapterInit(&s_snmpSecurityCtx);
  s_snmpSecurityPort = MockSnmpSecurityAdapterCreatePort(&s_snmpSecurityCtx);
  ConfigurationServiceInit(&s_configService, &s_repoPort);
  IntersectionEngineInit(&s_engine);
  IntersectionControllerInit(&s_controller);
  EventReportServiceInit(&s_eventReportService);
  LWIPSNMPAdapterInit(&s_adapter,
                      &s_configService,
                      &s_engine,
                      &s_controller);
  LWIPSNMPAdapterBindEventReportService(&s_adapter, &s_eventReportService);
  LWIPSNMPAdapterBindSnmpSecurityPort(&s_adapter, &s_snmpSecurityPort);
}

void tearDown(void)
{
}

void test_adapter_classifies_exact_prefix_and_unmanaged_oids(void)
{
  TEST_ASSERT_EQUAL_INT(LWIP_SNMP_MANAGED_STATE_EXACT,
                        LWIPSNMPAdapterGetManagedState(&s_adapter,
                                                      kPhaseMinimumGreenOid,
                                                      15U,
                                                      NULL));
  TEST_ASSERT_EQUAL_INT(LWIP_SNMP_MANAGED_STATE_PREFIX_ONLY,
                        LWIPSNMPAdapterGetManagedState(
                          &s_adapter,
                          kPhaseMinimumGreenInvalidRowOid,
                          15U,
                          NULL));
  TEST_ASSERT_EQUAL_INT(LWIP_SNMP_MANAGED_STATE_EXACT,
                        LWIPSNMPAdapterGetManagedState(
                          &s_adapter,
                          kEventSourcePowerOnCountOid,
                          13U,
                          NULL));
  TEST_ASSERT_EQUAL_INT(LWIP_SNMP_MANAGED_STATE_UNMANAGED,
                        LWIPSNMPAdapterGetManagedState(&s_adapter,
                                                      kUnknownOid,
                                                      12U,
                                                      NULL));
}

void test_adapter_rebuilds_transaction_context_per_session(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x1111U, 0U, 0U);
  NtcipRequestContext_t rebuilt;
  NtcipRequestContext_t otherSession;

  SetUnsigned32(kDbCreateTransactionOid,
                13U,
                &request,
                (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION);
  SetUnsigned32(kDbTransactionIdOid, 13U, &request, 37U);

  LWIPSNMPAdapterBuildRequestContext(&s_adapter, 0x1111U, &rebuilt);
  TEST_ASSERT_EQUAL_UINT32(0x1111U, rebuilt.sessionKey);
  TEST_ASSERT_EQUAL_UINT8(1U, rebuilt.transactionIdValid);
  TEST_ASSERT_EQUAL_UINT8(37U, rebuilt.transactionId);

  LWIPSNMPAdapterBuildRequestContext(&s_adapter, 0x2222U, &otherSession);
  TEST_ASSERT_EQUAL_UINT32(0x2222U, otherSession.sessionKey);
  TEST_ASSERT_EQUAL_UINT8(0U, otherSession.transactionIdValid);
}

void test_adapter_clears_cached_transaction_id_when_transaction_returns_normal(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x1111U, 0U, 0U);
  NtcipRequestContext_t rebuilt;

  SetUnsigned32(kDbCreateTransactionOid,
                13U,
                &request,
                (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION);
  SetUnsigned32(kDbTransactionIdOid, 13U, &request, 44U);

  LWIPSNMPAdapterBuildRequestContext(&s_adapter, 0x1111U, &rebuilt);
  TEST_ASSERT_EQUAL_UINT8(1U, rebuilt.transactionIdValid);
  TEST_ASSERT_EQUAL_UINT8(44U, rebuilt.transactionId);

  SetUnsigned32(kDbCreateTransactionOid,
                13U,
                &rebuilt,
                (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL);

  LWIPSNMPAdapterBuildRequestContext(&s_adapter, 0x1111U, &rebuilt);
  TEST_ASSERT_EQUAL_UINT8(0U, rebuilt.transactionIdValid);
}

void test_adapter_drives_event_report_transaction_state(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x3333U, 0U, 0U);
  NtcipRequestContext_t rebuilt;

  SetUnsigned32(kDbCreateTransactionOid,
                13U,
                &request,
                (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION);
  TEST_ASSERT_EQUAL_UINT8(1U, s_eventReportService.transactionActive);
  TEST_ASSERT_EQUAL_UINT8(0U, s_eventReportService.transactionVerified);

  LWIPSNMPAdapterBuildRequestContext(&s_adapter, 0x3333U, &rebuilt);
  SetUnsigned32(kDbCreateTransactionOid,
                13U,
                &rebuilt,
                (uint32_t) NTCIP_DB_CREATE_STATE_VERIFY);
  TEST_ASSERT_EQUAL_UINT8(1U, s_eventReportService.transactionActive);
  TEST_ASSERT_EQUAL_UINT8(1U, s_eventReportService.transactionVerified);
}

void test_adapter_persists_committed_event_report_communities(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x4444U, 0U, 0U);
  NtcipRequestContext_t rebuilt;
  NtcipValue_t value;

  SetUnsigned32(kDbCreateTransactionOid,
                13U,
                &request,
                (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION);
  SetUnsigned32(kDbTransactionIdOid, 13U, &request, 9U);
  request.transactionIdValid = 1U;
  request.transactionId = 9U;
  request.authModel = (uint8_t) NTCIP_AUTH_MODEL_LOCAL;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetCString(&value, "READCOMM"));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        LWIPSNMPAdapterSetValue(
                          &s_adapter,
                          (const uint32_t[])
                          {
                            1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 5U,
                            3U, 1U, 2U, 1U
                          },
                          15U,
                          &request,
                          &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetCString(&value, "WRITECOM"));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        LWIPSNMPAdapterSetValue(
                          &s_adapter,
                          (const uint32_t[])
                          {
                            1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 5U,
                            3U, 1U, 2U, 2U
                          },
                          15U,
                          &request,
                          &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetCString(&value, "TRAPCOMM"));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        LWIPSNMPAdapterSetValue(
                          &s_adapter,
                          (const uint32_t[])
                          {
                            1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 5U,
                            3U, 1U, 2U, 3U
                          },
                          15U,
                          &request,
                          &value));

  LWIPSNMPAdapterBuildRequestContext(&s_adapter, 0x4444U, &rebuilt);
  rebuilt.authModel = (uint8_t) NTCIP_AUTH_MODEL_LOCAL;
  SetUnsigned32(kDbCreateTransactionOid,
                13U,
                &rebuilt,
                (uint32_t) NTCIP_DB_CREATE_STATE_VERIFY);
  SetUnsigned32(kDbCreateTransactionOid,
                13U,
                &rebuilt,
                (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL);

  TEST_ASSERT_EQUAL_UINT32(1U, s_snmpSecurityCtx.setV2cCount);
  TEST_ASSERT_EQUAL_STRING("READCOMM", &s_snmpSecurityCtx.readCommunity[0]);
  TEST_ASSERT_EQUAL_STRING("WRITECOM", &s_snmpSecurityCtx.writeCommunity[0]);
  TEST_ASSERT_EQUAL_STRING("TRAPCOMM", &s_snmpSecurityCtx.trapCommunity[0]);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_adapter_classifies_exact_prefix_and_unmanaged_oids);
  RUN_TEST(test_adapter_rebuilds_transaction_context_per_session);
  RUN_TEST(test_adapter_clears_cached_transaction_id_when_transaction_returns_normal);
  RUN_TEST(test_adapter_drives_event_report_transaction_state);
  RUN_TEST(test_adapter_persists_committed_event_report_communities);
  return UNITY_END();
}
