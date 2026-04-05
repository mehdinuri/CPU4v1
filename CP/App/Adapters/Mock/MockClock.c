/*
 * App/Adapters/Mock/MockClock.c
 */
#include "MockClock.h"

static uint32_t mock_get_epoch(void *ctx)
{
  return ((MockClockCtx_t *) ctx)->epoch;
}

static bool mock_set_epoch(void *ctx, uint32_t epoch)
{
  ((MockClockCtx_t *) ctx)->epoch = epoch;

  return true;
}

static int32_t mock_get_dst(void *ctx)
{
  return ((MockClockCtx_t *) ctx)->dstOffsetSeconds;
}

ISystemClockPort_t MockClock_Create(MockClockCtx_t *ctx)
{
  ISystemClockPort_t port;

  port.ctx = ctx;
  port.getEpoch = mock_get_epoch;
  port.setEpoch = mock_set_epoch;
  port.getDstOffsetSeconds = mock_get_dst;

  return port;
}
