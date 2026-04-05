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
#include <string.h>
#include "gpio.h"
#include "data.h"

#define MAX_BATTERY_VOLTAGE_SAMPLES 100
#define MAX_BATTERY_VOLTAGE_TOTAL_SAMPLES 10
#define MAX_BATTERY_VOLTAGE_TREND_SAMPLES 60
#define MAX_IS_ALREADY_DISABLED_FLAG_SAMPLES 60
#define MIN_TREND_SAMPLES (double) (MAX_BATTERY_VOLTAGE_TREND_SAMPLES / 6.0)
#define MIN_TREND_DIFFERENCE (double) 10.0

#define VCC_VOLTAGE_VALUE (double) 3300
#define ADC_RESOLUTION_OUTPUTS (double) 4096
#define ADC_RESOULTION_COEFFICIENT (double)(VCC_VOLTAGE_VALUE / ADC_RESOLUTION_OUTPUTS)
#define RESISTANCE_COEFFICIENT (double) 2.0
#define VBAT_VOLTAGE_COEFFICIENT (double)(ADC_RESOULTION_COEFFICIENT * RESISTANCE_COEFFICIENT)
	
#define MIN_TARGETED_PRESERVING_BATTERY_VOLTAGE (float) 3700
#define MAX_BATTERY_VOLTAGE (double) 4200
#define MIN_GPRS_OPERATING_VOLTAGE (double) 3450

volatile uint32_t laBatteryTotalVolts[MAX_BATTERY_VOLTAGE_TOTAL_SAMPLES] = {0};
volatile uint32_t laBatteryVoltTrends[MAX_BATTERY_VOLTAGE_TREND_SAMPLES] = {0};
volatile uint8_t baBatteryChargingTotalStates[MAX_BATTERY_VOLTAGE_TOTAL_SAMPLES] = {0};

volatile uint32_t lTotVoltDuringSampling = 0;
volatile uint8_t bTotPinStateDuringSampling = 0;
volatile uint8_t bVoltSampleCntr = 0;
volatile uint8_t bTotVoltSampleCntr = 0;
volatile uint8_t bTrendSampleCntr = 0;
volatile uint8_t bIsAlreadyDisabledFlagCntr = 0;
volatile uint8_t fIsAlreadyDisabled = FALSE;

static tSADCBatteryRuntime SBatteryRuntime;

