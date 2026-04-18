/**
 ******************************************************************************
 * File Name          : storage.c
 * Description        : Code for freertos applications
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "main.h"
#include "task.h"
#include <string.h>
#include <stdlib.h>
#include "storage.h"
#include "utilities.h"
#include "flash.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t lStorageFaultCount = 0U;

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
void StorageInit(void)
{
  lStorageFaultCount = 0U;
}

static void StorageFaultRecord(void)
{
  lStorageFaultCount++;
  MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_STORAGE_FAULT);
}

uint8_t StorageRequestParse(tpSStorageReq pSReq)
{
  switch (pSReq->bReqId)
  {
      case STORAGE_REQ_FLASH_WRITE:
      case STORAGE_REQ_FLASH_WRITE_ASYNCH:
      {
        return FlashWrite(pSReq->lAddress, pSReq->pvData, pSReq->lDataSize);
      }

      case STORAGE_REQ_FLASH_READ:
      {
        return FlashRead(pSReq->lAddress, pSReq->pvData, pSReq->lDataSize);
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
    osThreadFlagsClear(THREAD_FLAGS_STORAGE_REQ_PROCESS_ALL);

    if (osMessageQueuePut(StorageReqsQueueHandle, &pSReq, 0, 0) == osOK)
    {
      if (bReqId == STORAGE_REQ_FLASH_WRITE_ASYNCH)
      {
        return TRUE;
      }
      else
      {
        uint32_t lFlags =
          osThreadFlagsWait(THREAD_FLAGS_STORAGE_REQ_PROCESS_OK
                            | THREAD_FLAGS_STORAGE_REQ_PROCESS_ERROR,
                            osFlagsWaitAny,
                            osWaitForever);

        if ((lFlags & 0x80000000U) != 0U)
        {
          StorageFaultRecord();

          return FALSE;
        }

        return (lFlags & THREAD_FLAGS_STORAGE_REQ_PROCESS_OK) != 0U;
      }
    }
    else
    {
      osMemoryPoolFree(StorageReqsMemPoolHandle, pSReq);
      StorageFaultRecord();
    }
  }
  else
  {
    StorageFaultRecord();
  }

  return FALSE;
} /* StorageRequest */

uint8_t StorageFaultLatched(void)
{
  return (lStorageFaultCount != 0U) ? 1U : 0U;
}

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
    if (osMessageQueueGet(StorageReqsQueueHandle, &pSReq, NULL,
                          MAINTENANCE_TASK_HEARTBEAT_PERIOD_MS) == osOK)
    {
      uint8_t fResult = StorageRequestParse(pSReq);

      if (fResult == FALSE)
      {
        StorageFaultRecord();
      }

      if (pSReq->bReqId != STORAGE_REQ_FLASH_WRITE_ASYNCH)
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
