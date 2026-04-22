/*
 * Tests/Unit/Test_PhaseObjects.c
 *
 * Focused compliance tests for the NTCIP 1202 phase table group.
 */
#include "unity.h"

#include <string.h>

#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/NTCIP/NTCIP1202.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"
#include "MockConfigRepositoryAdapter.h"

#define PHASE_OID_LEN 15U
#define PHASE_BASE 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U

static const uint32_t kDbCreateTransactionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 1U, 0U
};
static const uint32_t kDbTransactionIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 4U, 0U
};
static const uint32_t kPhaseOptionsOid[] = { PHASE_BASE, 21U, 1U };
static const uint32_t kPhaseMaximum2Oid[] = { PHASE_BASE, 7U, 1U };
static const uint32_t kPhaseRedRevertOid[] = { PHASE_BASE, 10U, 1U };
static const uint32_t kPhaseAddedInitialOid[] = { PHASE_BASE, 11U, 1U };
static const uint32_t kPhaseTimeBeforeReductionOid[] = { PHASE_BASE, 13U, 1U };
static const uint32_t kPhaseCarsBeforeReductionOid[] = { PHASE_BASE, 14U, 1U };
static const uint32_t kPhaseTimeToReduceOid[] = { PHASE_BASE, 15U, 1U };
static const uint32_t kPhaseReduceByOid[] = { PHASE_BASE, 16U, 1U };
static const uint32_t kPhaseMinimumGapOid[] = { PHASE_BASE, 17U, 1U };
static const uint32_t kPhaseDynamicMaxLimitOid[] = { PHASE_BASE, 18U, 1U };
static const uint32_t kPhaseDynamicMaxStepOid[] = { PHASE_BASE, 19U, 1U };
static const uint32_t kPhaseStartupOid[] = { PHASE_BASE, 20U, 1U };
static const uint32_t kPhaseRingOid[] = { PHASE_BASE, 22U, 1U };
static const uint32_t kPhaseConcurrencyOid[] = { PHASE_BASE, 23U, 1U };
static const uint32_t kPhaseMaximum3Oid[] = { PHASE_BASE, 24U, 1U };
static const uint32_t kPhaseYellowRedBeforeEndPedClearOid[] =
{
  PHASE_BASE, 25U, 1U
};
static const uint32_t kPhasePedWalkServiceOid[] = { PHASE_BASE, 26U, 1U };
static const uint32_t kPhaseDontWalkRevertOid[] = { PHASE_BASE, 27U, 1U };
static const uint32_t kPhasePedAlternateClearanceOid[] =
{
  PHASE_BASE, 28U, 1U
};
static const uint32_t kPhasePedAlternateWalkOid[] = { PHASE_BASE, 29U, 1U };
static const uint32_t kPhasePedAdvanceWalkOid[] = { PHASE_BASE, 30U, 1U };
static const uint32_t kPhasePedDelayOid[] = { PHASE_BASE, 31U, 1U };
static const uint32_t kPhaseAdvWarnGrnStartTimeOid[] =
{
  PHASE_BASE, 32U, 1U
};
static const uint32_t kPhaseAdvWarnRedStartTimeOid[] =
{
  PHASE_BASE, 33U, 1U
};
static const uint32_t kPhaseAltMinTimeTransitionOid[] =
{
  PHASE_BASE, 34U, 1U
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

static void BeginTx(NtcipRequestContext_t *request, uint8_t transactionId)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, transactionId);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbTransactionIdOid,
                                                     13U,
                                                     request,
                                                     &value));
  request->transactionIdValid = 1U;
  request->transactionId = transactionId;
}

static void CommitTx(NtcipRequestContext_t *request)
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

static void SetPhaseUnsigned32(const uint32_t *oid,
                               NtcipRequestContext_t *request,
                               uint32_t value32)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, value32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     oid,
                                                     PHASE_OID_LEN,
                                                     request,
                                                     &value));
}

static uint32_t GetPhaseUnsigned32(const uint32_t *oid)
{
  NtcipValue_t value;

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                oid,
                                                PHASE_OID_LEN,
                                                NULL,
                                                &value));

  return value.data.unsigned32;
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

void test_phase_options_roundtrip_full_bitmask(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x1000U, 0U, 0U);
  uint32_t options =
    (uint32_t) (PHASE_OPTIONS_ENABLED
                | PHASE_OPTIONS_MIN_RECALL
                | PHASE_OPTIONS_MAX_RECALL
                | PHASE_OPTIONS_GUARANTEED_PASS);

  BeginTx(&request, 1U);
  SetPhaseUnsigned32(kPhaseOptionsOid, &request, options);
  CommitTx(&request);

  TEST_ASSERT_EQUAL_UINT32(options, GetPhaseUnsigned32(kPhaseOptionsOid));
}

