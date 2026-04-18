/**
 ******************************************************************************
 * @file    Adapters/Mock/MockTimerAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockTimerAdapter.h"

static void MockStart(void *pCtx, tETimerId eId)
{
  tSMockTimerAdapterCtx *pC = (tSMockTimerAdapterCtx *) pCtx;

  if ((uint32_t) eId < (uint32_t) TIMER_ID__COUNT)
  {
    pC->aStartCount[eId]++;
  }
}

void MockTimerAdapter_Init(tSMockTimerAdapterCtx *pCtx)
{
  memset(pCtx, 0, sizeof(*pCtx));
}

ITimerPort_t MockTimerAdapter_CreatePort(tSMockTimerAdapterCtx *pCtx)
{
  ITimerPort_t port;

  port.pCtx = pCtx;
  port.Start = MockStart;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
