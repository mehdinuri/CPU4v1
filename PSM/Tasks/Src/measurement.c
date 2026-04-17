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
#include "adc.h"
#include "storage.h"
#include "tim.h"
#include "utilities.h"
#include "i2c.h"
#include "gpio.h"
/* Private typedef -----------------------------------------------------------*/
typedef enum
{
	OFFSET_OPERATION_NONE = 0,
	OFFSET_OPERATION_SUM,
	OFFSET_OPERATION_SUBTRACT
	
} tEOffsetOperations;
/* Private define ------------------------------------------------------------*/
#define GET_MOST_SIG_BIT(x) ((unsigned char)((x & 0xFF00) >> 8))
#define GET_LEASTT_SIG_BIT(x) ((unsigned char)(x & 0x00FF))
	
#define COMM_LED_PERIOD 5

#define REG_VIN_OK_MIN 220 // 22.0V
#define REG_VIN_OK_MAX 260 // 26.0V

#define REG_VOUT_OK_MIN 47	//	4.7V
#define REG_VOUT_OK_MAX 67	//	6.7V

#define NET_LEVEL_OK_MIN 226	//	165V = 226 * 0.73029
#define NET_LEVEL_OK_MAX 363	//	265V = 363 * 0.73029

#define MAX_COMM_ERROR 999

#define CP_NET_VOLTAGE_COEFFICIENT (float)(0.73029f)
#define CP_REG_VIN_COEFFICIENT (float)(10.0f)
#define CP_REG_VOUT_COEFFICIENT (float)(10.0f)

#define MAX_PERIOD 40
#define DEFAULT_PERIOD 5
/* Private variables ---------------------------------------------------------*/
static tSPSMRuntime SPSMRuntime;
static tSPSMMeasurement SCANMeasurements;
static uint8_t bCommLedToggleCntr = 0;

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
static void MeasurementsInit(void)
{
	memset(&SPSMRuntime, 0, sizeof(SPSMRuntime));
}

static void TaskInit(void)
{
	bCommLedToggleCntr = 0;
	MeasurementsInit();
}

static void GridVoltMeasurementStart(void)
{
	ADC1StartDMA();
	Tim3Start();
}

static void GridFreqMeasurementStart(void)
{
	Tim2ICStartIT();
	Tim2OCStartIT();
}

static void RegulatorMeasurementsStart(void)
{
	ADC2StartDMA();
	ADC3StartDMA();
}

static void MeasurmentsSet(void)
{
	osMutexAcquire(VoltagesMutexHandle, osWaitForever);
	
	memset(&SCANMeasurements, 0, sizeof(SCANMeasurements));
	SCANMeasurements.b24V1Low = GET_LEASTT_SIG_BIT(SPSMRuntime.sRegVIn);
	SCANMeasurements.b24V1High = GET_MOST_SIG_BIT(SPSMRuntime.sRegVIn);
	SCANMeasurements.b24V2Low = GET_LEASTT_SIG_BIT(SPSMRuntime.sRegVIn);
	SCANMeasurements.b24V2High = GET_MOST_SIG_BIT(SPSMRuntime.sRegVIn);
	SCANMeasurements.b5V1Low = GET_LEASTT_SIG_BIT(SPSMRuntime.sRegVOut);
	SCANMeasurements.b5V1High = GET_MOST_SIG_BIT(SPSMRuntime.sRegVOut);
	SCANMeasurements.b5V2Low = GET_LEASTT_SIG_BIT(SPSMRuntime.sRegVOut);
	SCANMeasurements.b5V2High = GET_MOST_SIG_BIT(SPSMRuntime.sRegVOut);
	SCANMeasurements.bACLow = GET_LEASTT_SIG_BIT(SPSMRuntime.sNetVoltage);
	SCANMeasurements.bACHigh = GET_MOST_SIG_BIT(SPSMRuntime.sNetVoltage);
	SCANMeasurements.bIsolatedVoltage = GPIOCommOKStateGet();
	SCANMeasurements.bFrequency = SPSMRuntime.bNetFrequency;
	
	osMutexRelease(VoltagesMutexHandle);
}

static void MeasurmentsSend(void)
{
	CANTxRequest(&hfdcan1, FDCAN_STANDARD_ID, FDCAN_PSM_MEASUREMENT_STD_ID, FDCAN_DATA_FRAME, FDCAN_BRS_OFF, FDCAN_CLASSIC_CAN, (uint8_t*)&SCANMeasurements, sizeof(SCANMeasurements));	
}

static uint8_t OffsetWrite(void)
{
	return (StorageRequest(STORAGE_REQ_EEPROM_WRITE, I2C_E2PROM_ADD_OFFSET, (void *)&SPSMRuntime.SOffset, sizeof(SPSMRuntime.SOffset)));
}

