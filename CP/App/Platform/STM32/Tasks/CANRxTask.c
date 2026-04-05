/*
 * Platform/STM32/Tasks/CANRxTask.c
 *
 * FreeRTOS wrapper that reads FDCAN RxFIFO0 and dispatches frames to:
 *   - DetectorInputAdapter_UpdateFromCAN()  for Detector status frames
 *   - SignalCardAdapter_NotifyAck()         for SSM acknowledgement frames
 *
 * Frame routing by CAN ID:
 *   0x100..0x17F  Detector status (base 0x100, each frame covers 8 Detectors)
 *   0x280..0x28B  SSM ACK frames  (base 0x280 = 0x200 + cardIdx | ACK flag)
 *
 * Priority : osPriorityHigh
 * Trigger  : HAL_FDCAN_RxFifo0Callback sets a task notification (osThreadFlagsSet)
 * Argument : CANTaskArgs_t*  (contains adapter context pointers)
 */
#include "Tasks.h"
#include "Adapters/STM32/DetectorInputAdapter.h"
#include "Adapters/STM32/SignalCardAdapter.h"

/* Argument struct injected from main_stm32.c */
typedef struct
{
  DetectorInputAdapterCtx_t *detCtx;
  SignalCardAdapterCtx_t    *sigCtx;
} CANTaskArgs_t;

/* CAN frame descriptor — mirrors FDCAN_RxHeaderTypeDef + data. */
typedef struct
{
  uint32_t identifier;
  uint8_t data[8];
  uint8_t dataLen;
} CANFrame_t;

/* CAN ID ranges */
#define CAN_ID_DETECTOR_BASE  0x100U
#define CAN_ID_DETECTOR_MAX   0x17FU
#define CAN_ID_SSM_ACK_BASE   0x280U
#define CAN_ID_SSM_ACK_MAX    0x28BU

void CANRxTask(void *argument)
{
  CANTaskArgs_t *args = (CANTaskArgs_t *) argument;
  CANFrame_t frame;

  for (;;)
  {
    /* Block until the FDCAN Rx interrupt notifies this task. */
    osThreadFlagsWait(0x0001U, osFlagsWaitAny, osWaitForever);

    /* Drain all pending frames from RxFIFO0. */

    /* TODO: HAL impl — loop while HAL_FDCAN_GetRxFifoFillLevel() > 0,
     * calling HAL_FDCAN_GetRxMessage() for each frame.
     *
     * FDCAN_RxHeaderTypeDef rxHdr;
     * while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0) {
     *     uint8_t rxData[8] = {0};
     *     HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxHdr, rxData);
     *     frame.identifier = rxHdr.Identifier;
     *     frame.dataLen    = (uint8_t)(rxHdr.DataLength >> 16U);  // DLC decode
     *     memcpy(frame.data, rxData, frame.dataLen);
     *     // ... dispatch below ...
     * }
     */

    /* --- Frame dispatch (skeleton, runs when HAL is wired) --- */
    if ((frame.identifier >= CAN_ID_DETECTOR_BASE)
        && (frame.identifier <= CAN_ID_DETECTOR_MAX) )
    {
      if ((args != NULL) && (args->detCtx != NULL) )
      {
        DetectorInputAdapter_UpdateFromCAN(args->detCtx,
                                           frame.data,
                                           frame.dataLen);
      }
    }
    else if ((frame.identifier >= CAN_ID_SSM_ACK_BASE)
             && (frame.identifier <= CAN_ID_SSM_ACK_MAX) )
    {
      if ((args != NULL) && (args->sigCtx != NULL) )
      {
        /* Map ACK CAN ID back to the SSM card base ID (0x200 range). */
        uint32_t cardBase = 0x200U + (frame.identifier - CAN_ID_SSM_ACK_BASE);

        SignalCardAdapter_NotifyAck(args->sigCtx, cardBase);
      }
    }

    /* Unknown IDs are silently discarded. */
  }
}
