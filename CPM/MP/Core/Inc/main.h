/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

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
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define I2C4_OWN_ADDR 0x30F
#define TRIAC_Pin GPIO_PIN_13
#define TRIAC_GPIO_Port GPIOC
#define COM_LED_Pin GPIO_PIN_0
#define COM_LED_GPIO_Port GPIOC
#define WKUP_100Hz_Pin GPIO_PIN_0
#define WKUP_100Hz_GPIO_Port GPIOA
#define TERMISTOR_Pin GPIO_PIN_2
#define TERMISTOR_GPIO_Port GPIOA
#define CHRGR_CEN_Pin GPIO_PIN_6
#define CHRGR_CEN_GPIO_Port GPIOA
#define CHRGING_Pin GPIO_PIN_7
#define CHRGING_GPIO_Port GPIOA
#define VBAT_Pin GPIO_PIN_0
#define VBAT_GPIO_Port GPIOB
#define FLASH_WP_Pin GPIO_PIN_10
#define FLASH_WP_GPIO_Port GPIOB
#define FLASH_HOLD_Pin GPIO_PIN_11
#define FLASH_HOLD_GPIO_Port GPIOB
#define EEPROM_WP_Pin GPIO_PIN_8
#define EEPROM_WP_GPIO_Port GPIOA
#define RELAY_Pin GPIO_PIN_10
#define RELAY_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
#ifndef DEBUG
//#define DEBUG
#endif

//#define TRACE

void SystemReset(void);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
