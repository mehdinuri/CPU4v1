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

/* Private function prototypes -----------------------------------------------*/
void vCANMsgSenderInit(void);

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
void CANMsgSenderInit(void)
{
}

/* Public application code --------------------------------------------------*/
void CANTxRequest(FDCAN_HandleTypeDef *hfdcan,
                  uint32_t lIDType,
                  uint32_t lID,
                  uint32_t lFrameType,
                  uint32_t lBitRateSwitch,
                  uint32_t lFDFormat,
                  uint8_t *baData,
                  uint8_t bDataLen)
{
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
    memcpy(pSReq->baData, baData, bDataLen);

    if (osMessageQueuePut(CANTxReqsQueueHandle, &pSReq, 0, 0) != osOK)
    {
      osMemoryPoolFree(CANTxReqsMemPoolHandle, pSReq);
    }
  }
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
                          osWaitForever) == osOK)
    {
      CANSendMessage(pSTxMsg);
      CANWaitTxComplete(pSTxMsg->hfdcan);
      osMemoryPoolFree(CANTxReqsMemPoolHandle, pSTxMsg);
    }
  }

  /* USER CODE END vCANMsgSenderTask */
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
