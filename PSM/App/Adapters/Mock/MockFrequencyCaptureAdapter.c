/**
 ******************************************************************************
 * @file    Adapters/Mock/MockFrequencyCaptureAdapter.c
 ******************************************************************************
 */

#include "MockFrequencyCaptureAdapter.h"

#include <string.h>

static void AdapterRestart(void *ctx, uint32_t lNowMs)
{
  MockFrequencyCaptureAdapterCtx_t *c = (MockFrequencyCaptureAdapterCtx_t *) ctx;
  c->lRestartCount++;
  c->lLastRestartNowMs = lNowMs;
}

static void AdapterOnEdge(void *ctx, uint32_t lCaptureValue, uint32_t lNowMs)
{
  MockFrequencyCaptureAdapterCtx_t *c = (MockFrequencyCaptureAdapterCtx_t *) ctx;
  c->lOnEdgeCount++;
  c->lLastCaptureValue = lCaptureValue;
  c->lLastOnEdgeNowMs  = lNowMs;

  if ((c->fPublishOnEdge != 0U) && (c->publish != NULL))
  {
    c->publish(c->bPublishFreq);
  }
}

static uint8_t AdapterEvaluate(void *ctx, uint32_t lNowMs)
{
  MockFrequencyCaptureAdapterCtx_t *c = (MockFrequencyCaptureAdapterCtx_t *) ctx;
  c->lEvaluateCount++;
  c->lLastEvaluateNowMs = lNowMs;
  return c->eEvaluateVerdict;
}

void MockFrequencyCaptureAdapterInit(MockFrequencyCaptureAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->eEvaluateVerdict = FREQ_CAPTURE_VERDICT_OK;
}

IFrequencyCapturePort_t MockFrequencyCaptureAdapterCreatePort(
    MockFrequencyCaptureAdapterCtx_t *ctx)
{
  IFrequencyCapturePort_t port;
  port.ctx       = ctx;
  port.Restart   = AdapterRestart;
  port.OnEdge    = AdapterOnEdge;
  port.Evaluate  = AdapterEvaluate;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
