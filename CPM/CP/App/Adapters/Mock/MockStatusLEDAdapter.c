/* App/Adapters/Mock/MockStatusLEDAdapter.c */
#include "MockStatusLEDAdapter.h"

static void AdapterToggle(void *ctx)
{
  MockStatusLEDAdapterCtx_t *c = (MockStatusLEDAdapterCtx_t *) ctx;

  c->ledState ^= 1U;
  c->toggleCount++;
}

static void AdapterSetState(void *ctx, uint8_t on)
{
  MockStatusLEDAdapterCtx_t *c = (MockStatusLEDAdapterCtx_t *) ctx;

  c->ledState = (on != 0U) ? 1U : 0U;
}

void MockStatusLEDAdapterInit(MockStatusLEDAdapterCtx_t *ctx)
{
  ctx->ledState = 0U;
  ctx->toggleCount = 0U;
}

IStatusLEDPort_t MockStatusLEDAdapterCreatePort(MockStatusLEDAdapterCtx_t *ctx)
{
  IStatusLEDPort_t port;

  port.ctx = ctx;
  port.Toggle = AdapterToggle;
  port.SetState = AdapterSetState;

  return port;
}
