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
// bPacket Type Definitions
#define	PACKET_TYPE_CP_NONE											0x00
	#define	PACKET_SUB_TYPE_CP_NONE									0x00
#define	PACKET_TYPE_CP_SIGNAL_DEFS							0x01
	#define	PACKET_SUB_TYPE_CP_SIGNAL_DEFS_1				0x01
	#define	PACKET_SUB_TYPE_CP_SIGNAL_DEFS_2				0x02
	#define	PACKET_SUB_TYPE_CP_SIGNAL_DEFS_LAST			0x02
#define	PACKET_TYPE_CP_SIGNALS_DEFINED					0x02
#define	PACKET_TYPE_CP_SG_DEFS									0x03
	#define	PACKET_SUB_TYPE_CP_SG_DEFS_1						0x01
	#define	PACKET_SUB_TYPE_CP_SG_DEFS_2						0x02
	#define	PACKET_SUB_TYPE_CP_SG_DEFS_3						0x03
	#define	PACKET_SUB_TYPE_CP_SG_DEFS_4						0x04
	#define	PACKET_SUB_TYPE_CP_SG_DEFS_5						0x05
	#define	PACKET_SUB_TYPE_CP_SG_DEFS_6						0x06
	#define	PACKET_SUB_TYPE_CP_SG_LAST							0x06
#define	PACKET_TYPE_CP_SO_DEFS									0x04
	#define	PACKET_SUB_TYPE_CP_SO_DEFS_1						0x01
	#define	PACKET_SUB_TYPE_CP_SO_DEFS_2						0x02
	#define	PACKET_SUB_TYPE_CP_SO_LAST							0x02
#define	PACKET_TYPE_CP_CVS_DEFS									0x05
	#define	PACKET_SUB_TYPE_CP_CVS_DEFS_1						0x01
	#define	PACKET_SUB_TYPE_CP_CVS_DEFS_2						0x02
	#define	PACKET_SUB_TYPE_CP_CVS_DEFS_3						0x03
	#define	PACKET_SUB_TYPE_CP_CVS_DEFS_4						0x04
	#define	PACKET_SUB_TYPE_CP_CVS_DEFS_5						0x05
	#define	PACKET_SUB_TYPE_CP_CVS_LAST							0x05
#define	PACKET_TYPE_CP_CONFLICTS_EM							0x06
#define	PACKET_TYPE_CP_FLASH_CFG								0x07
#define	PACKET_TYPE_CP_DEFAULT									0x08
#define	PACKET_TOTAL_CNT												0x08

//	CP Tx Std Id Defs
#define	CAN_TX_CP_EXT_ID_SIGNAL_DEFS_0		0x1000
#define	CAN_TX_CP_EXT_ID_SIGNAL_DEFS_1		0x1100
#define	CAN_TX_CP_EXT_ID_SIGNALS_DEFINED	0x1200
#define	CAN_TX_CP_EXT_ID_SG_DEFS_0				0x1300
#define	CAN_TX_CP_EXT_ID_SG_DEFS_1				0x1400
#define	CAN_TX_CP_EXT_ID_SG_DEFS_2				0x1500
#define	CAN_TX_CP_EXT_ID_SG_DEFS_3				0x1600
#define	CAN_TX_CP_EXT_ID_SG_DEFS_4				0x1700
#define	CAN_TX_CP_EXT_ID_SG_DEFS_5				0x1800
#define	CAN_TX_CP_EXT_ID_SO_DEFS_0				0x1900
#define	CAN_TX_CP_EXT_ID_SO_DEFS_1				0x1A00
#define	CAN_TX_CP_EXT_ID_CVS_DEFS_0				0x1B00
#define	CAN_TX_CP_EXT_ID_CVS_DEFS_1				0x1C00
#define	CAN_TX_CP_EXT_ID_CVS_DEFS_2				0x1D00
#define	CAN_TX_CP_EXT_ID_CVS_DEFS_3				0x1E00
#define	CAN_TX_CP_EXT_ID_CVS_DEFS_4				0x1F00
#define	CAN_TX_CP_EXT_ID_CONFLICTS_EM			0x2000
#define	CAN_TX_CP_EXT_ID_FLASH_CFG				0x2100
#define	CAN_TX_CP_EXT_ID_DEFAULT					0x2200
#define	CAN_TX_CP_EXT_ID_EVENT						0x2300
#define	CAN_TX_CP_EXT_ID_PERIPHERAL_STATE	0x2400
#define CAN_TX_CP_EXT_ID_RESET						0x2500
#define CAN_TX_CP_EXT_ID_MP_VERSION				0x2600

