/* USER CODE BEGIN Header */

/**
 ******************************************************************************
 * File Name          : freertos.c
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
#include "FreeRTOS.h"
#include "cmsis_os2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <freertos_mpool.h>

#include "CanMsgParser.h"
#include "IAP.h"
#include "MCSAsynch.h"
#include "MLM.h"
#include "MSM.h"
#include "PPPOSAsynch.h"
#include "comm.h"
#include "cpmpcomm.h"
#include "data.h"
#include "defs.h"
#include "dma.h"
#include "ethernetif.h"
#include "fdcan.h"
#include "flash.h"
#include "gpio.h"
#include "gps.h"
#include "iwdg.h"
#include "IntersectionControlTask.h"
#include "lcd.h"
#include "lcdDrv.h"
#include "MCS.h"
#include "mmi.h"
#include "program.h"
#include "rng.h"
#include "rtc.h"
#include "signalCardDrv.h"
#include "tim.h"
#include "time.h"
#include "ui.h"
#include "usart.h"
#include "usb.h"
#include "PPPOSAsynch.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticMutexDef_t;
typedef StaticEventGroup_t osStaticEventGroupDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
typedef StaticMemPool_t osStaticMemPoolDef_t;

typedef enum
{
  APP_TASK_NONE = 0,
  APP_TASK_START,
  APP_TASK_TIME,
  APP_TASK_CAN_MSG_PARSER,
  APP_TASK_CAN_MSG_SENDER,
  APP_TASK_SIGNAL_CARD_DRIVE,
  APP_TASK_INTERSECTION_CONTROL,
  APP_TASK_CPMP_COMM,
  APP_TASK_GPS_MSG_PARSER,
  APP_TASK_GPS,
  APP_TASK_MMI,
  APP_TASK_MCS,
  APP_TASK_MCS_ASYNCH_MSG_PARSER,
  APP_TASK_MCS_ASYNCH_MSG_SENDER,
  APP_TASK_MCS_ASYNCH,
  APP_TASK_UI_MSG_PARSER,
  APP_TASK_UI_MSG_SENDER,
  APP_TASK_LCD,
  APP_TASK_PROGRAM,
  APP_TASK_MLM,
  APP_TASK_IAP,
  APP_TASK_MSM,
  APP_TASK_PPP_ASYNCH_MSG_PARSER,
  APP_TASK_PPP_ASYNCH_MSG_SENDER,
  APP_TASK_ETH_IF_INPUT,
  APP_TASK_ETH_LINK_STATE,
  APP_TASK_MAINTENANCE,
} tEAppTasks;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
__attribute__((section(".itcm_bss"), aligned(32)))
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];

/* Definitions for FDCANRxReqsMemPool */
osMemoryPoolId_t FDCANRxReqsMemPoolHandle;
__attribute__((section(".dtcm_bss"), aligned(32)))
uint8_t FDCANRxReqsMemPoolBuf[32 * sizeof(tSFDCANRxMsg)];
osStaticMemPoolDef_t FDCANRxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t FDCANRxReqsMemPool_attributes = {
  .name = "FDCANRxReqsMemPool",
  .cb_mem = &FDCANRxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(FDCANRxReqsMemPoolCtrlBlk),
  .mp_mem = &FDCANRxReqsMemPoolBuf,
  .mp_size = sizeof(FDCANRxReqsMemPoolBuf)
};

/* Definitions for FDCANTxReqsMemPool */
osMemoryPoolId_t FDCANTxReqsMemPoolHandle;
__attribute__((section(".dtcm_bss"), aligned(32)))
uint8_t FDCANTxReqsMemPoolBuf[32 * sizeof(tSFDCANTxMsg)];
osStaticMemPoolDef_t FDCANTxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t FDCANTxReqsMemPool_attributes = {
  .name = "FDCANTxReqsMemPool",
  .cb_mem = &FDCANTxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(FDCANTxReqsMemPoolCtrlBlk),
  .mp_mem = &FDCANTxReqsMemPoolBuf,
  .mp_size = sizeof(FDCANTxReqsMemPoolBuf)
};

/* Definitions for GPSRxReqsMemPool */
osMemoryPoolId_t GPSRxReqsMemPoolHandle;
__attribute__((section(".dtcm_bss"), aligned(32)))
uint8_t GPSRxReqsMemPoolBuf[2 * sizeof(tSGPSRxMsg)];
osStaticMemPoolDef_t GPSRxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t GPSRxReqsMemPool_attributes = {
  .name = "GPSRxReqsMemPool",
  .cb_mem = &GPSRxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(GPSRxReqsMemPoolCtrlBlk),
  .mp_mem = &GPSRxReqsMemPoolBuf,
  .mp_size = sizeof(GPSRxReqsMemPoolBuf)
};

/* Definitions for GPSTimeMemPool */
osMemoryPoolId_t GPSTimeMemPoolHandle;
__attribute__((section(".dtcm_bss"), aligned(32)))
uint8_t GPSTimeMemPoolBuf[8 * sizeof(tSTime)];
osStaticMemPoolDef_t GPSTimeMemPoolCtrlBlk;
const osMemoryPoolAttr_t GPSTimeMemPool_attributes = {
  .name = "GPSTimeMemPool",
  .cb_mem = &GPSTimeMemPoolCtrlBlk,
  .cb_size = sizeof(GPSTimeMemPoolCtrlBlk),
  .mp_mem = &GPSTimeMemPoolBuf,
  .mp_size = sizeof(GPSTimeMemPoolBuf)
};

/* Definitions for MMIReqsMemPool */
osMemoryPoolId_t MMIReqsMemPoolHandle;
__attribute__((section(".dtcm_bss"), aligned(32)))
uint8_t MMIReqsMemPoolBuf[16 * sizeof(tSFDCANRxMsg)];
osStaticMemPoolDef_t MMIReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t MMIReqsMemPool_attributes = {
  .name = "MMIReqsMemPool",
  .cb_mem = &MMIReqsMemPoolCtrlBlk,
  .cb_size = sizeof(MMIReqsMemPoolCtrlBlk),
  .mp_mem = &MMIReqsMemPoolBuf,
  .mp_size = sizeof(MMIReqsMemPoolBuf)
};

/* Definitions for MCSAsyRxReqsMemPool */
osMemoryPoolId_t MCSAsyRxReqsMemPoolHandle;
__attribute__((section(".dtcm_bss"), aligned(32)))
uint8_t MCSAsyRxReqsMemPoolBuf[4 * sizeof(tSMCSAsynchRxTxMsg)];
osStaticMemPoolDef_t MCSAsyRxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t MCSAsyRxReqsMemPool_attributes = {
  .name = "MCSAsyRxReqsMemPool",
  .cb_mem = &MCSAsyRxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(MCSAsyRxReqsMemPoolCtrlBlk),
  .mp_mem = &MCSAsyRxReqsMemPoolBuf,
  .mp_size = sizeof(MCSAsyRxReqsMemPoolBuf)
};

/* Definitions for MCSAsyTxReqsMemPool */
osMemoryPoolId_t MCSAsyTxReqsMemPoolHandle;
__attribute__((section(".ram_d2_dma_buffers"), aligned(32)))
uint8_t MCSAsyTxReqsMemPoolBuf[4 * sizeof(tSMCSAsynchRxTxMsg)];
osStaticMemPoolDef_t MCSAsyTxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t MCSAsyTxReqsMemPool_attributes = {
  .name = "MCSAsyTxReqsMemPool",
  .cb_mem = &MCSAsyTxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(MCSAsyTxReqsMemPoolCtrlBlk),
  .mp_mem = &MCSAsyTxReqsMemPoolBuf,
  .mp_size = sizeof(MCSAsyTxReqsMemPoolBuf)
};

