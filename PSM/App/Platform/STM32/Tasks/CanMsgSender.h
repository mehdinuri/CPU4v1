/**
 ******************************************************************************
 * @file           : CanMsgSender.h
 * @brief          : Header for CanMsgSender.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_PLATFORM_STM32_TASKS_CAN_MSG_SENDER_H
#define APP_PLATFORM_STM32_TASKS_CAN_MSG_SENDER_H

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "fdcan.h"

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/
extern void CANTxRequest(FDCAN_HandleTypeDef *hfdcan,
                         uint32_t idType,
                         uint32_t id,
                         uint32_t frameType,
                         uint32_t bitRateSwitch,
                         uint32_t fdFormat,
                         uint8_t *data,
                         uint8_t dataLen);

#endif /* APP_PLATFORM_STM32_TASKS_CAN_MSG_SENDER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
