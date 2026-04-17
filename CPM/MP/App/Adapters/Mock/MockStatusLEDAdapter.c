/* App/Adapters/Mock/MockStatusLEDAdapter.c */

#include "MockStatusLEDAdapter.h"

#include <string.h>

static uint8_t MockSetState(void *ctx, StatusLEDState_t state)
{
  MockStatusLEDAdapterCtx_t *self = (MockStatusLEDAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  self->lastState = state;
  self->setCallCount++;

  return 1U;
}

void MockStatusLEDAdapterInit(MockStatusLEDAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
  ctx->lastState = STATUS_LED_STATE_OFF;
}

IStatusLEDPort_t MockStatusLEDAdapterCreatePort(MockStatusLEDAdapterCtx_t *ctx)
{
  IStatusLEDPort_t port;

  port.ctx = ctx;
  port.SetState = MockSetState;

  return port;
}
