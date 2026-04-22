/*
 * Tests/Unit/Test_ClockObjects.c
 *
 * Focused compliance tests for the NTCIP 1202 ascClock subtree.
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

#define DB_SCALAR_OID_LEN 13U
#define CLOCK_SCALAR_OID_LEN 14U
#define CLOCK_TABLE_OID_LEN 16U

typedef struct
{
  uint8_t sourceCount;
  uint8_t sources[4];
  uint8_t commandedSource;
  uint8_t currentSource;
  uint8_t currentStatus;
  uint8_t nonSequentialSource;
  uint32_t nonSequentialChangeSeconds;
  UnitClockNonSequentialDelta_t nonSequentialDelta;
  uint32_t acknowledgeCount;
} FakeUnitClockPortCtx_t;

static const uint32_t kDbCreateTransactionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 1U, 0U
};
static const uint32_t kDbTransactionIdOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 4U, 0U
};
static const uint32_t kMaxTimeSourcesOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 1U, 0U
};
static const uint32_t kUnitTimeIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 2U, 1U, 1U, 1U
};
static const uint32_t kUnitTimeSourceAvailableOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 2U, 1U, 2U, 1U
};
static const uint32_t kUnitTimeSourceAvailableOidRow2[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 2U, 1U, 2U, 2U
};
static const uint32_t kUnitTimeSourceAvailableOidRow3[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 2U, 1U, 2U, 3U
};
static const uint32_t kUnitTimeSourceCommandedOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 3U, 0U
};
static const uint32_t kUnitTimeSourceCurrentOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 4U, 0U
};
static const uint32_t kUnitTimeSourceStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 5U, 0U
};
static const uint32_t kUnitTimeNonSequentialSourceOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 6U, 0U
};
static const uint32_t kUnitTimeNonSequentialChangeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 7U, 0U
};
static const uint32_t kUnitTimeNonSequentialDeltaOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 8U, 0U
};

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_configService;
static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
static NtcipDbTransactionService_t s_dbTxService;
static NtcipObjectDirectory_t s_directory;
static NtcipContext_t s_context;
static FakeUnitClockPortCtx_t s_clockCtx;
static IUnitClockPort_t s_clockPort;

static uint8_t FakeGetSourceCount(void *ctx)
{
  return ((FakeUnitClockPortCtx_t *) ctx)->sourceCount;
}

static uint8_t FakeGetSourceAvailable(void *ctx,
                                      uint8_t sourceIndex,
                                      uint8_t *sourceAvailable)
{
  FakeUnitClockPortCtx_t *clock = (FakeUnitClockPortCtx_t *) ctx;

  if ((sourceAvailable == NULL) || (sourceIndex >= clock->sourceCount))
  {
    return 0U;
  }

  *sourceAvailable = clock->sources[sourceIndex];

  return 1U;
}

static uint8_t FakeGetCommandedSource(void *ctx, uint8_t *commandedSource)
{
  FakeUnitClockPortCtx_t *clock = (FakeUnitClockPortCtx_t *) ctx;

  if (commandedSource == NULL)
  {
    return 0U;
  }

  *commandedSource = clock->commandedSource;

  return 1U;
}

static uint8_t FakeGetCurrentSource(void *ctx, uint8_t *currentSource)
{
  FakeUnitClockPortCtx_t *clock = (FakeUnitClockPortCtx_t *) ctx;

  if (currentSource == NULL)
  {
    return 0U;
  }

  *currentSource = clock->currentSource;

  return 1U;
}

static uint8_t FakeGetCurrentStatus(void *ctx, uint8_t *currentStatus)
{
  FakeUnitClockPortCtx_t *clock = (FakeUnitClockPortCtx_t *) ctx;

  if (currentStatus == NULL)
  {
    return 0U;
  }

  *currentStatus = clock->currentStatus;

  return 1U;
}

static uint8_t FakeGetNonSequentialSource(void *ctx,
                                          uint8_t *nonSequentialSource)
{
  FakeUnitClockPortCtx_t *clock = (FakeUnitClockPortCtx_t *) ctx;

  if (nonSequentialSource == NULL)
  {
    return 0U;
  }

  *nonSequentialSource = clock->nonSequentialSource;

  return 1U;
}

static uint8_t FakeGetNonSequentialChange(void *ctx,
                                          uint32_t *nonSequentialChangeSeconds)
{
  FakeUnitClockPortCtx_t *clock = (FakeUnitClockPortCtx_t *) ctx;

  if (nonSequentialChangeSeconds == NULL)
  {
    return 0U;
  }

  *nonSequentialChangeSeconds = clock->nonSequentialChangeSeconds;

  return 1U;
}

static uint8_t FakeGetNonSequentialDelta(void *ctx,
                                         UnitClockNonSequentialDelta_t *delta)
{
  FakeUnitClockPortCtx_t *clock = (FakeUnitClockPortCtx_t *) ctx;

  if (delta == NULL)
  {
    return 0U;
  }

  *delta = clock->nonSequentialDelta;

  return 1U;
}

static void FakeAcknowledgeCurrentStatusRead(void *ctx)
{
  FakeUnitClockPortCtx_t *clock = (FakeUnitClockPortCtx_t *) ctx;

  clock->acknowledgeCount++;
}

static void InitClockPort(void)
{
  memset(&s_clockCtx, 0, sizeof(s_clockCtx));
  s_clockCtx.sourceCount = 3U;
  s_clockCtx.sources[0] = (uint8_t) UNIT_CLOCK_SOURCE_RTC_SQWR;
  s_clockCtx.sources[1] = (uint8_t) UNIT_CLOCK_SOURCE_GNSS;
  s_clockCtx.sources[2] = (uint8_t) UNIT_CLOCK_SOURCE_LINE_SYNC;
  s_clockCtx.commandedSource = (uint8_t) UNIT_CLOCK_SOURCE_GNSS;
  s_clockCtx.currentSource = (uint8_t) UNIT_CLOCK_SOURCE_RTC_SQWR;
  s_clockCtx.currentStatus = (uint8_t) UNIT_CLOCK_SOURCE_STATUS_ACTIVE;
  s_clockCtx.nonSequentialSource =
    (uint8_t) UNIT_CLOCK_NON_SEQUENTIAL_SOURCE_UNKNOWN;
  s_clockPort.ctx = &s_clockCtx;
  s_clockPort.GetSourceCount = FakeGetSourceCount;
  s_clockPort.GetSourceAvailable = FakeGetSourceAvailable;
  s_clockPort.GetCommandedSource = FakeGetCommandedSource;
  s_clockPort.GetCurrentSource = FakeGetCurrentSource;
  s_clockPort.GetCurrentStatus = FakeGetCurrentStatus;
  s_clockPort.GetNonSequentialSource = FakeGetNonSequentialSource;
  s_clockPort.GetNonSequentialChange = FakeGetNonSequentialChange;
  s_clockPort.GetNonSequentialDelta = FakeGetNonSequentialDelta;
  s_clockPort.AcknowledgeCurrentStatusRead = FakeAcknowledgeCurrentStatusRead;
}

static void BeginTx(NtcipRequestContext_t *request, uint8_t transactionId)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     DB_SCALAR_OID_LEN,
                                                     request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, transactionId);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbTransactionIdOid,
                                                     DB_SCALAR_OID_LEN,
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
                                                     DB_SCALAR_OID_LEN,
                                                     request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     DB_SCALAR_OID_LEN,
                                                     request,
                                                     &value));
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
  TEST_ASSERT_EQUAL_INT(NTCIP_VALUE_TYPE_UNSIGNED32, value.type);

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
  InitClockPort();
  NtcipContextBindUnitClockPort(&s_context, &s_clockPort);
  NtcipObjectDirectoryInit(&s_directory);
  Ntcip1201RegisterObjects(&s_directory, &s_context);
  Ntcip1202RegisterObjects(&s_directory, &s_context);
}

void tearDown(void)
{
}

void test_max_time_sources_and_available_table_follow_bound_clock_port(void)
{
  TEST_ASSERT_EQUAL_UINT32(3U,
                           GetUnsigned32(kMaxTimeSourcesOid,
                                         CLOCK_SCALAR_OID_LEN));
  TEST_ASSERT_EQUAL_UINT32(1U,
                           GetUnsigned32(kUnitTimeIndexOid, CLOCK_TABLE_OID_LEN));
  TEST_ASSERT_EQUAL_UINT32((uint32_t) UNIT_CLOCK_SOURCE_RTC_SQWR,
                           GetUnsigned32(kUnitTimeSourceAvailableOid,
                                         CLOCK_TABLE_OID_LEN));
  TEST_ASSERT_EQUAL_UINT32((uint32_t) UNIT_CLOCK_SOURCE_GNSS,
                           GetUnsigned32(kUnitTimeSourceAvailableOidRow2,
                                         CLOCK_TABLE_OID_LEN));
  TEST_ASSERT_EQUAL_UINT32((uint32_t) UNIT_CLOCK_SOURCE_LINE_SYNC,
                           GetUnsigned32(kUnitTimeSourceAvailableOidRow3,
                                         CLOCK_TABLE_OID_LEN));
}

void test_unit_time_source_commanded_roundtrips_through_transaction_commit(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x2100U, 0U, 0U);
  NtcipValue_t value;

  BeginTx(&request, 1U);
  NtcipValueSetUnsigned32(&value, (uint32_t) UNIT_CLOCK_SOURCE_LINE_SYNC);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kUnitTimeSourceCommandedOid,
                                                     CLOCK_SCALAR_OID_LEN,
                                                     &request,
                                                     &value));
  CommitTx(&request);

  TEST_ASSERT_EQUAL_UINT32((uint32_t) UNIT_CLOCK_SOURCE_LINE_SYNC,
                           GetUnsigned32(kUnitTimeSourceCommandedOid,
                                         CLOCK_SCALAR_OID_LEN));
}

void test_unit_time_source_commanded_rejects_unsupported_sources(void)
{
  NtcipRequestContext_t request =
    NTCIP_REQUEST_CONTEXT_INIT(0x2101U, 0U, 0U);
  NtcipValue_t value;

  BeginTx(&request, 3U);
  NtcipValueSetUnsigned32(&value, (uint32_t) UNIT_CLOCK_SOURCE_NTP);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_BAD_VALUE,
                        NtcipObjectDirectorySetTest(&s_directory,
                                                    kUnitTimeSourceCommandedOid,
                                                    CLOCK_SCALAR_OID_LEN,
                                                    &request,
                                                    &value));
}

void test_clock_runtime_scalars_and_non_sequential_data_are_port_backed(void)
{
  NtcipValue_t value;

  s_clockCtx.currentSource = (uint8_t) UNIT_CLOCK_SOURCE_GNSS;
  s_clockCtx.currentStatus = (uint8_t) UNIT_CLOCK_SOURCE_STATUS_NON_SEQUENTIAL;
  s_clockCtx.nonSequentialSource =
    (uint8_t) UNIT_CLOCK_NON_SEQUENTIAL_SOURCE_TIME_SOURCE_CHANGE;
  s_clockCtx.nonSequentialChangeSeconds = 123456U;
  s_clockCtx.nonSequentialDelta.length = 5U;
  s_clockCtx.nonSequentialDelta.bytes[0] = 1U;
  s_clockCtx.nonSequentialDelta.bytes[1] = 2U;
  s_clockCtx.nonSequentialDelta.bytes[2] = 3U;
  s_clockCtx.nonSequentialDelta.bytes[3] = 0U;
  s_clockCtx.nonSequentialDelta.bytes[4] = 4U;

  TEST_ASSERT_EQUAL_UINT32((uint32_t) UNIT_CLOCK_SOURCE_GNSS,
                           GetUnsigned32(kUnitTimeSourceCurrentOid,
                                         CLOCK_SCALAR_OID_LEN));
  TEST_ASSERT_EQUAL_UINT32((uint32_t) UNIT_CLOCK_SOURCE_STATUS_NON_SEQUENTIAL,
                           GetUnsigned32(kUnitTimeSourceStatusOid,
                                         CLOCK_SCALAR_OID_LEN));
  TEST_ASSERT_EQUAL_UINT32(1U, s_clockCtx.acknowledgeCount);
  TEST_ASSERT_EQUAL_UINT32(
    (uint32_t) UNIT_CLOCK_NON_SEQUENTIAL_SOURCE_TIME_SOURCE_CHANGE,
    GetUnsigned32(kUnitTimeNonSequentialSourceOid, CLOCK_SCALAR_OID_LEN));
  TEST_ASSERT_EQUAL_UINT32(123456U,
                           GetUnsigned32(kUnitTimeNonSequentialChangeOid,
                                         CLOCK_SCALAR_OID_LEN));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kUnitTimeNonSequentialDeltaOid,
                                                CLOCK_SCALAR_OID_LEN,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_INT(NTCIP_VALUE_TYPE_OCTET_STRING, value.type);
  TEST_ASSERT_EQUAL_UINT16(5U, value.data.octetString.length);
  TEST_ASSERT_EQUAL_UINT8(1U, value.data.octetString.bytes[0]);
  TEST_ASSERT_EQUAL_UINT8(4U, value.data.octetString.bytes[4]);
}

int main(void)
{
  UNITY_BEGIN();

  RUN_TEST(test_max_time_sources_and_available_table_follow_bound_clock_port);
  RUN_TEST(test_unit_time_source_commanded_roundtrips_through_transaction_commit);
  RUN_TEST(test_unit_time_source_commanded_rejects_unsupported_sources);
  RUN_TEST(test_clock_runtime_scalars_and_non_sequential_data_are_port_backed);

  return UNITY_END();
}
