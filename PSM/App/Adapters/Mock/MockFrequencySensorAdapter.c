/**
 ******************************************************************************
 * @file    Adapters/Mock/MockFrequencySensorAdapter.c
 * @brief   Mock adapter for IFrequencySensorPort.
 ******************************************************************************
 */

#include "MockFrequencySensorAdapter.h"
#include <string.h>

/* ---------------------------------------------------------------------------
 * Private implementation
 * ---------------------------------------------------------------------------*/
static uint8_t AdapterGetNetFrequency(void *ctx)
{
  return ((MockFrequencySensorAdapterCtx_t *) ctx)->frequency;
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void MockFrequencySensorAdapterInit(MockFrequencySensorAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IFrequencySensorPort_t MockFrequencySensorAdapterCreatePort(MockFrequencySensorAdapterCtx_t *ctx)
{
  IFrequencySensorPort_t port;
  port.ctx             = ctx;
  port.GetNetFrequency = AdapterGetNetFrequency;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
