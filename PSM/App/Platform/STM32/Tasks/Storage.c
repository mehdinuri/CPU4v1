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

uint8_t StorageRequestParse(StorageReq_t *req)
{
  switch (req->reqId)
  {
      case STORAGE_REQ_EEPROM_WRITE:
      case STORAGE_REQ_EEPROM_WRITE_ASYNCH:
      {
        return I2CEEPROMWrite((uint32_t) req->address,
                              req->buf,
                              req->dataSize);
      }

      case STORAGE_REQ_EEPROM_READ:
      {
        uint8_t ok = I2CEEPROMRead((uint32_t) req->address,
                                   req->buf,
                                   req->dataSize);

        /* Only touch the caller's buffer after a successful read, and only
         * once — the request struct owns the staged bytes so a timed-out
         * caller's stack is never aliased. */
        if ((ok != FALSE) && (req->readTarget != NULL))
        {
          memcpy(req->readTarget, req->buf, req->dataSize);
        }

        return ok;
      }

      default:
      {
        return FALSE;
      }
  }
}

/* Public application code --------------------------------------------------*/
uint8_t StorageRequest(uint8_t reqId,
                       uint32_t address,
                       void *data,
                       uint32_t dataSize)
{
  osThreadId selfId = osThreadGetId();

  if (selfId == NULL)
  {
    Error_Handler();
  }

  /* Request payload must fit in the inline buffer — anything larger would
   * silently truncate on EEPROM, so reject up front. */
  if (dataSize > STORAGE_MAX_PAYLOAD)
  {
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
    req->dataSize = dataSize;

    if (reqId == STORAGE_REQ_EEPROM_READ)
    {
      req->readTarget = data;
    }
    else if (data != NULL)
    {
      /* WRITE / WRITE_ASYNCH — snapshot caller's bytes into the request so
       * the I2C layer no longer touches the caller's memory. */
      memcpy(req->buf, data, dataSize);
    }

    /* Clear any stale completion flags left over from a previously
     * timed-out request — otherwise the wait below could observe the late
     * signal from that old request and return a bogus success. */
    if (reqId != STORAGE_REQ_EEPROM_WRITE_ASYNCH)
    {
      (void) osThreadFlagsClear(THREAD_FLAGS_STORAGE_REQ_PROCESS_OK
                                | THREAD_FLAGS_STORAGE_REQ_PROCESS_ERROR);
    }

    if (osMessageQueuePut(StorageReqsQueueHandle, &req, 0, 0) == osOK)
    {
      if (reqId == STORAGE_REQ_EEPROM_WRITE_ASYNCH)
      {
        return TRUE;
      }
      else
      {
        uint32_t flags =
          osThreadFlagsWait(THREAD_FLAGS_STORAGE_REQ_PROCESS_OK
                            | THREAD_FLAGS_STORAGE_REQ_PROCESS_ERROR,
                            osFlagsWaitAny,
                            STORAGE_REQUEST_TIMEOUT_MS);

        /* On timeout, flags contains osFlagsErrorTimeout (a negative
         * error code cast to uint32_t) — the OK bit won't be set, so
         * the return value is correctly FALSE. */
        return (flags & THREAD_FLAGS_STORAGE_REQ_PROCESS_OK) != 0;
      }
    }
    else
    {
      osMemoryPoolFree(StorageReqsMemPoolHandle, req);
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

  StorageReq_t *req = NULL;

  StorageInit();

  /* Infinite loop */
  while (pdTRUE)
  {
    if (osMessageQueueGet(StorageReqsQueueHandle,
                          &req,
                          NULL,
                          MAINTENANCE_TASK_TIMEOUT_MS) == osOK)
    {
      uint8_t result = StorageRequestParse(req);

      if (req->reqId != STORAGE_REQ_EEPROM_WRITE_ASYNCH)
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
