/**
 ******************************************************************************
 * @file    Adapters/STM32/VoltageSensorAdapter.c
 * @brief   STM32 adapter for IVoltageSensorPort.
 ******************************************************************************
 */

#include "VoltageSensorAdapter.h"
#include <string.h>

/* ---------------------------------------------------------------------------
 * Private adapter implementations
 * ---------------------------------------------------------------------------*/
static float AdapterGetNetVoltage(void *ctx)
{
  return ((VoltageSensorAdapterCtx_t *) ctx)->netVoltage;
}

static float AdapterGetRegVIn(void *ctx)
{
  return ((VoltageSensorAdapterCtx_t *) ctx)->regVIn;
}

static float AdapterGetRegVOut(void *ctx)
{
  return ((VoltageSensorAdapterCtx_t *) ctx)->regVOut;
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void VoltageSensorAdapterInit(VoltageSensorAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IVoltageSensorPort_t VoltageSensorAdapterCreatePort(VoltageSensorAdapterCtx_t *ctx)
{
  IVoltageSensorPort_t port;
  port.ctx            = ctx;
  port.GetNetVoltage  = AdapterGetNetVoltage;
  port.GetRegVIn      = AdapterGetRegVIn;
  port.GetRegVOut     = AdapterGetRegVOut;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
