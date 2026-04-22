/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.c
  * @brief   This file provides code for the configuration
  *          of the FDCAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "fdcan.h"

/* USER CODE BEGIN 0 */
#include "cmsis_os.h"
#include "can_msg_parser.h"
#include <string.h>
/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;
FDCAN_HandleTypeDef hfdcan2;

/* FDCAN1 init function */
void MX_FDCAN1_Init(void)
{
  FDCAN_FilterTypeDef filterConfig;

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = ENABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 4;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 8;
  hfdcan1.Init.NominalTimeSeg2 = 1;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 3;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */
  filterConfig.IdType = FDCAN_STANDARD_ID;
  filterConfig.FilterType = FDCAN_FILTER_RANGE;
  filterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;

  filterConfig.FilterIndex = 0;
  filterConfig.FilterID1 = FDCAN_CP_SIGNAL_OUTPUTS_1_STD_ID;
  filterConfig.FilterID2 = FDCAN_CP_SIGNAL_OUTPUTS_2_STD_ID;
  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &filterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  filterConfig.FilterIndex = 1;
  filterConfig.FilterID1 = FDCAN_PSM_FLASH_SYNC_1_STD_ID;
  filterConfig.FilterID2 = FDCAN_PSM_FLASH_SYNC_2_STD_ID;
  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &filterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  filterConfig.FilterIndex = 2;
  filterConfig.FilterID1 = FDCAN_CP_FLASH_SIGNALS_1_STD_ID;
  filterConfig.FilterID2 = FDCAN_CP_FLASH_SIGNALS_2_STD_ID;
  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &filterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT,
                                   FDCAN_REJECT_REMOTE,
                                   FDCAN_REJECT_REMOTE) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE END FDCAN1_Init 2 */

}
/* FDCAN2 init function */
void MX_FDCAN2_Init(void)
{

  /* USER CODE BEGIN FDCAN2_Init 0 */
	FDCAN_FilterTypeDef filterConfig;
  /* USER CODE END FDCAN2_Init 0 */

  /* USER CODE BEGIN FDCAN2_Init 1 */

  /* USER CODE END FDCAN2_Init 1 */
  hfdcan2.Instance = FDCAN2;
  hfdcan2.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan2.Init.AutoRetransmission = ENABLE;
  hfdcan2.Init.TransmitPause = DISABLE;
  hfdcan2.Init.ProtocolException = DISABLE;
  hfdcan2.Init.NominalPrescaler = 4;
  hfdcan2.Init.NominalSyncJumpWidth = 1;
  hfdcan2.Init.NominalTimeSeg1 = 8;
  hfdcan2.Init.NominalTimeSeg2 = 1;
  hfdcan2.Init.DataPrescaler = 1;
  hfdcan2.Init.DataSyncJumpWidth = 1;
  hfdcan2.Init.DataTimeSeg1 = 1;
  hfdcan2.Init.DataTimeSeg2 = 1;
  hfdcan2.Init.StdFiltersNbr = 1;
  hfdcan2.Init.ExtFiltersNbr = 0;
  hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN2_Init 2 */
	filterConfig.IdType = FDCAN_STANDARD_ID;
  filterConfig.FilterIndex = 0;
  filterConfig.FilterType = FDCAN_FILTER_RANGE;
  filterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  filterConfig.FilterID1 = 0;
  filterConfig.FilterID2 = 0x7FF;
  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filterConfig) != HAL_OK)
  {
    Error_Handler();
  }
	
  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_REJECT, FDCAN_REJECT,
                                   FDCAN_FILTER_REMOTE,
                                   FDCAN_FILTER_REMOTE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END FDCAN2_Init 2 */

}

