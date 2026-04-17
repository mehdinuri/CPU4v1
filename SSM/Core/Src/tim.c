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
#include "utilities.h"
#include "measurement.h"

#define TIM2_CLOCK_FREQ 100000
#define TIM2_MAX_COUNT 0xFFFFFFFF

#define TIM2_CH1_CAPTURE_TIMEOUT_MS 100
#define TIM2_CH1_EVALUATION_INTERVAL_MS 10
#define TIM2_CH1_EVALUATION_INTERVAL_COUNTS (TIM2_CH1_EVALUATION_INTERVAL_MS * (TIM2_CLOCK_FREQ / 1000))

#define TIM4_CLOCK_FREQ 100000
#define TIM4_MAX_COUNT 0xFFFF

#define TIM4_CH1_CAPTURE_TIMEOUT_MS 100
#define TIM4_CH1_EVALUATION_INTERVAL_MS 1
#define TIM4_CH1_EVALUATION_INTERVAL_COUNTS (TIM4_CH1_EVALUATION_INTERVAL_MS * (TIM4_CLOCK_FREQ / 1000))

#define TARGET_FREQ_HZ 50
#define FREQ_TOLERANCE_HZ 5
#define BAD_READINGS_BEFORE_SLEEP 10

volatile uint64_t lICValue1 = 0;
volatile uint64_t lICValue2 = 0;
volatile uint64_t lICCaptureDiff = 0;
volatile uint8_t fFirstValueCaptured = FALSE;
volatile uint8_t bMeasuredFreqInHz = 0;

volatile uint32_t lLastCaptureTime = 0;
volatile uint16_t sBadReadingsCntr = 0;

volatile uint64_t lTim2Ch1CurrentCounterValue = 0;
volatile uint64_t lTim2Ch1NextCompareValue = 0;

volatile uint32_t lTim4Ch1CurrentCounterValue = 0;
volatile uint32_t lTim4Ch1NextCompareValue = 0;
/* USER CODE END 0 */

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* TIM2 init function */
void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1600-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 1000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0x0F;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}
/* TIM3 init function */
void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 24999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}
/* TIM4 init function */
void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 1600-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_OC_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 100;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

