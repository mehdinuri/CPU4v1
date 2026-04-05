/* USER CODE BEGIN Header */

/**
 ******************************************************************************
 * @file    fdcan.h
 * @brief   This file contains all the function prototypes for
 *          the fdcan.c file
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
#define FDCAN_DATA_MAX_LEN 8

typedef struct
{
  FDCAN_HandleTypeDef *hfdcan;
  FDCAN_RxHeaderTypeDef RxHeader;
  uint8_t Data[FDCAN_DATA_MAX_LEN];
} tSFDCANRxMsg, *tpSFDCANRxMsg;

typedef struct
{
  FDCAN_HandleTypeDef *hfdcan;
  FDCAN_TxHeaderTypeDef TxHeader;
  uint8_t Data[FDCAN_DATA_MAX_LEN];
} tSFDCANTxMsg, *tpSFDCANTxMsg;
/* USER CODE END Private defines */

void MX_FDCAN1_Init(void);
void MX_FDCAN2_Init(void);

/* USER CODE BEGIN Prototypes */
extern void CANStart(FDCAN_HandleTypeDef *hfdcan);
extern void CANStop(FDCAN_HandleTypeDef *hfdcan);
extern void CANDeInit(FDCAN_HandleTypeDef *hfdcan);
extern void CANSendMessage(tpSFDCANTxMsg pxCANTxMessage);
extern void CANWaitTransmissionComplete(FDCAN_HandleTypeDef *hfdcan);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */

