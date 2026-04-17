/*
 * Tests/Unit/Test_DisplayPort.c
 *
 * Unit tests for the character-display port contract using the mock adapter.
 */
#include "unity.h"

#include "MockDisplayAdapter.h"

static MockDisplayAdapterCtx_t s_ctx;
static IDisplayPort_t s_port;

void setUp(void)
{
  MockDisplayAdapterInit(&s_ctx);
  s_port = MockDisplayAdapterCreatePort(&s_ctx);
}

void tearDown(void)
{
}

void test_display_write_places_text_at_requested_position(void)
{
  DisplayWrite(&s_port, 2U, 3U, "TEST", 4U);

  TEST_ASSERT_EQUAL_CHAR('T', s_ctx.framebuffer[2][3]);
  TEST_ASSERT_EQUAL_CHAR('E', s_ctx.framebuffer[2][4]);
  TEST_ASSERT_EQUAL_CHAR('S', s_ctx.framebuffer[2][5]);
  TEST_ASSERT_EQUAL_CHAR('T', s_ctx.framebuffer[2][6]);
  TEST_ASSERT_EQUAL_UINT32(1U, s_ctx.writeCount);
}

void test_display_clear_blanks_all_rows(void)
{
  DisplayWrite(&s_port, 0U, 0U, "ABC", 3U);
  DisplayClear(&s_port);

  TEST_ASSERT_EQUAL_CHAR(' ', s_ctx.framebuffer[0][0]);
  TEST_ASSERT_EQUAL_CHAR(' ', s_ctx.framebuffer[1][0]);
  TEST_ASSERT_EQUAL_CHAR(' ', s_ctx.framebuffer[2][0]);
  TEST_ASSERT_EQUAL_CHAR(' ', s_ctx.framebuffer[3][0]);
  TEST_ASSERT_EQUAL_UINT32(1U, s_ctx.clearCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_display_write_places_text_at_requested_position);
  RUN_TEST(test_display_clear_blanks_all_rows);

  return UNITY_END();
}
