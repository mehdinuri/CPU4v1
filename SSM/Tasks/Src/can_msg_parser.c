/**
 ******************************************************************************
 * File Name          : can_msg_parser.c
 * Description        : Code for freertos applications
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include <string.h>
#include "task.h"
#include "utilities.h"
#include "gpio.h"
#include "can_msg_parser.h"
#include "measurement.h"
#include "fdcan.h"
#include "storage.h"
#include "Domain/SignalFlashConfig.h"
#include "Domain/FlashSyncWatchdog.h"
#include "Platform/STM32/Bootstrap/HardwarePorts.h"
/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  OUTPUT_SET_ACTIVE = 0,
  OUTPUT_SET_FLASH
} tEOutputSetType;

/* Private variables ---------------------------------------------------------*/
static uint8_t fFlashStatus = FALSE;
static uint8_t fFlashSyncStatus = FALSE;
static tSOutputGroup SaSignalOutputGroups[SIGNAL_GROUPS_PER_SSM];
/* Last flash-config we successfully saved; used to skip redundant writes. */
static tSSignalFlashConfig SLastSavedFlashCfg;

/* Private function prototypes -----------------------------------------------*/
void vCanMsgParserInit(void);

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
void CANMsgParserInit(void)
{
  memset(SaSignalOutputGroups, 0, sizeof(SaSignalOutputGroups));
  memset(&SLastSavedFlashCfg, 0, sizeof(SLastSavedFlashCfg));
}

static void SignalOutputFlashConfigSet(void)
{
  tSSignalFlashConfig SNewCfg;
  uint8_t bGroupIdx, bOutputIdx;

  memset(&SNewCfg, 0, sizeof(SNewCfg));
  for (bGroupIdx = 0; bGroupIdx < SIGNAL_GROUPS_PER_SSM; bGroupIdx++)
  {
    for (bOutputIdx = 0;
         bOutputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP;
         bOutputIdx++)
    {
      uint8_t bIdx = (bGroupIdx * SIGNAL_OUTPUTS_PER_SIGNAL_GROUP) + bOutputIdx;

      SNewCfg.aIsFlashing[bIdx] =
        SaSignalOutputGroups[bGroupIdx].SaSignalOutputs[bOutputIdx].fIsFlashing
        ? 1U : 0U;
    }
  }

  if (memcmp(&SNewCfg, &SLastSavedFlashCfg, sizeof(SNewCfg)) != 0)
  {
    if (SignalFlashConfig_Save(&g_PersistencePort, &SNewCfg))
    {
      SLastSavedFlashCfg = SNewCfg;
    }
  }
}

static void FlashStatusSet(uint8_t fStatus)
{
  fFlashStatus = fStatus;
}

static void FlashSyncStatusSet(uint8_t fStatus)
{
  FlashStatusSet(TRUE);
  fFlashSyncStatus = fStatus;
}

static void SignalOutputSet(uint8_t bGroupIdx,
                            uint8_t bOutputIdx,
                            tEOutputSetType eType,
                            uint8_t fStatus)
{
  if (bGroupIdx < SIGNAL_GROUPS_PER_SSM)
  {
    if (bOutputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP)
    {
      switch (eType)
      {
          case OUTPUT_SET_ACTIVE:
          {
            FlashStatusSet(FALSE);
            SaSignalOutputGroups[bGroupIdx].SaSignalOutputs[bOutputIdx].
            fIsActive = fStatus;
            break;
          }

          case OUTPUT_SET_FLASH:
          {
            SaSignalOutputGroups[bGroupIdx].SaSignalOutputs[bOutputIdx].
            fIsFlashing = fStatus;
            break;
          }
      }
    }
  }
}

static void SignalOutputsUpdate(uint8_t *pbData, tEOutputSetType eSetType)
{
  uint8_t bGroupIdx, bOutputIdx;
  uint8_t bOutputBitIdx = (GPIOGetCardID() % SSM_MODULES_PER_SSM_GROUP)
                          * (SIGNAL_OUTPUTS_PER_SSM);

  for (bGroupIdx = 0; bGroupIdx < SIGNAL_GROUPS_PER_SSM; bGroupIdx++)
  {
    for (bOutputIdx = 0;
         bOutputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP;
         bOutputIdx++)
    {
      uint8_t fStatus = GET_BIT_VALUE(pbData[bOutputBitIdx / NUM_BITS_IN_BYTE],
                                      (bOutputBitIdx % NUM_BITS_IN_BYTE));

      SignalOutputSet(bGroupIdx, bOutputIdx, eSetType, fStatus);

      bOutputBitIdx++;
    }
  }

  SignalOutputFlashConfigSet();
}

