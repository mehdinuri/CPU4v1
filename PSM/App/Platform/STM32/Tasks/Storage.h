/**
 ******************************************************************************
 * @file           : Storage.h
 * @brief          : Header for Storage.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_PLATFORM_STM32_TASKS_STORAGE_H
#define APP_PLATFORM_STM32_TASKS_STORAGE_H

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
/* Public define ------------------------------------------------------------*/

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/
typedef enum
{
  STORAGE_REQ_NONE = 0,
  STORAGE_REQ_FIRST,
  STORAGE_REQ_EEPROM_WRITE_ASYNCH = STORAGE_REQ_FIRST,
  STORAGE_REQ_EEPROM_WRITE,
  STORAGE_REQ_EEPROM_READ,
  STORAGE_REQ_LAST
} StorageRequestType_e;

/* Maximum payload size that can round-trip through a single Storage request.
 * PSM persists a 4-byte period and a 2-byte offset, so 16 is generous and
 * leaves headroom for future per-record headers (magic + checksum). */
#define STORAGE_MAX_PAYLOAD 16U

/* Storage request: the buffer is carried *inside* the request struct so that
 * a caller that times out cannot leave StorageTask dereferencing a dead
 * stack frame.  For READs, readTarget points at the caller's buffer, and
 * StorageTask copies the staged bytes back only on a successful read. */
typedef struct StorageReq
{
  osThreadId_t threadId;
  uint32_t address;
  uint32_t dataSize;
  uint8_t reqId;
  uint8_t buf[STORAGE_MAX_PAYLOAD];
  void        *readTarget;
} StorageReq_t;

/* Public type values -------------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/
extern uint8_t StorageRequest(uint8_t reqId,
                              uint32_t address,
                              void *data,
                              uint32_t dataSize);

#endif /* APP_PLATFORM_STM32_TASKS_STORAGE_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
