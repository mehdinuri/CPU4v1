/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "maintenance.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "defs.h"
#include "data.h"
#include "MLM.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  members */
#define MAINTAINANCE_TASK_MAX_TIMEOUT 10000
#define MAINTAINANCE_MAX_TASK_FAILURES 3

#ifdef TRACE
/* #define FREERTOS_TRACE */
/* #define LWIP_TRACE */

#include "stdio.h"

int _write(int file, char *ptr, int len)
{
  int i = 0;

  for (i = 0; i < len; i++)
  {
    ITM_SendChar(*ptr++);
  }

  return len;
}

#endif /* ifdef TRACE */

#ifdef LWIP_TRACE
#include <lwip/stats.h>

#endif /* ifdef LWIP_TRACE */

#ifdef FREERTOS_TRACE

#define APP_THREADS_MAX 32

HeapStats_t SHeapStats;
TaskStatus_t pxTaskStatusArray[APP_THREADS_MAX];

#endif /* ifdef FREERTOS_TRACE */
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  methods */

#ifdef FREERTOS_TRACE

void CheckOSHeapUsage(void)
{
  memset(&SHeapStats, 0, sizeof(SHeapStats));

  printf("\n--- Start of OS Heap Usage ---\n\n");

  vPortGetHeapStats(&SHeapStats);

  printf("\tINFO: xAvailableHeapSpaceInBytes: %lu\n",
         (unsigned long) SHeapStats.xAvailableHeapSpaceInBytes);
  printf("\tINFO: xMinimumEverFreeBytesRemaining: %lu\n",
         (unsigned long) SHeapStats.xMinimumEverFreeBytesRemaining);
  printf("\tINFO: xNumberOfFreeBlocks: %lu\n",
         (unsigned long) SHeapStats.xNumberOfFreeBlocks);
  printf("\tINFO: xNumberOfSuccessfulAllocations: %lu\n",
         (unsigned long) SHeapStats.xNumberOfSuccessfulAllocations);
  printf("\tINFO: xNumberOfSuccessfulFrees: %lu\n",
         (unsigned long) SHeapStats.xNumberOfSuccessfulFrees);
  printf("\tINFO: xNumberOfSuccessfulAllocations: %lu\n",
         (unsigned long) SHeapStats.xSizeOfLargestFreeBlockInBytes);
  printf("\tINFO: xSizeOfSmallestFreeBlockInBytes: %lu\n",
         (unsigned long) SHeapStats.xSizeOfSmallestFreeBlockInBytes);

  printf("\n--- End of OS Heap Usage ---\n");
}

void CheckThreadStackUsage(void)
{
  UBaseType_t uxNumberOfTasks;

  volatile UBaseType_t x;
  uint32_t ulTotalRunTime;

  printf("\n--- Start of Task Stack High Water Marks ---\n\n");

  vTaskSuspendAll();

  uxNumberOfTasks = uxTaskGetSystemState( pxTaskStatusArray,
                                          APP_THREADS_MAX,
                                          &ulTotalRunTime);

  xTaskResumeAll();

  printf("Found %lu tasks:\n", (unsigned long) uxNumberOfTasks);
  printf("----------------------------------------------------\n");
  printf("%-*s\tUnused Stack (Words)\n", configMAX_TASK_NAME_LEN, "Task Name");
  printf("----------------------------------------------------\n");

  for (x = 0; x < uxNumberOfTasks; x++)
  {
    printf("%-*s\t%lu\n",
           configMAX_TASK_NAME_LEN,
           pxTaskStatusArray[x].pcTaskName,
           (unsigned long) pxTaskStatusArray[x].usStackHighWaterMark);

    if (pxTaskStatusArray[x].usStackHighWaterMark == 0)
    {
      printf(
        "  WARNING: Task '%s' stack potentially overflowed or critically low!\n",
        pxTaskStatusArray[x].pcTaskName);
    }
    else if (pxTaskStatusArray[x].usStackHighWaterMark < 32)
    {
      printf("\tINFO: Task '%s' stack usage high (low watermark: %lu words).\n",
             pxTaskStatusArray[x].pcTaskName,
             (unsigned long) pxTaskStatusArray[x].usStackHighWaterMark);
    }
  }

  printf("----------------------------------------------------\n");

  printf("\n--- End of Task Stack High Water Marks ---\n");
} /* CheckThreadStackUsage */