void test_extended_phase_columns_roundtrip_in_mib_units(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x1001U, 0U, 0U);

  BeginTx(&request, 2U);
  SetPhaseUnsigned32(kPhaseMaximum2Oid, &request, 55U);
  SetPhaseUnsigned32(kPhaseRedRevertOid, &request, 7U);
  SetPhaseUnsigned32(kPhaseAddedInitialOid, &request, 9U);
  SetPhaseUnsigned32(kPhaseTimeBeforeReductionOid, &request, 12U);
  SetPhaseUnsigned32(kPhaseCarsBeforeReductionOid, &request, 5U);
  SetPhaseUnsigned32(kPhaseTimeToReduceOid, &request, 9U);
  SetPhaseUnsigned32(kPhaseReduceByOid, &request, 4U);
  SetPhaseUnsigned32(kPhaseMinimumGapOid, &request, 11U);
  SetPhaseUnsigned32(kPhaseDynamicMaxLimitOid, &request, 67U);
  SetPhaseUnsigned32(kPhaseDynamicMaxStepOid, &request, 14U);
  SetPhaseUnsigned32(kPhaseMaximum3Oid, &request, 444U);
  SetPhaseUnsigned32(kPhaseYellowRedBeforeEndPedClearOid, &request, 13U);
  SetPhaseUnsigned32(kPhasePedWalkServiceOid, &request, 2U);
  SetPhaseUnsigned32(kPhaseDontWalkRevertOid, &request, 6U);
  SetPhaseUnsigned32(kPhasePedAlternateClearanceOid, &request, 19U);
  SetPhaseUnsigned32(kPhasePedAlternateWalkOid, &request, 21U);
  SetPhaseUnsigned32(kPhasePedAdvanceWalkOid, &request, 8U);
  SetPhaseUnsigned32(kPhasePedDelayOid, &request, 3U);
  SetPhaseUnsigned32(kPhaseAdvWarnGrnStartTimeOid, &request, 12U);
  SetPhaseUnsigned32(kPhaseAdvWarnRedStartTimeOid, &request, 18U);
  SetPhaseUnsigned32(kPhaseAltMinTimeTransitionOid, &request, 25U);
  CommitTx(&request);

  TEST_ASSERT_EQUAL_UINT32(55U, GetPhaseUnsigned32(kPhaseMaximum2Oid));
  TEST_ASSERT_EQUAL_UINT32(7U, GetPhaseUnsigned32(kPhaseRedRevertOid));
  TEST_ASSERT_EQUAL_UINT32(9U, GetPhaseUnsigned32(kPhaseAddedInitialOid));
  TEST_ASSERT_EQUAL_UINT32(12U,
                           GetPhaseUnsigned32(kPhaseTimeBeforeReductionOid));
  TEST_ASSERT_EQUAL_UINT32(5U,
                           GetPhaseUnsigned32(kPhaseCarsBeforeReductionOid));
  TEST_ASSERT_EQUAL_UINT32(9U, GetPhaseUnsigned32(kPhaseTimeToReduceOid));
  TEST_ASSERT_EQUAL_UINT32(4U, GetPhaseUnsigned32(kPhaseReduceByOid));
  TEST_ASSERT_EQUAL_UINT32(11U, GetPhaseUnsigned32(kPhaseMinimumGapOid));
  TEST_ASSERT_EQUAL_UINT32(67U, GetPhaseUnsigned32(kPhaseDynamicMaxLimitOid));
  TEST_ASSERT_EQUAL_UINT32(14U, GetPhaseUnsigned32(kPhaseDynamicMaxStepOid));
  TEST_ASSERT_EQUAL_UINT32(444U, GetPhaseUnsigned32(kPhaseMaximum3Oid));
  TEST_ASSERT_EQUAL_UINT32(13U,
                           GetPhaseUnsigned32(
                             kPhaseYellowRedBeforeEndPedClearOid));
  TEST_ASSERT_EQUAL_UINT32(2U, GetPhaseUnsigned32(kPhasePedWalkServiceOid));
  TEST_ASSERT_EQUAL_UINT32(6U, GetPhaseUnsigned32(kPhaseDontWalkRevertOid));
  TEST_ASSERT_EQUAL_UINT32(19U,
                           GetPhaseUnsigned32(kPhasePedAlternateClearanceOid));
  TEST_ASSERT_EQUAL_UINT32(21U,
                           GetPhaseUnsigned32(kPhasePedAlternateWalkOid));
  TEST_ASSERT_EQUAL_UINT32(8U, GetPhaseUnsigned32(kPhasePedAdvanceWalkOid));
  TEST_ASSERT_EQUAL_UINT32(3U, GetPhaseUnsigned32(kPhasePedDelayOid));
  TEST_ASSERT_EQUAL_UINT32(12U,
                           GetPhaseUnsigned32(kPhaseAdvWarnGrnStartTimeOid));
  TEST_ASSERT_EQUAL_UINT32(18U,
                           GetPhaseUnsigned32(kPhaseAdvWarnRedStartTimeOid));
  TEST_ASSERT_EQUAL_UINT32(25U,
                           GetPhaseUnsigned32(kPhaseAltMinTimeTransitionOid));
}

