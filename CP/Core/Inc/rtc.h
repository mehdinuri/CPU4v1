/* USER CODE BEGIN Header */

/**
 ******************************************************************************
 * @file    rtc.h
 * @brief   This file contains all the function prototypes for
 *          the rtc.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN Private defines */
typedef struct _STime
{
  RTC_DateTypeDef SCurrentDate;
  RTC_TimeTypeDef SCurrentTime;

  uint8_t fDST;
  uint8_t bCentury;
  uint16_t sFullYear;
  uint32_t lSecondOfDay;
  uint16_t sMinuteOfDay;
  uint16_t sDayOfYear;
} tSTime, *tpSTime;
/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
extern void RTCWriteConfigValue(uint32_t lValue);
extern uint32_t RTCReadConfigValue(void);
extern void RTCWriteCentury(uint8_t bCentury);
extern uint8_t RTCReadCentury(void);
extern void RTCWaitSync(void);
extern void RTCGetTime(RTC_TimeTypeDef *pSNewTime);
extern void RTCGetDate(RTC_DateTypeDef *pSNewDate);
extern void RTCSetTime(RTC_TimeTypeDef *pSNewTime);
extern void RTCSetDate(RTC_DateTypeDef *pSNewDate);
extern void RTCDSTAddOneHour(void);
extern void RTCDSTSubtractOneHour(void);
extern void RTCDSTSetStoreOperation(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

