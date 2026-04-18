/* USER CODE BEGIN Header */

/**
 ******************************************************************************
 * @file    adc.c
 * @brief   This file provides code for the configuration
 *          of the ADC instances.
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
#include "adc.h"

/* USER CODE BEGIN 0 */
#include <math.h>
#include <string.h>
#include "measurement.h"
#include "Platform/STM32/Bootstrap/HardwarePorts.h"
#include "Adapters/STM32/AdcCurrentAdapter.h"

/* ADC half-buffer index last signalled as ready by the DMA ISR.
 * 0xFF = no data yet (boot or first cycle not yet complete).
 *
 * Writer: HAL_ADC_Conv{Half,}CpltCallback (ISR context)
 * Reader: ADCSGCurrentProcessLatestHalf (task context, MeasurementTask)
 *
 * A single-byte write is atomic on Cortex-M4; no additional lock is needed
 * since the reader snapshots into a local before use and doesn't care which
 * specific half it sees — only that it's the latest one the ISR produced.
 */
static volatile uint8_t s_LatestReadyHalf = 0xFFU;

#define MP_MAX_CURRENT_MS 1024.0f

#define SIGNAL_GROUP1_INDEX 0
#define SIGNAL_GROUP2_INDEX 1
#define SIGNAL_GROUP3_INDEX 2
#define SIGNAL_GROUP4_INDEX 3

#define SIGNAL_GROUPS_PER_SSM_MAX 4

#define ADC1_MAX_CHANNELS 4
#define ADC2_MAX_CHANNELS 4

#define ADC_MAX_CHANNELS (ADC1_MAX_CHANNELS + ADC2_MAX_CHANNELS)

#define SAMPLES_PER_MAINS_CYCLE 128
#define SAMPLES_PER_CHANNEL_BUF_LEN (SAMPLES_PER_MAINS_CYCLE * 2)

#define V_REF 3.3264f
#define ADC_MAX 4095.0f /* ADC Resolution (12-bit) */

#define SCALING_FACTOR_H 39.8255f /* ~25 * 2.0f */
#define SCALING_FACTOR_P 0.3983f /* 42.01f / 100.0f */

#define SIGNAL_GROUP_CURRENT_DEADZONE_OFFSET_MA 5.5f
#define SIGNAL_GROUP_NO_LOAD_THRESHOLD_MA 4.0f  /* Safely below 1W (~4mA) but above ambient noise */

uint16_t saADC1ConvVals[SAMPLES_PER_CHANNEL_BUF_LEN * ADC1_MAX_CHANNELS];
uint16_t saADC2ConvVals[SAMPLES_PER_CHANNEL_BUF_LEN * ADC2_MAX_CHANNELS];

float faSGP[SIGNAL_GROUPS_PER_SSM_MAX];
float faSGH[SIGNAL_GROUPS_PER_SSM_MAX];
float faSGCurrentRMS[SIGNAL_GROUPS_PER_SSM_MAX];
/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;

/* ADC1 init function */
void MX_ADC1_Init(void)
{
  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = { 0 };
  ADC_ChannelConfTypeDef sConfig = { 0 };

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 4;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T3_TRGO;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.OversamplingMode = ENABLE;
  hadc1.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_16;
  hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
  hadc1.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  hadc1.Init.Oversampling.OversamplingStopReset =
    ADC_REGOVERSAMPLING_CONTINUED_MODE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
   */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN ADC1_Init 2 */
  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE END ADC1_Init 2 */
} /* MX_ADC1_Init */

/* ADC2 init function */
void MX_ADC2_Init(void)
{
  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = { 0 };

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
   */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.GainCompensation = 0;
  hadc2.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 4;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T3_TRGO;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc2.Init.DMAContinuousRequests = ENABLE;
  hadc2.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc2.Init.OversamplingMode = ENABLE;
  hadc2.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_16;
  hadc2.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
  hadc2.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  hadc2.Init.Oversampling.OversamplingStopReset =
    ADC_REGOVERSAMPLING_CONTINUED_MODE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_17;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN ADC2_Init 2 */
  if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE END ADC2_Init 2 */
} /* MX_ADC2_Init */

