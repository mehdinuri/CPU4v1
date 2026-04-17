/**
  ******************************************************************************
  * File Name          : signal_monitor.c
  * Description        : Code for monitoring signals from CAN
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "measurement.h"
#include "fdcan.h"
#include "can_msg_sender.h"
#include "can_msg_parser.h"
#include "adc.h"
#include "storage.h"
#include "tim.h"
#include "utilities.h"
#include "gpio.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define COMM_LED_PERIOD 5

/* Private variables ---------------------------------------------------------*/
static uint8_t bCommLedToggleCntr = 0;

static tUOutputs USignalOutputStates;
static tSSignalOutputStateCntr SaSignalOutputStateCntrs[SIGNAL_OUTPUTS_PER_SSM];

static tSVoltageCurrent_t SVoltageCurrents;
static float faSGCurrents[SIGNAL_GROUPS_PER_SSM];

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
static void SignalOutputStatesInit(void)
{
	memset(&USignalOutputStates, 0, sizeof(USignalOutputStates));
}

static void SignalOutputStateCntrsInit(void)
{
	memset(SaSignalOutputStateCntrs, 0, sizeof(SaSignalOutputStateCntrs));
}

static void TaskInit(void)
{
	bCommLedToggleCntr = 0;
	
	memset(&SVoltageCurrents, 0, sizeof(SVoltageCurrents));
	memset(faSGCurrents, 0, sizeof(faSGCurrents));
	
	SignalOutputStatesInit();
	SignalOutputStateCntrsInit();
}

static void SignalOutputStatesTimerStart(void)
{
	Tim4OCStartIT();
}

static void SGCurrentMeasurementStart(void)
{
	ADCSGCurrentMeasurementStart();
	Tim3Start();
}

static void GridFreqMeasurementStart(void)
{
	Tim2ICStartIT();
	Tim2OCStartIT();
}

static void SignalOutputStatesCheck(void)
{
	osMutexAcquire(OutputStatesMutexHandle, osWaitForever);

	if(SaSignalOutputStateCntrs[0].bOnCntr > SaSignalOutputStateCntrs[0].bOffCntr)
	{
		USignalOutputStates.SBitFlags16.fBit0 = TRUE;
	}
	else
	{
		USignalOutputStates.SBitFlags16.fBit0 = FALSE;
	}
	
	if(SaSignalOutputStateCntrs[1].bOnCntr > SaSignalOutputStateCntrs[1].bOffCntr)
	{
		USignalOutputStates.SBitFlags16.fBit1 = TRUE;
	}
	else
	{
		USignalOutputStates.SBitFlags16.fBit1 = FALSE;
	}
	
	if(SaSignalOutputStateCntrs[2].bOnCntr > SaSignalOutputStateCntrs[2].bOffCntr)
	{
		USignalOutputStates.SBitFlags16.fBit2 = TRUE;
	}
	else
	{
		USignalOutputStates.SBitFlags16.fBit2 = FALSE;
	}
	
	if(SaSignalOutputStateCntrs[3].bOnCntr > SaSignalOutputStateCntrs[3].bOffCntr)
	{
		USignalOutputStates.SBitFlags16.fBit3 = TRUE;
	}
	else
	{
		USignalOutputStates.SBitFlags16.fBit3 = FALSE;
	}
	
	if(SaSignalOutputStateCntrs[4].bOnCntr > SaSignalOutputStateCntrs[4].bOffCntr)
	{
		USignalOutputStates.SBitFlags16.fBit4 = TRUE;
	}
	else
	{
		USignalOutputStates.SBitFlags16.fBit4 = FALSE;
	}
	
	if(SaSignalOutputStateCntrs[5].bOnCntr > SaSignalOutputStateCntrs[5].bOffCntr)
	{
		USignalOutputStates.SBitFlags16.fBit5 = TRUE;
	}
	else
	{
		USignalOutputStates.SBitFlags16.fBit5 = FALSE;
	}
	
	if(SaSignalOutputStateCntrs[6].bOnCntr > SaSignalOutputStateCntrs[6].bOffCntr)
	{
		USignalOutputStates.SBitFlags16.fBit6 = TRUE;
	}
	else
	{
		USignalOutputStates.SBitFlags16.fBit6 = FALSE;
	}
	
	if(SaSignalOutputStateCntrs[7].bOnCntr > SaSignalOutputStateCntrs[7].bOffCntr)
	{
		USignalOutputStates.SBitFlags16.fBit7 = TRUE;
	}
	else
	{
		USignalOutputStates.SBitFlags16.fBit7 = FALSE;
	}
	
	if(SaSignalOutputStateCntrs[8].bOnCntr > SaSignalOutputStateCntrs[8].bOffCntr)
	{
		USignalOutputStates.SBitFlags16.fBit8 = TRUE;
	}
	else
	{
		USignalOutputStates.SBitFlags16.fBit8 = FALSE;
	}
	
	if(SaSignalOutputStateCntrs[9].bOnCntr > SaSignalOutputStateCntrs[9].bOffCntr)
	{
		USignalOutputStates.SBitFlags16.fBit9 = TRUE;
	}
	else
	{
		USignalOutputStates.SBitFlags16.fBit9 = FALSE;
	}
	
	if(SaSignalOutputStateCntrs[10].bOnCntr > SaSignalOutputStateCntrs[10].bOffCntr)
	{
		USignalOutputStates.SBitFlags16.fBit10 = TRUE;
	}
	else
	{
		USignalOutputStates.SBitFlags16.fBit10 = FALSE;
	}
	
	if(SaSignalOutputStateCntrs[11].bOnCntr > SaSignalOutputStateCntrs[11].bOffCntr)
	{
		USignalOutputStates.SBitFlags16.fBit11 = TRUE;
	}
	else
	{
		USignalOutputStates.SBitFlags16.fBit11 = FALSE;
	}
	
	SignalOutputStateCntrsInit();
		
	osMutexRelease(OutputStatesMutexHandle);
}


