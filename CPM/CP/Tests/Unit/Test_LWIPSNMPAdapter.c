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
#include "LWIPSNMPAdapter.h"
#include "MockConfigRepositoryAdapter.h"

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

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_configService;
static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
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
  ConfigurationServiceInit(&s_configService, &s_repoPort);
  IntersectionEngineInit(&s_engine);
  IntersectionControllerInit(&s_controller);
  LWIPSNMPAdapterInit(&s_adapter,
                      &s_configService,
                      &s_engine,
                      &s_controller);
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
  TEST_ASSERT_EQUAL_INT(LWIP_SNMP_MANAGED_STATE_UNMANAGED,
                        LWIPSNMPAdapterGetManagedState(&s_adapter,
                                                      kUnknownOid,
                                                      12U,
                                                      NULL));
}

void test_adapter_rebuilds_transaction_context_per_session(void)
{
  NtcipRequestContext_t request = { 0x1111U, 0U, 0U };
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
  NtcipRequestContext_t request = { 0x1111U, 0U, 0U };
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

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_adapter_classifies_exact_prefix_and_unmanaged_oids);
  RUN_TEST(test_adapter_rebuilds_transaction_context_per_session);
  RUN_TEST(test_adapter_clears_cached_transaction_id_when_transaction_returns_normal);
  return UNITY_END();
}
