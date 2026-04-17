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
#include "cmsis_os2.h"

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/


/* Public function prototypes -----------------------------------------------*/
extern void MeasurementSGStatesCntrsSet(void);
extern void MeasurementSGCurrentsSet(float *pfCurrents);
extern void MeasurementThreadFlagSet(void);

#endif /* __SIGNAL_MONITOR_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****__SIGNAL_MONITOR_H__ OF FILE****/
