/*
 * Tests/Unit/Test_PersistenceJournal.c
 *
 * Exercises the dual-slot record helpers used by FlashPersistenceAdapter.
 */
#include <string.h>
#include "unity.h"
#include "Domain/PersistenceJournal.h"

static uint8_t payload[PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE];

void setUp(void)
{
  for (uint8_t i = 0U; i < sizeof(payload); i++)
  {
    payload[i] = i;
  }
}

void tearDown(void)
{
}

void test_build_produces_valid_record(void)
{
  PersistenceJournalRecord_t record;

  TEST_ASSERT_EQUAL_UINT8(1U, PersistenceJournal_RecordBuild(&record,
                                                             7U,
                                                             payload,
                                                             12U));
  TEST_ASSERT_EQUAL_UINT8(1U, PersistenceJournal_RecordIsValid(&record, 12U));
  TEST_ASSERT_EQUAL_UINT32(7U, record.sequence);
  TEST_ASSERT_EQUAL_UINT16(12U, record.payloadSize);
  TEST_ASSERT_EQUAL_UINT8(payload[0], record.payload[0]);
  TEST_ASSERT_EQUAL_UINT8(payload[11], record.payload[11]);
}

void test_build_rejects_invalid_payload_size(void)
{
  PersistenceJournalRecord_t record;

  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordBuild(&record,
                                                             1U,
                                                             payload,
                                                             0U));
  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordBuild(&record,
                                                             1U,
                                                             payload,
                                                             17U));
}

void test_validation_rejects_bad_crc(void)
{
  PersistenceJournalRecord_t record;

  TEST_ASSERT_EQUAL_UINT8(1U, PersistenceJournal_RecordBuild(&record,
                                                             9U,
                                                             payload,
                                                             12U));
  record.crc32 ^= 0xFFFFFFFFU;

  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordIsValid(&record, 12U));
}

void test_validation_rejects_wrong_commit_marker(void)
{
  PersistenceJournalRecord_t record;

  TEST_ASSERT_EQUAL_UINT8(1U, PersistenceJournal_RecordBuild(&record,
                                                             3U,
                                                             payload,
                                                             12U));
  record.commit = 0U;

  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordIsValid(&record, 12U));
}

void test_select_latest_prefers_higher_sequence_when_both_valid(void)
{
  PersistenceJournalRecord_t older;
  PersistenceJournalRecord_t newer;

  PersistenceJournal_RecordBuild(&older, 10U, payload, 12U);
  PersistenceJournal_RecordBuild(&newer, 11U, payload, 12U);

  TEST_ASSERT_EQUAL_PTR(&newer,
                        PersistenceJournal_SelectLatest(&older, &newer, 12U));
}

void test_select_latest_returns_only_valid_record(void)
{
  PersistenceJournalRecord_t invalid;
  PersistenceJournalRecord_t valid;

  memset(&invalid, 0, sizeof(invalid));
  PersistenceJournal_RecordBuild(&valid, 2U, payload, 12U);

  TEST_ASSERT_EQUAL_PTR(&valid,
                        PersistenceJournal_SelectLatest(&invalid,
                                                        &valid,
                                                        12U));
}

void test_select_latest_returns_null_when_neither_record_is_valid(void)
{
  PersistenceJournalRecord_t a;
  PersistenceJournalRecord_t b;

  memset(&a, 0, sizeof(a));
  memset(&b, 0, sizeof(b));

  TEST_ASSERT_NULL(PersistenceJournal_SelectLatest(&a, &b, 12U));
}

void test_build_rejects_null_record_pointer(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordBuild(NULL,
                                                             1U,
                                                             payload,
                                                             12U));
}

void test_build_rejects_null_payload_pointer(void)
{
  PersistenceJournalRecord_t record;

  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordBuild(&record,
                                                             1U,
                                                             NULL,
                                                             12U));
}

void test_validation_rejects_null_record_pointer(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordIsValid(NULL, 12U));
}

void test_validation_rejects_wrong_magic(void)
{
  PersistenceJournalRecord_t record;

  PersistenceJournal_RecordBuild(&record, 5U, payload, 12U);
  record.magic = 0xDEADBEEFU;

  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordIsValid(&record, 12U));
}

