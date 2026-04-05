#pragma once

/*
 * App/Ports/IDetectorInputPort.h
 *
 * Inbound vehicle Detector state. The concrete adapter reads from the IO
 * module CAN frames (or GPIO) and exposes polled per-Detector state.
 * In tests, the mock adapter injects Detector events directly.
 */
#include <stdint.h>
#include <stdbool.h>

/* Current occupancy state for one Detector. */
typedef enum
{
  DETECTOR_STATE_EMPTY  = 0,   /* No vehicle present */
  DETECTOR_STATE_BUSY   = 1,   /* Vehicle detected */
  DETECTOR_STATE_BROKEN = 2,   /* Detector hardware fault */
} DetectorState_t;

typedef struct IDetectorInputPort
{
  void *ctx;

  /* Poll the occupancy state of Detector at index (0-based). */
  DetectorState_t (*getState)(void *ctx, uint8_t DetectorIndex);

  /* Number of demand pulses observed since the last call to clearDemand().
   * A "demand pulse" is a rising edge on the Detector input. */
  uint8_t (*getDemandCount)(void *ctx, uint8_t DetectorIndex);

  /* Reset the demand counter for a Detector (called at start of each phase). */
  void (*clearDemand)(void *ctx, uint8_t DetectorIndex);

  /* Cumulative occupancy duration (ms) since the last clearOccupancy() call. */
  uint32_t (*getOccupancyMs)(void *ctx, uint8_t DetectorIndex);

  /* Reset the occupancy accumulator (called at start of each phase). */
  void (*clearOccupancy)(void *ctx, uint8_t DetectorIndex);
} IDetectorInputPort_t;

static inline DetectorState_t Detector_GetState(IDetectorInputPort_t *p,
                                                uint8_t idx)
{
  return p->getState(p->ctx, idx);
}

static inline uint8_t Detector_GetDemandCount(IDetectorInputPort_t *p,
                                              uint8_t idx)
{
  return p->getDemandCount(p->ctx, idx);
}

static inline void Detector_ClearDemand(IDetectorInputPort_t *p, uint8_t idx)
{
  p->clearDemand(p->ctx, idx);
}

static inline uint32_t Detector_GetOccupancyMs(IDetectorInputPort_t *p,
                                               uint8_t idx)
{
  return p->getOccupancyMs(p->ctx, idx);
}

static inline void Detector_ClearOccupancy(IDetectorInputPort_t *p, uint8_t idx)
{
  p->clearOccupancy(p->ctx, idx);
}