static uint8_t OffsetRead(void)
{
	return (StorageRequest(STORAGE_REQ_EEPROM_READ, I2C_E2PROM_ADD_OFFSET, (void *)&SPSMRuntime.SOffset, sizeof(SPSMRuntime.SOffset)));
}

static void OffsetCheck(void)
{
	if (OffsetRead())
	{
		if(SPSMRuntime.SOffset.bOperation < OFFSET_OPERATION_SUBTRACT && SPSMRuntime.SOffset.bValue < 0xFF)
		{
			return;
		}
	}
	
	SPSMRuntime.SOffset.bOperation = OFFSET_OPERATION_NONE;
	SPSMRuntime.SOffset.bValue = 0;
	OffsetWrite();
}


static uint8_t FlashPeriodWrite(void)
{
	return StorageRequest(STORAGE_REQ_EEPROM_WRITE, I2C_E2PROM_ADD_PERIOD, (void *)&SPSMRuntime.lFlashPeriod, sizeof(SPSMRuntime.lFlashPeriod));
}

static uint8_t FlashPeriodRead(void)
{	
	return (StorageRequest(STORAGE_REQ_EEPROM_READ, I2C_E2PROM_ADD_PERIOD, (void *)&SPSMRuntime.lFlashPeriod, sizeof(SPSMRuntime.lFlashPeriod)));
}

static void FlashPeriodCheck(void)
{
	if (FlashPeriodRead())
	{
		if(SPSMRuntime.lFlashPeriod <= MAX_PERIOD && SPSMRuntime.lFlashPeriod >= DEFAULT_PERIOD)
		{
			SPSMRuntime.lFlashPeriod *= 10;
			return;
		}
	}
	
	SPSMRuntime.lFlashPeriod = DEFAULT_PERIOD;
	FlashPeriodWrite();
}

static uint8_t DCIsOK(void)
{
	if(	(SPSMRuntime.sRegVIn >= REG_VIN_OK_MIN) && (SPSMRuntime.sRegVIn <= REG_VIN_OK_MAX) &&
				((SPSMRuntime.sRegVOut >= REG_VOUT_OK_MIN) && (SPSMRuntime.sRegVOut <= REG_VOUT_OK_MAX)))
	{
		return TRUE;
	}
	
	return FALSE;
}

static uint8_t ACIsOK(void)
{
	return (SPSMRuntime.sNetVoltage >= NET_LEVEL_OK_MIN && SPSMRuntime.sNetVoltage <= NET_LEVEL_OK_MAX);
}

static uint8_t IsError(void)
{
	return (GPIOACOKStateGet() == GPIO_PIN_SET || GPIODCOKStateGet() == GPIO_PIN_SET);
}

static void LEDStatesSet(void)
{
	if(DCIsOK())
	{
		GPIODCOKLEDOn();
	}
	else
	{
		GPIODCOKLEDOff();
	}

	if(ACIsOK())
	{
		GPIOACOKLEDOn();
	}
	else
	{
		GPIOACOKLEDOff();
	}

	if(IsError())
	{
		GPIOErrorLEDOn();
	}
	else
	{
		GPIOErrorLEDOff();
	}
}

static uint8_t IsFlash(void)
{
	return SPSMRuntime.fFlashState;
}

static void FlashSync(void)
{
	tSFlashSync SSyncMsg;
	SPSMRuntime.sFlashStateCntr++;
	if(SPSMRuntime.sFlashStateCntr >= SPSMRuntime.lFlashPeriod)
	{
		SPSMRuntime.sFlashStateCntr = 0;
	}
	
	if(SPSMRuntime.sFlashStateCntr == 0)
	{
		memset(&SSyncMsg, 0, sizeof(SSyncMsg));
		SSyncMsg.bFlashSync = 0;
		CANTxRequest(&hfdcan1, FDCAN_STANDARD_ID, FDCAN_PSM_FLASH_SYNC_1_STD_ID, FDCAN_DATA_FRAME, FDCAN_BRS_OFF, FDCAN_CLASSIC_CAN, (uint8_t*)&SSyncMsg, sizeof(SSyncMsg));
		GPIOComLEDOn();
	}
	else if(SPSMRuntime.sFlashStateCntr == (SPSMRuntime.lFlashPeriod / 2))
	{
		memset(&SSyncMsg, 0, sizeof(SSyncMsg));
		SSyncMsg.bFlashSync = 1;
		CANTxRequest(&hfdcan1, FDCAN_STANDARD_ID, FDCAN_PSM_FLASH_SYNC_1_STD_ID, FDCAN_DATA_FRAME, FDCAN_BRS_OFF, FDCAN_CLASSIC_CAN, (uint8_t*)&SSyncMsg, sizeof(SSyncMsg));
		GPIOComLEDOff();
	}
}

