/**
 ******************************************************************************
 * @file    Adapters/Mock/MockTickAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockTickAdapter.h"

static uint32_t MockNow_ms(void *pCtx)
{
  tSMockTickAdapterCtx *pC = (tSMockTickAdapterCtx *) pCtx;

  return pC->lNow_ms;
}

void MockTickAdapter_Init(tSMockTickAdapterCtx *pCtx)
{
  memset(pCtx, 0, sizeof(*pCtx));
}

void MockTickAdapter_SetNow(tSMockTickAdapterCtx *pCtx, uint32_t lNow_ms)
{
  pCtx->lNow_ms = lNow_ms;
}

ITickPort_t MockTickAdapter_CreatePort(tSMockTickAdapterCtx *pCtx)
{
  ITickPort_t port;

  port.pCtx = pCtx;
  port.Now_ms = MockNow_ms;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
