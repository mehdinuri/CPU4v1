/**
 ******************************************************************************
 * @file    Adapters/Mock/MockWatchdogAdapter.c
 ******************************************************************************
 */

#include "MockWatchdogAdapter.h"

#include <string.h>

static void AdapterRefresh(void *ctx)
{
  MockWatchdogAdapterCtx_t *c = (MockWatchdogAdapterCtx_t *) ctx;
  c->lRefreshCount++;
}

void MockWatchdogAdapterInit(MockWatchdogAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IWatchdogPort_t MockWatchdogAdapterCreatePort(MockWatchdogAdapterCtx_t *ctx)
{
  IWatchdogPort_t port;
  port.ctx     = ctx;
  port.Refresh = AdapterRefresh;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
