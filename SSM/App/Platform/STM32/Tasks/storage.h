/**
 ******************************************************************************
 * @file           : storage.h
 * @brief          : Header for storage.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STORAGE_H__
#define __STORAGE_H__

/* Includes ------------------------------------------------------------------*/
#include "flash.h"
#include "cmsis_os2.h"

/* Public define ------------------------------------------------------------*/

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/
typedef enum
{
  STORAGE_REQ_NONE = 0,
  STORAGE_REQ_FIRST,
  STORAGE_REQ_FLASH_WRITE_ASYNCH = STORAGE_REQ_FIRST,
  STORAGE_REQ_FLASH_WRITE,
  STORAGE_REQ_FLASH_READ,
  STORAGE_REQ_LAST
} StorageRequestType_e;

typedef struct
{
  osThreadId_t threadId;
  uint32_t address;
  void *data;
  uint32_t dataSize;
  uint8_t reqId;
} StorageReq_t;

/* Public type values -------------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/
extern uint8_t StorageRequest(uint8_t reqId,
                              uint32_t address,
                              void *data,
                              uint32_t dataSize);
extern uint8_t StorageFaultLatched(void);

#endif /* __STORAGE_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