/* Definitions for UIRxReqsMemPool */
osMemoryPoolId_t UIRxReqsMemPoolHandle;
__attribute__((section(".dtcm_bss"), aligned(32)))
uint8_t UIRxReqsMemPoolBuf[4 * sizeof(tSUIRequest)];
osStaticMemPoolDef_t UIRxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t UIRxReqsMemPool_attributes = {
  .name = "UIRxReqsMemPool",
  .cb_mem = &UIRxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(UIRxReqsMemPoolCtrlBlk),
  .mp_mem = &UIRxReqsMemPoolBuf,
  .mp_size = sizeof(UIRxReqsMemPoolBuf)
};

/* Definitions for UITxReqsMemPool */
osMemoryPoolId_t UITxReqsMemPoolHandle;
__attribute__((section(".ram_d2_dma_buffers"), aligned(32)))
uint8_t UITxReqsMemPoolBuf[4 * sizeof(tSUIRequest)];
osStaticMemPoolDef_t UITxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t UITxReqsMemPool_attributes = {
  .name = "UITxReqsMemPool",
  .cb_mem = &UITxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(UITxReqsMemPoolCtrlBlk),
  .mp_mem = &UITxReqsMemPoolBuf,
  .mp_size = sizeof(UITxReqsMemPoolBuf)
};

/* Definitions for MSMReqsMemPool */
osMemoryPoolId_t MSMReqsMemPoolHandle;
__attribute__((section(".dtcm_bss"), aligned(32)))
uint8_t MSMReqsMemPoolBuf[16 * sizeof(tSMSMRequest)];
osStaticMemPoolDef_t MSMReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t MSMReqsMemPool_attributes = {
  .name = "MSMReqsMemPool",
  .cb_mem = &MSMReqsMemPoolCtrlBlk,
  .cb_size = sizeof(MSMReqsMemPoolCtrlBlk),
  .mp_mem = &MSMReqsMemPoolBuf,
  .mp_size = sizeof(MSMReqsMemPoolBuf)
};

/* Definitions for LogReqsMemPool */
osMemoryPoolId_t LogReqsMemPoolHandle;
__attribute__((section(".dtcm_bss"), aligned(32)))
uint8_t LogReqsMemPoolBuf[16 * sizeof(tSLogReq)];
osStaticMemPoolDef_t LogReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t LogReqsMemPool_attributes = {
  .name = "LogReqsMemPool",
  .cb_mem = &LogReqsMemPoolCtrlBlk,
  .cb_size = sizeof(LogReqsMemPoolCtrlBlk),
  .mp_mem = &LogReqsMemPoolBuf,
  .mp_size = sizeof(LogReqsMemPoolBuf)
};

/* Definitions for IAPRxReqsMemPool */
osMemoryPoolId_t IAPRxReqsMemPoolHandle;
__attribute__((section(".dtcm_bss"), aligned(32)))
uint8_t IAPRxReqsMemPoolBuf[2 * sizeof(tSIAPRequest)];
osStaticMemPoolDef_t IAPRxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t IAPRxReqsMemPool_attributes = {
  .name = "IAPRxReqsMemPool",
  .cb_mem = &IAPRxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(IAPRxReqsMemPoolCtrlBlk),
  .mp_mem = &IAPRxReqsMemPoolBuf,
  .mp_size = sizeof(IAPRxReqsMemPoolBuf)
};

/* Definitions for PPPOSAsyRxReqsMemPool */
osMemoryPoolId_t PPPOSAsyRxReqsMemPoolHandle;
__attribute__((section(".dtcm_bss"), aligned(32)))
uint8_t PPPOSAsyRxReqsMemPoolBuf[4 * sizeof(tSPPPOSAsynchRxTxMsg)];
osStaticMemPoolDef_t PPPOSAsyRxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t PPPOSAsyRxReqsMemPool_attributes = {
  .name = "PPPOSAsyRxReqsMemPool",
  .cb_mem = &PPPOSAsyRxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(PPPOSAsyRxReqsMemPoolCtrlBlk),
  .mp_mem = &PPPOSAsyRxReqsMemPoolBuf,
  .mp_size = sizeof(PPPOSAsyRxReqsMemPoolBuf)
};

/* Definitions for PPPOSAsyTxReqsMemPool */
osMemoryPoolId_t PPPOSAsyTxReqsMemPoolHandle;
__attribute__((section(".ram_d2_dma_buffers"), aligned(32)))
uint8_t PPPOSAsyTxReqsMemPoolBuf[4 * sizeof(tSPPPOSAsynchRxTxMsg)];
osStaticMemPoolDef_t PPPOSAsyTxReqsMemPoolCtrlBlk;
const osMemoryPoolAttr_t PPPOSAsyTxReqsMemPool_attributes = {
  .name = "PPPOSAsyTxReqsMemPool",
  .cb_mem = &PPPOSAsyTxReqsMemPoolCtrlBlk,
  .cb_size = sizeof(PPPOSAsyTxReqsMemPoolCtrlBlk),
  .mp_mem = &PPPOSAsyTxReqsMemPoolBuf,
  .mp_size = sizeof(PPPOSAsyTxReqsMemPoolBuf)
};

