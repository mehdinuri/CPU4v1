/**
 ******************************************************************************
 * @file    Adapters/Mock/MockTickAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockTickAdapter.h"

static uint32_t MockNow_ms(void *ctx)
{
  MockTickAdapterCtx_t *pC = (MockTickAdapterCtx_t *) ctx;

  return pC->nowMs;
}

void MockTickAdapter_Init(MockTickAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

void MockTickAdapter_SetNow(MockTickAdapterCtx_t *ctx, uint32_t nowMs)
{
  ctx->nowMs = nowMs;
}

ITickPort_t MockTickAdapter_CreatePort(MockTickAdapterCtx_t *ctx)
{
  ITickPort_t port;

  port.ctx = ctx;
  port.Now_ms = MockNow_ms;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
