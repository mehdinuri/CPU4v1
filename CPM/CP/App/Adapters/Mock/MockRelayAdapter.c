/* App/Adapters/Mock/MockRelayAdapter.c */
#include "MockRelayAdapter.h"

static void AdapterSet(void *ctx, uint8_t on)
{
  MockRelayAdapterCtx_t *c = (MockRelayAdapterCtx_t *) ctx;

  c->relayState = (on != 0U) ? 1U : 0U;
  c->setCount++;
}

static uint8_t AdapterGet(void *ctx)
{
  MockRelayAdapterCtx_t *c = (MockRelayAdapterCtx_t *) ctx;

  return c->relayState;
}

void MockRelayAdapterInit(MockRelayAdapterCtx_t *ctx)
{
  ctx->relayState = 0U;
  ctx->setCount = 0U;
}

IRelayPort_t MockRelayAdapterCreatePort(MockRelayAdapterCtx_t *ctx)
{
  IRelayPort_t port;

  port.ctx = ctx;
  port.Set = AdapterSet;
  port.Get = AdapterGet;

  return port;
}
