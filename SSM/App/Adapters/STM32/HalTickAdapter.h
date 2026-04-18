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
  uint8_t bReserved;
} tSHalTickAdapterCtx;

void HalTickAdapter_Init(tSHalTickAdapterCtx *pCtx);
ITickPort_t HalTickAdapter_CreatePort(tSHalTickAdapterCtx *pCtx);

#endif /* ADAPTERS_STM32_HAL_TICK_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