static uint32_t HAL_RCC_ADC12_CLK_ENABLED = 0;

void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = { 0 };
  RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

  if (adcHandle->Instance == ADC1)
  {
    /* USER CODE BEGIN ADC1_MspInit 0 */

    /* USER CODE END ADC1_MspInit 0 */

    /** Initializes the peripherals clocks
     */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* ADC1 clock enable */
    HAL_RCC_ADC12_CLK_ENABLED++;
    if (HAL_RCC_ADC12_CLK_ENABLED == 1)
    {
      __HAL_RCC_ADC12_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /**ADC1 GPIO Configuration
     *  PA2     ------> ADC1_IN3
     *  PA3     ------> ADC1_IN4
     *  PB0     ------> ADC1_IN15
     *  PB1     ------> ADC1_IN12
     */
    GPIO_InitStruct.Pin = GRP3_P_Pin | GRP4_P_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GRP2_H_Pin | GRP1_H_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc1);

    /* USER CODE BEGIN ADC1_MspInit 1 */

    /* USER CODE END ADC1_MspInit 1 */
  }
  else if (adcHandle->Instance == ADC2)
  {
    /* USER CODE BEGIN ADC2_MspInit 0 */

    /* USER CODE END ADC2_MspInit 0 */

    /** Initializes the peripherals clocks
     */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* ADC2 clock enable */
    HAL_RCC_ADC12_CLK_ENABLED++;
    if (HAL_RCC_ADC12_CLK_ENABLED == 1)
    {
      __HAL_RCC_ADC12_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /**ADC2 GPIO Configuration
     *  PA4     ------> ADC2_IN17
     *  PA5     ------> ADC2_IN13
     *  PC4     ------> ADC2_IN5
     *  PC5     ------> ADC2_IN11
     */
    GPIO_InitStruct.Pin = GRP1_P_Pin | GRP2_P_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GRP3_H_Pin | GRP4_H_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* ADC2 DMA Init */
    /* ADC2 Init */
    hdma_adc2.Instance = DMA1_Channel2;
    hdma_adc2.Init.Request = DMA_REQUEST_ADC2;
    hdma_adc2.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc2.Init.Mode = DMA_CIRCULAR;
    hdma_adc2.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_adc2) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc2);

    /* USER CODE BEGIN ADC2_MspInit 1 */

    /* USER CODE END ADC2_MspInit 1 */
  }
} /* HAL_ADC_MspInit */

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *adcHandle)
{
  if (adcHandle->Instance == ADC1)
  {
    /* USER CODE BEGIN ADC1_MspDeInit 0 */

    /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_ADC12_CLK_ENABLED--;
    if (HAL_RCC_ADC12_CLK_ENABLED == 0)
    {
      __HAL_RCC_ADC12_CLK_DISABLE();
    }

    /**ADC1 GPIO Configuration
     *  PA2     ------> ADC1_IN3
     *  PA3     ------> ADC1_IN4
     *  PB0     ------> ADC1_IN15
     *  PB1     ------> ADC1_IN12
     */
    HAL_GPIO_DeInit(GPIOA, GRP3_P_Pin | GRP4_P_Pin);

    HAL_GPIO_DeInit(GPIOB, GRP2_H_Pin | GRP1_H_Pin);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
    /* USER CODE BEGIN ADC1_MspDeInit 1 */

    /* USER CODE END ADC1_MspDeInit 1 */
  }
  else if (adcHandle->Instance == ADC2)
  {
    /* USER CODE BEGIN ADC2_MspDeInit 0 */

    /* USER CODE END ADC2_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_ADC12_CLK_ENABLED--;
    if (HAL_RCC_ADC12_CLK_ENABLED == 0)
    {
      __HAL_RCC_ADC12_CLK_DISABLE();
    }

    /**ADC2 GPIO Configuration
     *  PA4     ------> ADC2_IN17
     *  PA5     ------> ADC2_IN13
     *  PC4     ------> ADC2_IN5
     *  PC5     ------> ADC2_IN11
     */
    HAL_GPIO_DeInit(GPIOA, GRP1_P_Pin | GRP2_P_Pin);

    HAL_GPIO_DeInit(GPIOC, GRP3_H_Pin | GRP4_H_Pin);

    /* ADC2 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
    /* USER CODE BEGIN ADC2_MspDeInit 1 */

    /* USER CODE END ADC2_MspDeInit 1 */
  }
} /* HAL_ADC_MspDeInit */

