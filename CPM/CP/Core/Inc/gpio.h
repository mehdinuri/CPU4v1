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
/* Full GPIO reset — used during fault transitions. */
extern void GPIODeInit(void);

/* Platform-internal helpers without a domain port yet.
 * LCD GPIO  → lcdDrv.h/lcdDrv.c
 * Heater    → HeaterAdapter.c     (IHeaterPort)
 * Relay     → RelayAdapter.c      (IRelayPort)
 * Door      → DoorSensorAdapter.c (IDoorSensorPort)
 * Comm LED  → CommLEDAdapter.c    (IStatusLEDPort)
 * Keypad    → KeypadAdapter.c     (IUserInputPort) */
extern void GPIOChargerShutdownEnable(void);
extern void GPIOChargerShutdownDisable(void);
extern void GPIOGPRSPowerEnable(void);
extern void GPIOGPRSPowerDisable(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

