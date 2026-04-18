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
#include "fdcan.h"
#include "storage.h"
#include "iwdg.h"
#include "utilities.h"
#include "Platform/STM32/Bootstrap/HardwarePorts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticMutexDef_t;
typedef StaticEventGroup_t osStaticEventGroupDef_t;
/* USER CODE BEGIN PTD */
typedef StaticMemPool_t osStaticMemPoolDef_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAINTENANCE_TASK_MAX_TIMEOUT 3000
#define MAINTENANCE_MAX_TASK_FAILURES 3
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osMemoryPoolId_t CANRxReqsMemPoolHandle;
uint8_t CANRxReqsMemPoolBuf[32 * sizeof(tSFDCANRxMsg)];
osStaticMemPoolDef_t CANRxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t CANRxReqsMemPool_attributes = {
  .name = "CANRxReqsMemPool",
  .cb_mem = &CANRxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(CANRxReqsMemPoolCtrlBlk),
  .mp_mem = &CANRxReqsMemPoolBuf,
  .mp_size = sizeof(CANRxReqsMemPoolBuf)
};

osMemoryPoolId_t CANTxReqsMemPoolHandle;
uint8_t CANTxReqsMemPoolBuf[32 * sizeof(tSFDCANTxMsg)];
osStaticMemPoolDef_t CANTxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t CANTxReqsMemPool_attributes = {
  .name = "CANTxReqsMemPool",
  .cb_mem = &CANTxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(CANTxReqsMemPoolCtrlBlk),
  .mp_mem = &CANTxReqsMemPoolBuf,
  .mp_size = sizeof(CANTxReqsMemPoolBuf)
};

