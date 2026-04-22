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
#define FDCAN_CP_SIGNAL_OUTPUTS_1_STD_ID 0x040
#define FDCAN_CP_SIGNAL_OUTPUTS_2_STD_ID 0x041

#define FDCAN_SSM_VOLTAGE_CURRENT_1_STD_ID 0x050
#define FDCAN_SSM_VOLTAGE_CURRENT_2_STD_ID 0x051
#define FDCAN_SSM_VOLTAGE_CURRENT_3_STD_ID 0x052
#define FDCAN_SSM_VOLTAGE_CURRENT_4_STD_ID 0x053
#define FDCAN_SSM_VOLTAGE_CURRENT_5_STD_ID 0x054
#define FDCAN_SSM_VOLTAGE_CURRENT_6_STD_ID 0x055
#define FDCAN_SSM_VOLTAGE_CURRENT_7_STD_ID 0x056
#define FDCAN_SSM_VOLTAGE_CURRENT_8_STD_ID 0x057

#define FDCAN_PSM_FLASH_SYNC_1_STD_ID 0x069
#define FDCAN_PSM_FLASH_SYNC_2_STD_ID 0x06A

#define FDCAN_CP_FLASH_SIGNALS_1_STD_ID 0x0B0
#define FDCAN_CP_FLASH_SIGNALS_2_STD_ID 0x0B1

#define FDCAN_MAX_DATA_LEN	8

#define SIGNAL_OUTPUTS_PER_SIGNAL_GROUP (uint8_t) 3

/* USER CODE END Private defines */

void MX_FDCAN1_Init(void);
void MX_FDCAN2_Init(void);

/* USER CODE BEGIN Prototypes */
typedef struct
{
	FDCAN_HandleTypeDef	*hfdcan;
  FDCAN_RxHeaderTypeDef rxHeader;
  uint8_t data[FDCAN_MAX_DATA_LEN];
	
} FdcanRxMsg_t;

typedef struct
{
	FDCAN_HandleTypeDef	*hfdcan;
  FDCAN_TxHeaderTypeDef txHeader;
  uint8_t data[FDCAN_MAX_DATA_LEN];
	
} FdcanTxMsg_t;

/* NOTE: the former BitFlags16_t, VoltageCurrent_t, and Outputs_t types
 * were deleted once the outgoing CAN telemetry path moved to the byte-
 * explicit Domain/VoltageCurrentFrame encoder. They were the main source
 * of GCC's "'packed' attribute ignored" warnings (nested __packed unions).
 *
 * The remaining types below are module-internal state only — never
 * memcpy'd onto a CAN frame — so __packed is redundant: all members are
 * already byte-aligned and the compiler's natural layout matches the
 * field order we use.
 */

typedef struct
{
	uint8_t onCntr;
	uint8_t offCntr;

	uint8_t isActive : 1;
	uint8_t isOn : 1;
	uint8_t isFlashing : 1;
	uint8_t reserved : 5;

} SignalOutput_t;

typedef struct
{
	SignalOutput_t signalOutputs[SIGNAL_OUTPUTS_PER_SIGNAL_GROUP];
	uint16_t current;

} OutputGroup_t;

typedef struct
{
	uint8_t onCntr;
	uint8_t offCntr;

} SignalOutputStateCntr_t;

extern uint8_t CANGetRxDataLength(uint32_t lenCode);
extern uint32_t CANGetTxDataLengthCode(uint8_t len);
extern void CANStart(FDCAN_HandleTypeDef* hfdcan);
extern void CANStop(FDCAN_HandleTypeDef* hfdcan);
extern void CANDeInit(FDCAN_HandleTypeDef* hfdcan);
extern uint8_t CANSendMessage(FdcanTxMsg_t * msg);
extern uint8_t CANWaitTxComplete(FDCAN_HandleTypeDef* hfdcan,
                                 uint32_t txBufferIndex,
                                 uint32_t timeoutMs);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */

