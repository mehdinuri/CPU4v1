/**
 ******************************************************************************
 * @file           : CanMsgParser.h
 * @brief          : Header for CanMsgParser.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_PLATFORM_STM32_TASKS_CAN_MSG_PARSER_H
#define APP_PLATFORM_STM32_TASKS_CAN_MSG_PARSER_H

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "fdcan.h"

/* Public define ------------------------------------------------------------*/

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/

/* Public type values -------------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/
extern void CANRxRequest(tpSFDCANRxMsg pSRxMsg);

#endif /* APP_PLATFORM_STM32_TASKS_CAN_MSG_PARSER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
