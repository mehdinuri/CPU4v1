/* USER CODE BEGIN Header */

/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define I2C1_MP_ADDR 0xCA
#define I2C1_OWN_ADDR 0x30F
#define ESP_EN_Pin GPIO_PIN_2
#define ESP_EN_GPIO_Port GPIOE
#define GPS_RESET_Pin GPIO_PIN_3
#define GPS_RESET_GPIO_Port GPIOE
#define DIMMING_Pin GPIO_PIN_5
#define DIMMING_GPIO_Port GPIOE
#define HEAT_Pin GPIO_PIN_6
#define HEAT_GPIO_Port GPIOE
#define DOOR_Pin GPIO_PIN_2
#define DOOR_GPIO_Port GPIOC
#define CHARGER_SD_Pin GPIO_PIN_3
#define CHARGER_SD_GPIO_Port GPIOC
#define GPRS_PWR_EN_Pin GPIO_PIN_3
#define GPRS_PWR_EN_GPIO_Port GPIOA
#define RELAY_Pin GPIO_PIN_4
#define RELAY_GPIO_Port GPIOA
#define COM_LED_Pin GPIO_PIN_5
#define COM_LED_GPIO_Port GPIOA
#define KEYPAD_ROW5_Pin GPIO_PIN_6
#define KEYPAD_ROW5_GPIO_Port GPIOA
#define KEYPAD_ROW1_Pin GPIO_PIN_0
#define KEYPAD_ROW1_GPIO_Port GPIOB
#define KEYPAD_ROW2_Pin GPIO_PIN_1
#define KEYPAD_ROW2_GPIO_Port GPIOB
#define KEYPAD_ROW3_Pin GPIO_PIN_2
#define KEYPAD_ROW3_GPIO_Port GPIOB
#define KEYPAD_ROW4_Pin GPIO_PIN_7
#define KEYPAD_ROW4_GPIO_Port GPIOE
#define KEYPAD_COL1_Pin GPIO_PIN_8
#define KEYPAD_COL1_GPIO_Port GPIOE
#define KEYPAD_COL2_Pin GPIO_PIN_9
#define KEYPAD_COL2_GPIO_Port GPIOE
#define KEYPAD_COL3_Pin GPIO_PIN_10
#define KEYPAD_COL3_GPIO_Port GPIOE
#define KEYPAD_COL4_Pin GPIO_PIN_11
#define KEYPAD_COL4_GPIO_Port GPIOE
#define LCD_CS1B_Pin GPIO_PIN_12
#define LCD_CS1B_GPIO_Port GPIOE
#define LCD_E2_Pin GPIO_PIN_13
#define LCD_E2_GPIO_Port GPIOE
#define LCD_WRITE_Pin GPIO_PIN_14
#define LCD_WRITE_GPIO_Port GPIOE
#define LCD_E1_Pin GPIO_PIN_15
#define LCD_E1_GPIO_Port GPIOE
#define LCD_RESET_Pin GPIO_PIN_10
#define LCD_RESET_GPIO_Port GPIOB
#define EEPROM_WP_Pin GPIO_PIN_8
#define EEPROM_WP_GPIO_Port GPIOD
#define LCD_PWR_Pin GPIO_PIN_9
#define LCD_PWR_GPIO_Port GPIOD
#define LCD_D7_Pin GPIO_PIN_10
#define LCD_D7_GPIO_Port GPIOD
#define LCD_D6_Pin GPIO_PIN_11
#define LCD_D6_GPIO_Port GPIOD
#define LCD_D5_Pin GPIO_PIN_14
#define LCD_D5_GPIO_Port GPIOD
#define LCD_D4_Pin GPIO_PIN_15
#define LCD_D4_GPIO_Port GPIOD
#define LCD_D3_Pin GPIO_PIN_6
#define LCD_D3_GPIO_Port GPIOC
#define LCD_D2_Pin GPIO_PIN_7
#define LCD_D2_GPIO_Port GPIOC
#define LCD_D1_Pin GPIO_PIN_8
#define LCD_D1_GPIO_Port GPIOC
#define LCD_D0_Pin GPIO_PIN_9
#define LCD_D0_GPIO_Port GPIOC
#define MCO_Pin GPIO_PIN_8
#define MCO_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
extern void SystemReset(void);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