/* USER CODE BEGIN 1 */
static void ADC1StartDMA(void)
{
  if (HAL_ADC_Start_DMA(&hadc1,
                        (uint32_t *) saADC1ConvVals,
                        SAMPLES_PER_CHANNEL_BUF_LEN * ADC1_MAX_CHANNELS)
      != HAL_OK)
  {
    Error_Handler();
  }
}

static void ADC2StartDMA(void)
{
  if (HAL_ADC_Start_DMA(&hadc2,
                        (uint32_t *) saADC2ConvVals,
                        SAMPLES_PER_CHANNEL_BUF_LEN * ADC2_MAX_CHANNELS)
      != HAL_OK)
  {
    Error_Handler();
  }
}

static float SGCurrentRMSCalculate(uint16_t *saConvVals,
                                   uint8_t bOffset,
                                   uint8_t bStride,
                                   uint16_t sLength)
{
  uint64_t lSumSquares = 0;
  uint16_t sIdx = 0;

  /* Accumulate Squares */
  for (sIdx = 0; sIdx < sLength; sIdx++)
  {
    float fVal = (float) saConvVals[bOffset + (sIdx * bStride)];

    lSumSquares += (uint64_t) (fVal * fVal);
  }

  return sqrtf((float) lSumSquares / sLength);
}

static float SGOptimalCurrentGet(float fRMSRawP,
                                 float fRMSRawH,
                                 float fScalingFactorP,
                                 float fScalingFactorH)
{
  if (fRMSRawP > ((ADC_MAX / 2.0f) * 0.50f))
  {
    return (fRMSRawH * (V_REF / ADC_MAX)) * fScalingFactorH; /* Use High Channel */
  }
  else
  {
    return (fRMSRawP * (V_REF / ADC_MAX)) * fScalingFactorP; /* Use Precision Channel */
  }
}