void CheckMessageQueueUsage(void)
{
  typedef struct
  {
    const char *strName;
    QueueHandle_t SHandle;
  } tSQueueInfo;

  tSQueueInfo SaQueues[] =
  {
    { "FDCANRxReqsQue", FDCANRxReqsQueHandle },
    { "FDCANTxReqsQue", FDCANTxReqsQueHandle },
    { "GPSRxReqsQue", GPSRxReqsQueHandle },

    { "GPSTimeQue", GPSTimeQueHandle },
    { "UIRxReqsQue", UIRxReqsQueHandle },
    { "UITxReqsQue", UITxReqsQueHandle },

    { "MSMReqsQue", MSMReqsQueHandle },
    { "LogReqsQue", LogReqsQueHandle },
  };

  printf("\n--- Start of Message Queue Usage ---\n\n");

  const size_t bNumQueues = sizeof(SaQueues) / sizeof(SaQueues[0]);
  size_t bQueIdx = 0;

  for (bQueIdx = 0; bQueIdx < bNumQueues; bQueIdx++)
  {
    printf("\n %s\n", SaQueues[bQueIdx].strName);
    printf("----------------------------------------------------\n");
    printf("\tINFO: Available Space: %lu\n",
           (unsigned long) osMessageQueueGetSpace(SaQueues[bQueIdx].SHandle));
    printf("\tINFO: Message Waiting: %lu\n",
           (unsigned long) osMessageQueueGetCount(SaQueues[bQueIdx].SHandle));
  }

  printf("\n--- End of Message Queue Usage ---\n\n");
} /* CheckMessageQueueUsage */

#endif /* ifdef FREERTOS_TRACE */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  tasks */
void MaintenanceTaskFunc(void *argument)
{
  UNUSED(argument);
  osDelay(1000);
  #if !defined(DEBUG)
  uint32_t lActiveTaskFlags = 0;
  uint8_t bTaskFailures = 0;
  #endif
  /* Infinite loop */
  for (;;)
  {
    #ifdef DEBUG
    #if defined(FREERTOS_TRACE)
    osDelay(10000);

    CheckOSHeapUsage();
    CheckThreadStackUsage();
    CheckMessageQueueUsage();
    #elif defined(LWIP_TRACE)
    osDelay(10000);

    stats_display();
    #else
    osThreadSuspend(osThreadGetId());
    #endif /* if defined(FREERTOS_TRACE) */
    #else  /* ifdef DEBUG */
    int32_t lFlags = osEventFlagsWait(MaintenanceEventHandle,
                                      EVENT_FLAGS_MAINTENANCE_ALL_TASKS_ACTIVE,
                                      osFlagsWaitAll,
                                      MAINTAINANCE_TASK_MAX_TIMEOUT);

    if (lFlags == osErrorTimeout)
    {
      if (lActiveTaskFlags != lFlags)
      {
        /* LogRequest(LOG_REQ_APPEND, NULL, EVENT_TASK_NOT_RUNNING, 0, 0, lFlags, 0); */
        lActiveTaskFlags = lFlags;
      }

      if (bTaskFailures++ > MAINTAINANCE_MAX_TASK_FAILURES)
      {
        bTaskFailures = 0;
        SecureSystemReset();
      }
    }
    else
    {
      lActiveTaskFlags = lFlags;
      bTaskFailures = 0;
    }

    #endif /* ifdef DEBUG */
  }
} /* MaintenanceTaskFunc */
