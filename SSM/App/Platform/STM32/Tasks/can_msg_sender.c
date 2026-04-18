/**
 ******************************************************************************
 * File Name          : can_msg_sender.c
 * Description        : Code for freertos applications
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include <string.h>
#include "task.h"
#include "can_msg_sender.h"
#include "utilities.h"
#include "fdcan.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t lCANTxFaultCount = 0U;

/* Private function prototypes -----------------------------------------------*/
void vCANMsgSenderInit(void);

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
void CANMsgSenderInit(void)
{
  lCANTxFaultCount = 0U;
}

static void CANTxFaultRecord(void)
{
  lCANTxFaultCount++;
  MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_TX_FAULT);
}

/* Public application code --------------------------------------------------*/
uint8_t CANTxRequest(FDCAN_HandleTypeDef *hfdcan,
                     uint32_t lIDType,
                     uint32_t lID,
                     uint32_t lFrameType,
                     uint32_t lBitRateSwitch,
                     uint32_t lFDFormat,
                     const uint8_t *baData,
                     uint8_t bDataLen)
{
  if ((hfdcan == NULL)
      || (bDataLen > FDCAN_MAX_DATA_LEN)
      || ((bDataLen != 0U) && (baData == NULL)))
  {
    CANTxFaultRecord();

    return 0U;
  }

  tpSFDCANTxMsg pSReq =
    (tpSFDCANTxMsg) osMemoryPoolAlloc(CANTxReqsMemPoolHandle,
                                      0);

  if (pSReq != NULL)
  {
    memset(pSReq, 0, sizeof(tSFDCANTxMsg));

    pSReq->hfdcan = hfdcan;
    pSReq->STxHeader.IdType = lIDType;
    pSReq->STxHeader.Identifier = lID;
    pSReq->STxHeader.TxFrameType = lFrameType;
    pSReq->STxHeader.BitRateSwitch = lBitRateSwitch;
    pSReq->STxHeader.FDFormat = lFDFormat;
    pSReq->STxHeader.DataLength = CANGetTxDataLengthCode(bDataLen);
    pSReq->STxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    pSReq->STxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    pSReq->STxHeader.MessageMarker = 0x00;
    if (bDataLen != 0U)
    {
      memcpy(pSReq->baData, baData, bDataLen);
    }

    if (osMessageQueuePut(CANTxReqsQueueHandle, &pSReq, 0, 0) != osOK)
    {
      osMemoryPoolFree(CANTxReqsMemPoolHandle, pSReq);
      CANTxFaultRecord();

      return 0U;
    }

    return 1U;
  }

  CANTxFaultRecord();

  return 0U;
} /* CANTxRequest */

uint8_t CANTxFaultLatched(void)
{
  return (lCANTxFaultCount != 0U) ? 1U : 0U;
}

/* USER CODE BEGIN Header_vCANMsgSenderTask */

/**
 * @brief Function implementing the xCANMsgSenderTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_vCANMsgSenderTask */
void CANMsgSenderTaskFunc(void *argument)
{
  /* USER CODE BEGIN vCANMsgSenderTask */
  UNUSED(argument);

  tpSFDCANTxMsg pSTxMsg = NULL;

  CANMsgSenderInit();

  /* Infinite loop */
  while (pdTRUE)
  {
    if (osMessageQueueGet(CANTxReqsQueueHandle, &pSTxMsg, NULL,
                          MAINTENANCE_TASK_HEARTBEAT_PERIOD_MS) == osOK)
    {
      uint32_t lTxBufferIndex = 0U;

      if (CANSendMessage(pSTxMsg) != 0U)
      {
        lTxBufferIndex =
          HAL_FDCAN_GetLatestTxFifoQRequestBuffer(pSTxMsg->hfdcan);
      }

      if ((lTxBufferIndex == 0U)
          || (CANWaitTxComplete(pSTxMsg->hfdcan,
                                lTxBufferIndex,
                                ONE_CENTI_SECOND) == 0U))
      {
        CANTxFaultRecord();
      }

      osMemoryPoolFree(CANTxReqsMemPoolHandle, pSTxMsg);
    }

    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_SENDER_TASK_ACTIVE);
  }

  /* USER CODE END vCANMsgSenderTask */
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
