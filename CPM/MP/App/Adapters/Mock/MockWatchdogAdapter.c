/* App/Adapters/Mock/MockWatchdogAdapter.c */

#include "MockWatchdogAdapter.h"

#include <string.h>

static uint8_t MockFeed(void *ctx)
{
  MockWatchdogAdapterCtx_t *self = (MockWatchdogAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  self->feedCount++;

  return 1U;
}

void MockWatchdogAdapterInit(MockWatchdogAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
}

IWatchdogPort_t MockWatchdogAdapterCreatePort(MockWatchdogAdapterCtx_t *ctx)
{
  IWatchdogPort_t port;

  port.ctx = ctx;
  port.Feed = MockFeed;

  return port;
}
