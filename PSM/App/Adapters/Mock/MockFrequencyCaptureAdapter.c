/**
 ******************************************************************************
 * @file    Adapters/Mock/MockFrequencyCaptureAdapter.c
 ******************************************************************************
 */

#include "MockFrequencyCaptureAdapter.h"

#include <string.h>

static void AdapterRestart(void *ctx, uint32_t nowMs)
{
  MockFrequencyCaptureAdapterCtx_t *c = (MockFrequencyCaptureAdapterCtx_t *) ctx;
  c->restartCount++;
  c->lastRestartNowMs = nowMs;
}

static void AdapterOnEdge(void *ctx, uint32_t captureValue, uint32_t nowMs)
{
  MockFrequencyCaptureAdapterCtx_t *c = (MockFrequencyCaptureAdapterCtx_t *) ctx;
  c->onEdgeCount++;
  c->lastCaptureValue = captureValue;
  c->lastOnEdgeNowMs  = nowMs;

  if ((c->publishOnEdge != 0U) && (c->publish != NULL))
  {
    c->publish(c->publishFreq);
  }
}

static uint8_t AdapterEvaluate(void *ctx, uint32_t nowMs)
{
  MockFrequencyCaptureAdapterCtx_t *c = (MockFrequencyCaptureAdapterCtx_t *) ctx;
  c->evaluateCount++;
  c->lastEvaluateNowMs = nowMs;
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
