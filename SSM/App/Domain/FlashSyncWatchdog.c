/**
 ******************************************************************************
 * @file    Domain/FlashSyncWatchdog.c
 ******************************************************************************
 */

#include <string.h>
#include "Domain/FlashSyncWatchdog.h"

void FlashSyncWatchdog_Reset(FlashSyncWatchdog_t *wdt)
{
  memset(wdt, 0, sizeof(*wdt));
}

void FlashSyncWatchdog_Feed(FlashSyncWatchdog_t *wdt, uint32_t nowMs)
{
  wdt->lastFeedMs = nowMs;
  wdt->everFed = 1U;
}

uint8_t FlashSyncWatchdog_IsStale(const FlashSyncWatchdog_t *wdt,
                                  uint32_t nowMs,
                                  uint32_t timeoutMs)
{
  if (wdt->everFed == 0U)
  {
    return 1U;
  }

  /* uint32 subtract handles SysTick wraparound naturally. */
  uint32_t elapsed = nowMs - wdt->lastFeedMs;

  return (elapsed > timeoutMs) ? 1U : 0U;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
