/*
 * Platform/STM32/Tasks/CANTxTask.c
 *
 * FreeRTOS task that drains a message queue of CAN frames and transmits
 * them via FDCAN TxFIFO.
 *
 * CAN frames are enqueued by Domain adapters (SignalCardAdapter_Flush)
 * or other producers.  Using a queue decouples the Domain tick rate from
 * the CAN bus bandwidth.
 *
 * Priority : osPriorityAboveNormal
 * Trigger  : osMessageQueueGet (blocking)
 * Argument : osMessageQueueId_t  (CAN Tx queue handle)
 *
 * Queue element type: CANTxFrame_t (defined below).
 */
#include "Tasks.h"

/* CAN frame descriptor enqueued by producers. */
typedef struct
{
  uint32_t identifier;    /* Standard 11-bit CAN ID */
  uint8_t data[8];
  uint8_t dataLen;        /* 0-8 bytes */
} CANTxFrame_t;

/* FDCAN handle — provided externally (CubeMX global or passed via args). */
#ifdef STM32H743xx
#include "stm32h7xx_hal.h"
extern FDCAN_HandleTypeDef hfdcan1;
#endif

void CANTxTask(void *argument)
{
  osMessageQueueId_t txQueue = (osMessageQueueId_t) argument;
  CANTxFrame_t frame;

  for (;;)
  {
    /* Block until a frame is available (indefinite timeout). */
    osStatus_t status = osMessageQueueGet(txQueue, &frame, NULL, osWaitForever);

    if (status != osOK)
    {
      continue;
    }

    #ifdef STM32H743xx

    /* TODO: HAL impl — transmit frame via FDCAN TxFIFO.
     *
     * FDCAN_TxHeaderTypeDef txHdr = {
     *     .Identifier          = frame.identifier,
     *     .IdType              = FDCAN_STANDARD_ID,
     *     .TxFrameType         = FDCAN_DATA_FRAME,
     *     .DataLength          = FDCAN_DLC_BYTES_8,
     *     .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
     *     .BitRateSwitch       = FDCAN_BRS_OFF,
     *     .FDFormat            = FDCAN_CLASSIC_CAN,
     *     .TxEventFifoControl  = FDCAN_NO_TX_EVENTS,
     *     .MessageMarker       = 0,
     * };
     * HAL_FDCAN_AddMessageToTxFifo(&hfdcan1, &txHdr, frame.data);
     */
    (void) frame;
    #else
    (void) frame;
    #endif
  }
}
