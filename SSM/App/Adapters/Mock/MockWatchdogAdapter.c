/**
 ******************************************************************************
 * @file    Adapters/Mock/MockWatchdogAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockWatchdogAdapter.h"

static void MockRefresh(void *ctx)
{
  MockWatchdogAdapterCtx_t *pC = (MockWatchdogAdapterCtx_t *) ctx;

  pC->refreshCount++;
}

void MockWatchdogAdapter_Init(MockWatchdogAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IWatchdogPort_t MockWatchdogAdapter_CreatePort(MockWatchdogAdapterCtx_t *ctx)
{
  IWatchdogPort_t port;

  port.ctx = ctx;
  port.Refresh = MockRefresh;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
