#pragma once

/*
 * App/Adapters/Mock/MockClock.h
 *
 * In-memory ISystemClockPort implementation for unit tests.
 * Tests advance time by writing to ctx.epoch directly.
 */
#include "Ports/ISystemClockPort.h"
#include <string.h>

typedef struct
{
  uint32_t epoch;               /* Current "wall clock" epoch — advance manually */
  int32_t dstOffsetSeconds;     /* Typically 0 for tests */
} MockClockCtx_t;

/* Initialise with epoch = 0, DST = 0. */
static inline void MockClock_Init(MockClockCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

/* Advance the clock by the given number of seconds. */
static inline void MockClock_AdvanceSeconds(MockClockCtx_t *ctx,
                                            uint32_t seconds)
{
  ctx->epoch += seconds;
}

/* Build an ISystemClockPort_t wired to ctx. */
ISystemClockPort_t MockClock_Create(MockClockCtx_t *ctx);
