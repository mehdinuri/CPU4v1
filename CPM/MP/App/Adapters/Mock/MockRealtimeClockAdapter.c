/* App/Adapters/Mock/MockRealtimeClockAdapter.c */

#include "MockRealtimeClockAdapter.h"

#include <string.h>

static uint8_t MockGetTime(void *ctx, RealtimeClockTime_t *time)
{
  const MockRealtimeClockAdapterCtx_t *self =
    (const MockRealtimeClockAdapterCtx_t *) ctx;

  if ((self == NULL) || (time == NULL))
  {
    return 0U;
  }

  *time = self->time;

  return 1U;
}

static uint8_t MockSetTime(void *ctx, const RealtimeClockTime_t *time)
{
  MockRealtimeClockAdapterCtx_t *self =
    (MockRealtimeClockAdapterCtx_t *) ctx;

  if ((self == NULL) || (time == NULL))
  {
    return 0U;
  }

  self->time = *time;

  return 1U;
}

static uint8_t MockGetEpochSeconds(void *ctx, uint32_t *epochSeconds)
{
  const MockRealtimeClockAdapterCtx_t *self =
    (const MockRealtimeClockAdapterCtx_t *) ctx;

  if ((self == NULL) || (epochSeconds == NULL))
  {
    return 0U;
  }

  *epochSeconds = self->epochSeconds;

  return 1U;
}

void MockRealtimeClockAdapterInit(MockRealtimeClockAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
  ctx->time.year = 2026U;
  ctx->time.month = 1U;
  ctx->time.day = 1U;
}

IRealtimeClockPort_t MockRealtimeClockAdapterCreatePort(
  MockRealtimeClockAdapterCtx_t *ctx)
{
  IRealtimeClockPort_t port;

  port.ctx = ctx;
  port.GetTime = MockGetTime;
  port.SetTime = MockSetTime;
  port.GetEpochSeconds = MockGetEpochSeconds;

  return port;
}
