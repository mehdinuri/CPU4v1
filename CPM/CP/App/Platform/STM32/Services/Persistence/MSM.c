/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "MSM.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MLM.h"
#include "cmsis_os2.h"
#include "defs.h"
#include "main.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  members */
static IFlashStoragePort_t *spFlashPort;
static IEepromStoragePort_t *spEepromPort;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  methods */
void MSMInit(IFlashStoragePort_t *flashPort,
             IEepromStoragePort_t *eepromPort)
{
  spFlashPort = flashPort;
  spEepromPort = eepromPort;
}

uint8_t MSMRequest(uint8_t bReqId, uint32_t lAddress, void *pvData,
                   uint32_t lDataSize)
{
  osThreadId_t SSelfId = osThreadGetId();

  if (SSelfId == NULL)
  {
    Error_Handler();
  }

  tpSMSMRequest pSMSMReq =
    (tpSMSMRequest) osMemoryPoolAlloc(MSMReqsMemPoolHandle,
                                      0);

  if (pSMSMReq != NULL)
  {
    memset(pSMSMReq, 0, sizeof(tSMSMRequest));

    pSMSMReq->SThreadId = SSelfId;
    pSMSMReq->bReqId = bReqId;
    pSMSMReq->lAddress = lAddress;
    pSMSMReq->pvData = pvData;
    pSMSMReq->lDataSize = lDataSize;

    if (osMessageQueuePut(MSMReqsQueHandle, &pSMSMReq, 0, 0) == osOK)
    {
      if (bReqId == MSM_REQ_FLASH_WRITE_ASYNCH)
      {
        return TRUE;
      }
      else
      {
        uint32_t lFlags = osThreadFlagsWait(THREAD_FLAGS_MSM_REQ_PROCESS_OK
                                            | THREAD_FLAGS_MSM_REQ_PROCESS_ERROR,
                                            osFlagsWaitAny,
                                            osWaitForever);

        return (lFlags & THREAD_FLAGS_MSM_REQ_PROCESS_OK) != 0;
      }
    }
    else
    {
      osMemoryPoolFree(MSMReqsMemPoolHandle, pSMSMReq);
    }
  }

  return FALSE;
} /* MSMRequest */

uint8_t MSMReqParse(tpSMSMRequest pSMSMReq)
{
  if (pSMSMReq == NULL)
  {
    return FALSE;
  }

  switch (pSMSMReq->bReqId)
  {
      case MSM_REQ_FLASH_WRITE:
      case MSM_REQ_FLASH_WRITE_ASYNCH:
      {
        if (spFlashPort == NULL)
        {
          return FALSE;
        }

        return FlashStorageWrite(spFlashPort,
                                 (uint32_t) pSMSMReq->lAddress,
                                 pSMSMReq->pvData,
                                 pSMSMReq->lDataSize);
      }

      case MSM_REQ_FLASH_READ:
      {
        if (spFlashPort == NULL)
        {
          return FALSE;
        }

        return FlashStorageRead(spFlashPort,
                                (uint32_t) pSMSMReq->lAddress,
                                pSMSMReq->pvData,
                                pSMSMReq->lDataSize);
      }

      case MSM_REQ_FLASH_ERASE:
      {
        if (spFlashPort == NULL)
        {
          return FALSE;
        }

        return FlashStorageErase(spFlashPort,
                                 (uint32_t) pSMSMReq->lAddress,
                                 pSMSMReq->lDataSize);
      }

      case MSM_REQ_EEPROM_WRITE:
      {
        if (spEepromPort == NULL)
        {
          return FALSE;
        }

        return EepromStorageWrite(spEepromPort,
                                  (uint32_t) pSMSMReq->lAddress,
                                  pSMSMReq->pvData,
                                  pSMSMReq->lDataSize);
      }

      case MSM_REQ_EEPROM_READ:
      {
        if (spEepromPort == NULL)
        {
          return FALSE;
        }

        return EepromStorageRead(spEepromPort,
                                 (uint32_t) pSMSMReq->lAddress,
                                 pSMSMReq->pvData,
                                 pSMSMReq->lDataSize);
      }
  } /* switch */

  return FALSE;
} /* MSMReqParse */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  tasks */
void MSMTaskFunc(void *argument)
{
  UNUSED(argument);

  tpSMSMRequest pSMSMReq = NULL;

  while (FOREVER)
  {
    if (osMessageQueueGet(MSMReqsQueHandle, &pSMSMReq, NULL,
                          osWaitForever) == osOK)
    {
      uint8_t fResult = MSMReqParse(pSMSMReq);

      if (pSMSMReq->bReqId != MSM_REQ_FLASH_WRITE_ASYNCH)
      {
        if (pSMSMReq->SThreadId != NULL)
        {
          if (fResult)
          {
            osThreadFlagsSet(pSMSMReq->SThreadId,
                             THREAD_FLAGS_MSM_REQ_PROCESS_OK);
          }
          else
          {
            osThreadFlagsSet(pSMSMReq->SThreadId,
                             THREAD_FLAGS_MSM_REQ_PROCESS_ERROR);
          }
        }
      }

      osMemoryPoolFree(MSMReqsMemPoolHandle, pSMSMReq);
    }
  }
} /* MSMTaskFunc */
