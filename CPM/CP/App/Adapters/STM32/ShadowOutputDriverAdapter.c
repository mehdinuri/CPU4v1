/* App/Adapters/STM32/ShadowOutputDriverAdapter.c */
#include "ShadowOutputDriverAdapter.h"

#include <string.h>

static uint8_t AdapterApply(void *ctx, const OutputDriverImage_t *image)
{
  ShadowOutputDriverAdapterCtx_t *adapter =
    (ShadowOutputDriverAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (image == NULL))
  {
    return 0U;
  }

  adapter->lastImage = *image;
  adapter->applyCount++;

  return 1U;
}

static uint8_t AdapterSetConfigEpoch(void *ctx, uint16_t configEpoch)
{
  ShadowOutputDriverAdapterCtx_t *adapter =
    (ShadowOutputDriverAdapterCtx_t *) ctx;

  if (adapter == NULL)
  {
    return 0U;
  }

  adapter->configEpoch = configEpoch;

  return 1U;
}

void ShadowOutputDriverAdapterInit(ShadowOutputDriverAdapterCtx_t *ctx,
                                   uint16_t configEpoch)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
  ctx->configEpoch = configEpoch;
}

IOutputDriverPort_t ShadowOutputDriverAdapterCreatePort(
  ShadowOutputDriverAdapterCtx_t *ctx)
{
  IOutputDriverPort_t port;

  port.ctx = ctx;
  port.Apply = AdapterApply;
  port.SetConfigEpoch = AdapterSetConfigEpoch;

  return port;
}
