/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.h
  * @brief   This file contains all the function prototypes for
  *          the fdcan.c file
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
#ifndef __FDCAN_H__
#define __FDCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern FDCAN_HandleTypeDef hfdcan1;

extern FDCAN_HandleTypeDef hfdcan2;

/* USER CODE BEGIN Private defines */
#define FDCAN_CP_FLASH_SIGNALS_1_STD_ID 0x0B0
#define FDCAN_CP_FLASH_SIGNALS_2_STD_ID 0x0B1
#define FDCAN_CP_DATE_TIME_STD_ID 0x100
#define FDCAN_CP_OFFSET_1_STD_ID 0x240
#define FDCAN_CP_OFFSET_2_STD_ID 0x241

#define FDCAN_PSM_MEASUREMENT_STD_ID 0x05A
#define FDCAN_PSM_FLASH_SYNC_1_STD_ID 0x069
#define FDCAN_PSM_FLASH_SYNC_2_STD_ID 0x06A

#define FDCAN_MAX_DATA_LEN	8
/* USER CODE END Private defines */

void MX_FDCAN1_Init(void);
void MX_FDCAN2_Init(void);

/* USER CODE BEGIN Prototypes */
typedef struct _tSFDCANRxMsg
{
	FDCAN_HandleTypeDef	*hfdcan;
  FDCAN_RxHeaderTypeDef SRxHeader;
  uint8_t baData[FDCAN_MAX_DATA_LEN];
	
} tSFDCANRxMsg, *tpSFDCANRxMsg;

typedef struct _tSFDCANTxMsg
{
	FDCAN_HandleTypeDef	*hfdcan;
  FDCAN_TxHeaderTypeDef STxHeader;
  uint8_t baData[FDCAN_MAX_DATA_LEN];
	
} tSFDCANTxMsg, *tpSFDCANTxMsg;

typedef struct __attribute__((packed)) _tSPSMMeasurement
{
	uint8_t bACLow : 8;
	uint8_t b24V1Low : 8;
	uint8_t b24V2Low : 8;
	uint8_t b5V1Low : 8;
	uint8_t b5V2Low : 8;
	uint8_t bACHigh : 2;
	uint8_t b24V1High : 2;
	uint8_t b24V2High : 2;
	uint8_t b5V1High : 2;
	uint8_t b5V2High : 2;
	uint8_t bIsolatedVoltage : 1;
	uint8_t bReserved : 5;
	uint8_t bFrequency : 8;

} tSPSMMeasurement, *tpSPSMMeasurement;

typedef struct __attribute__((packed)) _tSFlashSync
{
	uint8_t bFlashSync : 1;
	uint8_t bReserved : 7;
	uint8_t baReserved[7];

} tSFlashSync, *tpSFlashSync;

extern uint8_t CANGetRxDataLength(uint32_t lCode);
extern uint32_t CANGetTxDataLengthCode(uint8_t bLen);
extern void CANStart(FDCAN_HandleTypeDef* hfdcan);
extern void CANStop(FDCAN_HandleTypeDef* hfdcan);
extern void CANDeInit(FDCAN_HandleTypeDef* hfdcan);
extern void CANSendMessage(tpSFDCANTxMsg pSMsg);
extern void CANWaitTxComplete(FDCAN_HandleTypeDef* hfdcan);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */

