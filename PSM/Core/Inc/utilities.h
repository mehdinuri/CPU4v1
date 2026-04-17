/**
  ******************************************************************************
  * @file           : utilities.h
  * @brief          : Header for utilities.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UTILITIES_H__
#define __UTILITIES_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "cmsis_os.h"

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define ------------------------------------------------------------*/
#define FALSE 0
#define TRUE 1

#define OFF 0
#define ON 1

#define LOW 0
#define HIGH 1

#define NUM_BITS_IN_BYTE 8

#define ONE_MILLI_SECOND 1
#define ONE_DECI_SECOND	10
#define ONE_CENTI_SECOND 100
#define ONE_SECOND 1000

#define FLAG_BIT_0 (1UL << 0)
#define FLAG_BIT_1 (1UL << 1)
#define FLAG_BIT_2 (1UL << 2)
#define FLAG_BIT_3 (1UL << 3)
#define FLAG_BIT_4 (1UL << 4)
#define FLAG_BIT_5 (1UL << 5)
#define FLAG_BIT_6 (1UL << 6)
#define FLAG_BIT_7 (1UL << 7)
#define FLAG_BIT_8 (1UL << 8)
#define FLAG_BIT_9 (1UL << 9)
#define FLAG_BIT_10 (1UL << 10)
#define FLAG_BIT_11 (1UL << 11)
#define FLAG_BIT_12 (1UL << 12)
#define FLAG_BIT_13 (1UL << 13)
#define FLAG_BIT_14 (1UL << 14)
#define FLAG_BIT_15 (1UL << 15)
#define FLAG_BIT_16 (1UL << 16)
#define FLAG_BIT_17 (1UL << 17)
#define FLAG_BIT_18 (1UL << 18)
#define FLAG_BIT_19 (1UL << 19)
#define FLAG_BIT_20 (1UL << 20)
#define FLAG_BIT_21 (1UL << 21)
#define FLAG_BIT_22 (1UL << 22)
#define FLAG_BIT_23 (1UL << 23)
#define FLAG_BIT_24 (1UL << 24)
#define FLAG_BIT_25 (1UL << 25)
#define FLAG_BIT_26 (1UL << 26)
#define FLAG_BIT_27 (1UL << 27)
#define FLAG_BIT_28 (1UL << 28)
#define FLAG_BIT_29 (1UL << 29)
#define FLAG_BIT_30 (1UL << 30)
#define FLAG_BIT_31 (1UL << 31)

#define THREAD_FLAGS_STORAGE_REQ_PROCESS_OK FLAG_BIT_0
#define THREAD_FLAGS_STORAGE_REQ_PROCESS_ERROR FLAG_BIT_1
#define THREAD_FLAGS_STORAGE_REQ_PROCESS_ALL (THREAD_FLAGS_STORAGE_REQ_PROCESS_OK | \
				THREAD_FLAGS_STORAGE_REQ_PROCESS_ERROR
				
#define	THREAD_FLAGS_MEASUREMENT_DONE FLAG_BIT_0

#define EVENT_FLAGS_MAINTENANCE_MEASUREMENT_TASK_ACTIIVE FLAG_BIT_0
#define EVENT_FLAGS_MAINTENANCE_ALL_TASKS_ACTIVE EVENT_FLAGS_MAINTENANCE_MEASUREMENT_TASK_ACTIIVE

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/
extern osThreadId_t MaitenanceTaskHandle;
extern osThreadId_t CANMsgParserTaskHandle;
extern osThreadId_t CANMsgSenderTaskHandle;
extern osThreadId_t MeasurementTaskHandle;
extern osThreadId_t StorageTaskHandle;

extern osMessageQueueId_t CANRxReqsQueueHandle;
extern osMessageQueueId_t CANTxReqsQueueHandle;
extern osMessageQueueId_t StorageReqsQueueHandle;

extern osMemoryPoolId_t CANRxReqsMemPoolHandle;
extern osMemoryPoolId_t CANTxReqsMemPoolHandle;
extern osMemoryPoolId_t StorageReqsMemPoolHandle;

extern osMutexId_t VoltagesMutexHandle;

extern osEventFlagsId_t MaintenanceEventHandle;

extern const uint32_t laUtilsBitValues[32];

extern void MaintenanceTaskSignal(uint32_t ulSignal);
/* Public function prototypes -----------------------------------------------*/

#endif /* __UTILITIES_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