static void SGCurrentProcess(uint8_t bBufferHalf)
{
  uint16_t *psADC1ConvValPtr = (bBufferHalf
                                == 0) ? &saADC1ConvVals[0]
                               : &saADC1ConvVals[SAMPLES_PER_MAINS_CYCLE
                                                 * ADC1_MAX_CHANNELS];
  uint16_t *psADC2ConvValPtr = (bBufferHalf
                                == 0) ? &saADC2ConvVals[0]
                               : &saADC2ConvVals[SAMPLES_PER_MAINS_CYCLE
                                                 * ADC2_MAX_CHANNELS];

  /* Extract RMS for all 8 Channel */

  /* ADC1 Channels (Stride 4) */
  /* Rank1: SG3_P */
  /* Rank2: SG4_P */
  /* Rank1: SG1_H */
  /* Rank2: SG2_H */
  faSGP[SIGNAL_GROUP3_INDEX] = SGCurrentRMSCalculate(psADC1ConvValPtr,
                                                     0,
                                                     ADC1_MAX_CHANNELS,
                                                     SAMPLES_PER_MAINS_CYCLE);
  faSGP[SIGNAL_GROUP4_INDEX] = SGCurrentRMSCalculate(psADC1ConvValPtr,
                                                     1,
                                                     ADC1_MAX_CHANNELS,
                                                     SAMPLES_PER_MAINS_CYCLE);
  faSGH[SIGNAL_GROUP1_INDEX] = SGCurrentRMSCalculate(psADC1ConvValPtr,
                                                     2,
                                                     ADC1_MAX_CHANNELS,
                                                     SAMPLES_PER_MAINS_CYCLE);
  faSGH[SIGNAL_GROUP2_INDEX] = SGCurrentRMSCalculate(psADC1ConvValPtr,
                                                     3,
                                                     ADC1_MAX_CHANNELS,
                                                     SAMPLES_PER_MAINS_CYCLE);

  /* ADC2 Channels (Stride 4) */
  /* Rank1: SG3_H */
  /* Rank2: SG4_H */
  /* Rank3: SG2_P */
  /* Rank4: SG1_P */
  faSGH[SIGNAL_GROUP3_INDEX] = SGCurrentRMSCalculate(psADC2ConvValPtr,
                                                     0,
                                                     ADC2_MAX_CHANNELS,
                                                     SAMPLES_PER_MAINS_CYCLE);
  faSGH[SIGNAL_GROUP4_INDEX] = SGCurrentRMSCalculate(psADC2ConvValPtr,
                                                     1,
                                                     ADC2_MAX_CHANNELS,
                                                     SAMPLES_PER_MAINS_CYCLE);
  faSGP[SIGNAL_GROUP2_INDEX] = SGCurrentRMSCalculate(psADC2ConvValPtr,
                                                     2,
                                                     ADC2_MAX_CHANNELS,
                                                     SAMPLES_PER_MAINS_CYCLE);
  faSGP[SIGNAL_GROUP1_INDEX] = SGCurrentRMSCalculate(psADC2ConvValPtr,
                                                     3,
                                                     ADC2_MAX_CHANNELS,
                                                     SAMPLES_PER_MAINS_CYCLE);

  uint8_t bSGIdx = 0;
  uint8_t bMeasurementStatus = 0U;

  for (bSGIdx = 0; bSGIdx < SIGNAL_GROUPS_PER_SSM_MAX; bSGIdx++)
  {
    /* Auto-Range Selection */
    faSGCurrentRMS[bSGIdx] = SGOptimalCurrentGet(faSGP[bSGIdx],
                                                 faSGH[bSGIdx],
                                                 SCALING_FACTOR_P,
                                                 SCALING_FACTOR_H) * 1000.0f;

    /* Apply hardware dead-zone compensation */
    #ifndef DEBUG
    if (faSGCurrentRMS[bSGIdx] < SIGNAL_GROUP_NO_LOAD_THRESHOLD_MA)
    {
      faSGCurrentRMS[bSGIdx] = 0.0f;
    }

    /*else
    *  {
    *  faSGCurrentRMS[bSGIdx] += SIGNAL_GROUP_CURRENT_DEADZONE_OFFSET_MA;
    *  }*/

    #endif /* ifndef DEBUG */

    if (faSGCurrentRMS[bSGIdx] >= MP_MAX_CURRENT_MS)
    {
      faSGCurrentRMS[bSGIdx] = MP_MAX_CURRENT_MS - 1.0f;
      bMeasurementStatus = (uint8_t) (bMeasurementStatus
                                      | CURRENT_MEASUREMENT_STATUS_SATURATED);
    }
  }

  /* Publish the latest RMS values through the ICurrentMeasurementPort
   * double-buffer. Called from task context post-Step-6; the adapter's
   * __DMB + atomic swap also works from ISR, so the primitive is future-
   * proof if this ever needs to move back.
   */
  AdcCurrentAdapter_Publish(&g_AdcCurrentCtx, faSGCurrentRMS, bMeasurementStatus);
} /* SGCurrentProcess */

/* ISR now does the minimum: remember which half is fresh and wake the
 * measurement task. All the RMS / scaling / auto-range math runs in task
 * context via ADCSGCurrentProcessLatestHalf() so the DMA interrupt stays
 * well under 1 μs and doesn't delay FDCAN RX handling.
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC2)
  {
    s_LatestReadyHalf = 0U;
    MeasurementThreadFlagSet();
  }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC2)
  {
    s_LatestReadyHalf = 1U;
    MeasurementThreadFlagSet();
  }
}

void ADCSGCurrentProcessLatestHalf(void)
{
  uint8_t bHalf = s_LatestReadyHalf;

  if (bHalf == 0xFFU)
  {
    return;
  }

  SGCurrentProcess(bHalf);
}

void ADCSGCurrentMeasurementStart(void)
{
  ADC1StartDMA();
  ADC2StartDMA();
}

/* USER CODE END 1 */
