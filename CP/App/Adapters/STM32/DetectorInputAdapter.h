#pragma once

/*
 * App/Adapters/STM32/DetectorInputAdapter.h
 *
 * IDetectorInputPort concrete implementation for STM32H743.
 * State is populated by DetectorInputAdapter_UpdateFromCAN(), which is
 * called from CANRxTask whenever a Detector status frame arrives on the bus.
 * The Domain polls state through the port interface each 100 ms tick.
 */
#include "Ports/IDetectorInputPort.h"
#include "Domain/Intersection/Types.h"

typedef struct
{
  DetectorState_t state[DETECTORS_MAX];           /* Current occupancy state    */
  uint8_t demandCount[DETECTORS_MAX];             /* Rising-edge demand counter */
  uint32_t occupancyMs[DETECTORS_MAX];            /* Cumulative occupancy (ms)  */
} DetectorInputAdapterCtx_t;

/**
 * Initialise the adapter context to all-empty, zero counters.
 * Must be called before DetectorInputAdapter_CreatePort().
 */
void DetectorInputAdapter_Init(DetectorInputAdapterCtx_t *ctx);

/**
 * Build an IDetectorInputPort_t wired to ctx.
 */
IDetectorInputPort_t DetectorInputAdapter_CreatePort(
  DetectorInputAdapterCtx_t *ctx);

/**
 * Parse a CAN frame payload received from the Detector I/O module and
 * update ctx state/demand/occupancy arrays accordingly.
 * Called from CANRxTask — not from Domain code.
 *
 * canPayload  : raw bytes from the CAN frame data field
 * payloadLen  : number of valid bytes in canPayload (max 8)
 */
void DetectorInputAdapter_UpdateFromCAN(DetectorInputAdapterCtx_t *ctx,
                                        const uint8_t *canPayload,
                                        uint8_t payloadLen);
