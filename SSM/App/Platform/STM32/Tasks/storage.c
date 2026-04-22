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
static volatile uint32_t storageFaultCount = 0U;

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
void StorageInit(void)
{
  storageFaultCount = 0U;
}

static void StorageFaultRecord(void)
{
  (void) __atomic_fetch_add(&storageFaultCount, 1U, __ATOMIC_RELAXED);
  MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_STORAGE_FAULT);
}

uint8_t StorageRequestParse(StorageReq_t *req)
{
  if (req == NULL)
  {
    return FALSE;
  }

  switch (req->reqId)
  {
      case STORAGE_REQ_FLASH_WRITE:
      case STORAGE_REQ_FLASH_WRITE_ASYNCH:
      {
        return FlashWrite(req->address, req->data, req->dataSize);
      }

      case STORAGE_REQ_FLASH_READ:
      {
        return FlashRead(req->address, req->data, req->dataSize);
      }

      default:
      {
        return FALSE;
      }
  }
}

/* Public application code --------------------------------------------------*/

/* Bounded wait for a synchronous storage request. A stuck storage task
 * must not hang its callers and starve the maintenance watchdog — after
 * this window the request is reported failed and the caller moves on.
 * 1000 ms is ~2x the worst-case observed flash erase+program time.
 */
#define STORAGE_SYNC_REQ_TIMEOUT_MS 1000U

uint8_t StorageRequest(uint8_t reqId,
                       uint32_t address,
                       void *data,
                       uint32_t dataSize)
{
  osThreadId selfId = osThreadGetId();

  /* Called outside thread context (e.g. from an ISR) — we cannot wait on
   * thread flags. Record the fault and let the caller retry from a task.
   */
  if (selfId == NULL)
  {
    StorageFaultRecord();

    return FALSE;
  }

  StorageReq_t *req =
    (StorageReq_t *) osMemoryPoolAlloc(StorageReqsMemPoolHandle,
                                       0);

  if (req != NULL)
  {
    memset(req, 0, sizeof(StorageReq_t));

    req->threadId = selfId;
    req->reqId = reqId;
    req->address = address;
    req->data = data;
    req->dataSize = dataSize;
    osThreadFlagsClear(THREAD_FLAGS_STORAGE_REQ_PROCESS_ALL);

    if (osMessageQueuePut(StorageReqsQueueHandle, &req, 0, 0) == osOK)
    {
      if (reqId == STORAGE_REQ_FLASH_WRITE_ASYNCH)
      {
        return TRUE;
      }
      else
      {
        uint32_t flags =
          osThreadFlagsWait(THREAD_FLAGS_STORAGE_REQ_PROCESS_OK
                            | THREAD_FLAGS_STORAGE_REQ_PROCESS_ERROR,
                            osFlagsWaitAny,
                            STORAGE_SYNC_REQ_TIMEOUT_MS);

        /* CMSIS returns 0x8000_00xx on error, including osFlagsErrorTimeout. */
        if ((flags & 0x80000000U) != 0U)
        {
          StorageFaultRecord();

          return FALSE;
        }

        return (flags & THREAD_FLAGS_STORAGE_REQ_PROCESS_OK) != 0U;
      }
    }
    else
    {
      osMemoryPoolFree(StorageReqsMemPoolHandle, req);
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
  uint32_t count = __atomic_load_n(&storageFaultCount, __ATOMIC_RELAXED);

  return (count != 0U) ? 1U : 0U;
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

  StorageReq_t *req = NULL;

  StorageInit();

  /* Infinite loop */
  while (pdTRUE)
  {
    if (osMessageQueueGet(StorageReqsQueueHandle, &req, NULL,
                          MAINTENANCE_TASK_HEARTBEAT_PERIOD_MS) == osOK)
    {
      uint8_t result = StorageRequestParse(req);

      if (result == FALSE)
      {
        StorageFaultRecord();
      }

      if (req->reqId != STORAGE_REQ_FLASH_WRITE_ASYNCH)
      {
        if (req->threadId != NULL)
        {
          if (result)
          {
            osThreadFlagsSet(req->threadId,
                             THREAD_FLAGS_STORAGE_REQ_PROCESS_OK);
          }
          else
          {
            osThreadFlagsSet(req->threadId,
                             THREAD_FLAGS_STORAGE_REQ_PROCESS_ERROR);
          }
        }
      }

      osMemoryPoolFree(StorageReqsMemPoolHandle, req);
    }

    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_STORAGE_TASK_ACTIVE);
  }

  /* USER CODE END vStorageTask */
} /* StorageTaskFunc */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
