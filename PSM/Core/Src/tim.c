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
#include "Measurement.h"
#include "HardwarePorts.h"

#define TIM2_CLOCK_FREQ                     100000U

#define TIM2_CH2_EVALUATION_INTERVAL_MS      1U
#define TIM2_CH2_EVALUATION_INTERVAL_COUNTS  (TIM2_CH2_EVALUATION_INTERVAL_MS \
                                              * (TIM2_CLOCK_FREQ / 1000U))

#define TIM2_CH3_EVALUATION_INTERVAL_MS      10U
#define TIM2_CH3_EVALUATION_INTERVAL_COUNTS  (TIM2_CH3_EVALUATION_INTERVAL_MS \
                                              * (TIM2_CLOCK_FREQ / 1000U))

/* USER CODE END 0 */

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/* TIM2 init function */
void MX_TIM2_Init(void)
{
  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = { 0 };
  TIM_IC_InitTypeDef sConfigIC = { 0 };
  TIM_OC_InitTypeDef sConfigOC = { 0 };

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1600 - 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
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
  sConfigOC.Pulse = 100;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.Pulse = 1000;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
} /* MX_TIM2_Init */

/* TIM3 init function */
void MX_TIM3_Init(void)
{
  /* USER CODE BEGIN TIM3_Init 0 */
  /* 160MHz / 25000 = 6400 Hz (6.4kHz) */
  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
  TIM_MasterConfigTypeDef sMasterConfig = { 0 };

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

void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *tim_icHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = { 0 };

  if (tim_icHandle->Instance == TIM2)
  {
    /* USER CODE BEGIN TIM2_MspInit 0 */

    /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /**TIM2 GPIO Configuration
     *  PA0     ------> TIM2_CH1
     */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    /* USER CODE BEGIN TIM2_MspInit 1 */

    /* USER CODE END TIM2_MspInit 1 */
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *tim_baseHandle)
{
  if (tim_baseHandle->Instance == TIM3)
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

void HAL_TIM_IC_MspDeInit(TIM_HandleTypeDef *tim_icHandle)
{
  if (tim_icHandle->Instance == TIM2)
  {
    /* USER CODE BEGIN TIM2_MspDeInit 0 */

    /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /**TIM2 GPIO Configuration
     *  PA0     ------> TIM2_CH1
     */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);

    /* TIM2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);
    /* USER CODE BEGIN TIM2_MspDeInit 1 */

    /* USER CODE END TIM2_MspDeInit 1 */
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *tim_baseHandle)
{
  if (tim_baseHandle->Instance == TIM3)
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
  FrequencyCapture_Restart(&g_frequencyCapturePort, HAL_GetTick());

  if (HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
}

void Tim2OCStartIT(void)
{
  if (HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
}

void Tim2ICStopIT(void)
{
  HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
}

void Tim2OCStopIT(void)
{
  HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);
  HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_3);
}

void Tim2DeInit(void)
{
  HAL_TIM_OC_DeInit(&htim2);
  HAL_TIM_IC_DeInit(&htim2);
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

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if ((htim->Instance == TIM2) && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1))
  {
    uint32_t capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

    FrequencyCapture_OnEdge(&g_frequencyCapturePort, capture, HAL_GetTick());

    /* Re-phase TIM3 to the grid zero-crossing we just captured.  TIM3
     * drives the ADC1 external trigger (T3_TRGO) so its 128-sample DMA
     * half-buffer (20 ms at 6400 Hz) covers exactly one 50 Hz cycle at
     * nominal.  When grid frequency drifts, the window stays 20 ms but
     * now captures a partial cycle; anchoring TIM3 to each IC makes
     * that partial-cycle content identical from window to window, which
     * turns drift-induced bias into a constant calibration error rather
     * than a sample-to-sample wander.  Setting the counter to 0 means
     * the first ADC sample happens one TIM3 period (~156 µs) after the
     * zero-crossing — same phase offset every cycle. */
    __HAL_TIM_SET_COUNTER(&htim3, 0U);
  }
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  if ((htim->Instance == TIM2) && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2))
  {
    MeasurementCommCheck();

    /* uint32 addition already wraps modulo 2^32, which matches the timer's
     * own 32-bit counter rollover — no explicit guard needed. */
    uint32_t next = __HAL_TIM_GET_COUNTER(htim)
                     + TIM2_CH2_EVALUATION_INTERVAL_COUNTS;

    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, next);
  }
  else if ((htim->Instance == TIM2)
           && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) )
  {
    FrequencyCaptureVerdict_e verdict =
      FrequencyCapture_Evaluate(&g_frequencyCapturePort, HAL_GetTick());

    if (verdict == FREQ_CAPTURE_VERDICT_ENTER_FLASH)
    {
      MeasurementFlashStateSet(1U);
    }
    else
    {
      uint32_t next = __HAL_TIM_GET_COUNTER(htim)
                       + TIM2_CH3_EVALUATION_INTERVAL_COUNTS;

      __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3, next);
    }
  }
}

/* USER CODE END 1 */
