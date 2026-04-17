/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "freertos_mpool.h"

#include "data.h"
#include "fdcan.h"
#include "gpio.h"
#include "CANRxTx.h"
#include <string.h>
#include "iwdg.h"
#include "adc.h"
#include "i2c.h"
#include "maintenance.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticEventGroup_t osStaticEventGroupDef_t;
/* USER CODE BEGIN PTD */
typedef enum
{
	APP_TASK_NONE = 0,
	APP_TASK_DEFAULT,
	APP_TASK_CAN_MSG_PARSER,
	APP_TASK_CAN_MSG_SENDER,
	APP_TASK_SIGNAL_OUTPUT_CATCH,
	APP_TASK_SIGNAL_CHECK,
	APP_TASK_MAINTENANCE,
	
} tEAppTasks;

typedef StaticMemPool_t osStaticMemPoolDef_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define	COM_LED_TOGGLE_MIN_DURATION 5
#define	COM_LED_TOGGLE_MAX_DURATION 50
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osMemoryPoolId_t CANRxReqsMemPoolHandle;
uint8_t CANRxReqsMemPoolBuf[32 * sizeof(tSCanRxMsg)];
osStaticMemPoolDef_t CANRxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t CANRxReqsMemPool_attributes = {
  .name = "CANRxReqsMemPool",
  .cb_mem = &CANRxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(CANRxReqsMemPoolCtrlBlk),
  .mp_mem = &CANRxReqsMemPoolBuf,
  .mp_size = sizeof(CANRxReqsMemPoolBuf)
};

osMemoryPoolId_t CANTxReqsMemPoolHandle;
uint8_t CANTxReqsMemPoolBuf[32 * sizeof(tSCanTxMsg)];
osStaticMemPoolDef_t CANTxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t CANTxReqsMemPool_attributes = {
  .name = "CANTxReqsMemPool",
  .cb_mem = &CANTxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(CANTxReqsMemPoolCtrlBlk),
  .mp_mem = &CANTxReqsMemPoolBuf,
  .mp_size = sizeof(CANTxReqsMemPoolBuf)
};

osMemoryPoolId_t NewMeasurementsMemPoolHandle;
uint8_t NewMeasurementsMemPoolBuf[4 * sizeof(tSNewMeasurements)];
osStaticMemPoolDef_t NewMeasurementsMemPoolCtrlBlk;
const osMemoryPoolAttr_t NewMeasurementsMemPool_attributes = {
  .name = "NewMeasurementsMemPool",
  .cb_mem = &NewMeasurementsMemPoolCtrlBlk,
  .cb_size = sizeof(NewMeasurementsMemPoolCtrlBlk),
  .mp_mem = &NewMeasurementsMemPoolBuf,
  .mp_size = sizeof(NewMeasurementsMemPoolBuf)
};

osMemoryPoolId_t BatteryRuntimeMemPoolHandle;
uint8_t BatteryRuntimeMemPoolBuf[4 * sizeof(tSADCBatteryRuntime)];
osStaticMemPoolDef_t BatteryRuntimeMemPoolCtrlBlk;
const osMemoryPoolAttr_t BatteryRuntimeMemPool_attributes = {
  .name = "BatteryRuntimeMemPool",
  .cb_mem = &BatteryRuntimeMemPoolCtrlBlk,
  .cb_size = sizeof(BatteryRuntimeMemPoolCtrlBlk),
  .mp_mem = &BatteryRuntimeMemPoolBuf,
  .mp_size = sizeof(BatteryRuntimeMemPoolBuf)
};

osMemoryPoolId_t LogReqsMemPoolHandle;
uint8_t LogReqsMemPoolBuf[64 * sizeof(tSEvent)];
osStaticMemPoolDef_t LogReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t LogReqsMemPool_attributes = {
  .name = "LogReqsMemPool",
  .cb_mem = &LogReqsMemPoolCtrlBlk,
  .cb_size = sizeof(LogReqsMemPoolCtrlBlk),
  .mp_mem = &LogReqsMemPoolBuf,
  .mp_size = sizeof(LogReqsMemPoolBuf)
};

