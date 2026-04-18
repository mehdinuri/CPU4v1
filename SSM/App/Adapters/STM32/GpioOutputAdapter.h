/**
 ******************************************************************************
 * @file    Adapters/STM32/GpioOutputAdapter.h
 * @brief   STM32 adapter for ISignalOutputPort.
 *          Drives the 12 signal-output GPIOs via HAL_GPIO_WritePin.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_GPIO_OUTPUT_ADAPTER_H
#define ADAPTERS_STM32_GPIO_OUTPUT_ADAPTER_H

#include "Ports/ISignalOutputPort.h"

typedef struct
{
  /* No mutable adapter state today. The 12 pins are resolved from a
   * constant table in the .c. Kept as a struct so future additions
   * (shadow state, last-image cache) don't break the API.
   */
  uint8_t bReserved;
} tSGpioOutputAdapterCtx;

void GpioOutputAdapter_Init(tSGpioOutputAdapterCtx *pCtx);
ISignalOutputPort_t GpioOutputAdapter_CreatePort(tSGpioOutputAdapterCtx *pCtx);

/**
 * @brief Drive every signal output pin to GPIO_PIN_RESET with no RTOS /
 *        context dependency. Safe to call from Error_Handler, before
 *        MainApplication_Init, or from any non-ISR context where the
 *        g_SignalOutputPort may not yet be wired.
 *
 * MX_GPIO_Init() must already have run (pins in output mode); that happens
 * in main() before USER CODE BEGIN 2.
 */
void GpioOutputAdapter_ForceAllOff(void);

#endif /* ADAPTERS_STM32_GPIO_OUTPUT_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
