#include "unity.h"

#include <string.h>

#include "Domain/Lcd/LcdEventText.h"
#include "Domain/Lcd/LcdLanguage.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_long_event_text_uses_migrated_catalog(void)
{
  TEST_ASSERT_EQUAL_STRING("DOOR OPEN",
                           LcdEventText_GetEventLong(64U, LANGUAGE_ENGLISH));
  TEST_ASSERT_EQUAL_STRING("KAPI KAPATILDI",
                           LcdEventText_GetEventLong(65U, LANGUAGE_TURKISH));
  TEST_ASSERT_EQUAL_STRING("UNDEFINED",
                           LcdEventText_GetEventLong(200U, LANGUAGE_ENGLISH));
}

void test_safety_reason_short_text_uses_live_reason_codes(void)
{
  TEST_ASSERT_EQUAL_STRING("",
                           LcdEventText_GetSafetyReasonShort(0U,
                                                             LANGUAGE_ENGLISH));
  TEST_ASSERT_EQUAL_STRING("GGC",
                           LcdEventText_GetSafetyReasonShort(1U,
                                                             LANGUAGE_ENGLISH));
  TEST_ASSERT_EQUAL_STRING("SSH",
                           LcdEventText_GetSafetyReasonShort(6U,
                                                             LANGUAGE_TURKISH));
  TEST_ASSERT_EQUAL_STRING("CFG",
                           LcdEventText_GetSafetyReasonShort(53U,
                                                             LANGUAGE_ENGLISH));
  TEST_ASSERT_EQUAL_STRING("BIL",
                           LcdEventText_GetSafetyReasonShort(250U,
                                                             LANGUAGE_TURKISH));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_long_event_text_uses_migrated_catalog);
  RUN_TEST(test_safety_reason_short_text_uses_live_reason_codes);
  return UNITY_END();
}
