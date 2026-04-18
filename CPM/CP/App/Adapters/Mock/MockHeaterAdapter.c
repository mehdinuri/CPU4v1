/* App/Adapters/Mock/MockHeaterAdapter.c */
#include "MockHeaterAdapter.h"

#include <stddef.h>

static void AdapterEnable(void *ctx)
{
  MockHeaterAdapterCtx_t *c = (MockHeaterAdapterCtx_t *) ctx;

  c->heaterOn = 1U;
  c->enableCount++;
}

static void AdapterDisable(void *ctx)
{
  MockHeaterAdapterCtx_t *c = (MockHeaterAdapterCtx_t *) ctx;

  c->heaterOn = 0U;
  c->disableCount++;
}

static uint8_t AdapterGetState(void *ctx)
{
  MockHeaterAdapterCtx_t *c = (MockHeaterAdapterCtx_t *) ctx;

  return (c != NULL) ? c->heaterOn : 0U;
}

void MockHeaterAdapterInit(MockHeaterAdapterCtx_t *ctx)
{
  ctx->heaterOn = 0U;
  ctx->enableCount = 0U;
  ctx->disableCount = 0U;
}

IHeaterPort_t MockHeaterAdapterCreatePort(MockHeaterAdapterCtx_t *ctx)
{
  IHeaterPort_t port;

  port.ctx = ctx;
  port.Enable = AdapterEnable;
  port.Disable = AdapterDisable;
  port.GetState = AdapterGetState;

  return port;
}
