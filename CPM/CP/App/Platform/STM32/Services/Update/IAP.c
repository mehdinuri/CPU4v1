/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "IAP.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MCSAsynch.h"
#include "MSM.h"
#include "fdcan.h"
#include "main.h"
#include "math.h"
#include "PersistencePorts.h"
#include "ui.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  members */
__IO uint32_t ulFlashDestination;

__attribute__((section(".dtcm_bss"), aligned(32)))
uint8_t baDownloadPacketData[IAP_BLOCK_WRITE_MAX_SIZE
                             + IAP_DATA_PACKET_MAX_SIZE] = { 0 };

__IO uint32_t ulFlashSource;
uint8_t baUploadPacketData[IAP_DATA_PACKET_MAX_SIZE] = { 0 };

static tSIAPBinInfo SIAPBinInfo;

tSIAPData SIAPTransmit;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  methods */
void IAPInit(void)
{
  if (IAPReadBinInfo())
  {
    if (SIAPBinInfo.fIsInitialized != IAP_BINARY_FILES_INITIALIZED)
    {
      memset(&SIAPBinInfo, 0, sizeof(SIAPBinInfo));
      SIAPBinInfo.fIsInitialized = IAP_BINARY_FILES_INITIALIZED;

      if (!IAPWriteBinInfo())
      {
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_MAIN_STORAGE_SET_ERROR,
                   0,
                   0,
                   FLASH_STORAGE_ADDR_IAP_BINARY_INFO,
                   0);
      }
    }
  }
  else
  {
    LogRequest(LOG_REQ_APPEND_ASYNCH,
               NULL,
               EVENT_MAIN_STORAGE_GET_ERROR,
               0,
               0,
               FLASH_STORAGE_ADDR_IAP_BINARY_INFO,
               0);
  }
}

uint8_t IAPCalcByteChkSum(void *pvMem, uint16_t sLen)
{
  uint8_t bCheckSum = 0;
  uint16_t sBytes = sLen;
  uint8_t *pbByte = pvMem;

  while (sBytes)
  {
    bCheckSum ^= *pbByte;
    pbByte++;
    sBytes--;
  }

  return bCheckSum;
}

void IAPPrepareMessage(uint8_t bIAPType,
                       uint8_t bCmdType,
                       uint8_t bPacketType,
                       uint16_t sPacketNo,
                       uint16_t sTotPackets,
                       void *pvData,
                       uint8_t bLen)
{
  memset(&SIAPTransmit, 0, sizeof(SIAPTransmit));

  SIAPTransmit.bIAPType = bIAPType;
  SIAPTransmit.bCommandType = bCmdType;
  SIAPTransmit.bPacketType = bPacketType;
  SIAPTransmit.sPacketNumber = sPacketNo;
  SIAPTransmit.sPacketCount = sTotPackets;
  memcpy(SIAPTransmit.baData, pvData, bLen);
  SIAPTransmit.baData[bLen] = IAPCalcByteChkSum(&SIAPTransmit,
                                                (IAP_PACKET_HEAD_SIZE + bLen));
}

void IAPSendMessage(uint8_t bSrc, uint8_t bDataLen)
{
  switch (bSrc)
  {
      case IAP_REQUEST_SOURCE_MCS:
      {
        MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_START_IAP,
                          bDataLen + IAP_PACKET_OVERHEAD_SIZE,
                          &SIAPTransmit);
        break;
      }

      case IAP_REQUEST_SOURCE_UI_SERIAL:
      case IAP_REQUEST_SOURCE_UI_USB:
      {
        uint16_t sDataIndex = 0;
        char strData[UI_COMM_MAX_PACKET_LENGTH + 1] = { '\0' };
        char strPartData[4] = { '\0' };

        uint8_t bReqId = (bSrc
                          == IAP_REQUEST_SOURCE_UI_SERIAL)
                         ? UI_REQ_TYPE_SERIAL : UI_REQ_TYPE_USB;
        uint16_t sPacketLen = bDataLen + IAP_PACKET_OVERHEAD_SIZE;
        uint8_t sPacketLenSize = DigitCountsGet(sPacketLen);

        strcat(strData, UI_COMM_START_OF_PACKET_STR);
        strcat(strData, UI_COMM_PACKET_IAP_STR);
        strcat(strData, UI_COMM_DATA_FIELD_SEPARATOR_STR);
        sprintf(strPartData, "%d", sPacketLen);
        strcat(strData, strPartData);
        strcat(strData, UI_COMM_DATA_SEPARATOR_STR);
        sDataIndex = UI_COMM_IAP_DATA_LEN_INDEX + sPacketLenSize + 1;
        memcpy(&strData[sDataIndex], &SIAPTransmit, sPacketLen);
        sDataIndex += sPacketLen;
        strData[sDataIndex] = UI_COMM_END_OF_PACKET;
        sDataIndex += 1;

        UITxRequest(bReqId, strData, sDataIndex);
        break;
      }
  }
} /* IAPSendMessage */

