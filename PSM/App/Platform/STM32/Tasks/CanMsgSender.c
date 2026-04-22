/**
 ******************************************************************************
 * File Name          : CanMsgSender.c
 * Description        : Code for freertos applications
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include "CanMsgSender.h"
#include "utilities.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void vCANMsgSenderInit(void);

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Saturating overflow counter.  Widened from uint8_t so it can no longer
 * silently wrap at 256.  Incremented under a brief IRQ lock to serialise
 * concurrent producers; stays at UINT32_MAX once it hits the ceiling. */
volatile uint32_t g_canTxOverflowCount = 0U;

static void CanTxOverflowBump(void)
{
  uint32_t primask = __get_PRIMASK();

  __disable_irq();
  if (g_canTxOverflowCount < 0xFFFFFFFFU)
  {
    g_canTxOverflowCount++;
  }

  if (primask == 0U)
  {
    __enable_irq();
  }
}

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
void CANMsgSenderInit(void)
{
}

/* Public application code --------------------------------------------------*/
void CANTxRequest(FDCAN_HandleTypeDef *hfdcan,
                  uint32_t idType,
                  uint32_t id,
                  uint32_t frameType,
                  uint32_t bitRateSwitch,
                  uint32_t fdFormat,
                  uint8_t *data,
                  uint8_t dataLen)
{
  if (dataLen > FDCAN_MAX_DATA_LEN)
  {
    dataLen = FDCAN_MAX_DATA_LEN;
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
    memcpy(req->data, data, dataLen);

    if (osMessageQueuePut(CANTxReqsQueueHandle, &req, 0, 0) != osOK)
    {
      osMemoryPoolFree(CANTxReqsMemPoolHandle, req);
      CanTxOverflowBump();
    }
  }
  else
  {
    CanTxOverflowBump();
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

  FdcanTxMsg_t *txMsg = NULL;

  CANMsgSenderInit();

  /* Infinite loop */
  while (pdTRUE)
  {
    if (osMessageQueueGet(CANTxReqsQueueHandle,
                          &txMsg,
                          NULL,
                          MAINTENANCE_TASK_TIMEOUT_MS) == osOK)
    {
      CANSendMessage(txMsg);
      CANWaitTxComplete(txMsg->hfdcan);
      osMemoryPoolFree(CANTxReqsMemPoolHandle, txMsg);
    }

    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_SENDER_TASK_ACTIVE);
  }

  /* USER CODE END vCANMsgSenderTask */
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
