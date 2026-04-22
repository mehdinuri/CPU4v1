/* USER CODE BEGIN Header */

/**
 ******************************************************************************
 * @file           : defs.h
 * @brief          : This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DEFS_H
#define __DEFS_H

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define OFF 0
#define ON 1

#define LOW 0
#define HIGH 1

#define FOREVER 1

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

#define GetBitValue(data, bitindex) ((data & (1 << bitindex)) >> bitindex)
#define SetBitValue(data, bitindex) (data = (data | (1 << bitindex)))
#define ClearBitValue(data, bitindex) (data = ~((~data) | (1 << bitindex)))

#define mASCII2Number(ASCII) (ASCII - '0')
#define mNumber2ASCII(Number) ((unsigned char) Number + '0')

#define THREAD_FLAGS_UI_TX_COMPLETE FLAG_BIT_0
#define THREAD_FLAGS_GPS_TX_COMPLETE FLAG_BIT_1

#define THREAD_FLAGS_MLM_REQ_PROCESS_OK FLAG_BIT_3
#define THREAD_FLAGS_MLM_REQ_PROCESS_ERROR FLAG_BIT_4
#define THREAD_FLAGS_MLM_REQ_PROCESS_ALL (THREAD_FLAGS_MLM_REQ_PROCESS_OK | \
                                          THREAD_FLAGS_MLM_REQ_PROCESS_ERROR

#define THREAD_FLAGS_MSM_REQ_PROCESS_OK FLAG_BIT_5
#define THREAD_FLAGS_MSM_REQ_PROCESS_ERROR FLAG_BIT_6
#define THREAD_FLAGS_MSM_REQ_PROCESS_ALL (THREAD_FLAGS_MSM_REQ_PROCESS_OK | \
                                          THREAD_FLAGS_MSM_REQ_PROCESS_ERROR

#define EVENT_FLAGS_MAINTENANCE_TIME_TASK_ACTIVE FLAG_BIT_1
#define EVENT_FLAGS_MAINTENANCE_GPS_TASK_ACTIVE FLAG_BIT_4

#define EVENT_FLAGS_MAINTENANCE_ALL_TASKS_ACTIVE ( \
          EVENT_FLAGS_MAINTENANCE_TIME_TASK_ACTIVE \
          | \
          EVENT_FLAGS_MAINTENANCE_GPS_TASK_ACTIVE)

#define EVENT_FLAGS_UART4_TX_COMPLETE FLAG_BIT_0

#define EVENT_FLAGS_STANDBY_100HZ_MISSING FLAG_BIT_0
#define EVENT_FLAGS_STANDBY_INFO_MSG_SENT FLAG_BIT_1
#define EVENT_FLAGS_STANDBY_ALL (EVENT_FLAGS_STANDBY_100HZ_MISSING | \
                                 EVENT_FLAGS_STANDBY_INFO_MSG_SENT

#define EVENT_FLAGS_DNS_RESOLVED FLAG_BIT_0
#define EVENT_FLAGS_DNS_RESOLVE_ERROR FLAG_BIT_1
#define EVENT_FLAGS_DNS_RESOLVE_ALL (EVENT_FLAGS_DNS_RESOLVED | \
                                     EVENT_FLAGS_DNS_RESOLVE_ERROR

#define EVENT_FLAGS_PPP_ASY_CONNECTED FLAG_BIT_0

#define EVENT_FLAGS_I2C1_TX_COMPLETE FLAG_BIT_0
#define EVENT_FLAGS_I2C1_RX_COMPLETE FLAG_BIT_1
#define EVENT_FLAGS_I2C1_ERROR FLAG_BIT_2
#define EVENT_FLAGS_I2C1_ALL (EVENT_FLAGS_I2C1_TX_COMPLETE | \
                              EVENT_FLAGS_I2C1_RX_COMPLETE | \
                              EVENT_FLAGS_I2C1_ERROR)

#define EVENT_FLAGS_I2C4_TX_COMPLETE FLAG_BIT_0
#define EVENT_FLAGS_I2C4_RX_COMPLETE FLAG_BIT_1
#define EVENT_FLAGS_I2C4_ERROR FLAG_BIT_2
#define EVENT_FLAGS_I2C4_ALL (EVENT_FLAGS_I2C4_TX_COMPLETE | \
                              EVENT_FLAGS_I2C4_RX_COMPLETE | \
                              EVENT_FLAGS_I2C4_ERROR)

extern osThreadId_t DefaultTaskHandle;
extern osThreadId_t MaintenanceTaskHandle;
extern osThreadId_t MLMTaskHandle;
extern osThreadId_t MSMTaskHandle;
extern osThreadId_t LCDTaskHandle;
extern osThreadId_t UIMsgParserTaskHandle;
extern osThreadId_t FieldCanTxTaskHandle;
extern osThreadId_t GPSMsgParserTaskHandle;
extern osThreadId_t GPSTaskHandle;
extern osThreadId_t MCSTaskHandle;
extern osThreadId_t TimeTaskHandle;
extern osThreadId_t EthIfInputTaskHandle;
extern osThreadId_t EthLinkStateTaskHandle;
extern osThreadId_t UIMsgSenderTaskHandle;
extern osThreadId_t MaintenanceTaskHandle;
extern osThreadId_t PPPOSAsyMsgParserTaskHandle;
extern osThreadId_t PPPOSAsyMsgSenderTaskHandle;

extern osMemoryPoolId_t FDCANTxReqsMemPoolHandle;
extern osMemoryPoolId_t GPSRxReqsMemPoolHandle;
extern osMemoryPoolId_t GPSTimeMemPoolHandle;
extern osMemoryPoolId_t UIRxReqsMemPoolHandle;
extern osMemoryPoolId_t UITxReqsMemPoolHandle;
extern osMemoryPoolId_t MSMReqsMemPoolHandle;
extern osMemoryPoolId_t LogReqsMemPoolHandle;
extern osMemoryPoolId_t PPPOSAsyRxReqsMemPoolHandle;
extern osMemoryPoolId_t PPPOSAsyTxReqsMemPoolHandle;

extern osMessageQueueId_t FDCANTxReqsQueHandle;
extern osMessageQueueId_t GPSRxReqsQueHandle;
extern osMessageQueueId_t GPSTimeQueHandle;
extern osMessageQueueId_t UIRxReqsQueHandle;
extern osMessageQueueId_t UITxReqsQueHandle;
extern osMessageQueueId_t MSMReqsQueHandle;
extern osMessageQueueId_t LogReqsQueHandle;
extern osMessageQueueId_t PPPOSAsyRxReqsQueHandle;
extern osMessageQueueId_t PPPOSAsyTxReqsQueHandle;

extern osMutexId_t TimeMutexHandle;
extern osMutexId_t LogMutexHandle;
extern osMutexId_t LCDMutexHandle;

extern osEventFlagsId_t MaintenanceEventHandle;
extern osEventFlagsId_t UART4EventHandle;
extern osEventFlagsId_t PPPAsyEventHandle;
extern osEventFlagsId_t DNSEventHandle;
extern osEventFlagsId_t I2C1EventHandle;
extern osEventFlagsId_t I2C4EventHandle;
extern osEventFlagsId_t MLMEventHandle;
extern osEventFlagsId_t MSMEventHandle;
extern osEventFlagsId_t StandbyEventHandle;

/* #define DEBUG */

#ifdef DEBUG
/* #define RTOS_TRACE */
/* #define LWIP_TRACE */

#endif
/* USER CODE END Private defines */

#endif /* __DEFS_H */
