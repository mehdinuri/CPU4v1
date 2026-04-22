/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define GPIOSetR1(state) HAL_GPIO_WritePin(R1_GPIO_Port, R1_Pin, state)
#define GPIOSetY1(state) HAL_GPIO_WritePin(Y1_GPIO_Port, Y1_Pin, state)
#define GPIOSetG1(state) HAL_GPIO_WritePin(G1_GPIO_Port, G1_Pin, state)
#define GPIOSetR2(state) HAL_GPIO_WritePin(R2_GPIO_Port, R2_Pin, state)
#define GPIOSetY2(state) HAL_GPIO_WritePin(Y2_GPIO_Port, Y2_Pin, state)
#define GPIOSetG2(state) HAL_GPIO_WritePin(G2_GPIO_Port, G2_Pin, state)
#define GPIOSetR3(state) HAL_GPIO_WritePin(R3_GPIO_Port, R3_Pin, state)
#define GPIOSetY3(state) HAL_GPIO_WritePin(Y3_GPIO_Port, Y3_Pin, state)
#define GPIOSetG3(state) HAL_GPIO_WritePin(G3_GPIO_Port, G3_Pin, state)
#define GPIOSetR4(state) HAL_GPIO_WritePin(R4_GPIO_Port, R4_Pin, state)
#define GPIOSetY4(state) HAL_GPIO_WritePin(Y4_GPIO_Port, Y4_Pin, state)
#define GPIOSetG4(state) HAL_GPIO_WritePin(G4_GPIO_Port, G4_Pin, state)

#define GPIOGetR1_V(void) HAL_GPIO_ReadPin(R1_V_GPIO_Port, R1_V_Pin)
#define GPIOGetY1_V(void) HAL_GPIO_ReadPin(Y1_V_GPIO_Port, Y1_V_Pin)
#define GPIOGetG1_V(void) HAL_GPIO_ReadPin(G1_V_GPIO_Port, G1_V_Pin)
#define GPIOGetR2_V(void) HAL_GPIO_ReadPin(R2_V_GPIO_Port, R2_V_Pin)
#define GPIOGetY2_V(void) HAL_GPIO_ReadPin(Y2_V_GPIO_Port, Y2_V_Pin)
#define GPIOGetG2_V(void) HAL_GPIO_ReadPin(G2_V_GPIO_Port, G2_V_Pin)
#define GPIOGetR3_V(void) HAL_GPIO_ReadPin(R3_V_GPIO_Port, R3_V_Pin)
#define GPIOGetY3_V(void) HAL_GPIO_ReadPin(Y3_V_GPIO_Port, Y3_V_Pin)
#define GPIOGetG3_V(void) HAL_GPIO_ReadPin(G3_V_GPIO_Port, G3_V_Pin)
#define GPIOGetR4_V(void) HAL_GPIO_ReadPin(R4_V_GPIO_Port, R4_V_Pin)
#define GPIOGetY4_V(void) HAL_GPIO_ReadPin(Y4_V_GPIO_Port, Y4_V_Pin)
#define GPIOGetG4_V(void) HAL_GPIO_ReadPin(G4_V_GPIO_Port, G4_V_Pin)
/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
extern void GPIODeInit(void);
extern uint8_t GPIOGetCardID(void);
extern void GPIOComLEDOn(void);
extern void GPIOComLEDOff(void);
extern void GPIOComLEDToggle(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

