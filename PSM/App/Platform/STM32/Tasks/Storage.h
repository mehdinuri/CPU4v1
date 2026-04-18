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
} tEStorageRequestType;

typedef struct _tSStorageReq
{
  osThreadId_t SThreadId;
  uint32_t lAddress;
  void *pvData;
  uint32_t lDataSize;
  uint8_t bReqId;
} tSStorageReq, *tpSStorageReq;

/* Public type values -------------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/
extern uint8_t StorageRequest(uint8_t bReqId,
                              uint32_t lAddress,
                              void *pvData,
                              uint32_t lDataSize);

#endif /* APP_PLATFORM_STM32_TASKS_STORAGE_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
