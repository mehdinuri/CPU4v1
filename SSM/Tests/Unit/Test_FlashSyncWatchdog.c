/*
 * Tests/Unit/Test_FlashSyncWatchdog.c
 *
 * Covers the FlashSyncWatchdog domain service, including boot-time
 * never-fed semantics and SysTick wraparound. Uses MockTickAdapter to
 * verify the port-driven usage pattern end-to-end.
 */
#include "unity.h"
#include "Domain/FlashSyncWatchdog.h"
#include "Adapters/Mock/MockTickAdapter.h"

static tSFlashSyncWatchdog wdt;

void setUp(void)
{
  FlashSyncWatchdog_Reset(&wdt);
}

void tearDown(void)
{
}

#define TIMEOUT_MS 1500U

void test_never_fed_is_stale(void)
{
  TEST_ASSERT_EQUAL_UINT8(1U,
                          FlashSyncWatchdog_IsStale(&wdt, 0U,   TIMEOUT_MS));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          FlashSyncWatchdog_IsStale(&wdt, 500U, TIMEOUT_MS));
}

void test_just_fed_is_fresh(void)
{
  FlashSyncWatchdog_Feed(&wdt, 1000U);
  TEST_ASSERT_EQUAL_UINT8(0U,
                          FlashSyncWatchdog_IsStale(&wdt, 1000U, TIMEOUT_MS));
  TEST_ASSERT_EQUAL_UINT8(0U,
                          FlashSyncWatchdog_IsStale(&wdt, 1500U, TIMEOUT_MS));
  TEST_ASSERT_EQUAL_UINT8(0U,
                          FlashSyncWatchdog_IsStale(&wdt, 2500U, TIMEOUT_MS));
}

void test_becomes_stale_past_timeout(void)
{
  FlashSyncWatchdog_Feed(&wdt, 1000U);
  /* elapsed = 1501 > 1500 → stale */
  TEST_ASSERT_EQUAL_UINT8(1U,
                          FlashSyncWatchdog_IsStale(&wdt, 2501U, TIMEOUT_MS));
  /* elapsed = 100000 → still stale */
  TEST_ASSERT_EQUAL_UINT8(1U, FlashSyncWatchdog_IsStale(&wdt,
                                                        101000U,
                                                        TIMEOUT_MS));
}

void test_refeed_clears_stale(void)
{
  FlashSyncWatchdog_Feed(&wdt, 1000U);
  TEST_ASSERT_EQUAL_UINT8(1U,
                          FlashSyncWatchdog_IsStale(&wdt, 3000U, TIMEOUT_MS));
  FlashSyncWatchdog_Feed(&wdt, 3000U);
  TEST_ASSERT_EQUAL_UINT8(0U,
                          FlashSyncWatchdog_IsStale(&wdt, 3100U, TIMEOUT_MS));
}

void test_wraparound_is_handled(void)
{
  /* Feed just before SysTick rollover. */
  FlashSyncWatchdog_Feed(&wdt, 0xFFFFFF00U);

  /* 100 ms after — 0xFFFFFF00 + 0x100 = 0x0 (wrapped). elapsed = 0 - 0xFFFFFF00
   * = 0x100 (in uint32 arithmetic) → fresh.
   */
  TEST_ASSERT_EQUAL_UINT8(0U, FlashSyncWatchdog_IsStale(&wdt,
                                                        0x00000000U,
                                                        TIMEOUT_MS));
  /* 2000 ms past rollover: elapsed = 0x800 - 0xFFFFFF00 = 0x900 (2304 ms) → stale */
  TEST_ASSERT_EQUAL_UINT8(1U, FlashSyncWatchdog_IsStale(&wdt,
                                                        0x00000800U,
                                                        TIMEOUT_MS));
}

void test_reset_returns_to_never_fed(void)
{
  FlashSyncWatchdog_Feed(&wdt, 1000U);
  TEST_ASSERT_EQUAL_UINT8(0U,
                          FlashSyncWatchdog_IsStale(&wdt, 1100U, TIMEOUT_MS));
  FlashSyncWatchdog_Reset(&wdt);
  TEST_ASSERT_EQUAL_UINT8(1U,
                          FlashSyncWatchdog_IsStale(&wdt, 1100U, TIMEOUT_MS));
}

void test_driven_via_tick_port(void)
{
  tSMockTickAdapterCtx tickCtx;
  ITickPort_t tickPort;

  MockTickAdapter_Init(&tickCtx);
  tickPort = MockTickAdapter_CreatePort(&tickCtx);

  MockTickAdapter_SetNow(&tickCtx, 500U);
  FlashSyncWatchdog_Feed(&wdt, Tick_Now_ms(&tickPort));

  MockTickAdapter_SetNow(&tickCtx, 1800U);
  TEST_ASSERT_EQUAL_UINT8(0U, FlashSyncWatchdog_IsStale(&wdt,
                                                        Tick_Now_ms(&tickPort),
                                                        TIMEOUT_MS));

  MockTickAdapter_SetNow(&tickCtx, 2500U);
  TEST_ASSERT_EQUAL_UINT8(1U, FlashSyncWatchdog_IsStale(&wdt,
                                                        Tick_Now_ms(&tickPort),
                                                        TIMEOUT_MS));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_never_fed_is_stale);
  RUN_TEST(test_just_fed_is_fresh);
  RUN_TEST(test_becomes_stale_past_timeout);
  RUN_TEST(test_refeed_clears_stale);
  RUN_TEST(test_wraparound_is_handled);
  RUN_TEST(test_reset_returns_to_never_fed);
  RUN_TEST(test_driven_via_tick_port);

  return UNITY_END();
}
