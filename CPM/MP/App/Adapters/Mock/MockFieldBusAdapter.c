/* App/Adapters/Mock/MockFieldBusAdapter.c */

#include "MockFieldBusAdapter.h"

#include <string.h>

static uint8_t MockReadSnapshot(void *ctx, FieldBusSnapshot_t *snapshot)
{
  MockFieldBusAdapterCtx_t *self = (MockFieldBusAdapterCtx_t *) ctx;

  if ((self == NULL) || (snapshot == NULL))
  {
    return 0U;
  }

  *snapshot = self->snapshot;
  self->readCount++;

  return 1U;
}

void MockFieldBusAdapterInit(MockFieldBusAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
}

IFieldBusPort_t MockFieldBusAdapterCreatePort(MockFieldBusAdapterCtx_t *ctx)
{
  IFieldBusPort_t port;

  port.ctx = ctx;
  port.ReadSnapshot = MockReadSnapshot;

  return port;
}

void MockFieldBusAdapterSetSnapshot(MockFieldBusAdapterCtx_t *ctx,
                                    const FieldBusSnapshot_t *snapshot)
{
  if ((ctx == NULL) || (snapshot == NULL))
  {
    return;
  }

  ctx->snapshot = *snapshot;
}