//	MP Tx Std Id Defs
#define	CAN_TX_MP_EXT_ID_SIGNAL_DEFS_0		0x3000
#define	CAN_TX_MP_EXT_ID_SIGNAL_DEFS_1		0x3100
#define	CAN_TX_MP_EXT_ID_SIGNALS_DEFINED	0x3200
#define	CAN_TX_MP_EXT_ID_SG_DEFS_0				0x3300
#define	CAN_TX_MP_EXT_ID_SG_DEFS_1				0x3400
#define	CAN_TX_MP_EXT_ID_SG_DEFS_2				0x3500
#define	CAN_TX_MP_EXT_ID_SG_DEFS_3				0x3600
#define	CAN_TX_MP_EXT_ID_SG_DEFS_4				0x3700
#define	CAN_TX_MP_EXT_ID_SG_DEFS_5				0x3800
#define	CAN_TX_MP_EXT_ID_SO_DEFS_0				0x3900
#define	CAN_TX_MP_EXT_ID_SO_DEFS_1				0x3A00
#define	CAN_TX_MP_EXT_ID_CVS_DEFS_0				0x3B00
#define	CAN_TX_MP_EXT_ID_CVS_DEFS_1				0x3C00
#define	CAN_TX_MP_EXT_ID_CVS_DEFS_2				0x3D00
#define	CAN_TX_MP_EXT_ID_CVS_DEFS_3				0x3E00
#define	CAN_TX_MP_EXT_ID_CVS_DEFS_4				0x3F00
#define	CAN_TX_MP_EXT_ID_CONFLICTS_EM			0x4000
#define	CAN_TX_MP_EXT_ID_FLASH_CFG				0x4100
#define	CAN_TX_MP_EXT_ID_DEFAULT					0x4200
#define	CAN_TX_MP_EXT_ID_EVENT						0x4300
#define	CAN_TX_MP_EXT_ID_PERIPHERAL_STATE	0x4400
#define CAN_TX_MP_EXT_ID_RESET						0x4500
#define CAN_TX_MP_EXT_ID_MP_VERSION				0x4600

// sender: cpu, receiver: ssm
#define CAN_MID_CPU_SO0 0x040
#define CAN_MID_CPU_SO1 0x041

// sender: cpu, receiver: ssm
#define CAN_MID_CPU_FLASH_SIGNALS0 0x0B0
#define CAN_MID_CPU_FLASH_SIGNALS1 0x0B1

// sender: ssm, receiver: cpu
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS0 0x050
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS1 0x051
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS2 0x052
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS3 0x053
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS4 0x054
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS5 0x055
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS6 0x056
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS7 0x057

// sender: psm, receiver: cpu
#define CAN_MID_PSM_VOLT_MEASUREMENTS0 0x05A
#define CAN_MID_PSM_VOLT_MEASUREMENTS1 0x05B

//sender: cp, receiver: mp
#define CAN_MID_CPU_WORKING_LAMP_CHANGE_ACCEPT_TIME 0x0DA

// sender: cpu, pc, receiver: cpu, psm, ssm
#define CAN_MID_CPU_DATE_TIME 0x100

/* USER CODE END Private defines */

void MX_FDCAN1_Init(void);
void MX_FDCAN2_Init(void);

/* USER CODE BEGIN Prototypes */
typedef  struct _SCanSSMVoltCurMeas
{
	 union _USOVoltages
	{
		 struct _SfVoltages
		{
			uint8_t	fSOVoltage0						: 1;		// exists or doesn't exist
			uint8_t	fSOVoltage1						: 1;
			uint8_t	fSOVoltage2						: 1;
			uint8_t	fSOVoltage3						: 1;
			uint8_t	fSOVoltage4						: 1;
			uint8_t	fSOVoltage5						: 1;
			uint8_t	fSOVoltage6						: 1;
			uint8_t	fSOVoltage7						: 1;
			uint8_t	fSOVoltage8						: 1;
			uint8_t	fSOVoltage9						: 1;
			uint8_t	fSOVoltage10					: 1;
			uint8_t	fSOVoltage11					: 1;
			uint8_t	bReserved							: 4;
			
		} __attribute__((packed)) SfVoltages;
		
		uint16_t		sVoltages;
		
	} __attribute__((packed)) USOVoltages;

	 union _USOCurrentsL
	{
		 struct _SbCurrentsL
		{
			uint8_t	bSOCurrent0_2_L;
			uint8_t	bSOCurrent3_5_L;
			uint8_t	bSOCurrent6_8_L;
			uint8_t	bSOCurrent9_11_L;
			
		} __attribute__((packed)) SbCurrentsL;
		
		uint8_t	baCurrentsL[4];
		
	} __attribute__((packed)) USOCurrentsL;

	 union _USOCurrentsH
	{
		 struct _SbCurrentsH
		{
			uint8_t	bSOCurrent0_2_H					: 2;
			uint8_t	bSOCurrent3_5_H					: 2;
			uint8_t	bSOCurrent6_8_H					: 2;
			uint8_t	bSOCurrent9_11_H				: 2;
			
		} __attribute__((packed)) SbCurrentsH;
		
		uint8_t	bCurrentsH;
		
	} __attribute__((packed)) USOCurrentsH;

	uint8_t	bReserved2;
	
} __attribute__((packed)) tSCanSSMVoltCurMeas, *tpSCanSSMVoltCurMeas;

