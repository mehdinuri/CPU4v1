/*
 * Platform/STM32/Tasks/StorageTask.c
 *
 * FreeRTOS task that Services asynchronous persistent storage requests.
 * Callers (Domain config loader, SNMP SET handlers) enqueue
 * StorageRequest_t items; this task drains the queue and calls the
 * IPersistentStoragePort write/read/erase operations.
 *
 * Using a dedicated task serialises all flash operations and keeps them
 * off the high-priority ProgramTask.
 *
 * Priority : osPriorityLow
 * Trigger  : osMessageQueueGet (blocking)
 * Argument : StorageTaskArgs_t*
 */
#include "Tasks.h"
#include "Ports/IPersistentStoragePort.h"

/* Operation type codes. */
typedef enum
{
  STORAGE_REQ_WRITE = 0,
  STORAGE_REQ_READ,
  STORAGE_REQ_ERASE,
} StorageReqType_t;

/* Maximum inline data size in a storage request.
 * Larger blobs must be passed by pointer (caller must keep buffer alive). */
#define STORAGE_REQ_DATA_MAX  64U

/* Storage request item placed in the osMessageQueue. */
typedef struct
{
  StorageReqType_t type;
  uint16_t key;
  uint8_t data[STORAGE_REQ_DATA_MAX];
  uint16_t dataLen;
  StorageResult_t  *resultOut;     /* Optional: caller writes result here */
} StorageRequest_t;

/* Task argument struct injected from main_stm32.c. */
typedef struct
{
  IPersistentStoragePort_t *storage;     /* Flash adapter port           */
  osMessageQueueId_t queue;              /* Request queue handle         */
} StorageTaskArgs_t;

void StorageTask(void *argument)
{
  StorageTaskArgs_t *args = (StorageTaskArgs_t *) argument;
  StorageRequest_t req;

  for (;;)
  {
    if ((args == NULL) || (args->queue == NULL) )
    {
      osDelay(1000U);
      continue;
    }

    /* Block until a request arrives. */
    osStatus_t status = osMessageQueueGet(args->queue, &req, NULL,
                                          osWaitForever);

    if (status != osOK)
    {
      continue;
    }

    if (args->storage == NULL)
    {
      if (req.resultOut != NULL)
      {
        *req.resultOut = STORAGE_ERR_WRITE_FAILED;
      }

      continue;
    }

    StorageResult_t result = STORAGE_OK;

    switch (req.type)
    {
        case STORAGE_REQ_WRITE:
        {
          result = Storage_Write(args->storage, req.key,
                                 req.data, (size_t) req.dataLen);
          break;
        }

        case STORAGE_REQ_READ:
        {
          size_t outLen = 0U;

          result = Storage_Read(args->storage, req.key,
                                req.data, STORAGE_REQ_DATA_MAX, &outLen);
          req.dataLen = (uint16_t) outLen;
          break;
        }

        case STORAGE_REQ_ERASE:
        {
          result = Storage_Erase(args->storage, req.key);
          break;
        }

        default:
        {
          result = STORAGE_ERR_WRITE_FAILED;
          break;
        }
    }

    if (req.resultOut != NULL)
    {
      *req.resultOut = result;
    }
  }
} /* StorageTask */
