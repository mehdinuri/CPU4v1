/**
 ******************************************************************************
 * @file    Adapters/Mock/MockCurrentMeasurementAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockCurrentMeasurementAdapter.h"

static void MockGetLatest(void *ctx, CurrentMeasurementSnapshot_t *out)
{
  MockCurrentMeasurementAdapterCtx_t *pC =
    (MockCurrentMeasurementAdapterCtx_t *) ctx;

  *out = pC->stored;
  pC->getLatestCount++;
}

void MockCurrentMeasurementAdapter_Init(MockCurrentMeasurementAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

void MockCurrentMeasurementAdapter_SetSnapshot(
  MockCurrentMeasurementAdapterCtx_t *ctx,
  const
  CurrentMeasurementSnapshot_t *
  snap)
{
  ctx->stored = *snap;
}

ICurrentMeasurementPort_t MockCurrentMeasurementAdapter_CreatePort(
  MockCurrentMeasurementAdapterCtx_t *ctx)
{
  ICurrentMeasurementPort_t port;

  port.ctx = ctx;
  port.GetLatest = MockGetLatest;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
