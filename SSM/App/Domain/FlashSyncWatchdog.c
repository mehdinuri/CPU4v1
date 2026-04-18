/**
 ******************************************************************************
 * @file    Domain/FlashSyncWatchdog.c
 ******************************************************************************
 */

#include <string.h>
#include "Domain/FlashSyncWatchdog.h"

void FlashSyncWatchdog_Reset(tSFlashSyncWatchdog *pWdt)
{
  memset(pWdt, 0, sizeof(*pWdt));
}

void FlashSyncWatchdog_Feed(tSFlashSyncWatchdog *pWdt, uint32_t lNow_ms)
{
  pWdt->lLastFeed_ms = lNow_ms;
  pWdt->bEverFed = 1U;
}

uint8_t FlashSyncWatchdog_IsStale(const tSFlashSyncWatchdog *pWdt,
                                  uint32_t lNow_ms,
                                  uint32_t lTimeout_ms)
{
  if (pWdt->bEverFed == 0U)
  {
    return 1U;
  }

  /* uint32 subtract handles SysTick wraparound naturally. */
  uint32_t lElapsed = lNow_ms - pWdt->lLastFeed_ms;

  return (lElapsed > lTimeout_ms) ? 1U : 0U;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
