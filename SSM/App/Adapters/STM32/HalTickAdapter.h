/**
 ******************************************************************************
 * @file    Adapters/STM32/HalTickAdapter.h
 * @brief   STM32 adapter for ITickPort — wraps HAL_GetTick().
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_HAL_TICK_ADAPTER_H
#define ADAPTERS_STM32_HAL_TICK_ADAPTER_H

#include <stdint.h>
#include "Ports/ITickPort.h"

typedef struct
{
  uint8_t reserved;
} HalTickAdapterCtx_t;

void HalTickAdapter_Init(HalTickAdapterCtx_t *ctx);
ITickPort_t HalTickAdapter_CreatePort(HalTickAdapterCtx_t *ctx);

#endif /* ADAPTERS_STM32_HAL_TICK_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
