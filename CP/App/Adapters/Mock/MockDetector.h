#pragma once

/*
 * App/Adapters/Mock/MockDetector.h
 *
 * In-memory IDetectorInputPort implementation for unit tests.
 * Tests inject Detector events by directly modifying ctx fields.
 */
#include "Ports/IDetectorInputPort.h"
#include <string.h>

#define MOCK_DETECTOR_MAX  32U   /* matches DETECTORS_MAX */

typedef struct
{
  DetectorState_t state[MOCK_DETECTOR_MAX];
  uint8_t demandCount[MOCK_DETECTOR_MAX];
  uint32_t occupancyMs[MOCK_DETECTOR_MAX];
} MockDetectorCtx_t;

/* Initialise ctx: all Detectors EMPTY, zero demand/occupancy. */
static inline void MockDetector_Init(MockDetectorCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

/* Convenience: set a Detector to BUSY and bump its demand count by 1. */
static inline void MockDetector_SetBusy(MockDetectorCtx_t *ctx, uint8_t idx)
{
  if (idx < MOCK_DETECTOR_MAX)
  {
    ctx->state[idx] = DETECTOR_STATE_BUSY;
    ctx->demandCount[idx]++;
  }
}

/* Convenience: put a Detector into BROKEN state. */
static inline void MockDetector_SetBroken(MockDetectorCtx_t *ctx, uint8_t idx)
{
  if (idx < MOCK_DETECTOR_MAX)
  {
    ctx->state[idx] = DETECTOR_STATE_BROKEN;
  }
}

/* Convenience: clear a Detector back to EMPTY. */
static inline void MockDetector_SetEmpty(MockDetectorCtx_t *ctx, uint8_t idx)
{
  if (idx < MOCK_DETECTOR_MAX)
  {
    ctx->state[idx] = DETECTOR_STATE_EMPTY;
  }
}

/* Build an IDetectorInputPort_t wired to ctx. */
IDetectorInputPort_t MockDetector_Create(MockDetectorCtx_t *ctx);
