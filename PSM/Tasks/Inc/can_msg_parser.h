/**
  ******************************************************************************
  * @file           : can_msg_parser.h
  * @brief          : Header for can_msg_parser.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_MSG_PARSER_H__
#define __CAN_MSG_PARSER_H__

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "fdcan.h"

/* Public define ------------------------------------------------------------*/

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/

/* Public type values -------------------------------------------------------------*/


/* Public function prototypes -----------------------------------------------*/
extern void CANRxRequest(tpSFDCANRxMsg pSRxMsg);

#endif /* __CAN_MSG_PARSER_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
