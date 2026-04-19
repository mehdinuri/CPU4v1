/* App/Adapters/STM32/FactoryResetAdapter.c */
#include "FactoryResetAdapter.h"

#include <string.h>

#include "data.h"

static uint8_t RequestFactoryReset(void *ctx)
{
  (void) ctx;
  ReturnFactorySettings();
  return 1U;
}

void FactoryResetAdapterInit(FactoryResetAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
  }
}

IFactoryResetPort_t FactoryResetAdapterCreatePort(FactoryResetAdapterCtx_t *ctx)
{
  IFactoryResetPort_t port;

  port.ctx = ctx;
  port.RequestFactoryReset = RequestFactoryReset;
  return port;
}