static void SignalGroupSwitchStatesSet(void)
{
	if(CANFlashStatusGet())
	{
		if(CANFlashSyncStatusGet())
		{
			GPIOSetR1(CANSignalOutputFlashStateGet(0, 0));
			GPIOSetY1(CANSignalOutputFlashStateGet(0, 1));
			GPIOSetG1(CANSignalOutputFlashStateGet(0, 2));
			GPIOSetR2(CANSignalOutputFlashStateGet(1, 0));
			GPIOSetY2(CANSignalOutputFlashStateGet(1, 1));
			GPIOSetG2(CANSignalOutputFlashStateGet(1, 2));
			GPIOSetR3(CANSignalOutputFlashStateGet(2, 0));
			GPIOSetY3(CANSignalOutputFlashStateGet(2, 1));
			GPIOSetG3(CANSignalOutputFlashStateGet(2, 2));
			GPIOSetR4(CANSignalOutputFlashStateGet(3, 0));
			GPIOSetY4(CANSignalOutputFlashStateGet(3, 1));
			GPIOSetG4(CANSignalOutputFlashStateGet(3, 2));
		}
		else
		{
			GPIOSetR1(GPIO_PIN_RESET);
			GPIOSetY1(GPIO_PIN_RESET);
			GPIOSetG1(GPIO_PIN_RESET);
			GPIOSetR2(GPIO_PIN_RESET);
			GPIOSetY2(GPIO_PIN_RESET);
			GPIOSetG2(GPIO_PIN_RESET);
			GPIOSetR3(GPIO_PIN_RESET);
			GPIOSetY3(GPIO_PIN_RESET);
			GPIOSetG3(GPIO_PIN_RESET);
			GPIOSetR4(GPIO_PIN_RESET);
			GPIOSetY4(GPIO_PIN_RESET);
			GPIOSetG4(GPIO_PIN_RESET);
		}
	}
	else
	{
		GPIOSetR1(CANSignalOutputStateGet(0, 0));
		GPIOSetY1(CANSignalOutputStateGet(0, 1));
		GPIOSetG1(CANSignalOutputStateGet(0, 2));
		GPIOSetR2(CANSignalOutputStateGet(1, 0));
		GPIOSetY2(CANSignalOutputStateGet(1, 1));
		GPIOSetG2(CANSignalOutputStateGet(1, 2));
		GPIOSetR3(CANSignalOutputStateGet(2, 0));
		GPIOSetY3(CANSignalOutputStateGet(2, 1));
		GPIOSetG3(CANSignalOutputStateGet(2, 2));
		GPIOSetR4(CANSignalOutputStateGet(3, 0));
		GPIOSetY4(CANSignalOutputStateGet(3, 1));
		GPIOSetG4(CANSignalOutputStateGet(3, 2));
	}
}

