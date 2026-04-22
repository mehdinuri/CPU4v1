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
typedef struct FdcanRxMsg
{
  FDCAN_HandleTypeDef   *hfdcan;
  FDCAN_RxHeaderTypeDef  rxHeader;
  uint8_t                data[FDCAN_MAX_DATA_LEN];
} FdcanRxMsg_t;

typedef struct FdcanTxMsg
{
  FDCAN_HandleTypeDef   *hfdcan;
  FDCAN_TxHeaderTypeDef  txHeader;
  uint8_t                data[FDCAN_MAX_DATA_LEN];
} FdcanTxMsg_t;

typedef struct __attribute__((packed))
{
  uint8_t acLow           : 8;
  uint8_t dc24V1Low     : 8;
  uint8_t dc24V2Low     : 8;
  uint8_t dc5V1Low      : 8;
  uint8_t dc5V2Low      : 8;
  uint8_t acHigh          : 2;
  uint8_t dc24V1High    : 2;
  uint8_t dc24V2High    : 2;
  uint8_t dc5V1High     : 2;
  uint8_t dc5V2High     : 2;
  uint8_t isolatedVoltage : 1;
  uint8_t reserved        : 5;
  uint8_t frequency       : 8;
} PsmMeasurement_t;

typedef struct __attribute__((packed))
{
  uint8_t flashSync : 1;
  uint8_t reserved  : 7;
  uint8_t padding[7];
} FlashSync_t;

extern uint8_t CANGetRxDataLength(uint32_t code);
extern uint32_t CANGetTxDataLengthCode(uint8_t len);
extern void CANStart(FDCAN_HandleTypeDef* hfdcan);
extern void CANStop(FDCAN_HandleTypeDef* hfdcan);
extern void CANDeInit(FDCAN_HandleTypeDef* hfdcan);
extern void CANSendMessage(FdcanTxMsg_t * msg);
extern void CANWaitTxComplete(FDCAN_HandleTypeDef* hfdcan);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */

