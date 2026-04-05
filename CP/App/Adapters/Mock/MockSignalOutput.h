#pragma once

/*
 * App/Adapters/Mock/MockSignalOutput.h
 *
 * In-memory ISignalOutputPort implementation for unit tests.
 * Records every lamp state change so tests can assert on the resulting
 * lamp array; counts flush() calls; isReady() returns a configurable flag.
 */
#include "Ports/ISignalOutputPort.h"
#include <string.h>

#define MOCK_SIGNAL_OUTPUT_LAMPS_MAX  96U   /* matches SIGNAL_OUTPUTS_MAX */
#define MOCK_SIGNAL_SET_LOG_MAX       256U  /* max individual set-lamp calls to log */

typedef struct
{
  uint8_t outputId;
  SignalColor_t color;
} MockLampSetEntry_t;

typedef struct
{
  SignalColor_t lamps[MOCK_SIGNAL_OUTPUT_LAMPS_MAX];       /* latest state per lamp */
  MockLampSetEntry_t log[MOCK_SIGNAL_SET_LOG_MAX];         /* ordered call log */
  uint32_t logCount;                                       /* entries used in log[] */
  uint32_t flushCount;
  bool ready;
} MockSignalOutputCtx_t;

/* Initialise ctx to all-red, zero flush count, ready=true. */
static inline void MockSignalOutput_Init(MockSignalOutputCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
  for (uint32_t i = 0U; i < MOCK_SIGNAL_OUTPUT_LAMPS_MAX; i++)
  {
    ctx->lamps[i] = SIGNAL_COLOR_RED;
  }

  ctx->ready = true;
}

/* Build an ISignalOutputPort_t wired to ctx. */
ISignalOutputPort_t MockSignalOutput_Create(MockSignalOutputCtx_t *ctx);
