/**
 ******************************************************************************
 * @file    Adapters/STM32/IndicatorLEDAdapter.h
 * @brief   STM32 adapter for IIndicatorLEDPort — wraps gpio.h LED functions.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_INDICATORLEDADAPTER_H
#define ADAPTERS_STM32_INDICATORLEDADAPTER_H

#include "Ports/IIndicatorLEDPort.h"

typedef struct
{
  uint8_t initialised;
} IndicatorLEDAdapterCtx_t;

void                 IndicatorLEDAdapterInit(IndicatorLEDAdapterCtx_t *ctx);
IIndicatorLEDPort_t  IndicatorLEDAdapterCreatePort(IndicatorLEDAdapterCtx_t *ctx);

#endif /* ADAPTERS_STM32_INDICATORLEDADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
