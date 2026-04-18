/**
  ******************************************************************************
  * @file           : measurement.h
  * @brief          : Header for signal_monitor.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SIGNAL_MONITOR_H__
#define __SIGNAL_MONITOR_H__

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "Domain/Measurement.h"

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/
typedef struct _tSPSMRuntime
{
    tSMeasurementOffset SOffset;
	uint8_t fFlashState;
	uint32_t lFlashPeriod;
	uint16_t sFlashCntr;
	uint16_t sFlashStateCntr;
	uint16_t sCommErrorCntr;
	uint8_t bNetFrequency;
	
    float fNetVoltage;
    float fRegVIn;
    float fRegVOut;

    uint16_t sNetVoltage;
	uint16_t sRegVIn;
	uint16_t sRegVOut;
	
} tSPSMRuntime, *tpSPSMRuntime;

/* Public function prototypes -----------------------------------------------*/
extern void MeasurementNetVoltageSet(float fVolt);
extern void MeasurementRegVInSet(float fVIn);
extern void MeasurementRegVOutSet(float fVOut);
extern void MeasurementNetFrequencySet(uint8_t bFreq);
extern void MeasurementPeriodSet(uint8_t bPeriod);
extern void MeasurementFlashStateSet(uint8_t fState);
extern void MeasurementFlashStateCntrSet(uint16_t sCntr);
extern void MeasurementOffsetSet(uint8_t bOp, uint8_t bVal);
extern void MeasurementCommCheck(void);
extern void MeasurementCommCntrReset(void);
extern void MeasurementThreadFlagSet(void);

#endif /* __SIGNAL_MONITOR_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****__SIGNAL_MONITOR_H__ OF FILE****/