/* USER CODE END Variables */
/* Definitions for DefaultTask */
osThreadId_t DefaultTaskHandle;
uint32_t DefaultTaskBuf[ 128 ];
osStaticThreadDef_t DefaultTaskCtrlBlk;
const osThreadAttr_t DefaultTask_attributes = {
  .name = "DefaultTask",
  .stack_mem = &DefaultTaskBuf[0],
  .stack_size = sizeof(DefaultTaskBuf),
  .cb_mem = &DefaultTaskCtrlBlk,
  .cb_size = sizeof(DefaultTaskCtrlBlk),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SOCatchTask */
osThreadId_t SOCatchTaskHandle;
uint32_t SOCatchTaskBuf[ 256 ];
osStaticThreadDef_t SOCatchTaskCtrlBlk;
const osThreadAttr_t SOCatchTask_attributes = {
  .name = "SOCatchTask",
  .stack_mem = &SOCatchTaskBuf[0],
  .stack_size = sizeof(SOCatchTaskBuf),
  .cb_mem = &SOCatchTaskCtrlBlk,
  .cb_size = sizeof(SOCatchTaskCtrlBlk),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CANMsgParserTask */
osThreadId_t CANMsgParserTaskHandle;
uint32_t CANMsgParserTaskBuf[ 256 ];
osStaticThreadDef_t CANMsgParserTaskCtrlBlk;
const osThreadAttr_t CANMsgParserTask_attributes = {
  .name = "CANMsgParserTask",
  .stack_mem = &CANMsgParserTaskBuf[0],
  .stack_size = sizeof(CANMsgParserTaskBuf),
  .cb_mem = &CANMsgParserTaskCtrlBlk,
  .cb_size = sizeof(CANMsgParserTaskCtrlBlk),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CANMsgSenderTask */
osThreadId_t CANMsgSenderTaskHandle;
uint32_t CANMsgSenderTaskBuf[ 256 ];
osStaticThreadDef_t CANMsgSenderTaskCtrlBlk;
const osThreadAttr_t CANMsgSenderTask_attributes = {
  .name = "CANMsgSenderTask",
  .stack_mem = &CANMsgSenderTaskBuf[0],
  .stack_size = sizeof(CANMsgSenderTaskBuf),
  .cb_mem = &CANMsgSenderTaskCtrlBlk,
  .cb_size = sizeof(CANMsgSenderTaskCtrlBlk),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SignalCheckTask */
osThreadId_t SignalCheckTaskHandle;
uint32_t SignalCheckTaskBuf[ 256 ];
osStaticThreadDef_t SignalCheckTaskCtrlBlk;
const osThreadAttr_t SignalCheckTask_attributes = {
  .name = "SignalCheckTask",
  .stack_mem = &SignalCheckTaskBuf[0],
  .stack_size = sizeof(SignalCheckTaskBuf),
  .cb_mem = &SignalCheckTaskCtrlBlk,
  .cb_size = sizeof(SignalCheckTaskCtrlBlk),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MaintenanceTask */
osThreadId_t MaintenanceTaskHandle;
uint32_t MaintenanceTaskBuf[ 256 ];
osStaticThreadDef_t MaintenanceTaskCtrlBlk;
const osThreadAttr_t MaintenanceTask_attributes = {
  .name = "MaintenanceTask",
  .stack_mem = &MaintenanceTaskBuf[0],
  .stack_size = sizeof(MaintenanceTaskBuf),
  .cb_mem = &MaintenanceTaskCtrlBlk,
  .cb_size = sizeof(MaintenanceTaskCtrlBlk),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CANRxReqsQueue */
osMessageQueueId_t CANRxReqsQueueHandle;
uint8_t CANRxReqsQueueBuf[ 32 * sizeof( tpSCanRxMsg ) ];
osStaticMessageQDef_t CANRxReqsQueueCtrlBlk;
const osMessageQueueAttr_t CANRxReqsQueue_attributes = {
  .name = "CANRxReqsQueue",
  .cb_mem = &CANRxReqsQueueCtrlBlk,
  .cb_size = sizeof(CANRxReqsQueueCtrlBlk),
  .mq_mem = &CANRxReqsQueueBuf,
  .mq_size = sizeof(CANRxReqsQueueBuf)
};
/* Definitions for CANTxReqsQueue */
osMessageQueueId_t CANTxReqsQueueHandle;
uint8_t CANTxReqsQueueBuf[ 32 * sizeof( tpSCanTxMsg ) ];
osStaticMessageQDef_t CANTxReqsQueueCtrlBlk;
const osMessageQueueAttr_t CANTxReqsQueue_attributes = {
  .name = "CANTxReqsQueue",
  .cb_mem = &CANTxReqsQueueCtrlBlk,
  .cb_size = sizeof(CANTxReqsQueueCtrlBlk),
  .mq_mem = &CANTxReqsQueueBuf,
  .mq_size = sizeof(CANTxReqsQueueBuf)
};
/* Definitions for NewMeasurementsQueue */
osMessageQueueId_t NewMeasurementsQueueHandle;
uint8_t NewMeasurementsQueueBuf[ 4 * sizeof( tpSNewMeasurements ) ];
osStaticMessageQDef_t NewMeasurementsQueueCtrlBlk;
const osMessageQueueAttr_t NewMeasurementsQueue_attributes = {
  .name = "NewMeasurementsQueue",
  .cb_mem = &NewMeasurementsQueueCtrlBlk,
  .cb_size = sizeof(NewMeasurementsQueueCtrlBlk),
  .mq_mem = &NewMeasurementsQueueBuf,
  .mq_size = sizeof(NewMeasurementsQueueBuf)
};
/* Definitions for BatteryRuntimeQueue */
osMessageQueueId_t BatteryRuntimeQueueHandle;
uint8_t BatteryRuntimeQueueBuf[ 4 * sizeof( tpSADCBatteryRuntime ) ];
osStaticMessageQDef_t BatteryRuntimeQueueCtrlBlk;
const osMessageQueueAttr_t BatteryRuntimeQueue_attributes = {
  .name = "BatteryRuntimeQueue",
  .cb_mem = &BatteryRuntimeQueueCtrlBlk,
  .cb_size = sizeof(BatteryRuntimeQueueCtrlBlk),
  .mq_mem = &BatteryRuntimeQueueBuf,
  .mq_size = sizeof(BatteryRuntimeQueueBuf)
};
/* Definitions for LogReqsQueue */
osMessageQueueId_t LogReqsQueueHandle;
uint8_t LogReqsQueueBuf[ 64 * sizeof( tpSEvent ) ];
osStaticMessageQDef_t LogReqsQueueCtrlBlk;
const osMessageQueueAttr_t LogReqsQueue_attributes = {
  .name = "LogReqsQueue",
  .cb_mem = &LogReqsQueueCtrlBlk,
  .cb_size = sizeof(LogReqsQueueCtrlBlk),
  .mq_mem = &LogReqsQueueBuf,
  .mq_size = sizeof(LogReqsQueueBuf)
};
/* Definitions for I2C4Event */
osEventFlagsId_t I2C4EventHandle;
osStaticEventGroupDef_t I2C4EventCtrlBlk;
const osEventFlagsAttr_t I2C4Event_attributes = {
  .name = "I2C4Event",
  .cb_mem = &I2C4EventCtrlBlk,
  .cb_size = sizeof(I2C4EventCtrlBlk),
};
/* Definitions for MaintenanceEvent */
osEventFlagsId_t MaintenanceEventHandle;
osStaticEventGroupDef_t MaintenanceEventCtrlBlk;
const osEventFlagsAttr_t MaintenanceEvent_attributes = {
  .name = "MaintenanceEvent",
  .cb_mem = &MaintenanceEventCtrlBlk,
  .cb_size = sizeof(MaintenanceEventCtrlBlk),
};
/* Definitions for I2C3Event */
osEventFlagsId_t I2C3EventHandle;
osStaticEventGroupDef_t I2C3EventCtrlBlk;
const osEventFlagsAttr_t I2C3Event_attributes = {
  .name = "I2C3Event",
  .cb_mem = &I2C3EventCtrlBlk,
  .cb_size = sizeof(I2C3EventCtrlBlk),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void DefaultTaskFunc(void *argument);
extern void SOCatchTaskFunc(void *argument);
extern void CANMsgParserTaskFunc(void *argument);
extern void CANMsgSenderTaskFunc(void *argument);
extern void SignalCheckTaskFunc(void *argument);
extern void MaintenanceTaskFunc(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
	return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
	/* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
  UNUSED(pcTaskName);

	tEAppTasks eTask = APP_TASK_NONE;
	if(xTask == DefaultTaskHandle)
	{
		eTask = APP_TASK_DEFAULT;
	} 
	else if(xTask == CANMsgParserTaskHandle)
	{
		eTask = APP_TASK_CAN_MSG_PARSER;
	} 
	else if(xTask == CANMsgSenderTaskHandle)
	{
		eTask = APP_TASK_CAN_MSG_SENDER;
	} 
	else if(xTask == SOCatchTaskHandle)
	{
		eTask = APP_TASK_SIGNAL_OUTPUT_CATCH;
	} 
	else if(xTask == SignalCheckTaskHandle)
	{
		eTask = APP_TASK_SIGNAL_CHECK;
	}
	
	LogRequest(EVENT_TASK_STACK_OVERFLOW, (uint8_t)eTask, 1, 0);
	
	osDelay(100);
	
	Error_Handler();
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	DataInit();
	
	/* add memory pools, ... */
	/* creation of CANRxReqsMemPool */
	CANRxReqsMemPoolHandle = osMemoryPoolNew (32, sizeof(tSCanRxMsg), &CANRxReqsMemPool_attributes);

	/* creation of CANTxReqsMemPool */
  CANTxReqsMemPoolHandle = osMemoryPoolNew (32, sizeof(tSCanTxMsg), &CANTxReqsMemPool_attributes);

  /* creation of NewMeasurementsMemPool */
  NewMeasurementsMemPoolHandle = osMemoryPoolNew (4, sizeof(tSNewMeasurements), &NewMeasurementsMemPool_attributes);

  /* creation of BatteryRuntimeMemPool */
  BatteryRuntimeMemPoolHandle = osMemoryPoolNew (4, sizeof(tSADCBatteryRuntime), &BatteryRuntimeMemPool_attributes);
	
	/* creation of msgQueGPSTime */
  LogReqsMemPoolHandle = osMemoryPoolNew (64, sizeof(tSEvent), &LogReqsMemPool_attributes);
	
	if (CANRxReqsMemPoolHandle == NULL)
	{
		Error_Handler();
	}
	
	if (CANTxReqsMemPoolHandle == NULL)
	{
		Error_Handler();
	}
	
	if (NewMeasurementsMemPoolHandle == NULL)
	{
		Error_Handler();
	}
	
	if (BatteryRuntimeMemPoolHandle == NULL)
	{
		Error_Handler();
	}
	
	if (LogReqsMemPoolHandle == NULL)
	{
		Error_Handler();
	}
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of CANRxReqsQueue */
  CANRxReqsQueueHandle = osMessageQueueNew (32, sizeof(tpSCanRxMsg), &CANRxReqsQueue_attributes);

  /* creation of CANTxReqsQueue */
  CANTxReqsQueueHandle = osMessageQueueNew (32, sizeof(tpSCanTxMsg), &CANTxReqsQueue_attributes);

  /* creation of NewMeasurementsQueue */
  NewMeasurementsQueueHandle = osMessageQueueNew (4, sizeof(tpSNewMeasurements), &NewMeasurementsQueue_attributes);

  /* creation of BatteryRuntimeQueue */
  BatteryRuntimeQueueHandle = osMessageQueueNew (4, sizeof(tpSADCBatteryRuntime), &BatteryRuntimeQueue_attributes);

  /* creation of LogReqsQueue */
  LogReqsQueueHandle = osMessageQueueNew (64, sizeof(tpSEvent), &LogReqsQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  if (CANRxReqsQueueHandle == NULL)
	{
		Error_Handler();
	}
	
	if (CANTxReqsQueueHandle == NULL)
	{
		Error_Handler();
	}
	
	if (NewMeasurementsQueueHandle == NULL)
	{
		Error_Handler();
	}
	
	if (BatteryRuntimeQueueHandle == NULL)
	{
		Error_Handler();
	}
	
	if (LogReqsQueueHandle == NULL)
	{
		Error_Handler();
	}
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of DefaultTask */
  DefaultTaskHandle = osThreadNew(DefaultTaskFunc, NULL, &DefaultTask_attributes);

  /* creation of SOCatchTask */
  SOCatchTaskHandle = osThreadNew(SOCatchTaskFunc, NULL, &SOCatchTask_attributes);

  /* creation of CANMsgParserTask */
  CANMsgParserTaskHandle = osThreadNew(CANMsgParserTaskFunc, NULL, &CANMsgParserTask_attributes);

  /* creation of CANMsgSenderTask */
  CANMsgSenderTaskHandle = osThreadNew(CANMsgSenderTaskFunc, NULL, &CANMsgSenderTask_attributes);

  /* creation of SignalCheckTask */
  SignalCheckTaskHandle = osThreadNew(SignalCheckTaskFunc, NULL, &SignalCheckTask_attributes);

  /* creation of MaintenanceTask */
  MaintenanceTaskHandle = osThreadNew(MaintenanceTaskFunc, NULL, &MaintenanceTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	if (DefaultTaskHandle == NULL)
	{
		Error_Handler();
	}
	
	if (SOCatchTaskHandle == NULL)
	{
		Error_Handler();
	}
	
	if (CANMsgParserTaskHandle == NULL)
	{
		Error_Handler();
	}
	
	if (CANMsgSenderTaskHandle == NULL)
	{
		Error_Handler();
	}
	
	if (SignalCheckTaskHandle == NULL)
	{
		Error_Handler();
	}
  /* USER CODE END RTOS_THREADS */

  /* Create the event(s) */
  /* creation of I2C4Event */
  I2C4EventHandle = osEventFlagsNew(&I2C4Event_attributes);

  /* creation of MaintenanceEvent */
  MaintenanceEventHandle = osEventFlagsNew(&MaintenanceEvent_attributes);

  /* creation of I2C3Event */
  I2C3EventHandle = osEventFlagsNew(&I2C3Event_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
	if (I2C4EventHandle == NULL)
	{
		Error_Handler();
	}
	
	if (MaintenanceEventHandle == NULL)
	{
		Error_Handler();
	}
	
	if (I2C3EventHandle == NULL)
	{
		Error_Handler();
	}
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_DefaultTaskFunc */
/**
  * @brief  Function implementing the DefaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_DefaultTaskFunc */
void DefaultTaskFunc(void *argument)
{
  /* USER CODE BEGIN DefaultTaskFunc */
	UNUSED(argument);
	
	uint8_t bLEDToggleCnt = 0;
	tpSADCBatteryRuntime pSRuntime = NULL;
	
  /* Infinite loop */
  for(;;)
  {
    if(osMessageQueueGet(BatteryRuntimeQueueHandle, &pSRuntime, NULL, 10) == osOK)
		{
			BatteryRuntimeSet(pSRuntime);
			osMemoryPoolFree(BatteryRuntimeMemPoolHandle, pSRuntime);
		}
		
		CPMPCommIncCntr();

		bLEDToggleCnt++;
		if(CPCommStateGet())
		{
			if(bLEDToggleCnt >= COM_LED_TOGGLE_MIN_DURATION)
			{
				bLEDToggleCnt = 0;
				GPIOComLedPinToggle();
			}
		}
		else
		{
			if (bLEDToggleCnt >= COM_LED_TOGGLE_MAX_DURATION)
			{
				bLEDToggleCnt = 0;
				GPIOComLedPinToggle();
			}
		}
		
		MaintenanceSignalTask(EVENT_FLAGS_DEFAULT_TASK_ACTIVE);
  }
  /* USER CODE END DefaultTaskFunc */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