static void CANMsgParse(tpSFDCANRxMsg pSRxMsg)
{
  switch (pSRxMsg->SRxHeader.IdType)
  {
      case FDCAN_STANDARD_ID:
      {
        switch (pSRxMsg->SRxHeader.Identifier)
        {
            case FDCAN_CP_SIGNAL_OUTPUTS_1_STD_ID:
            {
              if (GPIOGetCardID() < SSM_MODULES_PER_SSM_GROUP)
              {
                SignalOutputsUpdate(pSRxMsg->baData, OUTPUT_SET_ACTIVE);
              }

              break;
            }

            case FDCAN_CP_SIGNAL_OUTPUTS_2_STD_ID:
            {
              if (GPIOGetCardID() >= SSM_MODULES_PER_SSM_GROUP)
              {
                SignalOutputsUpdate(pSRxMsg->baData, OUTPUT_SET_ACTIVE);
              }

              break;
            }

            case FDCAN_CP_FLASH_SIGNALS_1_STD_ID:
            {
              if (GPIOGetCardID() < SSM_MODULES_PER_SSM_GROUP)
              {
                SignalOutputsUpdate(pSRxMsg->baData, OUTPUT_SET_FLASH);
              }

              break;
            }

            case FDCAN_CP_FLASH_SIGNALS_2_STD_ID:
            {
              if (GPIOGetCardID() >= SSM_MODULES_PER_SSM_GROUP)
              {
                SignalOutputsUpdate(pSRxMsg->baData, OUTPUT_SET_FLASH);
              }

              break;
            }

            case FDCAN_PSM_FLASH_SYNC_1_STD_ID:
            case FDCAN_PSM_FLASH_SYNC_2_STD_ID:
            {
              FlashSyncStatusSet(GET_BIT_VALUE(pSRxMsg->baData[0], 0));

              /* Pet the deadman. If PSM goes silent the measurement
               * task will notice via IsStale() and force outputs off.
               */
              FlashSyncWatchdog_Feed(&g_FlashSyncWatchdog,
                                     Tick_Now_ms(&g_TickPort));
              break;
            }
        } /* switch */

        break;
      }
  } /* switch */
} /* CANMsgParse */

/* Public application code --------------------------------------------------*/
uint8_t CANFlashStatusGet(void)
{
  return fFlashStatus;
}

uint8_t CANFlashSyncStatusGet(void)
{
  return fFlashSyncStatus;
}

GPIO_PinState CANSignalOutputStateGet(uint8_t bGroupIdx, uint8_t bOutputIdx)
{
  return SaSignalOutputGroups[bGroupIdx].SaSignalOutputs[bOutputIdx].fIsActive
         ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

GPIO_PinState CANSignalOutputFlashStateGet(uint8_t bGroupIdx,
                                           uint8_t bOutputIdx)
{
  return SaSignalOutputGroups[bGroupIdx].SaSignalOutputs[bOutputIdx].fIsFlashing
         ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void CANSignalOutputFlashConfigCheck(void)
{
  tSSignalFlashConfig SLoaded;
  uint8_t bGroupIdx, bOutputIdx;

  if (SignalFlashConfig_Load(&g_PersistencePort, &SLoaded))
  {
    for (bGroupIdx = 0; bGroupIdx < SIGNAL_GROUPS_PER_SSM; bGroupIdx++)
    {
      for (bOutputIdx = 0;
           bOutputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP;
           bOutputIdx++)
      {
        uint8_t bIdx = (bGroupIdx * SIGNAL_OUTPUTS_PER_SIGNAL_GROUP)
                       + bOutputIdx;

        SaSignalOutputGroups[bGroupIdx].SaSignalOutputs[bOutputIdx].fIsFlashing
          =
            SLoaded.aIsFlashing[bIdx];
      }
    }

    SLastSavedFlashCfg = SLoaded;

    return;
  }

  /* No valid record — write defaults (all steady). */
  tSSignalFlashConfig SDefault;

  memset(&SDefault, 0, sizeof(SDefault));
  if (SignalFlashConfig_Save(&g_PersistencePort, &SDefault))
  {
    SLastSavedFlashCfg = SDefault;
  }
}

void CANRxRequest(tpSFDCANRxMsg pSRxMsg)
{
  tpSFDCANRxMsg pSReq =
    (tpSFDCANRxMsg) osMemoryPoolAlloc(CANRxReqsMemPoolHandle,
                                      0);

  if (pSRxMsg != NULL)
  {
    memcpy(pSReq, pSRxMsg, sizeof(tSFDCANRxMsg));
    if (osMessageQueuePut(CANRxReqsQueueHandle, &pSReq, 0, 0) != osOK)
    {
      osMemoryPoolFree(CANRxReqsMemPoolHandle, pSReq);
    }
  }
}

/* USER CODE BEGIN Header_CANMsgParserTaskFunc */

/**
 * @brief Function implementing the CANMsgParserTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_CANMsgParserTaskFunc */
void CANMsgParserTaskFunc(void *argument)
{
  /* USER CODE BEGIN CANMsgParserTaskFunc */
  UNUSED(argument);

  tpSFDCANRxMsg pSRxMsg = NULL;

  CANMsgParserInit();

  CANStart(&hfdcan1);

  /* Infinite loop */
  while (pdTRUE)
  {
    if (osMessageQueueGet(CANRxReqsQueueHandle, &pSRxMsg, NULL,
                          osWaitForever) == osOK)
    {
      CANMsgParse(pSRxMsg);
      osMemoryPoolFree(CANRxReqsMemPoolHandle, pSRxMsg);
    }
  }

  /* USER CODE END CANMsgParserTaskFunc */
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
