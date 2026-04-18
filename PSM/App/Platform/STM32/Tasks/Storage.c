/**
 ******************************************************************************
 * File Name          : Storage.c
 * Description        : Code for freertos applications
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Storage.h"
#include "utilities.h"
#include "i2c.h"
#include <string.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
void StorageInit(void)
{
}

uint8_t StorageRequestParse(tpSStorageReq pSReq)
{
  switch (pSReq->bReqId)
  {
      case STORAGE_REQ_EEPROM_WRITE:
      case STORAGE_REQ_EEPROM_WRITE_ASYNCH:
      {
        return I2CEEPROMWrite((uint32_t) pSReq->lAddress,
                              pSReq->pvData,
                              pSReq->lDataSize);
      }

      case STORAGE_REQ_EEPROM_READ:
      {
        return I2CEEPROMRead((uint32_t) pSReq->lAddress,
                             pSReq->pvData,
                             pSReq->lDataSize);
      }
  }

  return FALSE;
}

/* Public application code --------------------------------------------------*/
uint8_t StorageRequest(uint8_t bReqId,
                       uint32_t lAddress,
                       void *pvData,
                       uint32_t lDataSize)
{
  osThreadId SSelfId = osThreadGetId();

  if (SSelfId == NULL)
  {
    Error_Handler();
  }

  tpSStorageReq pSReq =
    (tpSStorageReq) osMemoryPoolAlloc(StorageReqsMemPoolHandle,
                                      0);

  if (pSReq != NULL)
  {
    memset(pSReq, 0, sizeof(tSStorageReq));

    pSReq->SThreadId = SSelfId;
    pSReq->bReqId = bReqId;
    pSReq->lAddress = lAddress;
    pSReq->pvData = pvData;
    pSReq->lDataSize = lDataSize;

    if (osMessageQueuePut(StorageReqsQueueHandle, &pSReq, 0, 0) == osOK)
    {
      if (bReqId == STORAGE_REQ_EEPROM_WRITE_ASYNCH)
      {
        return TRUE;
      }
      else
      {
        uint32_t lFlags =
          osThreadFlagsWait(THREAD_FLAGS_STORAGE_REQ_PROCESS_OK
                            | THREAD_FLAGS_STORAGE_REQ_PROCESS_ERROR,
                            osFlagsWaitAny,
                            STORAGE_REQUEST_TIMEOUT_MS);

        /* On timeout, lFlags contains osFlagsErrorTimeout (a negative
         * error code cast to uint32_t) — the OK bit won't be set, so
         * the return value is correctly FALSE. */
        return (lFlags & THREAD_FLAGS_STORAGE_REQ_PROCESS_OK) != 0;
      }
    }
    else
    {
      osMemoryPoolFree(StorageReqsMemPoolHandle, pSReq);
    }
  }

  return FALSE;
} /* StorageRequest */

/* USER CODE BEGIN Header_vStorageTask */

/**
 * @brief Function implementing the xStorageTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_vStorageTask */
void StorageTaskFunc(void *argument)
{
  /* USER CODE BEGIN vStorageTask */
  UNUSED(argument);

  tpSStorageReq pSReq = NULL;

  StorageInit();

  /* Infinite loop */
  while (pdTRUE)
  {
    if (osMessageQueueGet(StorageReqsQueueHandle,
                          &pSReq,
                          NULL,
                          MAINTENANCE_TASK_TIMEOUT_MS) == osOK)
    {
      uint8_t fResult = StorageRequestParse(pSReq);

      if (pSReq->bReqId != STORAGE_REQ_EEPROM_WRITE_ASYNCH)
      {
        if (pSReq->SThreadId != NULL)
        {
          if (fResult)
          {
            osThreadFlagsSet(pSReq->SThreadId,
                             THREAD_FLAGS_STORAGE_REQ_PROCESS_OK);
          }
          else
          {
            osThreadFlagsSet(pSReq->SThreadId,
                             THREAD_FLAGS_STORAGE_REQ_PROCESS_ERROR);
          }
        }
      }

      osMemoryPoolFree(StorageReqsMemPoolHandle, pSReq);
    }

    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_STORAGE_TASK_ACTIVE);
  }

  /* USER CODE END vStorageTask */
} /* StorageTaskFunc */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
