/* USER CODE BEGIN Header */

/**
 ******************************************************************************
 * @file    gpio.h
 * @brief   This file contains all the function prototypes for
 *          the gpio.c file
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
extern void GPIODeInit(void);
extern void GPIOCommLedToggle(void);
extern void GPIOLCDResetPinInit(void);
extern void GPIOLCDResetPinStateSet(uint8_t fState);
extern void GPIOLCDPowerPinInit(void);
extern void GPIOLCDPowerPinStateSet(uint8_t fState);
extern GPIO_PinState GPIOLCDPowerPinStateGet(void);
extern void GPIOLCDE1PinInit(void);
extern void GPIOLCDE1PinStateSet(uint8_t fState);
extern void GPIOLCDE2PinInit(void);
extern void GPIOLCDE2PinStateSet(uint8_t fState);
extern void GPIOLCDWritePinInit(void);
extern void GPIOLCDWritePinStateSet(uint8_t fState);
extern void GPIOLCDCS1BPinInit(void);
extern void GPIOLCDCS1BPinStateSet(uint8_t fState);
extern void GPIOLCDDataPinsInit(void);
extern void GPIOLCDDataPinsSet(uint8_t bData);
extern void GPIOKeypadInit(void);
extern GPIO_PinState GPIOKeypadColumnStateGet(uint8_t bColumnNo);
extern void GPIOKeypadSetOutput(uint32_t lData);
extern void GPIOEEPROMEnable(void);
extern void GPIOEEPROMDisable(void);
extern void GPIOChargerShutdownEnable(void);
extern void GPIOChargerShutdownDisable(void);
extern void GPIOGPRSPowerEnable(void);
extern void GPIOGPRSPowerDisable(void);
extern void GPIOHeaterEnable(void);
extern void GPIOHeaterDisable(void);
extern uint8_t GPIORelayPinGet(void);
extern void GPIORelayPinSet(GPIO_PinState eState);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

