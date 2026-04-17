/*
 * Tests/Unit/Test_UserInputPort.c
 *
 * Unit tests for the keypad snapshot port contract using the mock adapter.
 */
#include "unity.h"

#include "MockUserInputAdapter.h"

static MockUserInputAdapterCtx_t s_ctx;
static IUserInputPort_t s_port;

void setUp(void)
{
  MockUserInputAdapterInit(&s_ctx);
  s_port = MockUserInputAdapterCreatePort(&s_ctx);
}

void tearDown(void)
{
}

void test_scan_snapshot_returns_programmed_keys_and_resets_after_read(void)
{
  KeypadSnapshot_t snapshot;

  s_ctx.nextSnapshot = KeypadSnapshotBit(KEY_ENTER) | KeypadSnapshotBit(KEY_5);

  snapshot = UserInputScanSnapshot(&s_port);

  TEST_ASSERT_TRUE(KeypadSnapshotHasKey(snapshot, KEY_ENTER));
  TEST_ASSERT_TRUE(KeypadSnapshotHasKey(snapshot, KEY_5));
  TEST_ASSERT_EQUAL_UINT32(1U, s_ctx.scanCount);
  TEST_ASSERT_EQUAL_UINT32(KEYPAD_SNAPSHOT_NONE, s_ctx.nextSnapshot);
}

void test_snapshot_has_key_rejects_out_of_range_keys(void)
{
  KeypadSnapshot_t snapshot = KeypadSnapshotBit(KEY_CLEAR);

  TEST_ASSERT_TRUE(KeypadSnapshotHasKey(snapshot, KEY_CLEAR));
  TEST_ASSERT_FALSE(KeypadSnapshotHasKey(snapshot, KEY_ENTER));
  TEST_ASSERT_FALSE(KeypadSnapshotHasKey(snapshot, KEY_NONE));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_scan_snapshot_returns_programmed_keys_and_resets_after_read);
  RUN_TEST(test_snapshot_has_key_rejects_out_of_range_keys);

  return UNITY_END();
}
