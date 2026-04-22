/**
 ******************************************************************************
 * @file    Adapters/STM32/WatchdogAdapter.c
 * @brief   STM32 adapter for IWatchdogPort.
 ******************************************************************************
 */

#include "WatchdogAdapter.h"
#include "iwdg.h"

/* ---------------------------------------------------------------------------
 * Private adapter implementation
 * ---------------------------------------------------------------------------*/
static void AdapterRefresh(void *ctx)
{
  (void) ctx;
  IWDGRefresh();
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void WatchdogAdapterInit(WatchdogAdapterCtx_t *ctx)
{
  ctx->initialised = 1U;
}

IWatchdogPort_t WatchdogAdapterCreatePort(WatchdogAdapterCtx_t *ctx)
{
  IWatchdogPort_t port;
  port.ctx     = ctx;
  port.Refresh = AdapterRefresh;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
