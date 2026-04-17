/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN Private defines */
typedef enum
{
	CHARGING_NONE = 0,
	NOT_CHARGING,
	CHARGING,
	CHARGING_FAULT
	
} tEBatteryChargingStates;

typedef enum
{
	CHG_TREND_NONE = 0,
	CHG_TREND_STABLE,
	CHG_TREND_RISING,
	CHG_TREND_FALLING
	
} tEBatteryChargingTrends;

typedef struct _tSADCBatteryRuntime
{
	double dVoltage;
	tEBatteryChargingStates EChargingState;
	tEBatteryChargingTrends EChargingTrend;
	
} tSADCBatteryRuntime, *tpSADCBatteryRuntime;
/* USER CODE END Private defines */

void MX_ADC1_Init(void);

/* USER CODE BEGIN Prototypes */
void ADCStartIT(void);
void ADCStopIT(void);
void ADCDeInit(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

