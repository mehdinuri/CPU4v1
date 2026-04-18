/**
 ******************************************************************************
 * @file    Domain/FlashSyncWatchdog.h
 * @brief   "Have we heard a PSM flash-sync frame recently?" — a one-state
 *          deadman timer used to escape flash-mode to a known-safe (dark)
 *          output state if the synchronisation source goes silent.
 *
 *          Pure-data / pure-function: the watchdog struct is owned by the
 *          composition root, fed by CANMsgParser on every flash-sync RX,
 *          and queried by the measurement task each cycle. No RTOS, no
 *          HAL; time is supplied by the caller (typically via ITickPort).
 ******************************************************************************
 */

#ifndef DOMAIN_FLASH_SYNC_WATCHDOG_H
#define DOMAIN_FLASH_SYNC_WATCHDOG_H

#include <stdint.h>

typedef struct
{
  uint32_t lLastFeed_ms;
  uint8_t bEverFed;       /* 0 until the first Feed() call after Reset */
} tSFlashSyncWatchdog;

/**
 * @brief Zero the watchdog. IsStale() will return 1 until Feed() is called.
 */
void FlashSyncWatchdog_Reset(tSFlashSyncWatchdog *pWdt);

/**
 * @brief Record that we heard a flash-sync frame at tick lNow_ms.
 */
void FlashSyncWatchdog_Feed(tSFlashSyncWatchdog *pWdt, uint32_t lNow_ms);

/**
 * @brief Return 1 if the watchdog has never been fed OR if
 *        (lNow_ms - lLastFeed_ms) exceeds lTimeout_ms; 0 otherwise.
 *
 *        Arithmetic is uint32 subtract, which handles SysTick wraparound
 *        correctly provided real elapsed time stays below 2^31 ms (~25 days).
 */
uint8_t FlashSyncWatchdog_IsStale(const tSFlashSyncWatchdog *pWdt,
                                  uint32_t lNow_ms,
                                  uint32_t lTimeout_ms);

#endif /* DOMAIN_FLASH_SYNC_WATCHDOG_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
