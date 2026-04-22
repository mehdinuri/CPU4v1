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
static void AdapterRestart(void *ctx, uint32_t nowMs)
{
  FrequencyCaptureAdapterCtx_t *c = (FrequencyCaptureAdapterCtx_t *) ctx;
  FrequencyCaptureFSM_Init(&c->state, nowMs);
}

static void AdapterOnEdge(void *ctx, uint32_t captureValue, uint32_t nowMs)
{
  FrequencyCaptureAdapterCtx_t *c = (FrequencyCaptureAdapterCtx_t *) ctx;

  if (FrequencyCaptureFSM_OnEdge(&c->state, &c->config, captureValue, nowMs)
      != 0U)
  {
    if (c->publish != NULL)
    {
      c->publish(c->state.measuredFreqHz);
    }
  }
}

static uint8_t AdapterEvaluate(void *ctx, uint32_t nowMs)
{
  FrequencyCaptureAdapterCtx_t *c = (FrequencyCaptureAdapterCtx_t *) ctx;
  FrequencyCaptureFSMVerdict_e v =
      FrequencyCaptureFSM_Evaluate(&c->state, &c->config, nowMs);

  /* Port enum values match the FSM enum values 1:1 — same integer codes,
   * kept in sync by the port-interface unit tests (and, trivially, by these
   * casts not changing meaning). */
  return (uint8_t) v;
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void FrequencyCaptureAdapterInit(FrequencyCaptureAdapterCtx_t *ctx,
                                  const FrequencyCaptureFSMConfig_t *cfg,
                                  FrequencyPublishCb_t publish,
                                  uint32_t nowMs)
{
  ctx->config  = *cfg;
  ctx->publish = publish;
  FrequencyCaptureFSM_Init(&ctx->state, nowMs);
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