osMemoryPoolId_t StorageReqsMemPoolHandle;
uint8_t StorageReqsMemPoolBuf[16 * sizeof(tSStorageReq)];
osStaticMemPoolDef_t StorageReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t StorageReqsMemPool_attributes = {
  .name = "StorageReqsMemPool",
  .cb_mem = &StorageReqsMemPoolCtrlBlk,
  .cb_size = sizeof(StorageReqsMemPoolCtrlBlk),
  .mp_mem = &StorageReqsMemPoolBuf,
  .mp_size = sizeof(StorageReqsMemPoolBuf)
};
/* USER CODE END Variables */
/* Definitions for MaintenanceTask */
osThreadId_t MaintenanceTaskHandle;
uint32_t MaintenanceTaskBuf[256];
osStaticThreadDef_t MaintenanceTaskCtrlBlk;
const osThreadAttr_t MaintenanceTask_attributes = {
  .name = "MaintenanceTask",
  .stack_mem = &MaintenanceTaskBuf[0],
  .stack_size = sizeof(MaintenanceTaskBuf),
  .cb_mem = &MaintenanceTaskCtrlBlk,
  .cb_size = sizeof(MaintenanceTaskCtrlBlk),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for CANMsgParserTask */
osThreadId_t CANMsgParserTaskHandle;
uint32_t CANMsgParserTaskBuf[512];
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
uint32_t CANMsgSenderTaskBuf[512];
osStaticThreadDef_t CANMsgSenderTaskCtrlBlk;
const osThreadAttr_t CANMsgSenderTask_attributes = {
  .name = "CANMsgSenderTask",
  .stack_mem = &CANMsgSenderTaskBuf[0],
  .stack_size = sizeof(CANMsgSenderTaskBuf),
  .cb_mem = &CANMsgSenderTaskCtrlBlk,
  .cb_size = sizeof(CANMsgSenderTaskCtrlBlk),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MeasurementTask */
osThreadId_t MeasurementTaskHandle;
uint32_t MeasurementTaskBuf[256];
osStaticThreadDef_t MeasurementTaskCtrlBlk;
const osThreadAttr_t MeasurementTask_attributes = {
  .name = "MeasurementTask",
  .stack_mem = &MeasurementTaskBuf[0],
  .stack_size = sizeof(MeasurementTaskBuf),
  .cb_mem = &MeasurementTaskCtrlBlk,
  .cb_size = sizeof(MeasurementTaskCtrlBlk),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for StorageTask */
osThreadId_t StorageTaskHandle;
uint32_t StorageTaskBuf[512];
osStaticThreadDef_t StorageTaskCtrlBlk;
const osThreadAttr_t StorageTask_attributes = {
  .name = "StorageTask",
  .stack_mem = &StorageTaskBuf[0],
  .stack_size = sizeof(StorageTaskBuf),
  .cb_mem = &StorageTaskCtrlBlk,
  .cb_size = sizeof(StorageTaskCtrlBlk),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CANRxReqsQueue */
osMessageQueueId_t CANRxReqsQueueHandle;
uint8_t CANRxReqsQueueBuf[32 * sizeof(tpSFDCANRxMsg)];
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
uint8_t CANTxReqsQueueBuf[32 * sizeof(tpSFDCANTxMsg)];
osStaticMessageQDef_t CANTxReqsQueueCtrlBlk;
const osMessageQueueAttr_t CANTxReqsQueue_attributes = {
  .name = "CANTxReqsQueue",
  .cb_mem = &CANTxReqsQueueCtrlBlk,
  .cb_size = sizeof(CANTxReqsQueueCtrlBlk),
  .mq_mem = &CANTxReqsQueueBuf,
  .mq_size = sizeof(CANTxReqsQueueBuf)
};
/* Definitions for StorageReqsQueue */
osMessageQueueId_t StorageReqsQueueHandle;
uint8_t StorageReqsQueueBuf[16 * sizeof(tpSStorageReq)];
osStaticMessageQDef_t StorageReqsQueueCtrlBlk;
const osMessageQueueAttr_t StorageReqsQueue_attributes = {
  .name = "StorageReqsQueue",
  .cb_mem = &StorageReqsQueueCtrlBlk,
  .cb_size = sizeof(StorageReqsQueueCtrlBlk),
  .mq_mem = &StorageReqsQueueBuf,
  .mq_size = sizeof(StorageReqsQueueBuf)
};
/* Definitions for MaintenanceEvent */
osEventFlagsId_t MaintenanceEventHandle;
osStaticEventGroupDef_t MaintenanceEventCtrlBlk;
const osEventFlagsAttr_t MaintenanceEvent_attributes = {
  .name = "MaintenanceEvent",
  .cb_mem = &MaintenanceEventCtrlBlk,
  .cb_size = sizeof(MaintenanceEventCtrlBlk),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void MaintenanceTaskFunc(void *argument);
extern void CANMsgParserTaskFunc(void *argument);
extern void CANMsgSenderTaskFunc(void *argument);
extern void MeasurementTaskFunc(void *argument);
extern void StorageTaskFunc(void *argument);

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
   *  configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   *  called if a stack overflow is detected. */
  Error_Handler();
}

/* USER CODE END 4 */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */
  /* add memory pools, ... */
  /* creation of CANRxReqsMemPool */
  CANRxReqsMemPoolHandle = osMemoryPoolNew(32,
                                           sizeof(tSFDCANRxMsg),
                                           &CANRxReqsMemPool_attributes);

  /* creation of CANTxReqsMemPool */
  CANTxReqsMemPoolHandle = osMemoryPoolNew(32,
                                           sizeof(tSFDCANTxMsg),
                                           &CANTxReqsMemPool_attributes);

  /* creation of NewMeasurementsMemPool */
  StorageReqsMemPoolHandle = osMemoryPoolNew(16,
                                             sizeof(tSStorageReq),
                                             &StorageReqsMemPool_attributes);

  if (CANRxReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (CANTxReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (StorageReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  /* USER CODE END Init */
  /* USER CODE BEGIN RTOS_MUTEX */

  /* The former CurrentMutex and OutputStatesMutex are gone. The ADC/TIM4
   * ISR paths no longer depend on osMutex (which would have been UB from
   * ISR context); task-vs-ISR shared state is protected with
   * taskENTER_CRITICAL on the task side and a double-buffer in the
   * AdcCurrentAdapter.
   */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of CANRxReqsQueue */
  CANRxReqsQueueHandle = osMessageQueueNew(32,
                                           sizeof(tpSFDCANRxMsg),
                                           &CANRxReqsQueue_attributes);

  /* creation of CANTxReqsQueue */
  CANTxReqsQueueHandle = osMessageQueueNew(32,
                                           sizeof(tpSFDCANTxMsg),
                                           &CANTxReqsQueue_attributes);

  /* creation of StorageReqsQueue */
  StorageReqsQueueHandle = osMessageQueueNew(16,
                                             sizeof(tpSStorageReq),
                                             &StorageReqsQueue_attributes);

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

  if (StorageReqsQueueHandle == NULL)
  {
    Error_Handler();
  }

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of MaintenanceTask */
  MaintenanceTaskHandle = osThreadNew(MaintenanceTaskFunc,
                                      NULL,
                                      &MaintenanceTask_attributes);

  /* creation of CANMsgParserTask */
  CANMsgParserTaskHandle = osThreadNew(CANMsgParserTaskFunc,
                                       NULL,
                                       &CANMsgParserTask_attributes);

  /* creation of CANMsgSenderTask */
  CANMsgSenderTaskHandle = osThreadNew(CANMsgSenderTaskFunc,
                                       NULL,
                                       &CANMsgSenderTask_attributes);

  /* creation of MeasurementTask */
  MeasurementTaskHandle = osThreadNew(MeasurementTaskFunc,
                                      NULL,
                                      &MeasurementTask_attributes);

  /* creation of StorageTask */
  StorageTaskHandle = osThreadNew(StorageTaskFunc, NULL,
                                  &StorageTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  if (MaintenanceTaskHandle == NULL)
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

  if (MeasurementTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (StorageTaskHandle == NULL)
  {
    Error_Handler();
  }

  /* USER CODE END RTOS_THREADS */

  /* creation of MaintenanceEvent */
  MaintenanceEventHandle = osEventFlagsNew(&MaintenanceEvent_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  if (MaintenanceEventHandle == NULL)
  {
    Error_Handler();
  }

  /* USER CODE END RTOS_EVENTS */
} /* MX_FREERTOS_Init */

/* USER CODE BEGIN Header_MaintenanceTaskFunc */

/**
 * @brief  Function implementing the MaintenanceTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_MaintenanceTaskFunc */
void MaintenanceTaskFunc(void *argument)
{
  /* USER CODE BEGIN MaintenanceTaskFunc */
  UNUSED(argument);

  /* MX_IWDG_Init() is now called from main() before osKernelStart so the
   * watchdog is already ticking when this task first runs. The refresh
   * cadence stays this task's responsibility (see Watchdog_Refresh below).
   */
  #ifndef DEBUG
  uint32_t lActiveTaskFlags = 0;
  uint8_t bTaskFailures = 0;
  #endif

  /* Infinite loop */
  for (;;)
  {
    #ifndef DEBUG
    osEventFlagsClear(MaintenanceEventHandle,
                      EVENT_FLAGS_MAINTENANCE_ALL_TASKS_ACTIVE);
    uint32_t lFlags = osEventFlagsWait(MaintenanceEventHandle,
                                       EVENT_FLAGS_MAINTENANCE_ALL_TASKS_ACTIVE,
                                       osFlagsWaitAll,
                                       MAINTENANCE_TASK_MAX_TIMEOUT);

    if (lFlags != EVENT_FLAGS_MAINTENANCE_ALL_TASKS_ACTIVE)
    {
      if (lActiveTaskFlags != lFlags)
      {
        lActiveTaskFlags = lFlags;
      }

      if (bTaskFailures++ > MAINTENANCE_MAX_TASK_FAILURES)
      {
        bTaskFailures = 0;

        Error_Handler();
      }
    }
    else
    {
      lActiveTaskFlags = lFlags;
      bTaskFailures = 0;
    }

    Watchdog_Refresh(&g_WatchdogPort);
    #else  /* ifndef DEBUG */
    osDelay(10);
    #endif /* ifndef DEBUG */
  }

  /* USER CODE END MaintenanceTaskFunc */
} /* MaintenanceTaskFunc */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void MaintenanceTaskSignal(uint32_t ulSignal)
{
  if (MaintenanceEventHandle != NULL)
  {
    osEventFlagsSet(MaintenanceEventHandle, ulSignal);
  }
}

/* USER CODE END Application */
