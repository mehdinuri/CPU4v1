/* App/Adapters/Mock/MockSafetyRelayAdapter.c */

#include "MockSafetyRelayAdapter.h"

#include <stddef.h>

static void RefreshDrives(MockSafetyRelayAdapterCtx_t *self)
{
  uint8_t relayDrive;

  if (self == NULL)
  {
    return;
  }

  relayDrive = (self->commandedState == SAFETY_RELAY_STATE_CLOSED) ? 1U : 0U;
  if (self->topology == SAFETY_RELAY_TOPOLOGY_ECO_ACTIVE_HIGH_TRIP)
  {
    relayDrive = (uint8_t) (relayDrive == 0U);
  }

  self->actualState = self->commandedState;
  self->lastRelayDrive = relayDrive;
  self->lastTriacDrive = (self->commandedState == SAFETY_RELAY_STATE_CLOSED)
                          ? 0U : 1U;
}

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
  RefreshDrives(self);

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
  ctx->topology = SAFETY_RELAY_TOPOLOGY_LEGACY_ACTIVE_HIGH_CLOSE;
  ctx->lastRelayDrive = 0U;
  ctx->lastTriacDrive = 1U;
  ctx->transitionCount = 0U;
}

void MockSafetyRelayAdapterSetTopology(MockSafetyRelayAdapterCtx_t *ctx,
                                       SafetyRelayTopology_t topology)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->topology = topology;
  RefreshDrives(ctx);
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
