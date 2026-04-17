/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
        * Free pins are configured automatically as Analog (this feature is enabled through
        * the Code Generation settings)
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, COM_LED_Pin|DC_OK_LED_Pin|AC_OK_LED_Pin|ERR_LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(E2PROM_WC_GPIO_Port, E2PROM_WC_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : PC13 PC14 PC15 PC0
                           PC1 PC2 PC3 PC4
                           PC5 PC6 PC10 PC11
                           PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0
                          |GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PG10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA2 PA7 PA8
                           PA9 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : COM_LED_Pin DC_OK_LED_Pin AC_OK_LED_Pin ERR_LED_Pin */
  GPIO_InitStruct.Pin = COM_LED_Pin|DC_OK_LED_Pin|AC_OK_LED_Pin|ERR_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB14 PB15 PB5
                           PB6 PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : COMM_OK_Pin */
  GPIO_InitStruct.Pin = COMM_OK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(COMM_OK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : E2PROM_WC_Pin */
  GPIO_InitStruct.Pin = E2PROM_WC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(E2PROM_WC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */
void GPIOAllLEDsOff(void)
{
	GPIOComLEDOff();
	GPIOErrorLEDOff();
	GPIOACOKLEDOff();
	GPIODCOKLEDOff();
}

void GPIOComLEDOn(void)
{
	HAL_GPIO_WritePin(COM_LED_GPIO_Port, COM_LED_Pin, GPIO_PIN_RESET);
}

void GPIOComLEDOff(void)
{
	HAL_GPIO_WritePin(COM_LED_GPIO_Port, COM_LED_Pin, GPIO_PIN_SET);
}

void GPIOComLEDToggle(void)
{
	HAL_GPIO_TogglePin(COM_LED_GPIO_Port, COM_LED_Pin);
}

void GPIOACOKLEDOn(void)
{
	HAL_GPIO_WritePin(AC_OK_LED_GPIO_Port, AC_OK_LED_Pin, GPIO_PIN_RESET);
}

void GPIOACOKLEDOff(void)
{
	HAL_GPIO_WritePin(AC_OK_LED_GPIO_Port, AC_OK_LED_Pin, GPIO_PIN_SET);
}

GPIO_PinState	GPIOACOKStateGet(void)
{		
	return HAL_GPIO_ReadPin(AC_OK_LED_GPIO_Port, AC_OK_LED_Pin);
}

void GPIODCOKLEDOn(void)
{
	HAL_GPIO_WritePin(DC_OK_LED_GPIO_Port, DC_OK_LED_Pin, GPIO_PIN_RESET);
}

void GPIODCOKLEDOff(void)
{
	HAL_GPIO_WritePin(DC_OK_LED_GPIO_Port, DC_OK_LED_Pin, GPIO_PIN_SET);
}

GPIO_PinState	GPIODCOKStateGet(void)
{		
	return HAL_GPIO_ReadPin(DC_OK_LED_GPIO_Port, DC_OK_LED_Pin);
}

void GPIOErrorLEDOn(void)
{
	HAL_GPIO_WritePin(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_PIN_RESET);
}

void GPIOErrorLEDOff(void)
{
	HAL_GPIO_WritePin(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_PIN_SET);
}

GPIO_PinState	GPIOCommOKStateGet(void)
{		
	return HAL_GPIO_ReadPin(COMM_OK_GPIO_Port, COMM_OK_Pin);
}

void GPIOEEPROMEnable(void)
{
	HAL_GPIO_WritePin(E2PROM_WC_GPIO_Port, E2PROM_WC_Pin, GPIO_PIN_RESET);
}

void GPIOEEPROMDisable(void)
{
	HAL_GPIO_WritePin(E2PROM_WC_GPIO_Port, E2PROM_WC_Pin, GPIO_PIN_SET);
}
/* USER CODE END 2 */
