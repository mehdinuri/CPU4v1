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
  uint8_t reserved;
} GpioInputAdapterCtx_t;

void GpioInputAdapter_Init(GpioInputAdapterCtx_t *ctx);
ISignalInputPort_t GpioInputAdapter_CreatePort(GpioInputAdapterCtx_t *ctx);

#endif /* ADAPTERS_STM32_GPIO_INPUT_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
