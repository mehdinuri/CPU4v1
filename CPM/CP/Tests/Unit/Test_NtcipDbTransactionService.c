/*
 * Tests/Unit/Test_NtcipDbTransactionService.c
 *
 * Unit tests for the 1201 DB-management state machine.
 */
#include "unity.h"

#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "MockConfigRepositoryAdapter.h"

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_configurationService;
static NtcipDbTransactionService_t s_dbService;
static uint8_t s_commitObserverCount;

static void OnCommit(void *ctx)
{
  uint8_t *count = (uint8_t *) ctx;

  if (count != NULL)
  {
    (*count)++;
  }
}

void setUp(void)
{
  MockConfigRepositoryAdapterInit(&s_repoCtx);
  s_repoPort = MockConfigRepositoryAdapterCreatePort(&s_repoCtx);
  ConfigurationServiceInit(&s_configurationService, &s_repoPort);
  NtcipDbTransactionServiceInit(&s_dbService, &s_configurationService);
  s_commitObserverCount = 0U;
}

void tearDown(void)
{
}

void test_db_make_id_increments_on_every_read(void)
{
  TEST_ASSERT_EQUAL_UINT8(1U,
                          NtcipDbTransactionServiceReadMakeId(&s_dbService));
  TEST_ASSERT_EQUAL_UINT8(2U,
                          NtcipDbTransactionServiceReadMakeId(&s_dbService));
}

void test_db_transaction_id_requires_owner_session(void)
{
  NtcipRequestContext_t ownerRequest = { 0x1234U, 0U, 0U };
  NtcipRequestContext_t otherRequest = { 0x5678U, 0U, 0U };

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipDbTransactionServiceSetCreateTransactionValue(
                          &s_dbService,
                          &ownerRequest,
                          (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OWNER_MISMATCH,
                        NtcipDbTransactionServiceSetTransactionIdTest(
                          &s_dbService,
                          &otherRequest,
                          11U));
}

void test_db_verify_to_done_then_normal_commits_candidate(void)
{
  NtcipRequestContext_t ownerRequest = { 0x1234U, 1U, 77U };
  IntersectionPhaseConfig_t phase;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipDbTransactionServiceSetCreateTransactionValue(
                          &s_dbService,
                          &ownerRequest,
                          (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION));
  ownerRequest.transactionIdValid = 0U;
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipDbTransactionServiceSetTransactionIdValue(
                          &s_dbService,
                          &ownerRequest,
                          77U));
  ownerRequest.transactionIdValid = 1U;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipDbTransactionServiceValidateDatabaseWrite(
                          &s_dbService,
                          &ownerRequest));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseMinGreenDs(
                     &s_configurationService,
                     0U,
                     60U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipDbTransactionServiceSetCreateTransactionValue(
                          &s_dbService,
                          &ownerRequest,
                          (uint32_t) NTCIP_DB_CREATE_STATE_VERIFY));
  TEST_ASSERT_EQUAL_INT(NTCIP_DB_CREATE_STATE_DONE,
                        NtcipDbTransactionServiceGetState(&s_dbService));
  TEST_ASSERT_EQUAL_INT(NTCIP_DB_VERIFY_STATUS_DONE_WITH_NO_ERROR,
                        NtcipDbTransactionServiceGetVerifyStatus(&s_dbService));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipDbTransactionServiceSetCreateTransactionValue(
                          &s_dbService,
                          &ownerRequest,
                          (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL));
  TEST_ASSERT_TRUE(ConfigurationServiceGetActivePhaseConfig(
                     &s_configurationService,
                     0U,
                     &phase));
  TEST_ASSERT_EQUAL_UINT16(60U, phase.minGreenDs);
  TEST_ASSERT_EQUAL_INT(NTCIP_DB_CREATE_STATE_NORMAL,
                        NtcipDbTransactionServiceGetState(&s_dbService));
} /* test_db_verify_to_done_then_normal_commits_candidate */

void test_db_commit_notifies_observer_after_successful_commit(void)
{
  NtcipRequestContext_t ownerRequest = { 0x1234U, 1U, 21U };

  NtcipDbTransactionServiceBindCommitObserver(&s_dbService,
                                              OnCommit,
                                              &s_commitObserverCount);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipDbTransactionServiceSetCreateTransactionValue(
                          &s_dbService,
                          &ownerRequest,
                          (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION));
  ownerRequest.transactionIdValid = 0U;
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipDbTransactionServiceSetTransactionIdValue(
                          &s_dbService,
                          &ownerRequest,
                          21U));
  ownerRequest.transactionIdValid = 1U;
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseMinGreenDs(&s_configurationService,
                                                          0U,
                                                          65U));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipDbTransactionServiceSetCreateTransactionValue(
                          &s_dbService,
                          &ownerRequest,
                          (uint32_t) NTCIP_DB_CREATE_STATE_VERIFY));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipDbTransactionServiceSetCreateTransactionValue(
                          &s_dbService,
                          &ownerRequest,
                          (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL));
  TEST_ASSERT_EQUAL_UINT8(1U, s_commitObserverCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_db_make_id_increments_on_every_read);
  RUN_TEST(test_db_transaction_id_requires_owner_session);
  RUN_TEST(test_db_verify_to_done_then_normal_commits_candidate);
  RUN_TEST(test_db_commit_notifies_observer_after_successful_commit);

  return UNITY_END();
}
