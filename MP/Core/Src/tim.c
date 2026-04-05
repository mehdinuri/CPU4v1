/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
#include "tim.h"

/* USER CODE BEGIN 0 */
#include "data.h"
#include "gpio.h"

#define TARGET_FREQ_HZ 100
#define FREQ_TOLERANCE_HZ 5
#define TIMER_CLOCK_FREQ 100000
#define MAX_TIMER_COUNT 0xFFFF
#define CAPTURE_TIMEOUT_MS 100

#define EVALUATION_INTERVAL_MS 10
#define EVALUATION_INTERVAL_COUNTS (EVALUATION_INTERVAL_MS * (TIMER_CLOCK_FREQ / 1000))
#define BAD_READINGS_BEFORE_SLEEP 100

volatile uint32_t lICValue1 = 0;
volatile uint32_t lICValue2 = 0;
volatile uint32_t lICCaptureDiff = 0;
volatile uint8_t fFirstValueCaptured = FALSE;
volatile uint32_t lMeasuredFreqInHz = 0;
volatile uint32_t lLastCaptureTime = 0;
volatile uint16_t sBadReadingsCntr = 0;
volatile uint32_t lNextCompareValue = 0;

/* USER CODE END 0 */

TIM_HandleTypeDef htim2;

/* TIM2 init function */
void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1600-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0x0F;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 1000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
	Tim2StartICIT();
	Tim2StartOCIT();
  /* USER CODE END TIM2_Init 2 */

}

void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* tim_icHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(tim_icHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PA0     ------> TIM2_CH1
    */
    GPIO_InitStruct.Pin = WKUP_100Hz_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(WKUP_100Hz_GPIO_Port, &GPIO_InitStruct);

    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
}

void HAL_TIM_IC_MspDeInit(TIM_HandleTypeDef* tim_icHandle)
{

  if(tim_icHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /**TIM2 GPIO Configuration
    PA0     ------> TIM2_CH1
    */
    HAL_GPIO_DeInit(WKUP_100Hz_GPIO_Port, WKUP_100Hz_Pin);

    /* TIM2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void Tim2StartICIT(void)
{
	fFirstValueCaptured = 0;
	lLastCaptureTime = HAL_GetTick();
	
	if (HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1) != HAL_OK) 
	{
		Error_Handler();
	}
}

void Tim2StopICIT(void)
{
	HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
}

void Tim2StartOCIT(void)
{
	if (HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_2) != HAL_OK) 
	{
		Error_Handler();
	}
}

void Tim2StopOCIT(void)
{
	HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);
}

void Tim2DeInit(void)
{
	Tim2StopOCIT();
	Tim2StopICIT();
	
	HAL_TIM_OC_DeInit(&htim2);
	HAL_TIM_IC_DeInit(&htim2);
}

void Tim1StartIT(void)
{
	if (HAL_TIM_Base_Start_IT(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
}

void Tim1StopIT(void)
{
	HAL_TIM_Base_Stop_IT(&htim1);
}

void Tim1DeInit(void)
{
	Tim1StopIT();
	
	HAL_TIM_Base_DeInit(&htim1);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) 
	{
		lLastCaptureTime = HAL_GetTick();

		if (fFirstValueCaptured == FALSE) 
		{
			lICValue1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			fFirstValueCaptured = TRUE;
		} 
		else 
		{
			lICValue2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

			if (lICValue2 > lICValue1)
			{
				lICCaptureDiff = lICValue2 - lICValue1;
			} 
			else if (lICValue2 < lICValue1)
			{
				lICCaptureDiff = ((MAX_TIMER_COUNT - lICValue1) + lICValue2) + 1;
			}
			else
			{
				lICCaptureDiff = 0;
				fFirstValueCaptured = FALSE;
			}

			if (lICCaptureDiff != 0)
			{
				lMeasuredFreqInHz = TIMER_CLOCK_FREQ / lICCaptureDiff;
			}
			else
			{
				lMeasuredFreqInHz = 0;
			}

			lICValue1 = lICValue2;
		}
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) 
	{
		uint32_t lCurTime = HAL_GetTick();
		uint32_t lFreq = lMeasuredFreqInHz;
		uint8_t fSignalTimedout = FALSE;

		 if (!fFirstValueCaptured || ((lCurTime - lLastCaptureTime) > CAPTURE_TIMEOUT_MS))
		 {
			 fSignalTimedout = TRUE;
			 if (lMeasuredFreqInHz != 0 || fFirstValueCaptured) 
			 {
				 lMeasuredFreqInHz = 0;
				 fFirstValueCaptured = 0;
			 }

			 lFreq = 0;
		}

		if (!fSignalTimedout && (lFreq >= (TARGET_FREQ_HZ - FREQ_TOLERANCE_HZ)) && (lFreq <= (TARGET_FREQ_HZ + FREQ_TOLERANCE_HZ)))
		{
			sBadReadingsCntr = 0;
		}
		else
		{
			sBadReadingsCntr++;
		}

		if (sBadReadingsCntr >= BAD_READINGS_BEFORE_SLEEP)
		{						
			Tim2StopOCIT();
			Tim2StopICIT();
			
			EnterStandbyModeWithPreparation(TRUE);
		}
		else
		{
			uint32_t lCurrentCounterValue = __HAL_TIM_GET_COUNTER(htim);
			lNextCompareValue = (lCurrentCounterValue + EVALUATION_INTERVAL_COUNTS);
			
			if (lNextCompareValue > MAX_TIMER_COUNT) 
			{
				lNextCompareValue -= (MAX_TIMER_COUNT + 1);
			}
			
			__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, lNextCompareValue);
		}
	}
}
/* USER CODE END 1 */

