/*
 * Tests/Integration/Test_NTCIP1202Annex4.c
 *
 * Host-side compliance checks for the implemented NTCIP 1202 Annex 4
 * behaviors: GET access, row bounds, transaction-enforced writes, rollback,
 * commit, and remote-write lock handling.
 */
#include "unity.h"

#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/NTCIP/NTCIP1202.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"
#include "MockConfigRepositoryAdapter.h"

#define DB_SCALAR_OID_LEN 13U
#define PHASE_COLUMN_OID_LEN 15U
#define UNIT_SCALAR_OID_LEN 13U

static const uint32_t kDbCreateTransactionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 1U, 0U
};
static const uint32_t kDbTransactionIdOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 4U, 0U
};
static const uint32_t kPhaseMinimumGreenOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 4U, 1U
};
static const uint32_t kPhaseMaximumInitialOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 12U, 1U
};
static const uint32_t kPhaseMinimumGreenInvalidRowOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 4U, 255U
};
static const uint32_t kUnitControlOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 10U, 0U
};

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_configService;
static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
static NtcipDbTransactionService_t s_dbTxService;
static NtcipObjectDirectory_t s_directory;
static NtcipContext_t s_context;

static void ReloadEngine(void)
{
  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(
                     &s_engine,
                     ConfigurationServiceGetActiveConfig(&s_configService)));
  IntersectionEngineTick(&s_engine);
}

static uint32_t GetUnsigned32(const uint32_t *oid, uint8_t oidLength)
{
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                oid,
                                                oidLength,
                                                NULL,
                                                &value));

  return value.data.unsigned32;
}

static NtcipError_t SetUnsigned32(const uint32_t *oid,
                                  uint8_t oidLength,
                                  const NtcipRequestContext_t *request,
                                  uint32_t value32)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, value32);

  return NtcipObjectDirectorySetValue(&s_directory,
                                      oid,
                                      oidLength,
                                      request,
                                      &value);
}

static NtcipError_t SetTestUnsigned32(const uint32_t *oid,
                                      uint8_t oidLength,
                                      const NtcipRequestContext_t *request,
                                      uint32_t value32)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, value32);

  return NtcipObjectDirectorySetTest(&s_directory,
                                     oid,
                                     oidLength,
                                     request,
                                     &value);
}

static void BeginTransaction(NtcipRequestContext_t *request,
                             uint8_t transactionId)
{
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        SetUnsigned32(kDbCreateTransactionOid,
                                      DB_SCALAR_OID_LEN,
                                      request,
                                      (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        SetUnsigned32(kDbTransactionIdOid,
                                      DB_SCALAR_OID_LEN,
                                      request,
                                      transactionId));
  request->transactionIdValid = 1U;
  request->transactionId = transactionId;
}

static void VerifyAndCommitTransaction(NtcipRequestContext_t *request)
{
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        SetUnsigned32(kDbCreateTransactionOid,
                                      DB_SCALAR_OID_LEN,
                                      request,
                                      (uint32_t) NTCIP_DB_CREATE_STATE_VERIFY));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        SetUnsigned32(kDbCreateTransactionOid,
                                      DB_SCALAR_OID_LEN,
                                      request,
                                      (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL));
  ReloadEngine();
}

static void RollbackTransaction(NtcipRequestContext_t *request)
{
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        SetUnsigned32(kDbCreateTransactionOid,
                                      DB_SCALAR_OID_LEN,
                                      request,
                                      (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL));
  ReloadEngine();
}

void setUp(void)
{
  MockConfigRepositoryAdapterInit(&s_repoCtx);
  s_repoPort = MockConfigRepositoryAdapterCreatePort(&s_repoCtx);
  ConfigurationServiceInit(&s_configService, &s_repoPort);
  IntersectionEngineInit(&s_engine);
  IntersectionControllerInit(&s_controller);
  NtcipDbTransactionServiceInit(&s_dbTxService, &s_configService);
  NtcipContextInit(&s_context,
                   &s_configService,
                   &s_engine,
                   &s_controller,
                   &s_dbTxService);
  NtcipObjectDirectoryInit(&s_directory);
  Ntcip1201RegisterObjects(&s_directory, &s_context);
  Ntcip1202RegisterObjects(&s_directory, &s_context);
  ReloadEngine();
}

void tearDown(void)
{
}

void test_annex4_get_returns_phase_value_in_mib_units(void)
{
  TEST_ASSERT_EQUAL_UINT32(5U,
                           GetUnsigned32(kPhaseMinimumGreenOid,
                                         PHASE_COLUMN_OID_LEN));
}

