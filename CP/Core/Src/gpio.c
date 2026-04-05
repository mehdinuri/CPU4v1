/* USER CODE BEGIN Header */

/**
 ******************************************************************************
 * @file    gpio.c
 * @brief   This file provides code for the configuration
 *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PC14-OSC32_IN (OSC32_IN)   ------> RCC_OSC32_IN
     PC15-OSC32_OUT (OSC32_OUT)   ------> RCC_OSC32_OUT
     PH0-OSC_IN (PH0)   ------> RCC_OSC_IN
     PH1-OSC_OUT (PH1)   ------> RCC_OSC_OUT
     PA8   ------> RCC_MCO_1
     PA13 (JTMS/SWDIO)   ------> DEBUG_JTMS-SWDIO
     PA14 (JTCK/SWCLK)   ------> DEBUG_JTCK-SWCLK
     PA15 (JTDI)   ------> DEBUG_JTDI
     PB3 (JTDO/TRACESWO)   ------> DEBUG_JTDO-SWO
     PB4 (NJTRST)   ------> DEBUG_JTRST
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, ESP_EN_Pin|DIMMING_Pin|HEAT_Pin|KEYPAD_ROW4_Pin
                          |LCD_CS1B_Pin|LCD_E2_Pin|LCD_WRITE_Pin|LCD_E1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPS_RESET_GPIO_Port, GPS_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, CHARGER_SD_Pin|LCD_D3_Pin|LCD_D2_Pin|LCD_D1_Pin
                          |LCD_D0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPRS_PWR_EN_Pin|COM_LED_Pin|KEYPAD_ROW5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, KEYPAD_ROW1_Pin|KEYPAD_ROW2_Pin|KEYPAD_ROW3_Pin|LCD_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LCD_PWR_Pin|LCD_D7_Pin|LCD_D6_Pin|LCD_D5_Pin
                          |LCD_D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : ESP_EN_Pin GPS_RESET_Pin DIMMING_Pin HEAT_Pin
                           KEYPAD_ROW4_Pin LCD_CS1B_Pin LCD_E2_Pin LCD_WRITE_Pin
                           LCD_E1_Pin */
  GPIO_InitStruct.Pin = ESP_EN_Pin|GPS_RESET_Pin|DIMMING_Pin|HEAT_Pin
                          |KEYPAD_ROW4_Pin|LCD_CS1B_Pin|LCD_E2_Pin|LCD_WRITE_Pin
                          |LCD_E1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PE4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PC13 PC0 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : DOOR_Pin */
  GPIO_InitStruct.Pin = DOOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DOOR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CHARGER_SD_Pin LCD_D3_Pin LCD_D2_Pin LCD_D1_Pin
                           LCD_D0_Pin */
  GPIO_InitStruct.Pin = CHARGER_SD_Pin|LCD_D3_Pin|LCD_D2_Pin|LCD_D1_Pin
                          |LCD_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : GPRS_PWR_EN_Pin RELAY_Pin COM_LED_Pin KEYPAD_ROW5_Pin */
  GPIO_InitStruct.Pin = GPRS_PWR_EN_Pin|RELAY_Pin|COM_LED_Pin|KEYPAD_ROW5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : KEYPAD_ROW1_Pin KEYPAD_ROW2_Pin KEYPAD_ROW3_Pin LCD_RESET_Pin */
  GPIO_InitStruct.Pin = KEYPAD_ROW1_Pin|KEYPAD_ROW2_Pin|KEYPAD_ROW3_Pin|LCD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : KEYPAD_COL1_Pin KEYPAD_COL2_Pin KEYPAD_COL3_Pin KEYPAD_COL4_Pin */
  GPIO_InitStruct.Pin = KEYPAD_COL1_Pin|KEYPAD_COL2_Pin|KEYPAD_COL3_Pin|KEYPAD_COL4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PB14 PB15 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : EEPROM_WP_Pin */
  GPIO_InitStruct.Pin = EEPROM_WP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(EEPROM_WP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_PWR_Pin LCD_D7_Pin LCD_D6_Pin LCD_D5_Pin
                           LCD_D4_Pin */
  GPIO_InitStruct.Pin = LCD_PWR_Pin|LCD_D7_Pin|LCD_D6_Pin|LCD_D5_Pin
                          |LCD_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : MCO_Pin */
  GPIO_InitStruct.Pin = MCO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(MCO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD3 PD4 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*AnalogSwitch Config */
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC3, SYSCFG_SWITCH_PC3_CLOSE);

}

