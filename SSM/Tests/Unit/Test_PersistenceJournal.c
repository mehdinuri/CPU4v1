/*
 * Tests/Unit/Test_PersistenceJournal.c
 *
 * Exercises the dual-slot record helpers used by FlashPersistenceAdapter.
 */
#include <string.h>
#include "unity.h"
#include "Domain/PersistenceJournal.h"

static uint8_t abPayload[PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE];

void setUp(void)
{
  for (uint8_t i = 0U; i < sizeof(abPayload); i++)
  {
    abPayload[i] = i;
  }
}

void tearDown(void)
{
}

void test_build_produces_valid_record(void)
{
  tSPersistenceJournalRecord SRecord;

  TEST_ASSERT_EQUAL_UINT8(1U, PersistenceJournal_RecordBuild(&SRecord,
                                                             7U,
                                                             abPayload,
                                                             12U));
  TEST_ASSERT_EQUAL_UINT8(1U, PersistenceJournal_RecordIsValid(&SRecord, 12U));
  TEST_ASSERT_EQUAL_UINT32(7U, SRecord.lSequence);
  TEST_ASSERT_EQUAL_UINT16(12U, SRecord.sPayloadSize);
  TEST_ASSERT_EQUAL_UINT8(abPayload[0], SRecord.abPayload[0]);
  TEST_ASSERT_EQUAL_UINT8(abPayload[11], SRecord.abPayload[11]);
}

void test_build_rejects_invalid_payload_size(void)
{
  tSPersistenceJournalRecord SRecord;

  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordBuild(&SRecord,
                                                             1U,
                                                             abPayload,
                                                             0U));
  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordBuild(&SRecord,
                                                             1U,
                                                             abPayload,
                                                             17U));
}

void test_validation_rejects_bad_crc(void)
{
  tSPersistenceJournalRecord SRecord;

  TEST_ASSERT_EQUAL_UINT8(1U, PersistenceJournal_RecordBuild(&SRecord,
                                                             9U,
                                                             abPayload,
                                                             12U));
  SRecord.lCrc32 ^= 0xFFFFFFFFU;

  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordIsValid(&SRecord, 12U));
}

void test_validation_rejects_wrong_commit_marker(void)
{
  tSPersistenceJournalRecord SRecord;

  TEST_ASSERT_EQUAL_UINT8(1U, PersistenceJournal_RecordBuild(&SRecord,
                                                             3U,
                                                             abPayload,
                                                             12U));
  SRecord.bCommit = 0U;

  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordIsValid(&SRecord, 12U));
}

void test_select_latest_prefers_higher_sequence_when_both_valid(void)
{
  tSPersistenceJournalRecord SOld;
  tSPersistenceJournalRecord SNew;

  PersistenceJournal_RecordBuild(&SOld, 10U, abPayload, 12U);
  PersistenceJournal_RecordBuild(&SNew, 11U, abPayload, 12U);

  TEST_ASSERT_EQUAL_PTR(&SNew,
                        PersistenceJournal_SelectLatest(&SOld, &SNew, 12U));
}

void test_select_latest_returns_only_valid_record(void)
{
  tSPersistenceJournalRecord SInvalid;
  tSPersistenceJournalRecord SValid;

  memset(&SInvalid, 0, sizeof(SInvalid));
  PersistenceJournal_RecordBuild(&SValid, 2U, abPayload, 12U);

  TEST_ASSERT_EQUAL_PTR(&SValid,
                        PersistenceJournal_SelectLatest(&SInvalid,
                                                        &SValid,
                                                        12U));
}

void test_select_latest_returns_null_when_neither_record_is_valid(void)
{
  tSPersistenceJournalRecord SA;
  tSPersistenceJournalRecord SB;

  memset(&SA, 0, sizeof(SA));
  memset(&SB, 0, sizeof(SB));

  TEST_ASSERT_NULL(PersistenceJournal_SelectLatest(&SA, &SB, 12U));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_build_produces_valid_record);
  RUN_TEST(test_build_rejects_invalid_payload_size);
  RUN_TEST(test_validation_rejects_bad_crc);
  RUN_TEST(test_validation_rejects_wrong_commit_marker);
  RUN_TEST(test_select_latest_prefers_higher_sequence_when_both_valid);
  RUN_TEST(test_select_latest_returns_only_valid_record);
  RUN_TEST(test_select_latest_returns_null_when_neither_record_is_valid);

  return UNITY_END();
}