uint8_t IAPRequest(uint8_t bSrc, uint8_t bLen, void *pvData)
{
  #ifdef IAP_EN
  tpSIAPRequest pSReq =
    (tpSIAPRequest) osMemoryPoolAlloc(IAPRxReqsMemPoolHandle,
                                      0);

  if (pSReq != NULL)
  {
    memset(pSReq, 0, sizeof(tSIAPRequest));

    pSReq->bSource = bSrc;
    pSReq->bLength = bLen;
    memcpy(pSReq->UData.baData, pvData, bLen);

    if (osMessageQueuePut(IAPRxReqsQueHandle, &pSReq, 0, 0) == osOK)
    {
      return TRUE;
    }

    osMemoryPoolFree(IAPRxReqsMemPoolHandle,
                     pSReq);
  }

  #endif /* ifdef IAP_EN */

  return FALSE;
}

uint8_t IAPReadBinInfo(void)
{
  #ifdef IAP_EN

  return PersistenceRead(&g_persistencePort,
                         PERSIST_OBJECT_IAP_BINARY_INFO,
                         0U,
                         &SIAPBinInfo,
                         sizeof(SIAPBinInfo));
  #endif

  return FALSE;
}

uint8_t IAPWriteBinInfo(void)
{
  #ifdef IAP_EN
  if (PersistenceErase(&g_persistencePort,
                       PERSIST_OBJECT_IAP_BINARY_INFO,
                       0U,
                       sizeof(SIAPBinInfo)))
  {
    return PersistenceWrite(&g_persistencePort,
                            PERSIST_OBJECT_IAP_BINARY_INFO,
                            0U,
                            &SIAPBinInfo,
                            sizeof(SIAPBinInfo));
  }

  #endif /* ifdef IAP_EN */

  return FALSE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  tasks */
void IAPMsgParserTaskFunc(void *argument)
{
  UNUSED(argument);
  #ifndef IAP_EN
  osThreadTerminate(osThreadGetId());
  #endif
  tpSIAPRequest pSIAPReq = NULL;
  uint32_t ulDownloadTotBytes = 0;
  uint16_t sDownloadPacketsReceived = 0;
  uint32_t ulDownloadPacketDataIndex = 0;

  uint8_t bLastPacketReceived = FALSE;

  osDelay(2000);

  IAPInit();

  while (FOREVER)
  {
    if (osMessageQueueGet(IAPRxReqsQueHandle, &pSIAPReq, NULL,
                          osWaitForever) == osOK)
    {
      switch (pSIAPReq->UData.SData.bIAPType)
      {
          case IAP_TYPE_DOWNLOAD:
          {
            uint8_t bPacketDataLen = pSIAPReq->bLength
                                     - IAP_PACKET_OVERHEAD_SIZE;

            if (pSIAPReq->UData.SData.baData[bPacketDataLen]
                != IAPCalcByteChkSum(&pSIAPReq->UData.SData,
                                     (
                                       pSIAPReq->bLength - 1)))
            {
              uint8_t bData = IAP_STATUS_DATA_CHECKSUM_ERROR;

              IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                IAP_COMMAND_TYPE_STATUS,
                                pSIAPReq->UData.SData.bPacketType,
                                sDownloadPacketsReceived,
                                pSIAPReq->UData.SData.sPacketCount,
                                &bData,
                                1);
              IAPSendMessage(pSIAPReq->bSource, 1);
              break;
            }

            switch (pSIAPReq->UData.SData.bCommandType)
            {
                case IAP_COMMAND_TYPE_START:
                {
                  uint32_t ulFlashSize = 0;

                  switch (pSIAPReq->UData.SData.bPacketType)
                  {
                      case IAP_PACKET_TYPE_CP:
                      {
                        ulFlashDestination = 0U;
                        ulFlashSize = PersistenceGetCapacity(&g_persistencePort,
                                                             PERSIST_OBJECT_IAP_BACKUP_IMAGE);
                        break;
                      }

                      default:
                      {
                        break;
                      }
                  }

                  if (((pSIAPReq->bSource == IAP_REQUEST_SOURCE_UI_SERIAL)
                       && (pSIAPReq->UData.SData.sPacketCount
                           *
                           IAP_DATA_PACKET_SERIAL_MAX_SIZE
                           >
                           ulFlashSize) )
                      || ((pSIAPReq->bSource == IAP_REQUEST_SOURCE_UI_USB)
                          && (pSIAPReq->UData.SData.sPacketCount
                              *
                              IAP_DATA_PACKET_USB_MAX_SIZE
                              >
                              ulFlashSize) ))
                  {
                    uint8_t bData = IAP_STATUS_DATA_SIZE_ERROR;

                    IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                      IAP_COMMAND_TYPE_STATUS,
                                      pSIAPReq->UData.SData.bPacketType,
                                      sDownloadPacketsReceived,
                                      pSIAPReq->UData.SData.sPacketCount,
                                      &bData,
                                      1);
                    IAPSendMessage(pSIAPReq->bSource, 1);
                    break;
                  }

                  switch (pSIAPReq->UData.SData.baData[0])
                  {
                      case IAP_START_TYPE_START:
                      {
                        ulDownloadTotBytes = 0;
                        ulDownloadPacketDataIndex = 0;
                        sDownloadPacketsReceived = 0;
                        bLastPacketReceived = FALSE;
                        memset(baDownloadPacketData, 0,
                               sizeof(baDownloadPacketData));

                        if (PersistenceErase(&g_persistencePort,
                                             PERSIST_OBJECT_IAP_BACKUP_IMAGE,
                                             ulFlashDestination,
                                             ulFlashSize) == FALSE)
                        {
                          uint8_t bData = IAP_STATUS_DATA_STORAGE_ERROR;

                          IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                            IAP_COMMAND_TYPE_STATUS,
                                            pSIAPReq->UData.SData.bPacketType,
                                            sDownloadPacketsReceived,
                                            pSIAPReq->UData.SData.sPacketCount,
                                            &bData,
                                            1);
                          IAPSendMessage(pSIAPReq->bSource, 1);
                        }
                        else
                        {
                          uint8_t bData = MCS_ASYNCH_MSG_ACK;

                          IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                            IAP_COMMAND_TYPE_DATA,
                                            pSIAPReq->UData.SData.bPacketType,
                                            0,
                                            pSIAPReq->UData.SData.sPacketCount,
                                            &bData,
                                            1);
                          IAPSendMessage(pSIAPReq->bSource, 1);
                        }

                        break;
                      }

                      case IAP_START_TYPE_RESUME:
                      {
                        uint8_t bData = MCS_ASYNCH_MSG_NAK;

                        IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                          IAP_COMMAND_TYPE_DATA,
                                          pSIAPReq->UData.SData.bPacketType,
                                          sDownloadPacketsReceived,
                                          pSIAPReq->UData.SData.sPacketCount,
                                          &bData,
                                          1);
                        IAPSendMessage(pSIAPReq->bSource, 1);
                        break;
                      }
                  } /* switch */

                  break;
                }

                case IAP_COMMAND_TYPE_STATUS:
                {
                  switch (pSIAPReq->UData.SData.baData[0])
                  {
                      case IAP_STATUS_DATA_SIZE_ERROR:
                      case IAP_STATUS_DATA_DATA_ERROR:
                      case IAP_STATUS_DATA_STORAGE_ERROR:
                      case IAP_STATUS_DATA_COMMUNICATION_ERROR:
                      {
                        break;
                      }

                      case IAP_STATUS_DATA_FINISHED:
                      {
                        SIAPBinInfo.fIsInitialized =
                          IAP_BINARY_FILES_INITIALIZED;
                        SIAPBinInfo.ulSize = ulDownloadTotBytes;

                        uint8_t bData = IAP_STATUS_DATA_FINISHED;

                        IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                          IAP_COMMAND_TYPE_STATUS,
                                          pSIAPReq->UData.SData.bPacketType,
                                          sDownloadPacketsReceived,
                                          pSIAPReq->UData.SData.sPacketCount,
                                          &bData,
                                          1);
                        IAPSendMessage(pSIAPReq->bSource, 1);
                        break;
                      }

                      case IAP_STATUS_DATA_SWITCHING_MODE:
                      {
                        uint32_t ulVectorTableWord = 0U;

                        if ((SIAPBinInfo.ulSize != 0)
                            && (PersistenceRead(&g_persistencePort,
                                                PERSIST_OBJECT_IAP_BACKUP_IMAGE,
                                                0U,
                                                &ulVectorTableWord,
                                                sizeof(ulVectorTableWord))
                                != FALSE)
                            && ((ulVectorTableWord & 0x2FFC0000U)
                                == 0x20000000U))
                        {
                          switch (pSIAPReq->UData.SData.bPacketType)
                          {
                              case IAP_PACKET_TYPE_CP:
                              {
                                SIAPBinInfo.fIsUpdated = TRUE;

                                if (IAPWriteBinInfo())
                                {
                                  uint8_t bData =
                                    IAP_STATUS_DATA_SWITCHING_MODE;

                                  IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                                    IAP_COMMAND_TYPE_STATUS,
                                                    pSIAPReq->UData.SData.
                                                    bPacketType,
                                                    sDownloadPacketsReceived,
                                                    pSIAPReq->UData.SData.
                                                    sPacketCount,
                                                    &bData,
                                                    1);
                                  IAPSendMessage(pSIAPReq->bSource, 1);

                                  osDelay(1000);
                                  NVIC_SystemReset();
                                }
                                else
                                {
                                  uint8_t bData = IAP_STATUS_DATA_STORAGE_ERROR;

                                  LogRequest(LOG_REQ_APPEND_ASYNCH,
                                             NULL,
                                             EVENT_MAIN_STORAGE_SET_ERROR,
                                             0,
                                             0,
                                             FLASH_STORAGE_ADDR_IAP_BINARY_INFO,
                                             0);

                                  IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                                    IAP_COMMAND_TYPE_STATUS,
                                                    pSIAPReq->UData.SData.
                                                    bPacketType,
                                                    sDownloadPacketsReceived,
                                                    pSIAPReq->UData.SData.
                                                    sPacketCount,
                                                    &bData,
                                                    1);
                                  IAPSendMessage(pSIAPReq->bSource, 1);
                                }

                                break;
                              }

                              default:
                              {
                                break;
                              }
                          } /* switch */
                        }
                        else
                        {
                          uint8_t bData = IAP_STATUS_DATA_STORAGE_ERROR;

                          IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                            IAP_COMMAND_TYPE_STATUS,
                                            pSIAPReq->UData.SData.bPacketType,
                                            sDownloadPacketsReceived,
                                            pSIAPReq->UData.SData.sPacketCount,
                                            &bData,
                                            1);
                          IAPSendMessage(pSIAPReq->bSource,
                                         1);
                        }

                        break;
                      }
                  } /* switch */

                  break;
                }

                case IAP_COMMAND_TYPE_DATA:
                {
                  if (sDownloadPacketsReceived
                      != pSIAPReq->UData.SData.sPacketNumber)
                  {
                    uint8_t bData = MCS_ASYNCH_MSG_NAK;

                    IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                      IAP_COMMAND_TYPE_DATA,
                                      pSIAPReq->UData.SData.bPacketType,
                                      sDownloadPacketsReceived,
                                      pSIAPReq->UData.SData.sPacketCount,
                                      &bData,
                                      1);
                    IAPSendMessage(pSIAPReq->bSource, 1);
                    break;
                  }

                  bLastPacketReceived = FALSE;
                  uint32_t ulWriteablePacketLen = 0;

                  memcpy(&baDownloadPacketData[ulDownloadPacketDataIndex],
                         pSIAPReq->UData.SData.baData,
                         bPacketDataLen);
                  ulDownloadPacketDataIndex += bPacketDataLen;

                  if (sDownloadPacketsReceived
                      == pSIAPReq->UData.SData.sPacketCount)
                  {
                    ulWriteablePacketLen = ulDownloadPacketDataIndex;
                    bLastPacketReceived = TRUE;
                  }

                  if (ulDownloadPacketDataIndex > IAP_BLOCK_WRITE_MAX_SIZE)
                  {
                    ulWriteablePacketLen = IAP_BLOCK_WRITE_MAX_SIZE;
                  }

                  if (ulWriteablePacketLen == 0)
                  {
                    uint8_t bData = MCS_ASYNCH_MSG_ACK;

                    IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                      IAP_COMMAND_TYPE_DATA,
                                      pSIAPReq->UData.SData.bPacketType,
                                      sDownloadPacketsReceived,
                                      pSIAPReq->UData.SData.sPacketCount,
                                      &bData,
                                      1);
                    IAPSendMessage(pSIAPReq->bSource, 1);
                    sDownloadPacketsReceived++;
                  }
                  else
                  {
                    if (PersistenceWrite(&g_persistencePort,
                                         PERSIST_OBJECT_IAP_BACKUP_IMAGE,
                                         ulFlashDestination,
                                         baDownloadPacketData,
                                         ulWriteablePacketLen))
                    {
                      ulDownloadTotBytes += ulWriteablePacketLen;
                      ulFlashDestination += ulWriteablePacketLen;
                      ulDownloadPacketDataIndex -= ulWriteablePacketLen;

                      if ((ulDownloadPacketDataIndex > 0)
                          && (bLastPacketReceived == FALSE) )
                      {
                        memcpy(baDownloadPacketData,
                               &baDownloadPacketData[IAP_BLOCK_WRITE_MAX_SIZE],
                               ulDownloadPacketDataIndex); /* Copy surplus data to head */
                      }

                      uint8_t bData = MCS_ASYNCH_MSG_ACK;

                      IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                        IAP_COMMAND_TYPE_DATA,
                                        pSIAPReq->UData.SData.bPacketType,
                                        sDownloadPacketsReceived,
                                        pSIAPReq->UData.SData.sPacketCount,
                                        &bData,
                                        1);
                      IAPSendMessage(pSIAPReq->bSource, 1);
                      sDownloadPacketsReceived++;
                    }
                    else
                    {
                      uint8_t bData = IAP_STATUS_DATA_STORAGE_ERROR;

                      LogRequest(LOG_REQ_APPEND_ASYNCH,
                                 NULL,
                                 EVENT_MAIN_STORAGE_SET_ERROR,
                                 0,
                                 0,
                                 ulFlashDestination,
                                 0);

                      IAPPrepareMessage(IAP_TYPE_DOWNLOAD,
                                        IAP_COMMAND_TYPE_STATUS,
                                        pSIAPReq->UData.SData.bPacketType,
                                        sDownloadPacketsReceived,
                                        pSIAPReq->UData.SData.sPacketCount,
                                        &bData,
                                        1);
                      IAPSendMessage(pSIAPReq->bSource, 1);
                    }
                  }

                  break;
                }

                default:
                {
                  break;
                }
            } /* switch */

            break;
          }
      } /* switch */

      osMemoryPoolFree(IAPRxReqsQueHandle, pSIAPReq);
    }
  }
} /* IAPMsgParserTaskFunc */
