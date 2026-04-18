/**
 ******************************************************************************
 * File Name          : can_msg_parser.c
 * Description        : Code for freertos applications
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "utilities.h"
#include "can_msg_parser.h"
#include "measurement.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* STM32 HAL encodes DataLength as FDCAN_DLC_BYTES_x constants where the byte
 * count lives in bits [19:16]: FDCAN_DLC_BYTES_8 = 0x00080000.
 * Comparing DataLength directly to a raw byte count (e.g. >= 7U) is always
 * TRUE for any non-zero DLC because the constant values are >> 65535.
 * Use this macro to extract the actual byte count before range checks. */
#define CAN_DLC_TO_BYTES(dlc)  ((uint8_t) ((dlc) >> 16U))

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
void CANMsgParserInit(void)
{
}

void CANMsgParse(tpSFDCANRxMsg pSRxMsg)
{
  switch (pSRxMsg->SRxHeader.IdType)
  {
      case FDCAN_STANDARD_ID:
      {
        switch (pSRxMsg->SRxHeader.Identifier)
        {
            case FDCAN_CP_DATE_TIME_STD_ID:
            {
              MeasurementFlashStateSet(FALSE);
              MeasurementCommCntrReset();
              break;
            }

            case FDCAN_CP_FLASH_SIGNALS_1_STD_ID:
            {
              /* Period value lives at byte index 6 — frame must carry ≥ 7 bytes. */
              if (CAN_DLC_TO_BYTES(pSRxMsg->SRxHeader.DataLength) >= 7U)
              {
                MeasurementPeriodSet(pSRxMsg->baData[6]);
              }

              break;
            }

            case FDCAN_CP_OFFSET_1_STD_ID:
            case FDCAN_CP_OFFSET_2_STD_ID:
            {
              /* Op at byte 0, value at byte 1 — frame must carry ≥ 2 bytes. */
              if (CAN_DLC_TO_BYTES(pSRxMsg->SRxHeader.DataLength) >= 2U)
              {
                MeasurementOffsetSet(pSRxMsg->baData[0], pSRxMsg->baData[1]);
              }

              break;
            }

            default:
            {
              /* Unrecognised standard-ID — ignore */
              break;
            }
        }

        break;
      }

      default:
      {
        /* Extended-ID or reserved frame types — ignore */
        break;
      }
  }
}

/* USER CODE BEGIN Header_vCANMsgParserTask */

/**
 * @brief Function implementing the xCANMsgParserTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_vCANMsgParserTask */

/* Public application code --------------------------------------------------*/
void CANRxRequest(tpSFDCANRxMsg pSRxMsg)
{
  tpSFDCANRxMsg pSReq =
    (tpSFDCANRxMsg) osMemoryPoolAlloc(CANRxReqsMemPoolHandle,
                                      0);

  if (pSReq != NULL)
  {
    memcpy(pSReq, pSRxMsg, sizeof(tSFDCANRxMsg));
    if (osMessageQueuePut(CANRxReqsQueueHandle, &pSReq, 0, 0) != osOK)
    {
      osMemoryPoolFree(CANRxReqsMemPoolHandle, pSReq);
    }
  }
}

void CANMsgParserTaskFunc(void *argument)
{
  /* USER CODE BEGIN vCANMsgParserTask */
  UNUSED(argument);

  tpSFDCANRxMsg pSRxMsg = NULL;

  CANMsgParserInit();

  CANStart(&hfdcan1);

  /* Infinite loop */
  while (pdTRUE)
  {
    /* Wait up to one maintenance cycle for a new CAN frame; then signal
     * maintenance regardless so the watchdog knows this task is alive. */
    if (osMessageQueueGet(CANRxReqsQueueHandle,
                          &pSRxMsg,
                          NULL,
                          MAINTENANCE_TASK_TIMEOUT_MS) == osOK)
    {
      CANMsgParse(pSRxMsg);
      osMemoryPoolFree(CANRxReqsMemPoolHandle, pSRxMsg);
    }

    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_PARSER_TASK_ACTIVE);
  }

  /* USER CODE END vCANMsgParserTask */
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
