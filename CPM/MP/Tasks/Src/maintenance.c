/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "maintenance.h"

#include "freertos_mpool.h"

#include "data.h"
#include "fdcan.h"
#include "gpio.h"
#include "CANRxTx.h"
#include <string.h>
#include "iwdg.h"
#include "adc.h"
#include "i2c.h"
#include "FreeRTOS.h"
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  members */
#define MAINTAINANCE_TASK_MAX_TIMEOUT 1010
#define MAINTAINANCE_MAX_TASK_FAILURES 5

/* #define TRACE */

#ifdef TRACE
#include "stdio.h"
#include "cmsis_os.h"
#include "queue.h"

#define APP_THREADS_MAX 8

int _write(int file, char *ptr, int len)
{
  int i = 0;

  for (i = 0; i < len; i++)
  {
    ITM_SendChar(*ptr++);
  }

  return len;
}

HeapStats_t SHeapStats;
TaskStatus_t pxTaskStatusArray[APP_THREADS_MAX];
#endif /* ifdef TRACE */
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  methods */
#ifdef TRACE

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

  uxNumberOfTasks = uxTaskGetSystemState( pxTaskStatusArray,
                                          APP_THREADS_MAX,
                                          &ulTotalRunTime);

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
    { "CANRxReqsQueue", CANRxReqsQueueHandle },
    { "CANTxReqsQueue", CANTxReqsQueueHandle },
    { "NewMeasurementsQueue", NewMeasurementsQueueHandle },
    { "BatteryRuntimeQueue", BatteryRuntimeQueueHandle },
    { "LogReqsQueueQueue", LogReqsQueueHandle },
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
}

#endif /* ifdef TRACE */

static void ModuleVersionTxReq(void)
{
  tSVersion sMPVersion;

  memset(&sMPVersion, 0, sizeof(sMPVersion));

  sMPVersion.bArg1 = MAESTRO_MP_VERSION_ARG1;
  sMPVersion.bArg2 = MAESTRO_MP_VERSION_ARG2;
  sMPVersion.bArg3 = MAESTRO_MP_VERSION_ARG3;
  sMPVersion.bArg4 = MAESTRO_MP_VERSION_ARG4;

  CanTxRequest(&hfdcan1,
               FDCAN_EXTENDED_ID,
               CAN_TX_MP_EXT_ID_MP_VERSION,
               FDCAN_DATA_FRAME,
               FDCAN_BRS_OFF,
               FDCAN_CLASSIC_CAN,
               (uint8_t *) &(sMPVersion),
               sizeof(sMPVersion));
}

static void ModuleRestartedTxReq(void)
{
  CanTxRequest(&hfdcan1,
               FDCAN_EXTENDED_ID,
               CAN_TX_MP_EXT_ID_RESET,
               FDCAN_DATA_FRAME,
               FDCAN_BRS_OFF,
               FDCAN_CLASSIC_CAN,
               NULL,
               0);
}

void MaintenanceSignalTask(uint32_t ulSignal)
{
  if (MaintenanceEventHandle != NULL)
  {
    osEventFlagsSet(MaintenanceEventHandle, ulSignal);
  }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  tasks */
void MaintenanceTaskFunc(void *argument)
{
  UNUSED(argument);

  osDelay(100);

  #ifndef TRACE
  uint32_t lActiveTaskFlags = 0;
  uint8_t bTaskFailures = 0;

  #endif

  ModuleRestartedTxReq();
  ModuleVersionTxReq();

  MX_IWDG_Init();

  /* Infinite loop */
  for (;;)
  {
    #ifndef TRACE
    IWDGRefresh();

    osEventFlagsClear(MaintenanceEventHandle, UINT32_MAX);
    uint32_t lFlags = osEventFlagsWait(MaintenanceEventHandle,
                                       EVENT_FLAGS_ALL_TASKS_ACTIVE,
                                       osFlagsWaitAll,
                                       MAINTAINANCE_TASK_MAX_TIMEOUT);

    if (lFlags != EVENT_FLAGS_ALL_TASKS_ACTIVE)
    {
      if (lActiveTaskFlags != lFlags)
      {
        LogRequest(EVENT_TASK_NOT_RUNNING, 0, 0, lFlags);
        lActiveTaskFlags = lFlags;
      }

      if (++bTaskFailures > MAINTAINANCE_MAX_TASK_FAILURES)
      {
        bTaskFailures = 0;
        SystemReset();
      }
    }
    else
    {
      lActiveTaskFlags = lFlags;
      bTaskFailures = 0;
    }

    #else  /* ifndef TRACE */
    IWDGSetMaxTimeout();
    osDelay(1000);

    CheckOSHeapUsage();
    CheckThreadStackUsage();
    CheckMessageQueueUsage();
    #endif /* ifndef TRACE */
  }
} /* MaintenanceTaskFunc */