/* USER CODE BEGIN 2 */
void GPIODeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = { 0 };

  GPIO_InitStruct.Pin = GPIO_PIN_All;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_All);
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_All);
  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_All);
  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_All);
  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_All);

  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();
}

void GPIOCommLedToggle(void)
{
  HAL_GPIO_TogglePin(COM_LED_GPIO_Port, COM_LED_Pin);
}

void GPIOLCDResetPinInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pin = LCD_RESET_Pin;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RESET_GPIO_Port, &GPIO_InitStructure);
}

void GPIOLCDResetPinStateSet(uint8_t fState)
{
  HAL_GPIO_WritePin(LCD_RESET_GPIO_Port,
                    LCD_RESET_Pin,
                    (fState) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void GPIOLCDPowerPinInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pin = LCD_PWR_Pin;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_PWR_GPIO_Port, &GPIO_InitStructure);
}

void GPIOLCDPowerPinStateSet(uint8_t fState)
{
  HAL_GPIO_WritePin(LCD_PWR_GPIO_Port,
                    LCD_PWR_Pin,
                    (fState) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

GPIO_PinState GPIOLCDPowerPinStateGet(void)
{
  return HAL_GPIO_ReadPin(LCD_PWR_GPIO_Port, LCD_PWR_Pin);
}

void GPIOLCDE1PinInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  __HAL_RCC_GPIOE_CLK_ENABLE();

  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pin = LCD_E1_Pin;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_E1_GPIO_Port, &GPIO_InitStructure);
}

void GPIOLCDE1PinStateSet(uint8_t fState)
{
  HAL_GPIO_WritePin(LCD_E1_GPIO_Port,
                    LCD_E1_Pin,
                    (fState) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void GPIOLCDE2PinInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  __HAL_RCC_GPIOE_CLK_ENABLE();

  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pin = LCD_E2_Pin;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_E2_GPIO_Port, &GPIO_InitStructure);
}

void GPIOLCDE2PinStateSet(uint8_t fState)
{
  HAL_GPIO_WritePin(LCD_E2_GPIO_Port,
                    LCD_E2_Pin,
                    (fState) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void GPIOLCDWritePinInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  __HAL_RCC_GPIOE_CLK_ENABLE();

  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pin = LCD_WRITE_Pin;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_WRITE_GPIO_Port, &GPIO_InitStructure);
}

void GPIOLCDWritePinStateSet(uint8_t fState)
{
  HAL_GPIO_WritePin(LCD_WRITE_GPIO_Port,
                    LCD_WRITE_Pin,
                    (fState) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void GPIOLCDCS1BPinInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  __HAL_RCC_GPIOE_CLK_ENABLE();

  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pin = LCD_CS1B_Pin;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_CS1B_GPIO_Port, &GPIO_InitStructure);
}

