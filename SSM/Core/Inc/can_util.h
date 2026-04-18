/**
	******************************************************************************
	* @file					 : can_util.h
	* @brief					: Header for can_util.c file.
	*									 This file contains the common defines of the application.
	******************************************************************************
	*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_UTIL_H__
#define __CAN_UTIL_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "utilities.h"
#include "fdcan.h"

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define ------------------------------------------------------------*/
#define SIGNAL_OUTPUTS_PER_SIGNAL_GROUP (uint8_t) 3
#define SIGNAL_GROUPS_PER_SSX (uint8_t) 4
#define SIGNAL_OUTPUTS_PER_SSX (uint8_t) (SIGNAL_OUTPUTS_PER_SIGNAL_GROUP * SIGNAL_GROUPS_PER_SSX) // 12

#define SSX_GROUPS_MAX (uint8_t) 2
#define SSX_MODULES_PER_SSX_GROUP (uint8_t) 4
#define SIGNAL_GROUPS_PER_SSX_GROUP (uint8_t) (SIGNAL_GROUPS_PER_SSX * SSX_MODULES_PER_SSX_GROUP) // 16
#define SIGNAL_OUTPUTS_PER_SSX_GROUP (uint8_t) (SIGNAL_GROUPS_PER_SSX_GROUP * SIGNAL_OUTPUTS_PER_SIGNAL_GROUP) // 48

#define FLASH_SIGNALS_CONFIG_ALREADY_WRITTEN (uint32_t)(0x12345678)
typedef enum
{
	ID_TYPE_NONE = 0,
	ID_TYPE_STANDARD,
	ID_TYPE_EXTENDED
	
} IdTypes_e;

typedef enum
{
	FORMAT_TYPE_NONE = 0,
	FORMAT_TYPE_CLASSIC,
	FORMAT_TYPE_FD
	
} FormatTypes_e;

typedef enum
{
	HANDLER_TYPE_NONE = 0,
	HANDLER_TYPE_CAN1,
	HANDLER_TYPE_CAN2
	
} HandlerTypes_e;

typedef enum
{
	OUTPUT_SWITCH_OFF = 0,
	OUTPUT_SWITCH_ON
	
} OutputSwitches_e;

typedef enum
{
	CPX_SIGNAL_OUTPUTS_1_STD_ID = 0x040,
	CPX_SIGNAL_OUTPUTS_2_STD_ID
	
} CPXSignalOutputs_e;

typedef enum
{
	CPX_FLASH_SIGNALS_1_STD_ID = 0x0B0,
	CPX_FLASH_SIGNALS_2_STD_ID
	
} CPXFlashSignals_e;

typedef enum
{
	PSX_FLASH_SYNC_1_STD_ID = 0x069,
	PSX_FLASH_SYNC_2_STD_ID
	
} PSXFlashSync_e;

typedef enum
{
	SSX_1_VOLT_AND_CUR_MEASURE_STD_ID = 0x050,
	SSX_2_VOLT_AND_CUR_MEASURE_STD_ID,
	SSX_3_VOLT_AND_CUR_MEASURE_STD_ID,
	SSX_4_VOLT_AND_CUR_MEASURE_STD_ID,
	SSX_5_VOLT_AND_CUR_MEASURE_STD_ID,
	SSX_6_VOLT_AND_CUR_MEASURE_STD_ID,
	SSX_7_VOLT_AND_CUR_MEASURE_STD_ID,
	SSX_8_VOLT_AND_CUR_MEASURE_STD_ID
	
} SSXVoltAndCurMeasurementStdIds_e;

typedef enum xSSX_MODULES
{
	SSX_MODULE_FIRST = 0,
	SSX_MODULE_1 = SSX_MODULE_FIRST,
	SSX_MODULE_2,
	SSX_MODULE_3,
	SSX_MODULE_4,
	SSX_MODULE_5,
	SSX_MODULE_6,
	SSX_MODULE_7,
	SSX_MODULE_8,
	SSX_MODULE_LAST = SSX_MODULE_8,
	SSX_MODULES_MAX = SSX_MODULE_LAST - SSX_MODULE_FIRST + 1

} CANUtlSSXModules_e;

#define SIGNAL_GROUPS_MAX (uint8_t) (SSX_MODULES_MAX * SIGNAL_GROUPS_PER_SSX)
#define SIGNAL_OUTPUTS_MAX (uint8_t) (SIGNAL_GROUPS_MAX * SIGNAL_OUTPUTS_PER_SIGNAL_GROUP)

typedef enum xSIGNAL_OUTPUT_TYPES
{
	SIGNAL_OUTPUT_TYPE_NONE = 0,
	SIGNAL_OUTPUT_TYPE_RED = 1,
	SIGNAL_OUTPUT_TYPE_YELLOW = 2,
	SIGNAL_OUTPUT_TYPE_GREEN = 4

} CANUtlSignalOutputType_e;

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/
typedef struct xFDCAN_RX_REQUEST
{
	uint8_t	ucStatus;
	tSFDCANRxMsg xRxMessage;
	
} FDCANUtlRxRequest_t, *pFDCANUtlRxRequest_t;

typedef struct xFDCAN_TX_REQUEST
{
	uint8_t	ucStatus;
	tSFDCANTxMsg xTxMessage;
	
} FDCANUtlTxRequest_t, *pFDCANUtlTxRequest_t;

