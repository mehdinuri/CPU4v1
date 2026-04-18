/**
 ******************************************************************************
 * @file    Adapters/Mock/MockVoltageSensorAdapter.c
 * @brief   Mock adapter for IVoltageSensorPort.
 ******************************************************************************
 */

#include "MockVoltageSensorAdapter.h"
#include <string.h>

/* ---------------------------------------------------------------------------
 * Private implementations
 * ---------------------------------------------------------------------------*/
static float AdapterGetNetVoltage(void *ctx)
{
  return ((MockVoltageSensorAdapterCtx_t *) ctx)->fNetVoltage;
}

static float AdapterGetRegVIn(void *ctx)
{
  return ((MockVoltageSensorAdapterCtx_t *) ctx)->fRegVIn;
}

static float AdapterGetRegVOut(void *ctx)
{
  return ((MockVoltageSensorAdapterCtx_t *) ctx)->fRegVOut;
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void MockVoltageSensorAdapterInit(MockVoltageSensorAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IVoltageSensorPort_t MockVoltageSensorAdapterCreatePort(MockVoltageSensorAdapterCtx_t *ctx)
{
  IVoltageSensorPort_t port;
  port.ctx           = ctx;
  port.GetNetVoltage = AdapterGetNetVoltage;
  port.GetRegVIn     = AdapterGetRegVIn;
  port.GetRegVOut    = AdapterGetRegVOut;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
