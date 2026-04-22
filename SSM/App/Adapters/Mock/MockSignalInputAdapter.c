/**
 ******************************************************************************
 * @file    Adapters/Mock/MockSignalInputAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockSignalInputAdapter.h"

static void MockSample(void *ctx, SignalInputSnapshot_t *out)
{
  MockSignalInputAdapterCtx_t *pC = (MockSignalInputAdapterCtx_t *) ctx;

  *out = pC->canned;
  pC->sampleCount++;
}

void MockSignalInputAdapter_Init(MockSignalInputAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

void MockSignalInputAdapter_SetSnapshot(MockSignalInputAdapterCtx_t *ctx,
                                        const SignalInputSnapshot_t *snap)
{
  ctx->canned = *snap;
}

ISignalInputPort_t MockSignalInputAdapter_CreatePort(
  MockSignalInputAdapterCtx_t *ctx)
{
  ISignalInputPort_t port;

  port.ctx = ctx;
  port.Sample = MockSample;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