static void VoltageCurrentSet(void)
{
	osMutexAcquire(OutputStatesMutexHandle, osWaitForever);
	
	SVoltageCurrents.UVoltages.SVoltageBits.fBit0 = USignalOutputStates.SBitFlags16.fBit0;
	SVoltageCurrents.UVoltages.SVoltageBits.fBit1 = USignalOutputStates.SBitFlags16.fBit1;
	SVoltageCurrents.UVoltages.SVoltageBits.fBit2 = USignalOutputStates.SBitFlags16.fBit2;
	SVoltageCurrents.UVoltages.SVoltageBits.fBit3 = USignalOutputStates.SBitFlags16.fBit3;
	SVoltageCurrents.UVoltages.SVoltageBits.fBit4 = USignalOutputStates.SBitFlags16.fBit4;
	SVoltageCurrents.UVoltages.SVoltageBits.fBit5 = USignalOutputStates.SBitFlags16.fBit5;
	SVoltageCurrents.UVoltages.SVoltageBits.fBit6 = USignalOutputStates.SBitFlags16.fBit6;
	SVoltageCurrents.UVoltages.SVoltageBits.fBit7 = USignalOutputStates.SBitFlags16.fBit7;
	SVoltageCurrents.UVoltages.SVoltageBits.fBit8 = USignalOutputStates.SBitFlags16.fBit8;
	SVoltageCurrents.UVoltages.SVoltageBits.fBit9 = USignalOutputStates.SBitFlags16.fBit9;
	SVoltageCurrents.UVoltages.SVoltageBits.fBit10 = USignalOutputStates.SBitFlags16.fBit10;
	SVoltageCurrents.UVoltages.SVoltageBits.fBit11 = USignalOutputStates.SBitFlags16.fBit11;
	
	osMutexRelease(OutputStatesMutexHandle);
	
	osMutexAcquire(CurrentMutexHandle, osWaitForever);
	
	SVoltageCurrents.USOCurrentsLow.SSOCurrentsLow.bSOCurLow0_2 = (uint8_t)((uint16_t)faSGCurrents[0]);
	SVoltageCurrents.USOCurrentsHighBits.SSOCurrentsHighBits.bSOCurHighBits0_2 = (uint8_t)((uint16_t)faSGCurrents[0] >> NUM_BITS_IN_BYTE);
	SVoltageCurrents.USOCurrentsLow.SSOCurrentsLow.bSOCurLow3_5 = (uint8_t)((uint16_t)faSGCurrents[1]);
	SVoltageCurrents.USOCurrentsHighBits.SSOCurrentsHighBits.bSOCurHighBits3_5 = (uint8_t)((uint16_t)faSGCurrents[1] >> NUM_BITS_IN_BYTE);
	SVoltageCurrents.USOCurrentsLow.SSOCurrentsLow.bSOCurLow6_8 = (uint8_t)((uint16_t)faSGCurrents[2]);
	SVoltageCurrents.USOCurrentsHighBits.SSOCurrentsHighBits.bSOCurHighBits6_8 = (uint8_t)((uint16_t)faSGCurrents[2] >> NUM_BITS_IN_BYTE);
	SVoltageCurrents.USOCurrentsLow.SSOCurrentsLow.bSOCurLow9_11 = (uint8_t)((uint16_t)faSGCurrents[3]);
	SVoltageCurrents.USOCurrentsHighBits.SSOCurrentsHighBits.bSOCurHighBits9_11 = (uint8_t)((uint16_t)faSGCurrents[3] >> NUM_BITS_IN_BYTE);
	
	osMutexRelease(CurrentMutexHandle);
}

static void VoltageCurrentSend(void)
{
	CANTxRequest(&hfdcan1, FDCAN_STANDARD_ID, (FDCAN_SSM_VOLTAGE_CURRENT_1_STD_ID + GPIOGetCardID()), FDCAN_DATA_FRAME, FDCAN_BRS_OFF, FDCAN_CLASSIC_CAN, (uint8_t*)&SVoltageCurrents, sizeof(SVoltageCurrents));
}

/* Public application code --------------------------------------------------*/
void MeasurementSGCurrentsSet(float *pfCurrents)
{
	osMutexAcquire(CurrentMutexHandle, osWaitForever);
	
	memcpy(faSGCurrents, pfCurrents, sizeof(faSGCurrents));
	
	osMutexRelease(CurrentMutexHandle);
}

