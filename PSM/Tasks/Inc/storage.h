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

#endif /* __STORAGE_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
