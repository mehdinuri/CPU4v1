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
static volatile uint32_t canTxFaultCount = 0U;

/* Private function prototypes -----------------------------------------------*/
void vCANMsgSenderInit(void);

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
void CANMsgSenderInit(void)
{
  canTxFaultCount = 0U;
}

static void CANTxFaultRecord(void)
{
  (void) __atomic_fetch_add(&canTxFaultCount, 1U, __ATOMIC_RELAXED);
  MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_TX_FAULT);
}

/* Public application code --------------------------------------------------*/
uint8_t CANTxRequest(FDCAN_HandleTypeDef *hfdcan,
                     uint32_t idType,
                     uint32_t id,
                     uint32_t frameType,
                     uint32_t bitRateSwitch,
                     uint32_t fdFormat,
                     const uint8_t *data,
                     uint8_t dataLen)
{
  if ((hfdcan == NULL)
      || (dataLen > FDCAN_MAX_DATA_LEN)
      || ((dataLen != 0U) && (data == NULL)))
  {
    CANTxFaultRecord();

    return 0U;
  }

  FdcanTxMsg_t *req =
    (FdcanTxMsg_t *) osMemoryPoolAlloc(CANTxReqsMemPoolHandle,
                                       0);

  if (req != NULL)
  {
    memset(req, 0, sizeof(FdcanTxMsg_t));

    req->hfdcan = hfdcan;
    req->txHeader.IdType = idType;
    req->txHeader.Identifier = id;
    req->txHeader.TxFrameType = frameType;
    req->txHeader.BitRateSwitch = bitRateSwitch;
    req->txHeader.FDFormat = fdFormat;
    req->txHeader.DataLength = CANGetTxDataLengthCode(dataLen);
    req->txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    req->txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    req->txHeader.MessageMarker = 0x00;
    if (dataLen != 0U)
    {
      memcpy(req->data, data, dataLen);
    }

    if (osMessageQueuePut(CANTxReqsQueueHandle, &req, 0, 0) != osOK)
    {
      osMemoryPoolFree(CANTxReqsMemPoolHandle, req);
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
  uint32_t count = __atomic_load_n(&canTxFaultCount, __ATOMIC_RELAXED);

  return (count != 0U) ? 1U : 0U;
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

  FdcanTxMsg_t *txMsg = NULL;

  CANMsgSenderInit();

  /* Infinite loop */
  while (pdTRUE)
  {
    if (osMessageQueueGet(CANTxReqsQueueHandle, &txMsg, NULL,
                          MAINTENANCE_TASK_HEARTBEAT_PERIOD_MS) == osOK)
    {
      uint32_t txBufferIndex = 0U;

      if (CANSendMessage(txMsg) != 0U)
      {
        txBufferIndex =
          HAL_FDCAN_GetLatestTxFifoQRequestBuffer(txMsg->hfdcan);
      }

      if ((txBufferIndex == 0U)
          || (CANWaitTxComplete(txMsg->hfdcan,
                                txBufferIndex,
                                ONE_CENTI_SECOND) == 0U))
      {
        CANTxFaultRecord();
      }

      osMemoryPoolFree(CANTxReqsMemPoolHandle, txMsg);
    }

    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_SENDER_TASK_ACTIVE);
  }

  /* USER CODE END vCANMsgSenderTask */
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