void MeasurementSGStatesCntrsSet(void)
{
	osMutexAcquire(OutputStatesMutexHandle, osWaitForever);
	
	if(!GPIOGetR1_V())
	{
		SaSignalOutputStateCntrs[0].bOnCntr++;
	}
	else
	{
		SaSignalOutputStateCntrs[0].bOffCntr++;
	}
	
	if(!GPIOGetY1_V())
	{
		SaSignalOutputStateCntrs[1].bOnCntr++;
	}
	else
	{
		SaSignalOutputStateCntrs[1].bOffCntr++;
	}
	
	if(!GPIOGetG1_V())
	{
		SaSignalOutputStateCntrs[2].bOnCntr++;
	}
	else
	{
		SaSignalOutputStateCntrs[2].bOffCntr++;
	}
	
	if(!GPIOGetR2_V())
	{
		SaSignalOutputStateCntrs[3].bOnCntr++;
	}
	else
	{
		SaSignalOutputStateCntrs[3].bOffCntr++;
	}
	
	if(!GPIOGetY2_V())
	{
		SaSignalOutputStateCntrs[4].bOnCntr++;
	}
	else
	{
		SaSignalOutputStateCntrs[4].bOffCntr++;
	}
	
	if(!GPIOGetG2_V())
	{
		SaSignalOutputStateCntrs[5].bOnCntr++;
	}
	else
	{
		SaSignalOutputStateCntrs[5].bOffCntr++;
	}
	
	if(!GPIOGetR3_V())
	{
		SaSignalOutputStateCntrs[6].bOnCntr++;
	}
	else
	{
		SaSignalOutputStateCntrs[6].bOffCntr++;
	}
	
	if(!GPIOGetY3_V())
	{
		SaSignalOutputStateCntrs[7].bOnCntr++;
	}
	else
	{
		SaSignalOutputStateCntrs[7].bOffCntr++;
	}
	
	if(!GPIOGetG3_V())
	{
		SaSignalOutputStateCntrs[8].bOnCntr++;
	}
	else
	{
		SaSignalOutputStateCntrs[8].bOffCntr++;
	}
	
	if(!GPIOGetR4_V())
	{
		SaSignalOutputStateCntrs[9].bOnCntr++;
	}
	else
	{
		SaSignalOutputStateCntrs[9].bOffCntr++;
	}
	
	if(!GPIOGetY4_V())
	{
		SaSignalOutputStateCntrs[10].bOnCntr++;
	}
	else
	{
		SaSignalOutputStateCntrs[10].bOffCntr++;
	}
	
	if(!GPIOGetG4_V())
	{
		SaSignalOutputStateCntrs[11].bOnCntr++;
	}
	else
	{
		SaSignalOutputStateCntrs[11].bOffCntr++;
	}
				
	osMutexRelease(OutputStatesMutexHandle);
}

void MeasurementThreadFlagSet(void)
{
	osThreadFlagsSet(MeasurementTaskHandle, THREAD_FLAGS_MEASUREMENT_DONE);
}

/* Public application code --------------------------------------------------*/

/* USER CODE BEGIN Header_vMeasurementTask */
uint32_t lMeasurementStart = 0;
uint32_t lMeasurementFinish = 0;
uint32_t lMeasurementDuration = 0;
/**
* @brief Function implementing the xSignalMonitorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vMeasurementTask */
void MeasurementTaskFunc(void * argument)
{
  /* USER CODE BEGIN vMeasurementTask */	
	TaskInit();
	
	CANSignalOutputFlashConfigCheck();
	
	GridFreqMeasurementStart();
	lMeasurementStart = HAL_GetTick();
	
	SignalOutputStatesTimerStart();
	SGCurrentMeasurementStart();

	/* Infinite loop */
	while(pdTRUE)
	{
		osThreadFlagsWait(THREAD_FLAGS_MEASUREMENT_DONE, osFlagsWaitAll, osWaitForever);
		
		lMeasurementFinish = HAL_GetTick();
		lMeasurementDuration = lMeasurementFinish - lMeasurementStart;
		lMeasurementStart = lMeasurementFinish;
		
		SignalGroupSwitchStatesSet();
		SignalOutputStatesCheck();
		
		VoltageCurrentSet();
		VoltageCurrentSend();
		
		SignalOutputStatesInit();
		
		if(bCommLedToggleCntr++ == COMM_LED_PERIOD)
		{
			bCommLedToggleCntr = 0;
			GPIOComLEDToggle();
		}
		
		MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_MEASUREMENT_TASK_ACTIIVE);
	}
  /* USER CODE END vMeasurementTask */
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