void test_annex4_phase_row_out_of_range_returns_range_error(void)
{
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseMinimumGreenInvalidRowOid,
                                                PHASE_COLUMN_OID_LEN,
                                                NULL,
                                                &value));
}

void test_annex4_write_requires_database_transaction(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x4001U, 0U, 0U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_TRANSACTION,
                        SetTestUnsigned32(kPhaseMinimumGreenOid,
                                          PHASE_COLUMN_OID_LEN,
                                          &request,
                                          12U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_TRANSACTION,
                        SetUnsigned32(kPhaseMinimumGreenOid,
                                      PHASE_COLUMN_OID_LEN,
                                      &request,
                                      12U));
}

void test_annex4_settest_rejects_phase_value_outside_supported_range(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x4002U, 0U, 0U);

  BeginTransaction(&request, 7U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        SetTestUnsigned32(kPhaseMinimumGreenOid,
                                          PHASE_COLUMN_OID_LEN,
                                          &request,
                                          256U));
}

void test_annex4_verify_and_commit_persists_phase_write(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x4003U, 0U, 0U);

  BeginTransaction(&request, 8U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        SetTestUnsigned32(kPhaseMaximumInitialOid,
                                          PHASE_COLUMN_OID_LEN,
                                          &request,
                                          13U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        SetUnsigned32(kPhaseMaximumInitialOid,
                                      PHASE_COLUMN_OID_LEN,
                                      &request,
                                      13U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        SetTestUnsigned32(kPhaseMinimumGreenOid,
                                          PHASE_COLUMN_OID_LEN,
                                          &request,
                                          13U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        SetUnsigned32(kPhaseMinimumGreenOid,
                                      PHASE_COLUMN_OID_LEN,
                                      &request,
                                      13U));
  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_UINT32(13U,
                           GetUnsigned32(kPhaseMinimumGreenOid,
                                         PHASE_COLUMN_OID_LEN));
}

void test_annex4_rollback_preserves_active_configuration(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x4004U, 0U, 0U);

  TEST_ASSERT_EQUAL_UINT32(5U,
                           GetUnsigned32(kPhaseMinimumGreenOid,
                                         PHASE_COLUMN_OID_LEN));

  BeginTransaction(&request, 9U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        SetUnsigned32(kPhaseMinimumGreenOid,
                                      PHASE_COLUMN_OID_LEN,
                                      &request,
                                      17U));
  RollbackTransaction(&request);

  TEST_ASSERT_EQUAL_UINT32(5U,
                           GetUnsigned32(kPhaseMinimumGreenOid,
                                         PHASE_COLUMN_OID_LEN));
}

void test_annex4_remote_write_lock_blocks_phase_set_until_unlocked(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x4005U, 0U, 0U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        SetUnsigned32(kUnitControlOid,
                                      UNIT_SCALAR_OID_LEN,
                                      NULL,
                                      0x02U));
  TEST_ASSERT_EQUAL_UINT32(0x02U,
                           GetUnsigned32(kUnitControlOid, UNIT_SCALAR_OID_LEN));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_ACCESS,
                        SetUnsigned32(kPhaseMinimumGreenOid,
                                      PHASE_COLUMN_OID_LEN,
                                      &request,
                                      12U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_ACCESS,
                        SetUnsigned32(kUnitControlOid,
                                      UNIT_SCALAR_OID_LEN,
                                      NULL,
                                      0x06U));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        SetUnsigned32(kUnitControlOid,
                                      UNIT_SCALAR_OID_LEN,
                                      NULL,
                                      0x00U));
  TEST_ASSERT_EQUAL_UINT32(0x00U,
                           GetUnsigned32(kUnitControlOid, UNIT_SCALAR_OID_LEN));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_NO_TRANSACTION,
                        SetUnsigned32(kPhaseMinimumGreenOid,
                                      PHASE_COLUMN_OID_LEN,
                                      &request,
                                      12U));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_annex4_get_returns_phase_value_in_mib_units);
  RUN_TEST(test_annex4_phase_row_out_of_range_returns_range_error);
  RUN_TEST(test_annex4_write_requires_database_transaction);
  RUN_TEST(test_annex4_settest_rejects_phase_value_outside_supported_range);
  RUN_TEST(test_annex4_verify_and_commit_persists_phase_write);
  RUN_TEST(test_annex4_rollback_preserves_active_configuration);
  RUN_TEST(test_annex4_remote_write_lock_blocks_phase_set_until_unlocked);
  return UNITY_END();
}