static void NetVoltageOffsetAdd(void)
{
	switch(SPSMRuntime.SOffset.bOperation)
	{
		case OFFSET_OPERATION_SUM:
		{
			SPSMRuntime.fOffsetNetVoltage = SPSMRuntime.fNetVoltage + (float)(SPSMRuntime.SOffset.bValue);
		}
		break;
			
		case OFFSET_OPERATION_SUBTRACT:
		{
			SPSMRuntime.fOffsetNetVoltage = SPSMRuntime.fNetVoltage - (float)(SPSMRuntime.SOffset.bValue);
		}
		break;
		
		default:
		{
			SPSMRuntime.fOffsetNetVoltage = SPSMRuntime.fNetVoltage;
		}
		break;
	}
}

static void CPNetVoltageSet(void)
{
	SPSMRuntime.fCPNetVoltage = (float)(SPSMRuntime.fOffsetNetVoltage / CP_NET_VOLTAGE_COEFFICIENT);
	SPSMRuntime.sNetVoltage = (uint16_t)(SPSMRuntime.fCPNetVoltage);
}

static void CPRegVoltagesSet(void)
{
	float fRegVIn = (float)(SPSMRuntime.fRegVIn * CP_REG_VIN_COEFFICIENT);
	float fRegVOut = (float)(SPSMRuntime.fRegVOut * CP_REG_VOUT_COEFFICIENT);
	
	SPSMRuntime.sRegVIn = (uint16_t)(fRegVIn);
	SPSMRuntime.sRegVOut = (uint16_t)(fRegVOut);
}

static void NetVoltageSet(void)
{	
	NetVoltageOffsetAdd();
	
	CPNetVoltageSet();
}

static void RegVoltagesSet(void)
{
	CPRegVoltagesSet();
}

/* Public application code --------------------------------------------------*/
void MeasurementPeriodSet(uint8_t bPeriod)
{
	if(SPSMRuntime.lFlashPeriod != bPeriod)
	{
		SPSMRuntime.lFlashPeriod = bPeriod;
		
		FlashPeriodWrite();
		
		SPSMRuntime.lFlashPeriod *= 10;
	}
	
	MeasurementFlashStateCntrSet(0);
}

void MeasurementFlashStateSet(uint8_t fState)
{
	SPSMRuntime.fFlashState = fState;
}

void MeasurementFlashStateCntrSet(uint16_t Cntr)
{
	SPSMRuntime.sFlashCntr = Cntr;
}

void MeasurementOffsetSet(uint8_t bOp, uint8_t bVal)
{
	if(SPSMRuntime.SOffset.bOperation != bOp || SPSMRuntime.SOffset.bValue != bVal)
	{
		SPSMRuntime.SOffset.bOperation = bOp;
		SPSMRuntime.SOffset.bValue = bVal;
		
		OffsetWrite();
	}
}

void MeasurementNetVoltageSet(float fVolt)
{
	SPSMRuntime.fNetVoltage = fVolt;
}

void MeasurementRegVInSet(float fVIn)
{
	SPSMRuntime.fRegVIn = fVIn;
}

void MeasurementRegVOutSet(float fVOut)
{
	SPSMRuntime.fRegVOut = fVOut;
}

void MeasurementNetFrequencySet(uint8_t bFreq)
{
	SPSMRuntime.bNetFrequency = bFreq;
}

void MeasurementCommCheck(void)
{
	if(SPSMRuntime.sCommErrorCntr++ > MAX_COMM_ERROR)
	{
		MeasurementFlashStateSet(TRUE);
		MeasurementCommCntrReset();
	}
}

void MeasurementCommCntrReset(void)
{
	SPSMRuntime.sCommErrorCntr = 0;
}

void MeasurementThreadFlagSet(void)
{
	osThreadFlagsSet(MeasurementTaskHandle, THREAD_FLAGS_MEASUREMENT_DONE);
}

/* Public application code --------------------------------------------------*/

/* USER CODE BEGIN Header_vMeasurementTask */
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
	
	FlashPeriodCheck();
	OffsetCheck();
	
	GridFreqMeasurementStart();
	GridVoltMeasurementStart();
	
	RegulatorMeasurementsStart();
	
	/* Infinite loop */
	while(pdTRUE)
	{
		osThreadFlagsWait(THREAD_FLAGS_MEASUREMENT_DONE, osFlagsWaitAll, osWaitForever);
		
		NetVoltageSet();
		
		RegVoltagesSet();
		
		LEDStatesSet();
		
		if(IsFlash())
		{
			FlashSync();
		}
		else
		{
			SPSMRuntime.sFlashCntr = 0;
			MeasurmentsSet();
			MeasurmentsSend();
			
			if(bCommLedToggleCntr++ == COMM_LED_PERIOD)
			{
				bCommLedToggleCntr = 0;
				GPIOComLEDToggle();
			}
		}
		
		MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_MEASUREMENT_TASK_ACTIIVE);
	}
  /* USER CODE END vMeasurementTask */
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