void HAL_TIM_OC_MspInit(TIM_HandleTypeDef* tim_ocHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(tim_ocHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PB10     ------> TIM2_CH3
    */
    GPIO_InitStruct.Pin = GRID_FREQ_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GRID_FREQ_GPIO_Port, &GPIO_InitStruct);

    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
  else if(tim_ocHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspInit 0 */

  /* USER CODE END TIM4_MspInit 0 */
    /* TIM4 clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* TIM4 interrupt Init */
    HAL_NVIC_SetPriority(TIM4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
  /* USER CODE BEGIN TIM4_MspInit 1 */

  /* USER CODE END TIM4_MspInit 1 */
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspInit 0 */

  /* USER CODE END TIM3_MspInit 0 */
    /* TIM3 clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* TIM3 interrupt Init */
    HAL_NVIC_SetPriority(TIM3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
  /* USER CODE BEGIN TIM3_MspInit 1 */

  /* USER CODE END TIM3_MspInit 1 */
  }
}

void HAL_TIM_OC_MspDeInit(TIM_HandleTypeDef* tim_ocHandle)
{

  if(tim_ocHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /**TIM2 GPIO Configuration
    PB10     ------> TIM2_CH3
    */
    HAL_GPIO_DeInit(GRID_FREQ_GPIO_Port, GRID_FREQ_Pin);

    /* TIM2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
  else if(tim_ocHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspDeInit 0 */

  /* USER CODE END TIM4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM4_CLK_DISABLE();

    /* TIM4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
  /* USER CODE BEGIN TIM4_MspDeInit 1 */

  /* USER CODE END TIM4_MspDeInit 1 */
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspDeInit 0 */

  /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();

    /* TIM3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
  /* USER CODE BEGIN TIM3_MspDeInit 1 */

  /* USER CODE END TIM3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void Tim2ICStartIT(void)
{
	fFirstValueCaptured = 0;
	lLastCaptureTime = HAL_GetTick();
	
	if (HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3) != HAL_OK)
	{
		Error_Handler();
	}
}

void Tim2ICStopIT(void)
{
	HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_3);
}

void Tim2OCStartIT(void)
{
	if (HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}
}

void Tim2OCStopIT(void)
{
	HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);
}

void Tim2DeInit(void)
{
	HAL_TIM_IC_DeInit(&htim2);
	HAL_TIM_OC_DeInit(&htim2);
}

void Tim3Start(void)
{
	if (HAL_TIM_Base_Start(&htim3) != HAL_OK)
	{
		Error_Handler();
	}
}

void Tim3Stop(void)
{
	HAL_TIM_Base_Stop(&htim3);
}

void Tim3DeInit(void)
{
	HAL_TIM_Base_DeInit(&htim3);
}

void Tim4OCStartIT(void)
{
	if (HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}
}

void Tim4OCStopIT(void)
{
	HAL_TIM_OC_Stop_IT(&htim4, TIM_CHANNEL_1);
}

void Tim4DeInit(void)
{
	HAL_TIM_OC_DeInit(&htim4);
}

void TimEnterStandby(void)
{
	__HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
	HAL_PWR_EnterSTANDBYMode();
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
	{
		lLastCaptureTime = HAL_GetTick();

		if (fFirstValueCaptured == FALSE) 
		{
			lICValue1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
			fFirstValueCaptured = TRUE;
		} 
		else 
		{
			lICValue2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);

			if (lICValue2 > lICValue1)
			{
				lICCaptureDiff = lICValue2 - lICValue1;
			} 
			else if (lICValue2 < lICValue1)
			{
				lICCaptureDiff = ((TIM2_MAX_COUNT - lICValue1) + lICValue2) + 1;
			}
			else
			{
				lICCaptureDiff = 0;
				fFirstValueCaptured = FALSE;
			}

			if (lICCaptureDiff != 0)
			{
				bMeasuredFreqInHz = TIM2_CLOCK_FREQ / lICCaptureDiff;
			}
			else
			{
				bMeasuredFreqInHz = 0;
			}
			
			lICValue1 = lICValue2;
		}
		
		// Re-phase TIM3 to the 50Hz reference
    __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
    __HAL_TIM_SET_COUNTER(&htim3, 0);
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) 
	{
		uint32_t lCurTime = HAL_GetTick();
		uint8_t bFreq = bMeasuredFreqInHz;
		uint8_t fSignalTimedout = FALSE;

		 if (!fFirstValueCaptured || ((lCurTime - lLastCaptureTime) > TIM2_CH1_CAPTURE_TIMEOUT_MS))
		 {
			 fSignalTimedout = TRUE;
			 if (bMeasuredFreqInHz != 0 || fFirstValueCaptured) 
			 {
				 bMeasuredFreqInHz = 0;
				 fFirstValueCaptured = 0;
			 }

			 bFreq = 0;
		}

		if (!fSignalTimedout && (bFreq >= (TARGET_FREQ_HZ - FREQ_TOLERANCE_HZ)) && (bFreq <= (TARGET_FREQ_HZ + FREQ_TOLERANCE_HZ)))
		{
			sBadReadingsCntr = 0;
		}
		else
		{
			sBadReadingsCntr++;
		}

		if (sBadReadingsCntr >= BAD_READINGS_BEFORE_SLEEP)
		{
			Tim2OCStopIT();
			Tim2ICStopIT();
			
			TimEnterStandby();
		}
		else
		{
			lTim2Ch1CurrentCounterValue = __HAL_TIM_GET_COUNTER(htim);
			lTim2Ch1NextCompareValue = (lTim2Ch1CurrentCounterValue + TIM2_CH1_EVALUATION_INTERVAL_COUNTS);
			
			if (lTim2Ch1NextCompareValue > TIM2_MAX_COUNT)
			{
				lTim2Ch1NextCompareValue -= (TIM2_MAX_COUNT + 1);
			}
			
			__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, lTim2Ch1NextCompareValue);
		}
	}
	else if (htim->Instance == TIM4 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) 
	{
		MeasurementSGStatesCntrsSet();
		
		lTim4Ch1CurrentCounterValue = __HAL_TIM_GET_COUNTER(htim);
		lTim4Ch1NextCompareValue = (lTim4Ch1CurrentCounterValue + TIM4_CH1_EVALUATION_INTERVAL_COUNTS);
			
		if (lTim4Ch1NextCompareValue > TIM4_MAX_COUNT) 
		{
			lTim4Ch1NextCompareValue -= (TIM4_MAX_COUNT + 1);
		}
		
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, lTim4Ch1NextCompareValue);
	}
}
/* USER CODE END 1 */