void GPIOLCDCS1BPinStateSet(uint8_t fState)
{
  HAL_GPIO_WritePin(LCD_CS1B_GPIO_Port,
                    LCD_CS1B_Pin,
                    (fState) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void GPIOLCDDataPinsInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pin = LCD_D0_Pin | LCD_D1_Pin | LCD_D2_Pin | LCD_D3_Pin;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = LCD_D4_Pin | LCD_D5_Pin | LCD_D6_Pin | LCD_D7_Pin;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void GPIOLCDDataPinsSet(uint8_t bData)
{
  HAL_GPIO_WritePin(GPIOC,
                    LCD_D0_Pin,
                    (bData & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC,
                    LCD_D1_Pin,
                    (bData & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC,
                    LCD_D2_Pin,
                    (bData & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC,
                    LCD_D3_Pin,
                    (bData & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD,
                    LCD_D4_Pin,
                    (bData & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD,
                    LCD_D5_Pin,
                    (bData & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD,
                    LCD_D6_Pin,
                    (bData & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD,
                    LCD_D7_Pin,
                    (bData & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void GPIOKeypadInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /* Init rows */
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pin = KEYPAD_ROW1_Pin | KEYPAD_ROW2_Pin | KEYPAD_ROW3_Pin;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = KEYPAD_ROW4_Pin;
  HAL_GPIO_Init(KEYPAD_ROW4_GPIO_Port, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = KEYPAD_ROW5_Pin;
  HAL_GPIO_Init(KEYPAD_ROW5_GPIO_Port, &GPIO_InitStructure);

  /* Init columns */
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructure.Pin =
    KEYPAD_COL1_Pin | KEYPAD_COL2_Pin | KEYPAD_COL3_Pin | KEYPAD_COL4_Pin;
  GPIO_InitStructure.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
}

GPIO_PinState GPIOKeypadColumnStateGet(uint8_t bColumnNo)
{
  switch (bColumnNo)
  {
      case 1:
      {
        return HAL_GPIO_ReadPin(KEYPAD_COL1_GPIO_Port, KEYPAD_COL1_Pin);
      }

      case 2:
      {
        return HAL_GPIO_ReadPin(KEYPAD_COL2_GPIO_Port, KEYPAD_COL2_Pin);
      }

      case 3:
      {
        return HAL_GPIO_ReadPin(KEYPAD_COL3_GPIO_Port, KEYPAD_COL3_Pin);
      }

      case 4:
      {
        return HAL_GPIO_ReadPin(KEYPAD_COL4_GPIO_Port,
                                KEYPAD_COL4_Pin);
      }

      default:
      {
        return GPIO_PIN_RESET;
      }
  }
}

void GPIOKeypadSetOutput(uint32_t lData)
{
  HAL_GPIO_WritePin(KEYPAD_ROW4_GPIO_Port, KEYPAD_ROW4_Pin,
                    (GPIO_PinState) ((lData >> 3) & 0x01));
  HAL_GPIO_WritePin(KEYPAD_ROW3_GPIO_Port, KEYPAD_ROW3_Pin,
                    (GPIO_PinState) ((lData >> 2) & 0x01));
  HAL_GPIO_WritePin(KEYPAD_ROW2_GPIO_Port, KEYPAD_ROW2_Pin,
                    (GPIO_PinState) ((lData >> 1) & 0x01));
  HAL_GPIO_WritePin(KEYPAD_ROW1_GPIO_Port, KEYPAD_ROW1_Pin,
                    (GPIO_PinState) ((lData >> 0) & 0x01));
}

void GPIOEEPROMEnable(void)
{
  HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_RESET);
}

void GPIOEEPROMDisable(void)
{
  HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_SET);
}

void GPIOChargerShutdownEnable(void)
{
  HAL_GPIO_WritePin(CHARGER_SD_GPIO_Port, CHARGER_SD_Pin, GPIO_PIN_RESET);
}

void GPIOChargerShutdownDisable(void)
{
  HAL_GPIO_WritePin(CHARGER_SD_GPIO_Port, CHARGER_SD_Pin, GPIO_PIN_SET);
}

void GPIOGPRSPowerEnable(void)
{
  HAL_GPIO_WritePin(GPRS_PWR_EN_GPIO_Port, GPRS_PWR_EN_Pin, GPIO_PIN_RESET);
}

void GPIOGPRSPowerDisable(void)
{
  HAL_GPIO_WritePin(GPRS_PWR_EN_GPIO_Port, GPRS_PWR_EN_Pin, GPIO_PIN_SET);
}

void GPIOHeaterEnable(void)
{
  HAL_GPIO_WritePin(HEAT_GPIO_Port, HEAT_Pin, GPIO_PIN_SET);
}

void GPIOHeaterDisable(void)
{
  HAL_GPIO_WritePin(HEAT_GPIO_Port, HEAT_Pin, GPIO_PIN_RESET);
}

uint8_t GPIORelayPinGet(void)
{
  return HAL_GPIO_ReadPin(RELAY_GPIO_Port, RELAY_Pin) == GPIO_PIN_SET;
}

void GPIORelayPinSet(GPIO_PinState eState)
{
  HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, eState);
}

uint8_t GPIODoorPinRead(void)
{
  return HAL_GPIO_ReadPin(DOOR_GPIO_Port, DOOR_Pin ) ==  GPIO_PIN_SET;
}

/* USER CODE END 2 */
