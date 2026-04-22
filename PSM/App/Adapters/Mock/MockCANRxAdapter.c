/**
 ******************************************************************************
 * @file    Adapters/Mock/MockCANRxAdapter.c
 ******************************************************************************
 */

#include "MockCANRxAdapter.h"

#include <string.h>

static void AdapterSubmitFrame(void *ctx, const CanRxFrame_t *frame)
{
  MockCANRxAdapterCtx_t *c = (MockCANRxAdapterCtx_t *) ctx;

  if (frame != NULL)
  {
    c->lastFrame = *frame;
  }
  c->submitCount++;
}

void MockCANRxAdapterInit(MockCANRxAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

ICANRxPort_t MockCANRxAdapterCreatePort(MockCANRxAdapterCtx_t *ctx)
{
  ICANRxPort_t port;
  port.ctx         = ctx;
  port.SubmitFrame = AdapterSubmitFrame;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
