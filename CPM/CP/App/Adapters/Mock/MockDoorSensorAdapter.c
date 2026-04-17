/* App/Adapters/Mock/MockDoorSensorAdapter.c */
#include "MockDoorSensorAdapter.h"

static uint8_t AdapterIsOpen(void *ctx)
{
  MockDoorSensorAdapterCtx_t *c = (MockDoorSensorAdapterCtx_t *) ctx;

  c->readCount++;

  return c->doorOpen;
}

void MockDoorSensorAdapterInit(MockDoorSensorAdapterCtx_t *ctx)
{
  ctx->doorOpen = 0U;
  ctx->readCount = 0U;
}

IDoorSensorPort_t MockDoorSensorAdapterCreatePort(
  MockDoorSensorAdapterCtx_t *ctx)
{
  IDoorSensorPort_t port;

  port.ctx = ctx;
  port.IsOpen = AdapterIsOpen;

  return port;
}
