/*
 * Tests/Unit/Test_PersistencePort.c
 *
 * Unit tests for the semantic persistence port using the mock adapter.
 */
#include "unity.h"
#include "MockPersistenceAdapter.h"

static MockPersistenceAdapterCtx_t ctx;
static IPersistencePort_t port;

typedef struct
{
  uint8_t bytes[8];
} SampleBlob_t;

void setUp(void)
{
  MockPersistenceAdapterInit(&ctx);
  port = MockPersistenceAdapterCreatePort(&ctx);
}

void tearDown(void)
{
}

void test_write_and_read_round_trip_for_semantic_object(void)
{
  SampleBlob_t written = { { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U } };
  SampleBlob_t readBack = { { 0U } };

  TEST_ASSERT_TRUE(PersistenceWrite(&port,
                                    PERSIST_OBJECT_USER_SETTINGS,
                                    0U,
                                    &written,
                                    sizeof(written)));
  TEST_ASSERT_TRUE(PersistenceRead(&port,
                                   PERSIST_OBJECT_USER_SETTINGS,
                                   0U,
                                   &readBack,
                                   sizeof(readBack)));
  TEST_ASSERT_EQUAL_MEMORY(&written, &readBack, sizeof(written));
}

void test_offset_access_avoids_object_base_leakage(void)
{
  uint32_t value = 0x11223344UL;
  uint32_t readBack = 0UL;

  TEST_ASSERT_TRUE(PersistenceWrite(&port,
                                    PERSIST_OBJECT_MCS_CONNECTION_INFO,
                                    12U,
                                    &value,
                                    sizeof(value)));
  TEST_ASSERT_TRUE(PersistenceRead(&port,
                                   PERSIST_OBJECT_MCS_CONNECTION_INFO,
                                   12U,
                                   &readBack,
                                   sizeof(readBack)));
  TEST_ASSERT_EQUAL_HEX32(value, readBack);
}

void test_erase_resets_object_range_to_ff(void)
{
  uint8_t written[4] = { 0U, 1U, 2U, 3U };
  uint8_t readBack[4] = { 0U, 0U, 0U, 0U };
  uint8_t expected[4] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };

  TEST_ASSERT_TRUE(PersistenceWrite(&port,
                                    PERSIST_OBJECT_CONFIG_SLOT_A,
                                    0U,
                                    written,
                                    sizeof(written)));
  TEST_ASSERT_TRUE(PersistenceErase(&port,
                                    PERSIST_OBJECT_CONFIG_SLOT_A,
                                    0U,
                                    sizeof(written)));
  TEST_ASSERT_TRUE(PersistenceRead(&port,
                                   PERSIST_OBJECT_CONFIG_SLOT_A,
                                   0U,
                                   readBack,
                                   sizeof(readBack)));
  TEST_ASSERT_EQUAL_MEMORY(expected, readBack, sizeof(expected));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_write_and_read_round_trip_for_semantic_object);
  RUN_TEST(test_offset_access_avoids_object_base_leakage);
  RUN_TEST(test_erase_resets_object_range_to_ff);

  return UNITY_END();
}
