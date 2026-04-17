/* App/Adapters/Mock/MockControlBusAdapter.c */

#include "MockControlBusAdapter.h"

#include <string.h>

static uint8_t MockSendFrame(void *ctx, const ControlBusFrame_t *frame)
{
  MockControlBusAdapterCtx_t *self = (MockControlBusAdapterCtx_t *) ctx;

  if ((self == NULL) || (frame == NULL))
  {
    return 0U;
  }

  if (self->txCount >= MOCK_CONTROL_BUS_TX_BUFFER)
  {
    return 0U;
  }

  self->txBuffer[self->txCount] = *frame;
  self->txCount++;

  return 1U;
}

static uint8_t MockRegisterRxCallback(void *ctx,
                                      ControlBusRxCallback_t cb,
                                      void *cbCtx)
{
  MockControlBusAdapterCtx_t *self = (MockControlBusAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  self->rxCallback = cb;
  self->rxCallbackCtx = cbCtx;

  return 1U;
}

void MockControlBusAdapterInit(MockControlBusAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
}

IControlBusPort_t MockControlBusAdapterCreatePort(
  MockControlBusAdapterCtx_t *ctx)
{
  IControlBusPort_t port;

  port.ctx = ctx;
  port.SendFrame = MockSendFrame;
  port.RegisterRxCallback = MockRegisterRxCallback;

  return port;
}

void MockControlBusAdapterInjectRxFrame(MockControlBusAdapterCtx_t *ctx,
                                        const ControlBusFrame_t *frame)
{
  if ((ctx == NULL) || (frame == NULL))
  {
    return;
  }

  if (ctx->rxCallback != NULL)
  {
    ctx->rxCallback(ctx->rxCallbackCtx, frame);
  }
}
