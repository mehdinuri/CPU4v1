/**
 ******************************************************************************
 * @file    Adapters/STM32/GpioInputAdapter.h
 * @brief   STM32 adapter for ISignalInputPort.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_GPIO_INPUT_ADAPTER_H
#define ADAPTERS_STM32_GPIO_INPUT_ADAPTER_H

#include <stdint.h>
#include "Ports/ISignalInputPort.h"

typedef struct
{
  uint8_t bReserved;
} tSGpioInputAdapterCtx;

void GpioInputAdapter_Init(tSGpioInputAdapterCtx *pCtx);
ISignalInputPort_t GpioInputAdapter_CreatePort(tSGpioInputAdapterCtx *pCtx);

#endif /* ADAPTERS_STM32_GPIO_INPUT_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
