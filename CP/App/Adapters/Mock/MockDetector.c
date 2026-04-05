/*
 * App/Adapters/Mock/MockDetector.c
 */
#include "MockDetector.h"

static DetectorState_t mock_get_state(void *ctx, uint8_t idx)
{
  MockDetectorCtx_t *m = (MockDetectorCtx_t *) ctx;

  if (idx >= MOCK_DETECTOR_MAX)
  {
    return DETECTOR_STATE_EMPTY;
  }

  return m->state[idx];
}

static uint8_t mock_get_demand(void *ctx, uint8_t idx)
{
  MockDetectorCtx_t *m = (MockDetectorCtx_t *) ctx;

  if (idx >= MOCK_DETECTOR_MAX)
  {
    return 0U;
  }

  return m->demandCount[idx];
}

static void mock_clear_demand(void *ctx, uint8_t idx)
{
  MockDetectorCtx_t *m = (MockDetectorCtx_t *) ctx;

  if (idx < MOCK_DETECTOR_MAX)
  {
    m->demandCount[idx] = 0U;
  }
}

static uint32_t mock_get_occupancy(void *ctx, uint8_t idx)
{
  MockDetectorCtx_t *m = (MockDetectorCtx_t *) ctx;

  if (idx >= MOCK_DETECTOR_MAX)
  {
    return 0U;
  }

  return m->occupancyMs[idx];
}

static void mock_clear_occupancy(void *ctx, uint8_t idx)
{
  MockDetectorCtx_t *m = (MockDetectorCtx_t *) ctx;

  if (idx < MOCK_DETECTOR_MAX)
  {
    m->occupancyMs[idx] = 0U;
  }
}

IDetectorInputPort_t MockDetector_Create(MockDetectorCtx_t *ctx)
{
  IDetectorInputPort_t port;

  port.ctx = ctx;
  port.getState = mock_get_state;
  port.getDemandCount = mock_get_demand;
  port.clearDemand = mock_clear_demand;
  port.getOccupancyMs = mock_get_occupancy;
  port.clearOccupancy = mock_clear_occupancy;

  return port;
}