void test_validation_rejects_wrong_version(void)
{
  PersistenceJournalRecord_t record;

  PersistenceJournal_RecordBuild(&record, 5U, payload, 12U);
  record.version = 0xFFU;

  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordIsValid(&record, 12U));
}

void test_validation_rejects_payload_size_over_caller_max(void)
{
  PersistenceJournalRecord_t record;

  /* Record claims 12 bytes of payload but caller says max is 8 → invalid. */
  PersistenceJournal_RecordBuild(&record, 5U, payload, 12U);

  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordIsValid(&record, 8U));
}

void test_validation_rejects_zero_payload_size(void)
{
  PersistenceJournalRecord_t record;

  /* Build refuses zero size, so forge a record manually with the right
   * magic/version/commit but payloadSize == 0 and an intentionally bad
   * CRC — the size check must fire before the CRC check.
   */
  PersistenceJournal_RecordBuild(&record, 5U, payload, 12U);
  record.payloadSize = 0U;

  TEST_ASSERT_EQUAL_UINT8(0U, PersistenceJournal_RecordIsValid(&record, 12U));
}

void test_select_latest_prefers_A_when_B_invalid(void)
{
  PersistenceJournalRecord_t valid;
  PersistenceJournalRecord_t invalid;

  PersistenceJournal_RecordBuild(&valid, 4U, payload, 12U);
  memset(&invalid, 0, sizeof(invalid));

  TEST_ASSERT_EQUAL_PTR(&valid,
                        PersistenceJournal_SelectLatest(&valid,
                                                        &invalid,
                                                        12U));
}

void test_select_latest_handles_sequence_wraparound(void)
{
  PersistenceJournalRecord_t oldNearMax;
  PersistenceJournalRecord_t newWrapped;

  /* oldNearMax just before uint32 wrap, newWrapped just after. An
   * unsigned subtraction (newWrapped - oldNearMax) yields a small
   * positive number, so SelectLatest must correctly pick the wrapped
   * record as the newer one.
   */
  PersistenceJournal_RecordBuild(&oldNearMax, 0xFFFFFFF0U, payload, 12U);
  PersistenceJournal_RecordBuild(&newWrapped, 0x00000003U, payload, 12U);

  TEST_ASSERT_EQUAL_PTR(&newWrapped,
                        PersistenceJournal_SelectLatest(&oldNearMax,
                                                        &newWrapped,
                                                        12U));
}

void test_select_latest_prefers_A_when_A_has_higher_sequence(void)
{
  PersistenceJournalRecord_t a;
  PersistenceJournalRecord_t b;

  /* A is newer than B, both valid — exercises the other arm of the final
   * wraparound-safe ternary compare.
   */
  PersistenceJournal_RecordBuild(&a, 42U, payload, 12U);
  PersistenceJournal_RecordBuild(&b, 7U, payload, 12U);

  TEST_ASSERT_EQUAL_PTR(&a, PersistenceJournal_SelectLatest(&a, &b, 12U));
}

void test_validation_rejects_oversize_payload_against_journal_max(void)
{
  PersistenceJournalRecord_t record;

  /* Build refuses anything > PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE, so
   * forge the header fields manually to exercise the "size exceeds
   * journal max" branch in RecordIsValid.
   */
  PersistenceJournal_RecordBuild(&record, 5U, payload, 12U);
  record.payloadSize = (uint16_t) (PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE
                                   + 1U);

  TEST_ASSERT_EQUAL_UINT8(0U,
                          PersistenceJournal_RecordIsValid(
                            &record,
                            (uint16_t) (PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE
                                        + 1U)));
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
  RUN_TEST(test_build_rejects_null_record_pointer);
  RUN_TEST(test_build_rejects_null_payload_pointer);
  RUN_TEST(test_validation_rejects_null_record_pointer);
  RUN_TEST(test_validation_rejects_wrong_magic);
  RUN_TEST(test_validation_rejects_wrong_version);
  RUN_TEST(test_validation_rejects_payload_size_over_caller_max);
  RUN_TEST(test_validation_rejects_zero_payload_size);
  RUN_TEST(test_select_latest_prefers_A_when_B_invalid);
  RUN_TEST(test_select_latest_handles_sequence_wraparound);
  RUN_TEST(test_select_latest_prefers_A_when_A_has_higher_sequence);
  RUN_TEST(test_validation_rejects_oversize_payload_against_journal_max);

  return UNITY_END();
}
