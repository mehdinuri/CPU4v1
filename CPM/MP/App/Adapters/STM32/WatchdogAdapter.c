/* App/Adapters/STM32/WatchdogAdapter.c */

#include "WatchdogAdapter.h"

#include <stddef.h>

#include "iwdg.h"
#include "stm32g4xx_hal.h"

static uint8_t AdapterFeed(void *ctx)
{
  WatchdogAdapterCtx_t *self = (WatchdogAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK)
  {
    return 0U;
  }

  self->feedCount++;

  return 1U;
}

void WatchdogAdapterInit(WatchdogAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->feedCount = 0U;
}

IWatchdogPort_t WatchdogAdapterCreatePort(WatchdogAdapterCtx_t *ctx)
{
  IWatchdogPort_t port;

  port.ctx = ctx;
  port.Feed = AdapterFeed;

  return port;
}
