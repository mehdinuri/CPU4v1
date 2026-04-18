/**
 ******************************************************************************
 * @file    Adapters/Mock/MockWatchdogAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockWatchdogAdapter.h"

static void MockRefresh(void *pCtx)
{
  tSMockWatchdogAdapterCtx *pC = (tSMockWatchdogAdapterCtx *) pCtx;

  pC->lRefreshCount++;
}

void MockWatchdogAdapter_Init(tSMockWatchdogAdapterCtx *pCtx)
{
  memset(pCtx, 0, sizeof(*pCtx));
}

IWatchdogPort_t MockWatchdogAdapter_CreatePort(tSMockWatchdogAdapterCtx *pCtx)
{
  IWatchdogPort_t port;

  port.pCtx = pCtx;
  port.Refresh = MockRefresh;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