typedef  struct _SCanPSMVoltMeas
{
	uint8_t	bNetVoltageL;
	uint8_t	b24V1L;
	uint8_t	b24V2L;
	uint8_t	b5V1L;
	uint8_t	b5V2L;
	uint8_t	bNetVoltageH					: 2;
	uint8_t	b24V1H							: 2;
	uint8_t	b24V2H							: 2;
	uint8_t	b5V1H							: 2;
	uint8_t	b5V2H							: 2;
	uint8_t	fIsolatedVoltage				: 1;
	uint8_t	bReserved						: 5;
	uint8_t	bNetFrequency;
	
} __attribute__((packed)) tSCanPSMVoltMeas, *tpSCanPSMVoltMeas;

typedef  struct _SCanCpuSO
{
	 union _USOOn0
	{
		 struct _SOOn0
		{
			uint8_t	fSOOn0							: 1;		// on or off
			uint8_t	fSOOn1							: 1;
			uint8_t	fSOOn2							: 1;
			uint8_t	fSOOn3							: 1;
			uint8_t	fSOOn4							: 1;
			uint8_t	fSOOn5							: 1;
			uint8_t	fSOOn6							: 1;
			uint8_t	fSOOn7							: 1;
			uint8_t	fSOOn8							: 1;
			uint8_t	fSOOn9							: 1;
			uint8_t	fSOOn10							: 1;
			uint8_t	fSOOn11							: 1;
			uint8_t	fSOOn12							: 1;
			uint8_t	fSOOn13							: 1;
			uint8_t	fSOOn14							: 1;
			uint8_t	fSOOn15							: 1;
			uint8_t	fSOOn16							: 1;
			uint8_t	fSOOn17							: 1;
			uint8_t	fSOOn18							: 1;
			uint8_t	fSOOn19							: 1;
			uint8_t	fSOOn20							: 1;
			uint8_t	fSOOn21							: 1;
			uint8_t	fSOOn22							: 1;
			uint8_t	fSOOn23							: 1;
			uint8_t	fSOOn24							: 1;
			uint8_t	fSOOn25							: 1;
			uint8_t	fSOOn26							: 1;
			uint8_t	fSOOn27							: 1;
			uint8_t	fSOOn28							: 1;
			uint8_t	fSOOn29							: 1;
			uint8_t	fSOOn30							: 1;
			uint8_t	fSOOn31							: 1;
		} __attribute__((packed)) SOOn;
		
		uint32_t	lSOOn;
		
	} __attribute__((packed)) USOOn0;
	
	 union _USOOn1
	{
		 struct _SOOn1
		{
			uint8_t	fSOOn32							: 1;
			uint8_t	fSOOn33							: 1;
			uint8_t	fSOOn34							: 1;
			uint8_t	fSOOn35							: 1;
			uint8_t	fSOOn36							: 1;
			uint8_t	fSOOn37							: 1;
			uint8_t	fSOOn38							: 1;
			uint8_t	fSOOn39							: 1;
			uint8_t	fSOOn40							: 1;
			uint8_t	fSOOn41							: 1;
			uint8_t	fSOOn42							: 1;
			uint8_t	fSOOn43							: 1;
			uint8_t	fSOOn44							: 1;
			uint8_t	fSOOn45							: 1;
			uint8_t	fSOOn46							: 1;
			uint8_t	fSOOn47							: 1;
			uint16_t	sReserved;
		} __attribute__((packed)) SOOn;
		uint32_t	lSOOn;
		
	} __attribute__((packed)) USOOn1;
	
} __attribute__((packed)) tSCanCpuSO, *tpSCanCpuSO;

#define FDCAN_MAX_DATA_LEN	8

typedef struct
{
	FDCAN_HandleTypeDef	*hfdcan;
  FDCAN_RxHeaderTypeDef RxHeader;
  uint8_t baData[FDCAN_MAX_DATA_LEN];
	
} tSCanRxMsg, *tpSCanRxMsg;

typedef struct
{
	FDCAN_HandleTypeDef	*hfdcan;
  FDCAN_TxHeaderTypeDef TxHeader;
  uint8_t baData[FDCAN_MAX_DATA_LEN];
	
} tSCanTxMsg, *tpSCanTxMsg;


/* USER CODE END Private defines */

void MX_FDCAN1_Init(void);
void MX_FDCAN2_Init(void);

/* USER CODE BEGIN Prototypes */
extern void CANStart(FDCAN_HandleTypeDef *hfdcan);
extern void CANStop(FDCAN_HandleTypeDef *hfdcan);
extern void CANDeInit(FDCAN_HandleTypeDef *hfdcan);
extern void CANSendMessage(tpSCanTxMsg pxCANTxMessage);
extern void CANWaitTransmissionComplete(FDCAN_HandleTypeDef *hfdcan);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */

