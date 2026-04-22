/**
 ******************************************************************************
 * @file    Adapters/STM32/FrequencySensorAdapter.c
 * @brief   STM32 adapter for IFrequencySensorPort.
 ******************************************************************************
 */

#include "FrequencySensorAdapter.h"
#include <string.h>

/* ---------------------------------------------------------------------------
 * Private adapter implementation
 * ---------------------------------------------------------------------------*/
static uint8_t AdapterGetNetFrequency(void *ctx)
{
  return ((FrequencySensorAdapterCtx_t *) ctx)->frequency;
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void FrequencySensorAdapterInit(FrequencySensorAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IFrequencySensorPort_t FrequencySensorAdapterCreatePort(FrequencySensorAdapterCtx_t *ctx)
{
  IFrequencySensorPort_t port;
  port.ctx              = ctx;
  port.GetNetFrequency  = AdapterGetNetFrequency;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
