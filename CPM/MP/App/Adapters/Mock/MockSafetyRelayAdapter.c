/* App/Adapters/Mock/MockSafetyRelayAdapter.c */

#include "MockSafetyRelayAdapter.h"

#include <stddef.h>

static uint8_t MockSetState(void *ctx, SafetyRelayState_t state)
{
  MockSafetyRelayAdapterCtx_t *self = (MockSafetyRelayAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  if (self->commandedState != state)
  {
    self->transitionCount++;
  }

  self->commandedState = state;
  self->actualState = state;

  return 1U;
}

static uint8_t MockGetCommandedState(void *ctx, SafetyRelayState_t *state)
{
  const MockSafetyRelayAdapterCtx_t *self =
    (const MockSafetyRelayAdapterCtx_t *) ctx;

  if ((self == NULL) || (state == NULL))
  {
    return 0U;
  }

  *state = self->commandedState;

  return 1U;
}

static uint8_t MockGetActualState(void *ctx, SafetyRelayState_t *state)
{
  const MockSafetyRelayAdapterCtx_t *self =
    (const MockSafetyRelayAdapterCtx_t *) ctx;

  if ((self == NULL) || (state == NULL))
  {
    return 0U;
  }

  *state = self->actualState;

  return 1U;
}

void MockSafetyRelayAdapterInit(MockSafetyRelayAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->commandedState = SAFETY_RELAY_STATE_OPEN;
  ctx->actualState = SAFETY_RELAY_STATE_OPEN;
  ctx->transitionCount = 0U;
}

ISafetyRelayPort_t MockSafetyRelayAdapterCreatePort(
  MockSafetyRelayAdapterCtx_t *ctx)
{
  ISafetyRelayPort_t port;

  port.ctx = ctx;
  port.SetState = MockSetState;
  port.GetCommandedState = MockGetCommandedState;
  port.GetActualState = MockGetActualState;

  return port;
}
