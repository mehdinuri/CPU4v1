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
#include "cmsis_os2.h"
#include "fdcan.h"

/* Public define ------------------------------------------------------------*/
#define SIGNAL_GROUPS_PER_SSM (uint8_t) 4
#define SIGNAL_OUTPUTS_PER_SSM (uint8_t) (SIGNAL_OUTPUTS_PER_SIGNAL_GROUP \
                                          * SIGNAL_GROUPS_PER_SSM)                                 /* 12 */

#define SSM_MODULES_PER_SSM_GROUP (uint8_t) 4
#define SIGNAL_GROUPS_PER_SSM_GROUP (uint8_t) (SIGNAL_GROUPS_PER_SSM \
                                               * SSM_MODULES_PER_SSM_GROUP)                       /* 16 */
#define SIGNAL_OUTPUTS_PER_SSM_GROUP (uint8_t) (SIGNAL_GROUPS_PER_SSM_GROUP \
                                                * \
                                                SIGNAL_OUTPUTS_PER_SIGNAL_GROUP)                               /* 48 */

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/

/* Public type values -------------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/
extern void CANRxRequest(tpSFDCANRxMsg pSRxMsg);
extern uint8_t CANFlashStatusGet(void);
extern uint8_t CANFlashSyncStatusGet(void);
extern GPIO_PinState CANSignalOutputStateGet(uint8_t bGroupIdx,
                                             uint8_t bOutputIdx);
extern GPIO_PinState CANSignalOutputFlashStateGet(uint8_t bGroupIdx,
                                                  uint8_t bOutputIdx);
extern void CANSignalOutputFlashConfigCheck(void);

#endif /* __CAN_MSG_PARSER_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