typedef __packed struct xBIT_FLAGS_16
{
	uint8_t ucBit0 : 1;
	uint8_t ucBit1 : 1;
	uint8_t ucBit2 : 1;
	uint8_t ucBit3 : 1;
	uint8_t ucBit4 : 1;
	uint8_t ucBit5 : 1;
	uint8_t ucBit6 : 1;
	uint8_t ucBit7 : 1;
	uint8_t ucBit8 : 1;
	uint8_t ucBit9 : 1;
	uint8_t ucBit10 : 1;
	uint8_t ucBit11 : 1;
	uint8_t ucBit12: 1;
  uint8_t ucBit13: 1;
  uint8_t ucBit14: 1;
  uint8_t ucBit15: 1;

} BitFlags16_t, *pBitFlags16_t;

typedef __packed struct xBIT_FLAGS_32
{
	uint8_t ucBit0 : 1;
	uint8_t ucBit1 : 1;
	uint8_t ucBit2 : 1;
	uint8_t ucBit3 : 1;
	uint8_t ucBit4 : 1;
	uint8_t ucBit5 : 1;
	uint8_t ucBit6 : 1;
	uint8_t ucBit7 : 1;
	uint8_t ucBit8 : 1;
	uint8_t ucBit9 : 1;
	uint8_t ucBit10 : 1;
	uint8_t ucBit11 : 1;
	uint8_t ucBit12: 1;
  uint8_t ucBit13: 1;
  uint8_t ucBit14: 1;
  uint8_t ucBit15: 1;
	uint8_t ucBit16: 1;
	uint8_t ucBit17: 1;
	uint8_t ucBit18: 1;
	uint8_t ucBit19: 1;
	uint8_t ucBit20: 1;
	uint8_t ucBit21: 1;
	uint8_t ucBit22: 1;
	uint8_t ucBit23: 1;
	uint8_t ucBit24: 1;
	uint8_t ucBit25: 1;
	uint8_t ucBit26: 1;
	uint8_t ucBit27: 1;
	uint8_t ucBit28: 1;
	uint8_t ucBit29: 1;
	uint8_t ucBit30: 1;
	uint8_t ucBit31: 1;

} BitFlags32_t, *pBitFlags32_t;

typedef __packed struct xSSM_VOLT_CUR_MEASUREMENTS
{
	__packed union 
	{
		BitFlags16_t xVoltageBits;
		uint16_t usVoltages;

	} xuSOVoltages;

	__packed union 
	{
		__packed struct
		{
			uint8_t ucSOCurLow0_2;
			uint8_t ucSOCurLow3_5;
			uint8_t ucSOCurLow6_8;
			uint8_t ucSOCurLow9_11;

		} xCurrentsLow;

		uint8_t ucaCurrentsLow[4];

	} xuSOCurrentsLow;

	__packed union 
	{
		__packed struct
		{
			uint8_t ucSOCurHighBits0_2 : 2;
			uint8_t ucSOCurHighBits3_5 : 2;
			uint8_t ucSOCurHighBits6_8 : 2;
			uint8_t ucSOCurHighBits9_11 : 2;

		} xCurrentsHighBits;

		uint8_t ucCurrentsHigh;

	} xuSOCurrentsHigh;

	uint8_t ucReserved2;

} SSXVoltCurMeasurements_t, *pSSXVoltCurMeasurements_t;

typedef __packed struct xSIGNAL_OUTPUT
{
	uint8_t ucOnCntr;
	uint8_t ucOffCntr;
	
	uint8_t ucIsActive;
	uint8_t ucIsOn;
	uint8_t ucIsFlashing;
	
} SignalOutput_t, *pSignalOutput_t;

typedef __packed struct xSIGNAL_GROUP
{
	SignalOutput_t xaOutputs[SIGNAL_OUTPUTS_PER_SIGNAL_GROUP];
	uint16_t usCurrent;
	
} OutputGroupt_t, *pOutputGroupt_t;

typedef __packed union xOUTPUTS
{
	BitFlags16_t xFlags;
	uint16_t usData;
	uint8_t ucaData[2];
	
} Outputs_ut, *pOutputs_ut;

typedef __packed struct xSIGNAL_OUTPUT_STATE_COUNTER
{
	uint8_t ucOnCntr;
	uint8_t ucOffCntr;
	
} SignalOutputStateCntr_t, *pSignalOutputStateCntr_t;

typedef __packed struct xFLASH_SIGNALS_CONFIG
{
	uint32_t ulIsWritten;
	uint16_t usFlashBits;
	
} FlashSignalsConfig_t, *pFlashSignalsConfig_t;


/* Public function prototypes -----------------------------------------------*/
extern uint8_t ucCANUtlGetFlashStatus(void);
extern uint8_t ucCANUtlGetFlashSyncStatus(void);
extern void vCANUtlReadFlashOutputs(void);
extern GPIO_PinState xCANUtlGetOutputState(uint8_t ucGroupIdx, uint8_t ucOutputIdx);
extern GPIO_PinState xCANUtlGetOutputFlashState(uint8_t ucGroupIdx, uint8_t ucOutputIdx);
extern void vCANUtlProcessStdMsgs(uint32_t ulId, uint8_t *pucData, uint8_t ucLen);
extern void vCANUtlProcessExtMsgs(uint32_t ulId, uint8_t *pucData, uint8_t ucLen);

#endif /* __CAN_UTIL_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