void test_phase_startup_configuration_is_applied_on_engine_reset(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x1002U, 0U, 0U);

  BeginTx(&request, 3U);
  SetPhaseUnsigned32(kPhaseStartupOid,
                     &request,
                     (uint32_t) INTERSECTION_PHASE_STARTUP_YELLOW_CHANGE);
  CommitTx(&request);

  TEST_ASSERT_EQUAL_UINT32((uint32_t) INTERSECTION_PHASE_STARTUP_YELLOW_CHANGE,
                           GetPhaseUnsigned32(kPhaseStartupOid));
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_PHASE_INTERVAL_YELLOW,
                          IntersectionEngineGetRuntime(&s_engine)->phases[0].
                          interval);
}

void test_phase_concurrency_roundtrips_octet_string(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x1003U, 0U, 0U);
  NtcipValue_t value;
  const uint8_t concurrency[] = { 5U, 6U };

  BeginTx(&request, 4U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetOctetString(&value,
                                                 concurrency,
                                                 sizeof(concurrency)));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPhaseConcurrencyOid,
                                                     PHASE_OID_LEN,
                                                     &request,
                                                     &value));
  CommitTx(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPhaseConcurrencyOid,
                                                PHASE_OID_LEN,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT16(sizeof(concurrency), value.data.octetString.length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(concurrency,
                                value.data.octetString.bytes,
                                value.data.octetString.length);
}

void test_phase_range_checks_reject_invalid_extended_values(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x1004U, 0U, 0U);
  NtcipValue_t value;
  uint8_t invalidConcurrency[INTERSECTION_PHASE_COUNT_MAX + 1U];

  memset(invalidConcurrency, 0, sizeof(invalidConcurrency));
  BeginTx(&request, 5U);

  NtcipValueSetUnsigned32(&value, 70000U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kPhaseOptionsOid,
                                                    PHASE_OID_LEN,
                                                    &request,
                                                    &value));
  NtcipValueSetUnsigned32(&value, 6001U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kPhaseMaximum3Oid,
                                                    PHASE_OID_LEN,
                                                    &request,
                                                    &value));
  NtcipValueSetUnsigned32(&value, 129U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kPhaseAdvWarnGrnStartTimeOid,
                                                    PHASE_OID_LEN,
                                                    &request,
                                                    &value));
  NtcipValueSetUnsigned32(&value, 0U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kPhasePedWalkServiceOid,
                                                    PHASE_OID_LEN,
                                                    &request,
                                                    &value));
  NtcipValueSetUnsigned32(&value, 7U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kPhaseStartupOid,
                                                    PHASE_OID_LEN,
                                                    &request,
                                                    &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipValueSetOctetString(&value,
                                                 invalidConcurrency,
                                                 sizeof(invalidConcurrency)));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_RANGE_ERROR,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kPhaseConcurrencyOid,
                                                    PHASE_OID_LEN,
                                                    &request,
                                                    &value));
}

void test_phase_ring_zero_disables_phase_and_phase_startup_reads_other(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x1005U, 0U, 0U);

  BeginTx(&request, 6U);
  SetPhaseUnsigned32(kPhaseStartupOid,
                     &request,
                     (uint32_t) INTERSECTION_PHASE_STARTUP_RED_CLEAR);
  SetPhaseUnsigned32(kPhaseRingOid, &request, 0U);
  CommitTx(&request);

  TEST_ASSERT_EQUAL_UINT32(0U, GetPhaseUnsigned32(kPhaseRingOid));
  TEST_ASSERT_EQUAL_UINT32((uint32_t) INTERSECTION_PHASE_STARTUP_OTHER,
                           GetPhaseUnsigned32(kPhaseStartupOid));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_phase_options_roundtrip_full_bitmask);
  RUN_TEST(test_extended_phase_columns_roundtrip_in_mib_units);
  RUN_TEST(test_phase_startup_configuration_is_applied_on_engine_reset);
  RUN_TEST(test_phase_concurrency_roundtrips_octet_string);
  RUN_TEST(test_phase_range_checks_reject_invalid_extended_values);
  RUN_TEST(test_phase_ring_zero_disables_phase_and_phase_startup_reads_other);

  return UNITY_END();
}
