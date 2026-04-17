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

typedef __packed struct _tSBitFlags16
{
	uint8_t fBit0 : 1;
	uint8_t fBit1 : 1;
	uint8_t fBit2 : 1;
	uint8_t fBit3 : 1;
	uint8_t fBit4 : 1;
	uint8_t fBit5 : 1;
	uint8_t fBit6 : 1;
	uint8_t fBit7 : 1;
	uint8_t fBit8 : 1;
	uint8_t fBit9 : 1;
	uint8_t fBit10 : 1;
	uint8_t fBit11 : 1;
	uint8_t fBit12: 1;
  uint8_t fBit13: 1;
  uint8_t fBit14: 1;
  uint8_t fBit15: 1;

} tSBitFlags16, *tpSBitFlags16;

typedef __packed struct _tSVoltageCurrent_t
{
	__packed union 
	{
		tSBitFlags16 SVoltageBits;
		uint16_t sVoltages;

	} UVoltages;

	__packed union 
	{
		__packed struct
		{
			uint8_t bSOCurLow0_2;
			uint8_t bSOCurLow3_5;
			uint8_t bSOCurLow6_8;
			uint8_t bSOCurLow9_11;

		} SSOCurrentsLow;

		uint8_t baSOCurrentsLow[4];

	} USOCurrentsLow;

	__packed union 
	{
		__packed struct
		{
			uint8_t bSOCurHighBits0_2 : 2;
			uint8_t bSOCurHighBits3_5 : 2;
			uint8_t bSOCurHighBits6_8 : 2;
			uint8_t bSOCurHighBits9_11 : 2;

		} SSOCurrentsHighBits;

		uint8_t bSOCurrentsHighBits;

	} USOCurrentsHighBits;

	uint8_t bReserved2;

} tSVoltageCurrent_t, *tpSVoltageCurrent_t;

typedef __packed struct _tSSignalOutput
{
	uint8_t bOnCntr;
	uint8_t bOffCntr;
					
	uint8_t fIsActive : 1;
	uint8_t fIsOn : 1;
	uint8_t fIsFlashing : 1;
	uint8_t bReserved : 5;
	
} tSSignalOutput, *tpSSignalOutput;

typedef __packed struct _tSOutputGroup
{
	tSSignalOutput SaSignalOutputs[SIGNAL_OUTPUTS_PER_SIGNAL_GROUP];
	uint16_t sCurrent;
	
} tSOutputGroup, *tpSOutputGroup;

typedef __packed union _tUOutputs
{
	tSBitFlags16 SBitFlags16;
	uint16_t sData;
	uint8_t baData[2];
	
} tUOutputs, *tpUOutputs;

typedef __packed struct _tSSignalOutputStateCntr
{
	uint8_t bOnCntr;
	uint8_t bOffCntr;
	
} tSSignalOutputStateCntr, *tpSSignalOutputStateCntr;

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

