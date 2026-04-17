/*
 * Tests/Unit/Test_LogRepository.c
 *
 * Unit tests for the circular log repository using the mock adapter.
 */
#include "unity.h"
#include "MockLogRepositoryAdapter.h"

typedef struct
{
  uint16_t sequence;
  uint8_t event;
} SampleLogRecord_t;

static MockLogRepositoryAdapterCtx_t ctx;
static ILogRepositoryPort_t port;

void setUp(void)
{
  MockLogRepositoryAdapterInit(&ctx);
  port = MockLogRepositoryAdapterCreatePort(&ctx);
}

void tearDown(void)
{
}

void test_append_sets_exists_and_tracks_indexes(void)
{
  SampleLogRecord_t record = { 7U, 9U };
  uint16_t writtenIndex = 99U;

  TEST_ASSERT_TRUE(LogRepositoryAppend(&port,
                                       &record,
                                       sizeof(record),
                                       &writtenIndex));
  TEST_ASSERT_TRUE(LogRepositoryExists(&port));
  TEST_ASSERT_EQUAL_UINT16(0U, writtenIndex);
  TEST_ASSERT_EQUAL_UINT16(1U, LogRepositoryGetWriteIndex(&port));
  TEST_ASSERT_EQUAL_UINT16(1U, LogRepositoryGetCount(&port));
}

void test_clear_resets_state(void)
{
  SampleLogRecord_t record = { 1U, 2U };

  TEST_ASSERT_TRUE(LogRepositoryAppend(&port,
                                       &record,
                                       sizeof(record),
                                       NULL));
  TEST_ASSERT_TRUE(LogRepositoryClear(&port));
  TEST_ASSERT_FALSE(LogRepositoryExists(&port));
  TEST_ASSERT_EQUAL_UINT16(0U, LogRepositoryGetWriteIndex(&port));
  TEST_ASSERT_EQUAL_UINT16(0U, LogRepositoryGetCount(&port));
}

void test_wraparound_keeps_count_bounded_and_overwrites_oldest_slot(void)
{
  uint16_t index;
  SampleLogRecord_t record;
  SampleLogRecord_t readBack = { 0U, 0U };
  uint16_t i;

  for (i = 0U; i <= MOCK_LOG_REPOSITORY_MAX_RECORDS; i++)
  {
    record.sequence = i;
    record.event = (uint8_t) (i & 0xFFU);
    TEST_ASSERT_TRUE(LogRepositoryAppend(&port,
                                         &record,
                                         sizeof(record),
                                         &index));
  }

  TEST_ASSERT_EQUAL_UINT16(1U, LogRepositoryGetWriteIndex(&port));
  TEST_ASSERT_EQUAL_UINT16(MOCK_LOG_REPOSITORY_MAX_RECORDS,
                           LogRepositoryGetCount(&port));
  TEST_ASSERT_TRUE(LogRepositoryRead(&port, 0U, &readBack, sizeof(readBack)));
  TEST_ASSERT_EQUAL_UINT16(MOCK_LOG_REPOSITORY_MAX_RECORDS, readBack.sequence);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_append_sets_exists_and_tracks_indexes);
  RUN_TEST(test_clear_resets_state);
  RUN_TEST(test_wraparound_keeps_count_bounded_and_overwrites_oldest_slot);

  return UNITY_END();
}
