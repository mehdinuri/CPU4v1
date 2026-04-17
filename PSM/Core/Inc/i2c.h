/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
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
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c3;

/* USER CODE BEGIN Private defines */
#define I2C_E2PROM_ADD_PERIOD	(uint32_t)(0x0004)
#define I2C_E2PROM_ADD_OFFSET	(uint32_t)(I2C_E2PROM_ADD_PERIOD + sizeof(uint32_t))


/* USER CODE END Private defines */

void MX_I2C3_Init(void);

/* USER CODE BEGIN Prototypes */
extern void I2CDeInit(I2C_HandleTypeDef* i2chandle);
extern uint8_t I2CEEPROMWrite(uint32_t lAddress, const void * pvData, uint32_t lDataSize);
extern uint8_t I2CEEPROMRead(uint32_t lAddress, void * pvData, uint32_t lDataSize);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