/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV10;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
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
  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */
	ADCStartIT();
  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* ADC1 clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA2     ------> ADC1_IN3
    PB0     ------> ADC1_IN15
    */
    GPIO_InitStruct.Pin = TERMISTOR_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(TERMISTOR_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = VBAT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(VBAT_GPIO_Port, &GPIO_InitStruct);

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC1_2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC12_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA2     ------> ADC1_IN3
    PB0     ------> ADC1_IN15
    */
    HAL_GPIO_DeInit(TERMISTOR_GPIO_Port, TERMISTOR_Pin);

    HAL_GPIO_DeInit(VBAT_GPIO_Port, VBAT_Pin);

    /* ADC1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(ADC1_2_IRQn);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void ADCStartIT(void)
{
	memset(&SBatteryRuntime, 0, sizeof(SBatteryRuntime));
	if(HAL_ADC_Start_IT(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
}

void ADCStopIT(void)
{
	HAL_ADC_Stop_IT(&hadc1);
}

void ADCDeInit(void)
{
	HAL_ADC_DeInit(&hadc1);
}

void ADCDisableBatteryCharging(void)
{
	if(GPIOIsBatteryChargingEnabled() && !fIsAlreadyDisabled)
	{
		fIsAlreadyDisabled = TRUE;
		bIsAlreadyDisabledFlagCntr = 0;
		GPIODisableBatteryCharging(); // Is enabled after 1s.
	}
}

float ADCCalculateBatteryVoltage(void)
{
	uint8_t ucIdx;
	uint32_t ulGrandTotBatteryVal = 0;
	uint32_t ulMinTotBatteryVal = UINT32_MAX;
	uint32_t ulMaxTotBatteryVal = 0;
	
	for(ucIdx = 0; ucIdx < MAX_BATTERY_VOLTAGE_TOTAL_SAMPLES; ucIdx++)
	{
		ulGrandTotBatteryVal += laBatteryTotalVolts[ucIdx];
		if(laBatteryTotalVolts[ucIdx] < ulMinTotBatteryVal)
		{
			ulMinTotBatteryVal = laBatteryTotalVolts[ucIdx];
		}
		
		if(laBatteryTotalVolts[ucIdx] > ulMaxTotBatteryVal)
		{
			ulMaxTotBatteryVal = laBatteryTotalVolts[ucIdx];
		}
	}
	
	ulGrandTotBatteryVal -= ulMinTotBatteryVal;
	ulGrandTotBatteryVal -= ulMaxTotBatteryVal;
	
	float fAverageBatteryVal = ulGrandTotBatteryVal / (MAX_BATTERY_VOLTAGE_TOTAL_SAMPLES - 2);
	return (float)(fAverageBatteryVal * VBAT_VOLTAGE_COEFFICIENT);
}

tEBatteryChargingStates ADCEvaluateBatteryChargingState(void)
{
	uint8_t ucIdx;
	uint16_t usTotBatStateVals = 0;
	
	for(ucIdx = 0; ucIdx < MAX_BATTERY_VOLTAGE_TOTAL_SAMPLES; ucIdx++)
	{
		usTotBatStateVals += baBatteryChargingTotalStates[ucIdx];
	}
	
	if(usTotBatStateVals == 0)
	{
		return CHARGING;
	}
	
	if(usTotBatStateVals == MAX_BATTERY_VOLTAGE_TOTAL_SAMPLES)
	{
		return NOT_CHARGING;
	}
	
	if(usTotBatStateVals > 0 && usTotBatStateVals < MAX_BATTERY_VOLTAGE_TOTAL_SAMPLES)
	{
		return CHARGING_FAULT;
	}
	
	return CHARGING_NONE;
}

tEBatteryChargingTrends ADCEvaluteBatteryChargingTrend(void)
{
	float fOldestTrendAvgVal = 0, fNewestTrendAvgVal = 0;
	uint32_t ulOldestTrendTotVal = 0, ulNewestTrendTotVal = 0;
	uint8_t ucOldTrendsIdx = 0;
	uint8_t ucNewTrendsIdx = MAX_BATTERY_VOLTAGE_TREND_SAMPLES - MIN_TREND_SAMPLES;
	
	while(ucOldTrendsIdx < MIN_TREND_SAMPLES)
	{
		ulOldestTrendTotVal += laBatteryVoltTrends[ucOldTrendsIdx];
		ucOldTrendsIdx++;
	}

	while(ucNewTrendsIdx < MAX_BATTERY_VOLTAGE_TREND_SAMPLES)
	{
		ulNewestTrendTotVal += laBatteryVoltTrends[ucNewTrendsIdx];
		ucNewTrendsIdx++;
	}
	
	fOldestTrendAvgVal = (ulOldestTrendTotVal / MIN_TREND_SAMPLES);
	fNewestTrendAvgVal = (ulNewestTrendTotVal / MIN_TREND_SAMPLES);
	
	if(fNewestTrendAvgVal > (fOldestTrendAvgVal + MIN_TREND_DIFFERENCE))
	{
		return CHG_TREND_RISING;
	}
	
	if(fNewestTrendAvgVal < (fOldestTrendAvgVal - MIN_TREND_DIFFERENCE))
	{
		return CHG_TREND_FALLING;
	}
	
	return CHG_TREND_STABLE;
}

void ADCSetIsAlreadyDisabledFlag(void)
{
	if(fIsAlreadyDisabled && (SBatteryRuntime.EChargingState != CHARGING || SBatteryRuntime.EChargingTrend != CHG_TREND_RISING))
	{
		fIsAlreadyDisabled = FALSE;
	}
}

void ADCSetBatteryCharging(void)
{
	if(SBatteryRuntime.dVoltage < MIN_GPRS_OPERATING_VOLTAGE)
	{
		ADCDisableBatteryCharging();
		return;
	}
	
	if(SBatteryRuntime.dVoltage < MIN_TARGETED_PRESERVING_BATTERY_VOLTAGE)
	{
		if(SBatteryRuntime.EChargingState != CHARGING)
		{
			ADCDisableBatteryCharging();
			return;
		}
		
		if(SBatteryRuntime.EChargingTrend == CHG_TREND_FALLING) // Battery seems to be charging, but voltage is not rising
		{
			ADCDisableBatteryCharging();
		}
	}
}

float ADCGetBatteryVoltageValue(void)
{
	return SBatteryRuntime.dVoltage;
}

tEBatteryChargingStates ADCGetBatteryChargingState(void)
{
	return SBatteryRuntime.EChargingState;
}

tEBatteryChargingTrends ADCGetBatteryChargingTrend(void)
{
	return SBatteryRuntime.EChargingTrend;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(hadc->Instance == ADC1)
	{
		/*lTotVoltDuringSampling += HAL_ADC_GetValue(hadc);
		bTotPinStateDuringSampling += GPIOBatteryChargingPinStateGet();
		
		if(++bVoltSampleCntr > MAX_BATTERY_VOLTAGE_SAMPLES)
		{
			baBatteryChargingTotalStates[bTotVoltSampleCntr] = bTotPinStateDuringSampling / MAX_BATTERY_VOLTAGE_SAMPLES;
			laBatteryTotalVolts[bTotVoltSampleCntr] = lTotVoltDuringSampling / MAX_BATTERY_VOLTAGE_SAMPLES;
			
			bTotVoltSampleCntr++;
			bVoltSampleCntr = 0;
			lTotVoltDuringSampling = 0;
			bTotPinStateDuringSampling = 0;
			
			if(bTotVoltSampleCntr >= MAX_BATTERY_VOLTAGE_TOTAL_SAMPLES)
			{
				if(!GPIOIsBatteryChargingEnabled())
				{
					GPIOEnableBatteryCharging();
				}
			
				SBatteryRuntime.dVoltage = ADCCalculateBatteryVoltage();
				laBatteryVoltTrends[bTrendSampleCntr] = SBatteryRuntime.dVoltage;
				SBatteryRuntime.EChargingState = ADCEvaluateBatteryChargingState();
				
				bIsAlreadyDisabledFlagCntr++;
				if(bIsAlreadyDisabledFlagCntr >= MAX_IS_ALREADY_DISABLED_FLAG_SAMPLES)
				{
					bIsAlreadyDisabledFlagCntr = 0;
					ADCSetIsAlreadyDisabledFlag();
				}
				
				bTrendSampleCntr++;
				if(bTrendSampleCntr >= MAX_BATTERY_VOLTAGE_TREND_SAMPLES)
				{
					SBatteryRuntime.EChargingTrend = ADCEvaluteBatteryChargingTrend();
					
					bTrendSampleCntr = 0;
					memset((void *)laBatteryVoltTrends, 0, sizeof(laBatteryVoltTrends));
				}
				
				ADCSetBatteryCharging();
				
				tpSADCBatteryRuntime pSRuntime = osMemoryPoolAlloc(BatteryRuntimeMemPoolHandle, 0);
				if (pSRuntime != 0)
				{
					memcpy(pSRuntime, &SBatteryRuntime, sizeof(SBatteryRuntime));
					if (osMessageQueuePut(BatteryRuntimeQueueHandle, &pSRuntime, 0, 0) != osOK)
					{
						osMemoryPoolFree(BatteryRuntimeMemPoolHandle, pSRuntime);
						Error_Handler();
					}
				}
				else
				{
					Error_Handler();
				}
				
				bTotVoltSampleCntr = 0;
				memset((void *)baBatteryChargingTotalStates, 0, sizeof(baBatteryChargingTotalStates));
				memset((void *)laBatteryTotalVolts, 0, sizeof(laBatteryTotalVolts));
			}
		}*/
	}
}
/* USER CODE END 1 */

