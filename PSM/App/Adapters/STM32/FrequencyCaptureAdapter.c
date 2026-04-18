/**
 ******************************************************************************
 * @file    Adapters/STM32/FrequencyCaptureAdapter.c
 * @brief   STM32 adapter for IFrequencyCapturePort.
 ******************************************************************************
 */

#include "FrequencyCaptureAdapter.h"

#include <stddef.h>

/* ---------------------------------------------------------------------------
 * Private adapter implementation
 * ---------------------------------------------------------------------------*/
static void AdapterRestart(void *ctx, uint32_t lNowMs)
{
  FrequencyCaptureAdapterCtx_t *c = (FrequencyCaptureAdapterCtx_t *) ctx;
  FrequencyCaptureFSM_Init(&c->state, lNowMs);
}

static void AdapterOnEdge(void *ctx, uint32_t lCaptureValue, uint32_t lNowMs)
{
  FrequencyCaptureAdapterCtx_t *c = (FrequencyCaptureAdapterCtx_t *) ctx;

  if (FrequencyCaptureFSM_OnEdge(&c->state, &c->config, lCaptureValue, lNowMs)
      != 0U)
  {
    if (c->publish != NULL)
    {
      c->publish(c->state.bMeasuredFreqHz);
    }
  }
}

static uint8_t AdapterEvaluate(void *ctx, uint32_t lNowMs)
{
  FrequencyCaptureAdapterCtx_t *c = (FrequencyCaptureAdapterCtx_t *) ctx;
  tEFrequencyCaptureFSMVerdict v =
      FrequencyCaptureFSM_Evaluate(&c->state, &c->config, lNowMs);

  /* Port enum values match the FSM enum values 1:1 — same integer codes,
   * kept in sync by the port-interface unit tests (and, trivially, by these
   * casts not changing meaning). */
  return (uint8_t) v;
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void FrequencyCaptureAdapterInit(FrequencyCaptureAdapterCtx_t *ctx,
                                  const tSFrequencyCaptureFSMConfig *pCfg,
                                  FrequencyPublishCb_t publish,
                                  uint32_t lNowMs)
{
  ctx->config  = *pCfg;
  ctx->publish = publish;
  FrequencyCaptureFSM_Init(&ctx->state, lNowMs);
}

IFrequencyCapturePort_t FrequencyCaptureAdapterCreatePort(
    FrequencyCaptureAdapterCtx_t *ctx)
{
  IFrequencyCapturePort_t port;
  port.ctx       = ctx;
  port.Restart   = AdapterRestart;
  port.OnEdge    = AdapterOnEdge;
  port.Evaluate  = AdapterEvaluate;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
