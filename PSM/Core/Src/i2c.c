/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */
#include "utilities.h"
#include "gpio.h"
#include "iwdg.h"

#define I2C_EEPROM_DEVICE_BASE_ADDR 0xA0
#define I2C_EEPROM_PAGE_SIZE 16

#define I2C_XFER_TIMEOUT_MAX 1000
#define I2C_EEPROM_MAX_TRIALS 300
/* USER CODE END 0 */

I2C_HandleTypeDef hi2c3;

/* I2C3 init function */
void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x10F0277E;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 7) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(i2cHandle->Instance==I2C3)
  {
  /* USER CODE BEGIN I2C3_MspInit 0 */

  /* USER CODE END I2C3_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C3;
    PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**I2C3 GPIO Configuration
    PC8     ------> I2C3_SCL
    PC9     ------> I2C3_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_I2C3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* I2C3 clock enable */
    __HAL_RCC_I2C3_CLK_ENABLE();

    /* I2C3 interrupt Init */
    HAL_NVIC_SetPriority(I2C3_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
    HAL_NVIC_SetPriority(I2C3_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);
  /* USER CODE BEGIN I2C3_MspInit 1 */

  /* USER CODE END I2C3_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C3)
  {
  /* USER CODE BEGIN I2C3_MspDeInit 0 */

  /* USER CODE END I2C3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C3_CLK_DISABLE();

    /**I2C3 GPIO Configuration
    PC8     ------> I2C3_SCL
    PC9     ------> I2C3_SDA
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_9);

    /* I2C3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
  /* USER CODE BEGIN I2C3_MspDeInit 1 */

  /* USER CODE END I2C3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void I2CDeInit(I2C_HandleTypeDef *i2chandle)
{
	HAL_I2C_DeInit(i2chandle);
}

uint8_t I2CEEPROMWrite(uint32_t lAddress, const void *pvData, uint32_t lDataSize)
{
	uint8_t *pbData = (uint8_t*) pvData;
	uint16_t sWriteAddr = (uint16_t) lAddress;
	uint16_t sBytesRemained = lDataSize;

	GPIOEEPROMEnable();

	while (sBytesRemained > 0)
	{
		IWDGRefresh();

		uint16_t sBytesInPage = I2C_EEPROM_PAGE_SIZE - (sWriteAddr % I2C_EEPROM_PAGE_SIZE);
		uint16_t sBytesToWrite = (sBytesRemained < sBytesInPage) ? sBytesRemained : sBytesInPage;
		
		if (HAL_I2C_Mem_Write(&hi2c3, (uint16_t)I2C_EEPROM_DEVICE_BASE_ADDR, sWriteAddr, I2C_MEMADD_SIZE_8BIT, pbData,
				sBytesToWrite, I2C_XFER_TIMEOUT_MAX)
				!= HAL_OK)
		{
			GPIOEEPROMDisable();
			return FALSE;
		}

		if (HAL_I2C_IsDeviceReady(&hi2c3, I2C_EEPROM_DEVICE_BASE_ADDR, I2C_EEPROM_MAX_TRIALS, I2C_XFER_TIMEOUT_MAX) != HAL_OK)
		{
			GPIOEEPROMDisable();
			return FALSE;
		}

		sBytesRemained -= sBytesToWrite;
		pbData += sBytesToWrite;
		sWriteAddr += sBytesToWrite;
	}

	GPIOEEPROMDisable();
	return TRUE;
}

uint8_t I2CEEPROMRead(uint32_t lAddress, void *pvData, uint32_t lDataSize)
{
	return  (HAL_I2C_Mem_Read(&hi2c3, (uint16_t)I2C_EEPROM_DEVICE_BASE_ADDR, (uint16_t)lAddress,
            I2C_MEMADD_SIZE_8BIT, (uint8_t*)pvData, lDataSize, I2C_XFER_TIMEOUT_MAX) == HAL_OK);
}

/* USER CODE END 1 */

