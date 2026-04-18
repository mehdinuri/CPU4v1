/**
 ******************************************************************************
 * @file           : can_msg_sender.h
 * @brief          : Header for can_msg_sender.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_MSG_SENDER_H__
#define __CAN_MSG_SENDER_H__

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os2.h"
#include "fdcan.h"
/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/
extern uint8_t CANTxRequest(FDCAN_HandleTypeDef *hfdcan,
                            uint32_t lIDType,
                            uint32_t lID,
                            uint32_t lFrameType,
                            uint32_t lBitRateSwitch,
                            uint32_t lFDFormat,
                            const uint8_t *baData,
                            uint8_t bDataLen);
extern uint8_t CANTxFaultLatched(void);

#endif /* __CAN_MSG_SENDER_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
