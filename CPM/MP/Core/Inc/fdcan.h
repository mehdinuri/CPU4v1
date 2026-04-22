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

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

/* USER CODE BEGIN Private defines */
/* sender: cpu, receiver: ssm */
#define CAN_MID_CPU_SO0 0x040
#define CAN_MID_CPU_SO1 0x041

/* sender: cpu, receiver: ssm */
#define CAN_MID_CPU_FLASH_SIGNALS0 0x0B0
#define CAN_MID_CPU_FLASH_SIGNALS1 0x0B1

/* sender: ssm, receiver: cpu */
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS0 0x050
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS1 0x051
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS2 0x052
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS3 0x053
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS4 0x054
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS5 0x055
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS6 0x056
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS7 0x057

/* sender: psm, receiver: cpu */
#define CAN_MID_PSM_VOLT_MEASUREMENTS0 0x05A
#define CAN_MID_PSM_VOLT_MEASUREMENTS1 0x05B
/* USER CODE END Private defines */

void MX_FDCAN1_Init(void);
void MX_FDCAN2_Init(void);

/* USER CODE BEGIN Prototypes */
extern void CANStart(FDCAN_HandleTypeDef *hfdcan);
extern void CANStop(FDCAN_HandleTypeDef *hfdcan);
extern void CANDeInit(FDCAN_HandleTypeDef *hfdcan);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */
