/* USER CODE BEGIN Header */

/**
 ******************************************************************************
 * @file    rtc.c
 * @brief   This file provides code for the configuration
 *          of the RTC instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include "rtc.h"

/* USER CODE BEGIN 0 */
#include "time.h"

#define RTC_CFG_VAL 0x2014
#define TIME_CURRENT_CENTURY 21

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
  tSTime SRTCTime;

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 30, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  if (RTCReadConfigValue() != RTC_CFG_VAL)
  {
    SRTCTime.bCentury = TIME_CURRENT_CENTURY - 1;
    SRTCTime.SCurrentDate.Month = RTC_MONTH_JANUARY;
    SRTCTime.SCurrentDate.Year = 24;
    SRTCTime.SCurrentDate.Date = 1;
    SRTCTime.SCurrentDate.WeekDay = RTC_WEEKDAY_MONDAY;

    SRTCTime.SCurrentTime.Hours = 8;
    SRTCTime.SCurrentTime.Minutes = 0;
    SRTCTime.SCurrentTime.Seconds = 0;
    SRTCTime.SCurrentTime.TimeFormat = RTC_HOURFORMAT_24;
    SRTCTime.SCurrentTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    SRTCTime.SCurrentTime.StoreOperation = RTC_STOREOPERATION_RESET;

    RTCSetDate(&SRTCTime.SCurrentDate);
    RTCSetTime(&SRTCTime.SCurrentTime);

    RTCWriteCentury(SRTCTime.bCentury);
    RTCWriteConfigValue(RTC_CFG_VAL);

    RTCWaitSync();
  }

  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();

    /* RTC interrupt Init */
    HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();

    /* RTC interrupt Deinit */
    HAL_NVIC_DisableIRQ(RTC_WKUP_IRQn);
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void RTCWriteConfigValue(uint32_t lValue)
{
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, lValue);
}

uint32_t RTCReadConfigValue(void)
{
  return HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
}

void RTCWriteCentury(uint8_t bCentury)
{
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, bCentury);
}

uint8_t RTCReadCentury(void)
{
  return HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
}

void RTCWaitSync(void)
{
  HAL_RTC_WaitForSynchro(&hrtc);
}

void RTCGetTime(RTC_TimeTypeDef *pSNewTime)
{
  HAL_RTC_GetTime(&hrtc, pSNewTime, RTC_FORMAT_BIN);
}

void RTCGetDate(RTC_DateTypeDef *pSNewDate)
{
  HAL_RTC_GetDate(&hrtc, pSNewDate, RTC_FORMAT_BIN);
}

void RTCSetTime(RTC_TimeTypeDef *pSNewTime)
{
  HAL_RTC_SetTime(&hrtc, pSNewTime, RTC_FORMAT_BIN);
}

void RTCSetDate(RTC_DateTypeDef *pSNewDate)
{
  HAL_RTC_SetDate(&hrtc, pSNewDate, RTC_FORMAT_BIN);
}

void RTCDSTAddOneHour(void)
{
  __HAL_RTC_DAYLIGHT_SAVING_TIME_ADD1H(&hrtc, RTC_STOREOPERATION_SET);
}

void RTCDSTSubtractOneHour(void)
{
  __HAL_RTC_DAYLIGHT_SAVING_TIME_SUB1H(&hrtc, RTC_STOREOPERATION_SET);
}

/* USER CODE END 1 */

