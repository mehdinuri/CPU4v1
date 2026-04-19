/* App/Adapters/STM32/SystemResetAdapter.c */
#include "SystemResetAdapter.h"

#include "data.h"

static void RequestReset(void *ctx)
{
  (void) ctx;
  SecureSystemReset();
}

void SystemResetAdapterInit(SystemResetAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    ctx->reserved0 = 0U;
  }
}

ISystemResetPort_t SystemResetAdapterCreatePort(SystemResetAdapterCtx_t *ctx)
{
  ISystemResetPort_t port;

  port.ctx = ctx;
  port.RequestReset = RequestReset;

  return port;
}
