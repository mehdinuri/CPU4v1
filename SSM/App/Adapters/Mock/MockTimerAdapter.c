/**
 ******************************************************************************
 * @file    Adapters/Mock/MockTimerAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockTimerAdapter.h"

static void MockStart(void *ctx, TimerId_e eId)
{
  MockTimerAdapterCtx_t *pC = (MockTimerAdapterCtx_t *) ctx;

  if ((uint32_t) eId < (uint32_t) TIMER_ID__COUNT)
  {
    pC->startCount[eId]++;
  }
}

void MockTimerAdapter_Init(MockTimerAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

ITimerPort_t MockTimerAdapter_CreatePort(MockTimerAdapterCtx_t *ctx)
{
  ITimerPort_t port;

  port.ctx = ctx;
  port.Start = MockStart;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