static uint32_t HAL_RCC_FDCAN_CLK_ENABLED=0;

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */

  /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN1 clock enable */
    HAL_RCC_FDCAN_CLK_ENABLED++;
    if(HAL_RCC_FDCAN_CLK_ENABLED==1){
      __HAL_RCC_FDCAN_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
  /* USER CODE BEGIN FDCAN1_MspInit 1 */

  /* USER CODE END FDCAN1_MspInit 1 */
  }
  else if(fdcanHandle->Instance==FDCAN2)
  {
  /* USER CODE BEGIN FDCAN2_MspInit 0 */

  /* USER CODE END FDCAN2_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN2 clock enable */
    HAL_RCC_FDCAN_CLK_ENABLED++;
    if(HAL_RCC_FDCAN_CLK_ENABLED==1){
      __HAL_RCC_FDCAN_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**FDCAN2 GPIO Configuration
    PB5     ------> FDCAN2_RX
    PB6     ------> FDCAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* FDCAN2 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN2_IT0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
    HAL_NVIC_SetPriority(FDCAN2_IT1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(FDCAN2_IT1_IRQn);
  /* USER CODE BEGIN FDCAN2_MspInit 1 */

  /* USER CODE END FDCAN2_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

  /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_FDCAN_CLK_ENABLED--;
    if(HAL_RCC_FDCAN_CLK_ENABLED==0){
      __HAL_RCC_FDCAN_CLK_DISABLE();
    }

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* FDCAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
    HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);
  /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

  /* USER CODE END FDCAN1_MspDeInit 1 */
  }
  else if(fdcanHandle->Instance==FDCAN2)
  {
  /* USER CODE BEGIN FDCAN2_MspDeInit 0 */

  /* USER CODE END FDCAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_FDCAN_CLK_ENABLED--;
    if(HAL_RCC_FDCAN_CLK_ENABLED==0){
      __HAL_RCC_FDCAN_CLK_DISABLE();
    }

    /**FDCAN2 GPIO Configuration
    PB5     ------> FDCAN2_RX
    PB6     ------> FDCAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5|GPIO_PIN_6);

    /* FDCAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(FDCAN2_IT0_IRQn);
    HAL_NVIC_DisableIRQ(FDCAN2_IT1_IRQn);
  /* USER CODE BEGIN FDCAN2_MspDeInit 1 */

  /* USER CODE END FDCAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
uint8_t CANGetRxDataLength(uint32_t lenCode)
{
	if (lenCode <= FDCAN_DLC_BYTES_8)
	{
		return (FDCAN_DLC_BYTES_0 | lenCode);
	}

	if (lenCode <= FDCAN_DLC_BYTES_12)
	{
		return 12;
	}

	if (lenCode <= FDCAN_DLC_BYTES_16)
	{
		return 16;
	}

	if (lenCode <= FDCAN_DLC_BYTES_20)
	{
		return 20;
	}

	if (lenCode <= FDCAN_DLC_BYTES_24)
	{
		return 24;
	}

	if (lenCode <= FDCAN_DLC_BYTES_32)
	{
		return 32;
	}

	if (lenCode <= FDCAN_DLC_BYTES_48)
	{
		return 48;
	}

	if (lenCode <= FDCAN_DLC_BYTES_64)
	{
		return 64;
	}

	return FDCAN_DLC_BYTES_0;
}

uint32_t CANGetTxDataLengthCode(uint8_t len)
{
	if (len <= 8)
	{
		return (FDCAN_DLC_BYTES_0 | len);
	}

	if (len <= 12)
	{
		return FDCAN_DLC_BYTES_12;
	}

	if (len <= 16)
	{
		return FDCAN_DLC_BYTES_16;
	}

	if (len <= 20)
	{
		return FDCAN_DLC_BYTES_20;
	}

	if (len <= 24)
	{
		return FDCAN_DLC_BYTES_24;
	}

	if (len <= 32)
	{
		return FDCAN_DLC_BYTES_32;
	}

	if (len <= 48)
	{
		return FDCAN_DLC_BYTES_48;
	}

	if (len <= 64)
	{
		return FDCAN_DLC_BYTES_64;
	}

	return FDCAN_DLC_BYTES_0;
}

void CANStart(FDCAN_HandleTypeDef* hfdcan)
{
  if (HAL_FDCAN_Start(hfdcan) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                     0) != HAL_OK)
  {
    Error_Handler();
  }
}

void CANStop(FDCAN_HandleTypeDef* hfdcan)
{
  HAL_FDCAN_Stop(hfdcan);
}

void CANDeInit(FDCAN_HandleTypeDef* hfdcan)
{
  CANStop(hfdcan);

  HAL_FDCAN_DeInit(hfdcan);
}

uint8_t CANSendMessage(FdcanTxMsg_t * msg)
{
  if (msg == NULL)
  {
    return 0U;
  }

  if (HAL_FDCAN_AddMessageToTxFifoQ(msg->hfdcan,
                                    &msg->txHeader,
                                    msg->data) != HAL_OK)
  {
    return 0U;
  }

  return 1U;
}

uint8_t CANWaitTxComplete(FDCAN_HandleTypeDef* hfdcan,
                          uint32_t txBufferIndex,
                          uint32_t timeoutMs)
{
  uint32_t start = HAL_GetTick();

  if ((hfdcan == NULL) || (txBufferIndex == 0U))
  {
    return 0U;
  }

  while (HAL_FDCAN_IsTxBufferMessagePending(hfdcan, txBufferIndex) != 0U)
  {
    if ((HAL_GetTick() - start) >= timeoutMs)
    {
      (void) HAL_FDCAN_AbortTxRequest(hfdcan, txBufferIndex);
      return 0U;
    }

    osDelay(1);
  }

  return 1U;
}

static uint8_t CANRxFrameShouldQueue(const FdcanRxMsg_t *rxMsg)
{
  if ((rxMsg->rxHeader.IdType != FDCAN_STANDARD_ID)
      || (rxMsg->rxHeader.RxFrameType != FDCAN_DATA_FRAME)
      || (rxMsg->rxHeader.DataLength != FDCAN_MAX_DATA_LEN))
  {
    return 0U;
  }

  switch (rxMsg->rxHeader.Identifier)
  {
      case FDCAN_CP_SIGNAL_OUTPUTS_1_STD_ID:
      case FDCAN_CP_SIGNAL_OUTPUTS_2_STD_ID:
      case FDCAN_PSM_FLASH_SYNC_1_STD_ID:
      case FDCAN_PSM_FLASH_SYNC_2_STD_ID:
      case FDCAN_CP_FLASH_SIGNALS_1_STD_ID:
      case FDCAN_CP_FLASH_SIGNALS_2_STD_ID:
      {
        return 1U;
      }

      default:
      {
        return 0U;
      }
  }
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs)
{
  (void) RxFifo0ITs;

  if (hfdcan->Instance == FDCAN1)
  {
		FdcanRxMsg_t rxMsg;
    memset(&rxMsg, 0, sizeof(rxMsg));

    rxMsg.hfdcan = hfdcan;

    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxMsg.rxHeader, rxMsg.data) != HAL_OK)
    {
      CANRxFaultRecord();
      return;
    }

    rxMsg.rxHeader.DataLength = CANGetRxDataLength(rxMsg.rxHeader.DataLength);

    if (CANRxFrameShouldQueue(&rxMsg) != 0U)
    {
		  CANRxRequest(&rxMsg);
    }
    else
    {
      CANRxFaultRecord();
    }
  }
}
/* USER CODE END 1 */