/* USER CODE END Variables */
/* Definitions for DefaultTask */
osThreadId_t DefaultTaskHandle;
uint32_t DefaultTaskBuf[ 512 ];
osStaticThreadDef_t DefaultTaskCtrlBlk;
const osThreadAttr_t DefaultTask_attributes = {
  .name = "DefaultTask",
  .cb_mem = &DefaultTaskCtrlBlk,
  .cb_size = sizeof(DefaultTaskCtrlBlk),
  .stack_mem = &DefaultTaskBuf[0],
  .stack_size = sizeof(DefaultTaskBuf),
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for TimeTask */
osThreadId_t TimeTaskHandle;
uint32_t TimeTaskBuf[ 128 ];
osStaticThreadDef_t TimeTaskCtrlBlk;
const osThreadAttr_t TimeTask_attributes = {
  .name = "TimeTask",
  .cb_mem = &TimeTaskCtrlBlk,
  .cb_size = sizeof(TimeTaskCtrlBlk),
  .stack_mem = &TimeTaskBuf[0],
  .stack_size = sizeof(TimeTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CANMsgParserTask */
osThreadId_t CANMsgParserTaskHandle;
uint32_t CANMsgParserTaskBuf[ 512 ];
osStaticThreadDef_t CANMsgParserTaskCtrlBlk;
const osThreadAttr_t CANMsgParserTask_attributes = {
  .name = "CANMsgParserTask",
  .cb_mem = &CANMsgParserTaskCtrlBlk,
  .cb_size = sizeof(CANMsgParserTaskCtrlBlk),
  .stack_mem = &CANMsgParserTaskBuf[0],
  .stack_size = sizeof(CANMsgParserTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CANMsgSenderTask */
osThreadId_t CANMsgSenderTaskHandle;
uint32_t CANMsgSenderTaskBuf[ 128 ];
osStaticThreadDef_t CANMsgSenderTaskCtrlBlk;
const osThreadAttr_t CANMsgSenderTask_attributes = {
  .name = "CANMsgSenderTask",
  .cb_mem = &CANMsgSenderTaskCtrlBlk,
  .cb_size = sizeof(CANMsgSenderTaskCtrlBlk),
  .stack_mem = &CANMsgSenderTaskBuf[0],
  .stack_size = sizeof(CANMsgSenderTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SignalCardDriveTask */
osThreadId_t SignalCardDriveTaskHandle;
uint32_t SignalCardDriveTaskBuf[ 512 ];
osStaticThreadDef_t SignalCardDriveTaskCtrlBlk;
const osThreadAttr_t SignalCardDriveTask_attributes = {
  .name = "SignalCardDriveTask",
  .cb_mem = &SignalCardDriveTaskCtrlBlk,
  .cb_size = sizeof(SignalCardDriveTaskCtrlBlk),
  .stack_mem = &SignalCardDriveTaskBuf[0],
  .stack_size = sizeof(SignalCardDriveTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for IntersectionControlTask */
osThreadId_t IntersectionControlTaskHandle;
uint32_t IntersectionControlTaskBuf[ 256 ];
osStaticThreadDef_t IntersectionControlTaskCtrlBlk;
const osThreadAttr_t IntersectionControlTask_attributes = {
  .name = "IntersectionControlTask",
  .cb_mem = &IntersectionControlTaskCtrlBlk,
  .cb_size = sizeof(IntersectionControlTaskCtrlBlk),
  .stack_mem = &IntersectionControlTaskBuf[0],
  .stack_size = sizeof(IntersectionControlTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CPMPComTask */
osThreadId_t CPMPComTaskHandle;
uint32_t CPMPComTaskBuf[ 256 ];
osStaticThreadDef_t CPMPComTaskCtrlBlk;
const osThreadAttr_t CPMPComTask_attributes = {
  .name = "CPMPComTask",
  .cb_mem = &CPMPComTaskCtrlBlk,
  .cb_size = sizeof(CPMPComTaskCtrlBlk),
  .stack_mem = &CPMPComTaskBuf[0],
  .stack_size = sizeof(CPMPComTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GPSMsgParserTask */
osThreadId_t GPSMsgParserTaskHandle;
uint32_t GPSMsgParserTaskBuf[ 128 ];
osStaticThreadDef_t GPSMsgParserTaskCtrlBlk;
const osThreadAttr_t GPSMsgParserTask_attributes = {
  .name = "GPSMsgParserTask",
  .cb_mem = &GPSMsgParserTaskCtrlBlk,
  .cb_size = sizeof(GPSMsgParserTaskCtrlBlk),
  .stack_mem = &GPSMsgParserTaskBuf[0],
  .stack_size = sizeof(GPSMsgParserTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GPSTask */
osThreadId_t GPSTaskHandle;
uint32_t GPSTaskBuf[ 128 ];
osStaticThreadDef_t GPSTaskCtrlBlk;
const osThreadAttr_t GPSTask_attributes = {
  .name = "GPSTask",
  .cb_mem = &GPSTaskCtrlBlk,
  .cb_size = sizeof(GPSTaskCtrlBlk),
  .stack_mem = &GPSTaskBuf[0],
  .stack_size = sizeof(GPSTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MMITask */
osThreadId_t MMITaskHandle;
uint32_t MMITaskBuf[ 256 ];
osStaticThreadDef_t MMITaskCtrlBlk;
const osThreadAttr_t MMITask_attributes = {
  .name = "MMITask",
  .cb_mem = &MMITaskCtrlBlk,
  .cb_size = sizeof(MMITaskCtrlBlk),
  .stack_mem = &MMITaskBuf[0],
  .stack_size = sizeof(MMITaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MCSTask */
osThreadId_t MCSTaskHandle;
uint32_t MCSTaskBuf[ 512 ];
osStaticThreadDef_t MCSTaskCtrlBlk;
const osThreadAttr_t MCSTask_attributes = {
  .name = "MCSTask",
  .cb_mem = &MCSTaskCtrlBlk,
  .cb_size = sizeof(MCSTaskCtrlBlk),
  .stack_mem = &MCSTaskBuf[0],
  .stack_size = sizeof(MCSTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MCSAsyMsgParserTask */
osThreadId_t MCSAsyMsgParserTaskHandle;
uint32_t MCSAsyMsgParserTaskBuf[ 512 ];
osStaticThreadDef_t MCSAsyMsgParserTaskCtrlBlk;
const osThreadAttr_t MCSAsyMsgParserTask_attributes = {
  .name = "MCSAsyMsgParserTask",
  .cb_mem = &MCSAsyMsgParserTaskCtrlBlk,
  .cb_size = sizeof(MCSAsyMsgParserTaskCtrlBlk),
  .stack_mem = &MCSAsyMsgParserTaskBuf[0],
  .stack_size = sizeof(MCSAsyMsgParserTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MCSAsyMsgSenderTask */
osThreadId_t MCSAsyMsgSenderTaskHandle;
uint32_t MCSAsyMsgSenderTaskBuf[ 256 ];
osStaticThreadDef_t MCSAsyMsgSenderTaskCtrlBlk;
const osThreadAttr_t MCSAsyMsgSenderTask_attributes = {
  .name = "MCSAsyMsgSenderTask",
  .cb_mem = &MCSAsyMsgSenderTaskCtrlBlk,
  .cb_size = sizeof(MCSAsyMsgSenderTaskCtrlBlk),
  .stack_mem = &MCSAsyMsgSenderTaskBuf[0],
  .stack_size = sizeof(MCSAsyMsgSenderTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MCSAsyTask */
osThreadId_t MCSAsyTaskHandle;
uint32_t MCSAsyTaskBuf[ 512 ];
osStaticThreadDef_t MCSAsyTaskCtrlBlk;
const osThreadAttr_t MCSAsyTask_attributes = {
  .name = "MCSAsyTask",
  .cb_mem = &MCSAsyTaskCtrlBlk,
  .cb_size = sizeof(MCSAsyTaskCtrlBlk),
  .stack_mem = &MCSAsyTaskBuf[0],
  .stack_size = sizeof(MCSAsyTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UIMsgParserTask */
osThreadId_t UIMsgParserTaskHandle;
uint32_t UIMsgParserTaskBuf[ 512 ];
osStaticThreadDef_t UIMsgParserTaskCtrlBlk;
const osThreadAttr_t UIMsgParserTask_attributes = {
  .name = "UIMsgParserTask",
  .cb_mem = &UIMsgParserTaskCtrlBlk,
  .cb_size = sizeof(UIMsgParserTaskCtrlBlk),
  .stack_mem = &UIMsgParserTaskBuf[0],
  .stack_size = sizeof(UIMsgParserTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UIMsgSenderTask */
osThreadId_t UIMsgSenderTaskHandle;
uint32_t UIMsgSenderTaskBuf[ 128 ];
osStaticThreadDef_t UIMsgSenderTaskCtrlBlk;
const osThreadAttr_t UIMsgSenderTask_attributes = {
  .name = "UIMsgSenderTask",
  .cb_mem = &UIMsgSenderTaskCtrlBlk,
  .cb_size = sizeof(UIMsgSenderTaskCtrlBlk),
  .stack_mem = &UIMsgSenderTaskBuf[0],
  .stack_size = sizeof(UIMsgSenderTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LCDTask */
osThreadId_t LCDTaskHandle;
uint32_t LCDTaskBuf[ 256 ];
osStaticThreadDef_t LCDTaskCtrlBlk;
const osThreadAttr_t LCDTask_attributes = {
  .name = "LCDTask",
  .cb_mem = &LCDTaskCtrlBlk,
  .cb_size = sizeof(LCDTaskCtrlBlk),
  .stack_mem = &LCDTaskBuf[0],
  .stack_size = sizeof(LCDTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ProgramTask */
osThreadId_t ProgramTaskHandle;
uint32_t ProgramTaskBuf[ 1024 ];
osStaticThreadDef_t ProgramTaskCtrlBlk;
const osThreadAttr_t ProgramTask_attributes = {
  .name = "ProgramTask",
  .cb_mem = &ProgramTaskCtrlBlk,
  .cb_size = sizeof(ProgramTaskCtrlBlk),
  .stack_mem = &ProgramTaskBuf[0],
  .stack_size = sizeof(ProgramTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MLMTask */
osThreadId_t MLMTaskHandle;
uint32_t MLMTaskBuf[ 128 ];
osStaticThreadDef_t MLMTaskCtrlBlk;
const osThreadAttr_t MLMTask_attributes = {
  .name = "MLMTask",
  .cb_mem = &MLMTaskCtrlBlk,
  .cb_size = sizeof(MLMTaskCtrlBlk),
  .stack_mem = &MLMTaskBuf[0],
  .stack_size = sizeof(MLMTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MSMTask */
osThreadId_t MSMTaskHandle;
uint32_t MSMTaskBuf[ 128 ];
osStaticThreadDef_t MSMTaskCtrlBlk;
const osThreadAttr_t MSMTask_attributes = {
  .name = "MSMTask",
  .cb_mem = &MSMTaskCtrlBlk,
  .cb_size = sizeof(MSMTaskCtrlBlk),
  .stack_mem = &MSMTaskBuf[0],
  .stack_size = sizeof(MSMTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for IAPMsgParserTask */
osThreadId_t IAPMsgParserTaskHandle;
uint32_t IAPMsgParserTaskBuf[ 512 ];
osStaticThreadDef_t IAPMsgParserTaskCtrlBlk;
const osThreadAttr_t IAPMsgParserTask_attributes = {
  .name = "IAPMsgParserTask",
  .cb_mem = &IAPMsgParserTaskCtrlBlk,
  .cb_size = sizeof(IAPMsgParserTaskCtrlBlk),
  .stack_mem = &IAPMsgParserTaskBuf[0],
  .stack_size = sizeof(IAPMsgParserTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for PPPOSAsyMsgParserTask */
osThreadId_t PPPOSAsyMsgParserTaskHandle;
uint32_t PPPOSAsyMsgParserTaskBuf[ 512 ];
osStaticThreadDef_t PPPOSAsyMsgParserTaskCtrlBlk;
const osThreadAttr_t PPPOSAsyMsgParserTask_attributes = {
  .name = "PPPOSAsyMsgParserTask",
  .cb_mem = &PPPOSAsyMsgParserTaskCtrlBlk,
  .cb_size = sizeof(PPPOSAsyMsgParserTaskCtrlBlk),
  .stack_mem = &PPPOSAsyMsgParserTaskBuf[0],
  .stack_size = sizeof(PPPOSAsyMsgParserTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for PPPOSAsyMsgSenderTask */
osThreadId_t PPPOSAsyMsgSenderTaskHandle;
uint32_t PPPOSAsyMsgSenderTaskBuf[ 256 ];
osStaticThreadDef_t PPPOSAsyMsgSenderTaskCtrlBlk;
const osThreadAttr_t PPPOSAsyMsgSenderTask_attributes = {
  .name = "PPPOSAsyMsgSenderTask",
  .cb_mem = &PPPOSAsyMsgSenderTaskCtrlBlk,
  .cb_size = sizeof(PPPOSAsyMsgSenderTaskCtrlBlk),
  .stack_mem = &PPPOSAsyMsgSenderTaskBuf[0],
  .stack_size = sizeof(PPPOSAsyMsgSenderTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MaintenanceTask */
osThreadId_t MaintenanceTaskHandle;
uint32_t MaintenanceTaskBuf[ 256 ];
osStaticThreadDef_t MaintenanceTaskCtrlBlk;
const osThreadAttr_t MaintenanceTask_attributes = {
  .name = "MaintenanceTask",
  .cb_mem = &MaintenanceTaskCtrlBlk,
  .cb_size = sizeof(MaintenanceTaskCtrlBlk),
  .stack_mem = &MaintenanceTaskBuf[0],
  .stack_size = sizeof(MaintenanceTaskBuf),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for FDCANRxReqsQue */
osMessageQueueId_t FDCANRxReqsQueHandle;
uint8_t FDCANRxReqsQueBuf[ 32 * sizeof( tpSFDCANRxMsg ) ];
osStaticMessageQDef_t FDCANRxReqsQueCtrlBlk;
const osMessageQueueAttr_t FDCANRxReqsQue_attributes = {
  .name = "FDCANRxReqsQue",
  .cb_mem = &FDCANRxReqsQueCtrlBlk,
  .cb_size = sizeof(FDCANRxReqsQueCtrlBlk),
  .mq_mem = &FDCANRxReqsQueBuf,
  .mq_size = sizeof(FDCANRxReqsQueBuf)
};
/* Definitions for FDCANTxReqsQue */
osMessageQueueId_t FDCANTxReqsQueHandle;
uint8_t FDCANTxReqsQueBuf[ 32 * sizeof( tpSFDCANTxMsg ) ];
osStaticMessageQDef_t FDCANTxReqsQueCtrlBlk;
const osMessageQueueAttr_t FDCANTxReqsQue_attributes = {
  .name = "FDCANTxReqsQue",
  .cb_mem = &FDCANTxReqsQueCtrlBlk,
  .cb_size = sizeof(FDCANTxReqsQueCtrlBlk),
  .mq_mem = &FDCANTxReqsQueBuf,
  .mq_size = sizeof(FDCANTxReqsQueBuf)
};
/* Definitions for GPSRxReqsQue */
osMessageQueueId_t GPSRxReqsQueHandle;
uint8_t GPSRxReqsQueBuf[ 2 * sizeof( tpSGPSRxMsg ) ];
osStaticMessageQDef_t GPSRxReqsQueCtrlBlk;
const osMessageQueueAttr_t GPSRxReqsQue_attributes = {
  .name = "GPSRxReqsQue",
  .cb_mem = &GPSRxReqsQueCtrlBlk,
  .cb_size = sizeof(GPSRxReqsQueCtrlBlk),
  .mq_mem = &GPSRxReqsQueBuf,
  .mq_size = sizeof(GPSRxReqsQueBuf)
};
/* Definitions for GPSTimeQue */
osMessageQueueId_t GPSTimeQueHandle;
uint8_t GPSTimeQueBuf[ 8 * sizeof( tpSTime ) ];
osStaticMessageQDef_t GPSTimeQueCtrlBlk;
const osMessageQueueAttr_t GPSTimeQue_attributes = {
  .name = "GPSTimeQue",
  .cb_mem = &GPSTimeQueCtrlBlk,
  .cb_size = sizeof(GPSTimeQueCtrlBlk),
  .mq_mem = &GPSTimeQueBuf,
  .mq_size = sizeof(GPSTimeQueBuf)
};
/* Definitions for MMIReqsQue */
osMessageQueueId_t MMIReqsQueHandle;
uint8_t MMIReqsQueBuf[ 16 * sizeof( tpSFDCANRxMsg ) ];
osStaticMessageQDef_t MMIReqsQueCtrlBlk;
const osMessageQueueAttr_t MMIReqsQue_attributes = {
  .name = "MMIReqsQue",
  .cb_mem = &MMIReqsQueCtrlBlk,
  .cb_size = sizeof(MMIReqsQueCtrlBlk),
  .mq_mem = &MMIReqsQueBuf,
  .mq_size = sizeof(MMIReqsQueBuf)
};
/* Definitions for MCSAsyRxReqsQue */
osMessageQueueId_t MCSAsyRxReqsQueHandle;
uint8_t MCSAsyRxReqsQueBuf[ 4 * sizeof( tpSMCSAsynchRxTxMsg ) ];
osStaticMessageQDef_t MCSAsyRxReqsQueCtrlBlk;
const osMessageQueueAttr_t MCSAsyRxReqsQue_attributes = {
  .name = "MCSAsyRxReqsQue",
  .cb_mem = &MCSAsyRxReqsQueCtrlBlk,
  .cb_size = sizeof(MCSAsyRxReqsQueCtrlBlk),
  .mq_mem = &MCSAsyRxReqsQueBuf,
  .mq_size = sizeof(MCSAsyRxReqsQueBuf)
};
/* Definitions for MCSAsyTxReqsQue */
osMessageQueueId_t MCSAsyTxReqsQueHandle;
uint8_t MCSAsyTxReqsQueBuf[ 4 * sizeof( tpSMCSAsynchRxTxMsg ) ];
osStaticMessageQDef_t MCSAsyTxReqsQueCtrlBlk;
const osMessageQueueAttr_t MCSAsyTxReqsQue_attributes = {
  .name = "MCSAsyTxReqsQue",
  .cb_mem = &MCSAsyTxReqsQueCtrlBlk,
  .cb_size = sizeof(MCSAsyTxReqsQueCtrlBlk),
  .mq_mem = &MCSAsyTxReqsQueBuf,
  .mq_size = sizeof(MCSAsyTxReqsQueBuf)
};
/* Definitions for UIRxReqsQue */
osMessageQueueId_t UIRxReqsQueHandle;
uint8_t UIRxReqsQueBuf[ 4 * sizeof( tpSUIRequest ) ];
osStaticMessageQDef_t UIRxReqsQueCtrlBlk;
const osMessageQueueAttr_t UIRxReqsQue_attributes = {
  .name = "UIRxReqsQue",
  .cb_mem = &UIRxReqsQueCtrlBlk,
  .cb_size = sizeof(UIRxReqsQueCtrlBlk),
  .mq_mem = &UIRxReqsQueBuf,
  .mq_size = sizeof(UIRxReqsQueBuf)
};
/* Definitions for UITxReqsQue */
osMessageQueueId_t UITxReqsQueHandle;
uint8_t UITxReqsQueBuf[ 4 * sizeof( tpSUIRequest ) ];
osStaticMessageQDef_t UITxReqsQueCtrlBlk;
const osMessageQueueAttr_t UITxReqsQue_attributes = {
  .name = "UITxReqsQue",
  .cb_mem = &UITxReqsQueCtrlBlk,
  .cb_size = sizeof(UITxReqsQueCtrlBlk),
  .mq_mem = &UITxReqsQueBuf,
  .mq_size = sizeof(UITxReqsQueBuf)
};
/* Definitions for MSMReqsQue */
osMessageQueueId_t MSMReqsQueHandle;
uint8_t MSMReqsQueBuf[ 16 * sizeof( tpSMSMRequest ) ];
osStaticMessageQDef_t MSMReqsQueCtrlBlk;
const osMessageQueueAttr_t MSMReqsQue_attributes = {
  .name = "MSMReqsQue",
  .cb_mem = &MSMReqsQueCtrlBlk,
  .cb_size = sizeof(MSMReqsQueCtrlBlk),
  .mq_mem = &MSMReqsQueBuf,
  .mq_size = sizeof(MSMReqsQueBuf)
};
/* Definitions for LogReqsQue */
osMessageQueueId_t LogReqsQueHandle;
uint8_t LogReqsQueBuf[ 16 * sizeof( tpSLogReq ) ];
osStaticMessageQDef_t LogReqsQueCtrlBlk;
const osMessageQueueAttr_t LogReqsQue_attributes = {
  .name = "LogReqsQue",
  .cb_mem = &LogReqsQueCtrlBlk,
  .cb_size = sizeof(LogReqsQueCtrlBlk),
  .mq_mem = &LogReqsQueBuf,
  .mq_size = sizeof(LogReqsQueBuf)
};
/* Definitions for IAPRxReqsQue */
osMessageQueueId_t IAPRxReqsQueHandle;
uint8_t IAPRxReqsQueBuf[ 2 * sizeof( tpSIAPRequest ) ];
osStaticMessageQDef_t IAPRxReqsQueCtrlBlk;
const osMessageQueueAttr_t IAPRxReqsQue_attributes = {
  .name = "IAPRxReqsQue",
  .cb_mem = &IAPRxReqsQueCtrlBlk,
  .cb_size = sizeof(IAPRxReqsQueCtrlBlk),
  .mq_mem = &IAPRxReqsQueBuf,
  .mq_size = sizeof(IAPRxReqsQueBuf)
};
/* Definitions for PPPOSAsyRxReqsQue */
osMessageQueueId_t PPPOSAsyRxReqsQueHandle;
uint8_t PPPOSAsyRxReqsQueBuf[ 4 * sizeof( tpSPPPOSAsynchRxTxMsg ) ];
osStaticMessageQDef_t PPPOSAsyRxReqsQueCtrlBlk;
const osMessageQueueAttr_t PPPOSAsyRxReqsQue_attributes = {
  .name = "PPPOSAsyRxReqsQue",
  .cb_mem = &PPPOSAsyRxReqsQueCtrlBlk,
  .cb_size = sizeof(PPPOSAsyRxReqsQueCtrlBlk),
  .mq_mem = &PPPOSAsyRxReqsQueBuf,
  .mq_size = sizeof(PPPOSAsyRxReqsQueBuf)
};
/* Definitions for PPPOSAsyTxReqsQue */
osMessageQueueId_t PPPOSAsyTxReqsQueHandle;
uint8_t PPPOSAsyTxReqsQueBuf[ 4 * sizeof( tpSPPPOSAsynchRxTxMsg ) ];
osStaticMessageQDef_t PPPOSAsyTxReqsQueCtrlBlk;
const osMessageQueueAttr_t PPPOSAsyTxReqsQue_attributes = {
  .name = "PPPOSAsyTxReqsQue",
  .cb_mem = &PPPOSAsyTxReqsQueCtrlBlk,
  .cb_size = sizeof(PPPOSAsyTxReqsQueCtrlBlk),
  .mq_mem = &PPPOSAsyTxReqsQueBuf,
  .mq_size = sizeof(PPPOSAsyTxReqsQueBuf)
};
/* Definitions for TimeMutex */
osMutexId_t TimeMutexHandle;
osStaticMutexDef_t TimeMutexCtrlBlk;
const osMutexAttr_t TimeMutex_attributes = {
  .name = "TimeMutex",
  .cb_mem = &TimeMutexCtrlBlk,
  .cb_size = sizeof(TimeMutexCtrlBlk),
};
/* Definitions for SignalGroupsMutex */
osMutexId_t SignalGroupsMutexHandle;
osStaticMutexDef_t SignalGroupsMutexCtrlBlk;
const osMutexAttr_t SignalGroupsMutex_attributes = {
  .name = "SignalGroupsMutex",
  .cb_mem = &SignalGroupsMutexCtrlBlk,
  .cb_size = sizeof(SignalGroupsMutexCtrlBlk),
};
/* Definitions for LogMutex */
osMutexId_t LogMutexHandle;
osStaticMutexDef_t LogMutexCtrlBlk;
const osMutexAttr_t LogMutex_attributes = {
  .name = "LogMutex",
  .cb_mem = &LogMutexCtrlBlk,
  .cb_size = sizeof(LogMutexCtrlBlk),
};
/* Definitions for LCDMutex */
osMutexId_t LCDMutexHandle;
osStaticMutexDef_t LCDMutexCtrlBlk;
const osMutexAttr_t LCDMutex_attributes = {
  .name = "LCDMutex",
  .cb_mem = &LCDMutexCtrlBlk,
  .cb_size = sizeof(LCDMutexCtrlBlk),
};
/* Definitions for UART4Event */
osEventFlagsId_t UART4EventHandle;
osStaticEventGroupDef_t UART4EventCtrlBlk;
const osEventFlagsAttr_t UART4Event_attributes = {
  .name = "UART4Event",
  .cb_mem = &UART4EventCtrlBlk,
  .cb_size = sizeof(UART4EventCtrlBlk),
};
/* Definitions for PPPAsyEvent */
osEventFlagsId_t PPPAsyEventHandle;
osStaticEventGroupDef_t PPPAsyEventCtrlBlk;
const osEventFlagsAttr_t PPPAsyEvent_attributes = {
  .name = "PPPAsyEvent",
  .cb_mem = &PPPAsyEventCtrlBlk,
  .cb_size = sizeof(PPPAsyEventCtrlBlk),
};
/* Definitions for TCPClientEvent */
osEventFlagsId_t TCPClientEventHandle;
osStaticEventGroupDef_t TCPClientEventCtrlBlk;
const osEventFlagsAttr_t TCPClientEvent_attributes = {
  .name = "TCPClientEvent",
  .cb_mem = &TCPClientEventCtrlBlk,
  .cb_size = sizeof(TCPClientEventCtrlBlk),
};
/* Definitions for DNSEvent */
osEventFlagsId_t DNSEventHandle;
osStaticEventGroupDef_t DNSEventCtrlBlk;
const osEventFlagsAttr_t DNSEvent_attributes = {
  .name = "DNSEvent",
  .cb_mem = &DNSEventCtrlBlk,
  .cb_size = sizeof(DNSEventCtrlBlk),
};
/* Definitions for MCSAsyEvent */
osEventFlagsId_t MCSAsyEventHandle;
osStaticEventGroupDef_t MCSAsyEventCtrlBlk;
const osEventFlagsAttr_t MCSAsyEvent_attributes = {
  .name = "MCSAsyEvent",
  .cb_mem = &MCSAsyEventCtrlBlk,
  .cb_size = sizeof(MCSAsyEventCtrlBlk),
};
/* Definitions for MaintenanceEvent */
osEventFlagsId_t MaintenanceEventHandle;
osStaticEventGroupDef_t MaintenanceEventCtrlBlk;
const osEventFlagsAttr_t MaintenanceEvent_attributes = {
  .name = "MaintenanceEvent",
  .cb_mem = &MaintenanceEventCtrlBlk,
  .cb_size = sizeof(MaintenanceEventCtrlBlk),
};
/* Definitions for I2C1Event */
osEventFlagsId_t I2C1EventHandle;
osStaticEventGroupDef_t I2C1EventCtrlBlk;
const osEventFlagsAttr_t I2C1Event_attributes = {
  .name = "I2C1Event",
  .cb_mem = &I2C1EventCtrlBlk,
  .cb_size = sizeof(I2C1EventCtrlBlk),
};
/* Definitions for I2C4Event */
osEventFlagsId_t I2C4EventHandle;
osStaticEventGroupDef_t I2C4EventCtrlBlk;
const osEventFlagsAttr_t I2C4Event_attributes = {
  .name = "I2C4Event",
  .cb_mem = &I2C4EventCtrlBlk,
  .cb_size = sizeof(I2C4EventCtrlBlk),
};
/* Definitions for StandbyEvent */
osEventFlagsId_t StandbyEventHandle;
osStaticEventGroupDef_t StandbyEventCtrlBlk;
const osEventFlagsAttr_t StandbyEvent_attributes = {
  .name = "StandbyEvent",
  .cb_mem = &StandbyEventCtrlBlk,
  .cb_size = sizeof(StandbyEventCtrlBlk),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern void MainApplication_Init(void);
/* USER CODE END FunctionPrototypes */

void DefaultTaskFunc(void *argument);
extern void TimeTaskFunc(void *argument);
extern void CANMsgParserTaskFunc(void *argument);
extern void CANMsgSenderTaskFunc(void *argument);
extern void SignalCardDriveTaskFunc(void *argument);
extern void IntersectionControlTaskFunc(void *argument);
extern void CPMPComTaskFunc(void *argument);
extern void GPSMsgParserTaskFunc(void *argument);
extern void GPSTaskFunc(void *argument);
extern void MMITaskFunc(void *argument);
extern void MCSTaskFunc(void *argument);
extern void MCSAsyMsgParserTaskFunc(void *argument);
extern void MCSAsyMsgSenderTaskFunc(void *argument);
extern void MCSAsyTaskFunc(void *argument);
extern void UIMsgParserTaskFunc(void *argument);
extern void UIMsgSenderTaskFunc(void *argument);
extern void LCDTaskFunc(void *argument);
extern void ProgramTaskFunc(void *argument);
extern void MLMTaskFunc(void *argument);
extern void MSMTaskFunc(void *argument);
extern void IAPMsgParserTaskFunc(void *argument);
extern void PPPOSAsyMsgParserTaskFunc(void *argument);
extern void PPPOSAsyMsgSenderTaskFunc(void *argument);
extern void MaintenanceTaskFunc(void *argument);

extern void MX_USB_DEVICE_Init(void);
extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, char *pcTaskName);

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, char *pcTaskName)
{
  /* Run time stack overflow checking is performed if
   *  configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   *  called if a stack overflow is detected. */
  UNUSED(pcTaskName);

  tEAppTasks eTask = APP_TASK_NONE;

  if (xTask == MaintenanceTaskHandle)
  {
    eTask = APP_TASK_MAINTENANCE;
  }
  else if (xTask == TimeTaskHandle)
  {
    eTask = APP_TASK_TIME;
  }
  else if (xTask == CANMsgParserTaskHandle)
  {
    eTask = APP_TASK_CAN_MSG_PARSER;
  }
  else if (xTask == CANMsgSenderTaskHandle)
  {
    eTask = APP_TASK_CAN_MSG_SENDER;
  }
  else if (xTask == SignalCardDriveTaskHandle)
  {
    eTask = APP_TASK_SIGNAL_CARD_DRIVE;
  }
  else if (xTask == IntersectionControlTaskHandle)
  {
    eTask = APP_TASK_INTERSECTION_CONTROL;
  }
  else if (xTask == CPMPComTaskHandle)
  {
    eTask = APP_TASK_CPMP_COMM;
  }
  else if (xTask == GPSMsgParserTaskHandle)
  {
    eTask = APP_TASK_GPS_MSG_PARSER;
  }
  else if (xTask == GPSTaskHandle)
  {
    eTask = APP_TASK_GPS;
  }
  else if (xTask == MMITaskHandle)
  {
    eTask = APP_TASK_MMI;
  }
  else if (xTask == MCSTaskHandle)
  {
    eTask = APP_TASK_MCS;
  }
  else if (xTask == MCSAsyMsgParserTaskHandle)
  {
    eTask = APP_TASK_MCS_ASYNCH_MSG_PARSER;
  }
  else if (xTask == MCSAsyMsgSenderTaskHandle)
  {
    eTask = APP_TASK_MCS_ASYNCH_MSG_SENDER;
  }
  else if (xTask == MCSAsyTaskHandle)
  {
    eTask = APP_TASK_MCS_ASYNCH;
  }
  else if (xTask == UIMsgParserTaskHandle)
  {
    eTask = APP_TASK_UI_MSG_PARSER;
  }
  else if (xTask == UIMsgSenderTaskHandle)
  {
    eTask = APP_TASK_UI_MSG_SENDER;
  }
  else if (xTask == LCDTaskHandle)
  {
    eTask = APP_TASK_LCD;
  }
  else if (xTask == ProgramTaskHandle)
  {
    eTask = APP_TASK_PROGRAM;
  }
  else if (xTask == MLMTaskHandle)
  {
    eTask = APP_TASK_MLM;
  }
  else if (xTask == IAPMsgParserTaskHandle)
  {
    eTask = APP_TASK_IAP;
  }
  else if (xTask == MSMTaskHandle)
  {
    eTask = APP_TASK_MSM;
  }

  LogRequest(LOG_REQ_APPEND_ASYNCH,
             NULL,
             EVENT_TASK_STACK_OVERFLOW,
             (uint8_t) eTask,
             0,
             0,
             0);

  osDelay(100);

  SystemReset();
} /* vApplicationStackOverflowHook */

/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  
  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of TimeMutex */
  TimeMutexHandle = osMutexNew(&TimeMutex_attributes);

  /* creation of SignalGroupsMutex */
  SignalGroupsMutexHandle = osMutexNew(&SignalGroupsMutex_attributes);

  /* creation of LogMutex */
  LogMutexHandle = osMutexNew(&LogMutex_attributes);

  /* creation of LCDMutex */
  LCDMutexHandle = osMutexNew(&LCDMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  if (TimeMutexHandle == NULL)
  {
    Error_Handler();
  }

  if (SignalGroupsMutexHandle == NULL)
  {
    Error_Handler();
  }

  if (LogMutexHandle == NULL)
  {
    Error_Handler();
  }

  if (LCDMutexHandle == NULL)
  {
    Error_Handler();
  }

  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of FDCANRxReqsQue */
  FDCANRxReqsQueHandle = osMessageQueueNew (32, sizeof(tpSFDCANRxMsg), &FDCANRxReqsQue_attributes);

  /* creation of FDCANTxReqsQue */
  FDCANTxReqsQueHandle = osMessageQueueNew (32, sizeof(tpSFDCANTxMsg), &FDCANTxReqsQue_attributes);

  /* creation of GPSRxReqsQue */
  GPSRxReqsQueHandle = osMessageQueueNew (2, sizeof(tpSGPSRxMsg), &GPSRxReqsQue_attributes);

  /* creation of GPSTimeQue */
  GPSTimeQueHandle = osMessageQueueNew (8, sizeof(tpSTime), &GPSTimeQue_attributes);

  /* creation of MMIReqsQue */
  MMIReqsQueHandle = osMessageQueueNew (16, sizeof(tpSFDCANRxMsg), &MMIReqsQue_attributes);

  /* creation of MCSAsyRxReqsQue */
  MCSAsyRxReqsQueHandle = osMessageQueueNew (4, sizeof(tpSMCSAsynchRxTxMsg), &MCSAsyRxReqsQue_attributes);

  /* creation of MCSAsyTxReqsQue */
  MCSAsyTxReqsQueHandle = osMessageQueueNew (4, sizeof(tpSMCSAsynchRxTxMsg), &MCSAsyTxReqsQue_attributes);

  /* creation of UIRxReqsQue */
  UIRxReqsQueHandle = osMessageQueueNew (4, sizeof(tpSUIRequest), &UIRxReqsQue_attributes);

  /* creation of UITxReqsQue */
  UITxReqsQueHandle = osMessageQueueNew (4, sizeof(tpSUIRequest), &UITxReqsQue_attributes);

  /* creation of MSMReqsQue */
  MSMReqsQueHandle = osMessageQueueNew (16, sizeof(tpSMSMRequest), &MSMReqsQue_attributes);

  /* creation of LogReqsQue */
  LogReqsQueHandle = osMessageQueueNew (16, sizeof(tpSLogReq), &LogReqsQue_attributes);

  /* creation of IAPRxReqsQue */
  IAPRxReqsQueHandle = osMessageQueueNew (2, sizeof(tpSIAPRequest), &IAPRxReqsQue_attributes);

  /* creation of PPPOSAsyRxReqsQue */
  PPPOSAsyRxReqsQueHandle = osMessageQueueNew (4, sizeof(tpSPPPOSAsynchRxTxMsg), &PPPOSAsyRxReqsQue_attributes);

  /* creation of PPPOSAsyTxReqsQue */
  PPPOSAsyTxReqsQueHandle = osMessageQueueNew (4, sizeof(tpSPPPOSAsynchRxTxMsg), &PPPOSAsyTxReqsQue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  if (FDCANRxReqsQueHandle == NULL)
  {
    Error_Handler();
  }

  if (FDCANTxReqsQueHandle == NULL)
  {
    Error_Handler();
  }

  if (GPSRxReqsQueHandle == NULL)
  {
    Error_Handler();
  }

  if (GPSTimeQueHandle == NULL)
  {
    Error_Handler();
  }

  if (MMIReqsQueHandle == NULL)
  {
    Error_Handler();
  }

  if (MCSAsyRxReqsQueHandle == NULL)
  {
    Error_Handler();
  }

  if (MCSAsyTxReqsQueHandle == NULL)
  {
    Error_Handler();
  }

  if (UIRxReqsQueHandle == NULL)
  {
    Error_Handler();
  }

  if (UITxReqsQueHandle == NULL)
  {
    Error_Handler();
  }

  if (MSMReqsQueHandle == NULL)
  {
    Error_Handler();
  }

  if (LogReqsQueHandle == NULL)
  {
    Error_Handler();
  }

  if (IAPRxReqsQueHandle == NULL)
  {
    Error_Handler();
  }
	
	if (PPPOSAsyRxReqsQueHandle == NULL)
  {
    Error_Handler();
  }

  if (PPPOSAsyTxReqsQueHandle == NULL)
  {
    Error_Handler();
  }

  /* add queues, ... */
  /* creation of FDCANRxReqsMemPool */
  FDCANRxReqsMemPoolHandle = osMemoryPoolNew(32,
                                             sizeof(tSFDCANRxMsg),
                                             &FDCANRxReqsMemPool_attributes);

  /* creation of FDCANTxReqsMemPool */
  FDCANTxReqsMemPoolHandle = osMemoryPoolNew(32,
                                             sizeof(tSFDCANTxMsg),
                                             &FDCANTxReqsMemPool_attributes);

  /* creation of GPSRxReqMemPool */
  GPSRxReqsMemPoolHandle = osMemoryPoolNew(2,
                                           sizeof(tSGPSRxMsg),
                                           &GPSRxReqsMemPool_attributes);

  /* creation of GPSTimeMemPool */
  GPSTimeMemPoolHandle = osMemoryPoolNew(8,
                                         sizeof(tSTime),
                                         &GPSTimeMemPool_attributes);

  /* creation of MMIReqsMemPool */
  MMIReqsMemPoolHandle = osMemoryPoolNew(16,
                                         sizeof(tSFDCANRxMsg),
                                         &MMIReqsMemPool_attributes);

  /* creation of MCSAsyRxReqsMemPool */
  MCSAsyRxReqsMemPoolHandle = osMemoryPoolNew(4,
                                              sizeof(tSMCSAsynchRxTxMsg),
                                              &MCSAsyRxReqsMemPool_attributes);

  /* creation of MCSAsyTxReqsMemPool */
  MCSAsyTxReqsMemPoolHandle = osMemoryPoolNew(4,
                                              sizeof(tSMCSAsynchRxTxMsg),
                                              &MCSAsyTxReqsMemPool_attributes);

  /* creation of UIRxReqsMemPool */
  UIRxReqsMemPoolHandle = osMemoryPoolNew(4,
                                          sizeof(tSUIRequest),
                                          &UIRxReqsMemPool_attributes);

  /* creation of UITxReqsMemPool */
  UITxReqsMemPoolHandle = osMemoryPoolNew(4,
                                          sizeof(tSUIRequest),
                                          &UITxReqsMemPool_attributes);

  /* creation of MSMReqsMemPool */
  MSMReqsMemPoolHandle = osMemoryPoolNew(16,
                                         sizeof(tSMSMRequest),
                                         &MSMReqsMemPool_attributes);

  /* creation of LogReqsMemPool */
  LogReqsMemPoolHandle = osMemoryPoolNew(16,
                                         sizeof(tSLogReq),
                                         &LogReqsMemPool_attributes);

  /* creation of IAPRxReqsMemPool */
  IAPRxReqsMemPoolHandle = osMemoryPoolNew(2,
                                           sizeof(tSIAPRequest),
                                           &IAPRxReqsMemPool_attributes);
																					 
	/* creation of PPPOSAsyRxReqsMemPool */
  PPPOSAsyRxReqsMemPoolHandle = osMemoryPoolNew(4,
                                              sizeof(tSPPPOSAsynchRxTxMsg),
                                              &PPPOSAsyRxReqsMemPool_attributes);

  /* creation of PPPOSAsyTxReqsMemPool */
  PPPOSAsyTxReqsMemPoolHandle = osMemoryPoolNew(4,
                                              sizeof(tSPPPOSAsynchRxTxMsg),
                                              &PPPOSAsyTxReqsMemPool_attributes);

  if (FDCANRxReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (FDCANTxReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (GPSRxReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (GPSTimeMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (MMIReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (MCSAsyRxReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (MCSAsyTxReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (UIRxReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (UITxReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (MSMReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (LogReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (IAPRxReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }
	
	if (PPPOSAsyRxReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  if (PPPOSAsyTxReqsMemPoolHandle == NULL)
  {
    Error_Handler();
  }

  /* USER CODE END RTOS_QUEUES */

  /* USER CODE BEGIN RTOS_PRE_THREADS */
  MainApplication_Init(); /* build all adapter-backed global ports before any
                           * application task can consume them */
  /* USER CODE END RTOS_PRE_THREADS */

  /* Create the thread(s) */
  /* creation of DefaultTask */
  DefaultTaskHandle = osThreadNew(DefaultTaskFunc, NULL, &DefaultTask_attributes);

  /* creation of TimeTask */
  TimeTaskHandle = osThreadNew(TimeTaskFunc, NULL, &TimeTask_attributes);

  /* creation of CANMsgParserTask */
  CANMsgParserTaskHandle = osThreadNew(CANMsgParserTaskFunc, NULL, &CANMsgParserTask_attributes);

  /* creation of CANMsgSenderTask */
  CANMsgSenderTaskHandle = osThreadNew(CANMsgSenderTaskFunc, NULL, &CANMsgSenderTask_attributes);

  /* legacy signal-card task intentionally disabled:
   * the clean FieldOutputCanAdapter on IntersectionControlTask owns
   * FDCAN1 SSM/PSM publishing now. */
  SignalCardDriveTaskHandle = NULL;

  /* creation of IntersectionControlTask */
  IntersectionControlTaskHandle = osThreadNew(IntersectionControlTaskFunc, NULL, &IntersectionControlTask_attributes);

  /* legacy CP<->MP comm task intentionally disabled:
   * the clean CpMpLinkService on FDCAN2 owns this path now. */
  CPMPComTaskHandle = NULL;

  /* creation of GPSMsgParserTask */
  GPSMsgParserTaskHandle = osThreadNew(GPSMsgParserTaskFunc, NULL, &GPSMsgParserTask_attributes);

  /* creation of GPSTask */
  GPSTaskHandle = osThreadNew(GPSTaskFunc, NULL, &GPSTask_attributes);

  /* creation of MMITask */
  MMITaskHandle = osThreadNew(MMITaskFunc, NULL, &MMITask_attributes);

  /* creation of MCSTask */
  MCSTaskHandle = osThreadNew(MCSTaskFunc, NULL, &MCSTask_attributes);

  /* creation of MCSAsyMsgParserTask */
  MCSAsyMsgParserTaskHandle = osThreadNew(MCSAsyMsgParserTaskFunc, NULL, &MCSAsyMsgParserTask_attributes);

  /* creation of MCSAsyMsgSenderTask */
  MCSAsyMsgSenderTaskHandle = osThreadNew(MCSAsyMsgSenderTaskFunc, NULL, &MCSAsyMsgSenderTask_attributes);

  /* creation of MCSAsyTask */
  MCSAsyTaskHandle = osThreadNew(MCSAsyTaskFunc, NULL, &MCSAsyTask_attributes);

  /* creation of UIMsgParserTask */
  UIMsgParserTaskHandle = osThreadNew(UIMsgParserTaskFunc, NULL, &UIMsgParserTask_attributes);

  /* creation of UIMsgSenderTask */
  UIMsgSenderTaskHandle = osThreadNew(UIMsgSenderTaskFunc, NULL, &UIMsgSenderTask_attributes);

  /* creation of LCDTask */
  LCDTaskHandle = osThreadNew(LCDTaskFunc, NULL, &LCDTask_attributes);

  /* creation of ProgramTask */
  ProgramTaskHandle = osThreadNew(ProgramTaskFunc, NULL, &ProgramTask_attributes);

  /* creation of MLMTask */
  MLMTaskHandle = osThreadNew(MLMTaskFunc, NULL, &MLMTask_attributes);

  /* creation of MSMTask */
  MSMTaskHandle = osThreadNew(MSMTaskFunc, NULL, &MSMTask_attributes);

  /* creation of IAPMsgParserTask */
  IAPMsgParserTaskHandle = osThreadNew(IAPMsgParserTaskFunc, NULL, &IAPMsgParserTask_attributes);

  /* creation of PPPOSAsyMsgParserTask */
  PPPOSAsyMsgParserTaskHandle = osThreadNew(PPPOSAsyMsgParserTaskFunc, NULL, &PPPOSAsyMsgParserTask_attributes);

  /* creation of PPPOSAsyMsgSenderTask */
  PPPOSAsyMsgSenderTaskHandle = osThreadNew(PPPOSAsyMsgSenderTaskFunc, NULL, &PPPOSAsyMsgSenderTask_attributes);

  /* creation of MaintenanceTask */
  MaintenanceTaskHandle = osThreadNew(MaintenanceTaskFunc, NULL, &MaintenanceTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  if (DefaultTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (TimeTaskHandle == NULL)
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

  if (GPSMsgParserTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (GPSTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (MMITaskHandle == NULL)
  {
    Error_Handler();
  }

  if (MCSTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (MCSAsyMsgParserTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (MCSAsyMsgSenderTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (MCSAsyTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (UIMsgParserTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (UIMsgSenderTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (LCDTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (ProgramTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (MLMTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (MSMTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (IAPMsgParserTaskHandle == NULL)
  {
    Error_Handler();
  }
	
	if (PPPOSAsyMsgParserTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (PPPOSAsyMsgSenderTaskHandle == NULL)
  {
    Error_Handler();
  }

  if (MaintenanceTaskHandle == NULL)
  {
    Error_Handler();
  }

  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* creation of UART4Event */
  UART4EventHandle = osEventFlagsNew(&UART4Event_attributes);

  /* creation of PPPAsyEvent */
  PPPAsyEventHandle = osEventFlagsNew(&PPPAsyEvent_attributes);

  /* creation of TCPClientEvent */
  TCPClientEventHandle = osEventFlagsNew(&TCPClientEvent_attributes);

  /* creation of DNSEvent */
  DNSEventHandle = osEventFlagsNew(&DNSEvent_attributes);

  /* creation of MCSAsyEvent */
  MCSAsyEventHandle = osEventFlagsNew(&MCSAsyEvent_attributes);

  /* creation of MaintenanceEvent */
  MaintenanceEventHandle = osEventFlagsNew(&MaintenanceEvent_attributes);

  /* creation of I2C1Event */
  I2C1EventHandle = osEventFlagsNew(&I2C1Event_attributes);

  /* creation of I2C4Event */
  I2C4EventHandle = osEventFlagsNew(&I2C4Event_attributes);

  /* creation of StandbyEvent */
  StandbyEventHandle = osEventFlagsNew(&StandbyEvent_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  if (UART4EventHandle == NULL)
  {
    Error_Handler();
  }

  if (PPPAsyEventHandle == NULL)
  {
    Error_Handler();
  }

  if (TCPClientEventHandle == NULL)
  {
    Error_Handler();
  }

  if (DNSEventHandle == NULL)
  {
    Error_Handler();
  }

  if (MCSAsyEventHandle == NULL)
  {
    Error_Handler();
  }

  if (MaintenanceEventHandle == NULL)
  {
    Error_Handler();
  }

  if (I2C1EventHandle == NULL)
  {
    Error_Handler();
  }

  if (I2C4EventHandle == NULL)
  {
    Error_Handler();
  }

  if (StandbyEventHandle == NULL)
  {
    Error_Handler();
  }

  /* add events, ... */
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
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN DefaultTaskFunc */
  DataInit(0, FALSE);  

  IWDGInit();

  /* Infinite loop */
  for (;;)
  {
    if (osEventFlagsWait(StandbyEventHandle, EVENT_FLAGS_STANDBY_100HZ_MISSING,
                         osFlagsWaitAll,
                         osWaitForever) == EVENT_FLAGS_STANDBY_100HZ_MISSING)
    {
      ExecStandbyInfoOps();
    }
  }

  /* USER CODE END DefaultTaskFunc */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

