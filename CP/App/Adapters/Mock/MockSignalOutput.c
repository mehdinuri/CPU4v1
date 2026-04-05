/*
 * App/Adapters/Mock/MockSignalOutput.c
 */
#include "MockSignalOutput.h"

static void mock_set_lamp(void *ctx, uint8_t outputId, SignalColor_t color)
{
  MockSignalOutputCtx_t *m = (MockSignalOutputCtx_t *) ctx;

  if (outputId < MOCK_SIGNAL_OUTPUT_LAMPS_MAX)
  {
    m->lamps[outputId] = color;
  }

  if (m->logCount < MOCK_SIGNAL_SET_LOG_MAX)
  {
    m->log[m->logCount].outputId = outputId;
    m->log[m->logCount].color = color;
    m->logCount++;
  }
}

static void mock_flush(void *ctx)
{
  ((MockSignalOutputCtx_t *) ctx)->flushCount++;
}

static bool mock_is_ready(void *ctx)
{
  return ((MockSignalOutputCtx_t *) ctx)->ready;
}

ISignalOutputPort_t MockSignalOutput_Create(MockSignalOutputCtx_t *ctx)
{
  ISignalOutputPort_t port;

  port.ctx = ctx;
  port.setLampState = mock_set_lamp;
  port.flush = mock_flush;
  port.isReady = mock_is_ready;

  return port;
}
