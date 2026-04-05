/*
 * App/Adapters/STM32/DetectorInputAdapter.c
 *
 * IDetectorInputPort implementation — CAN-sourced Detector I/O adapter.
 *
 * CAN frame Format (8 bytes, one Detector status frame per IO module):
 *   Byte 0: base Detector index (first Detector reported in this frame)
 *   Bytes 1-7: packed Detector status
 *     Bits [1:0] per nibble: 00=EMPTY, 01=BUSY, 10=BROKEN
 *     Each byte covers two Detectors (lower nibble = first, upper = second)
 *
 * TODO: Confirm exact CAN frame Format with SSM hardware documentation.
 */
#include "DetectorInputAdapter.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------*/
static DetectorState_t DetInput_GetState(void *ctx, uint8_t DetectorIndex);
static uint8_t DetInput_GetDemandCount(void *ctx, uint8_t DetectorIndex);
static void DetInput_ClearDemand(void *ctx, uint8_t DetectorIndex);
static uint32_t DetInput_GetOccupancyMs(void *ctx, uint8_t DetectorIndex);
static void DetInput_ClearOccupancy(void *ctx, uint8_t DetectorIndex);

/* --------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------*/

void DetectorInputAdapter_Init(DetectorInputAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
  /* All Detectors start as EMPTY — safe default while waiting for first CAN frame. */
  for (uint8_t i = 0U; i < DETECTORS_MAX; i++)
  {
    ctx->state[i] = DETECTOR_STATE_EMPTY;
  }
}

IDetectorInputPort_t DetectorInputAdapter_CreatePort(
  DetectorInputAdapterCtx_t *ctx)
{
  IDetectorInputPort_t port;

  port.ctx = ctx;
  port.getState = DetInput_GetState;
  port.getDemandCount = DetInput_GetDemandCount;
  port.clearDemand = DetInput_ClearDemand;
  port.getOccupancyMs = DetInput_GetOccupancyMs;
  port.clearOccupancy = DetInput_ClearOccupancy;

  return port;
}

void DetectorInputAdapter_UpdateFromCAN(DetectorInputAdapterCtx_t *ctx,
                                        const uint8_t *canPayload,
                                        uint8_t payloadLen)
{
  /* TODO: HAL impl — parse Detector CAN frame into ctx->state[],
   * ctx->demandCount[], and ctx->occupancyMs[].
   *
   * Skeleton implementation assumes:
   *   canPayload[0] = base Detector index
   *   canPayload[1..7] = packed 2-bit status per Detector
   *     (bits 1:0 = Detector[base + 0*(byte-1)], bits 3:2 = Detector[base+1], ...)
   *
   * Each byte covers 4 Detectors packed as 2 bits each.
   * A rising edge (EMPTY → BUSY transition) increments demandCount.
   */

  if ((payloadLen < 2U) || (canPayload == NULL) )
  {
    return;
  }

  uint8_t baseIdx = canPayload[0];

  for (uint8_t byteIdx = 1U; byteIdx < payloadLen; byteIdx++)
  {
    uint8_t packed = canPayload[byteIdx];

    for (uint8_t nibble = 0U; nibble < 4U; nibble++)
    {
      uint8_t detIdx = baseIdx + (uint8_t) ((byteIdx - 1U) * 4U + nibble);

      if (detIdx >= DETECTORS_MAX)
      {
        break;
      }

      uint8_t rawState = (packed >> (nibble * 2U)) & 0x03U;
      DetectorState_t newState;

      switch (rawState)
      {
          case 1U:
          { newState = DETECTOR_STATE_BUSY;   break; }

          case 2U:
          { newState = DETECTOR_STATE_BROKEN; break; }

          default:
          { newState = DETECTOR_STATE_EMPTY;  break; }
      }

      /* Detect rising edge for demand counting. */
      if ((ctx->state[detIdx] == DETECTOR_STATE_EMPTY)
          && (newState == DETECTOR_STATE_BUSY) )
      {
        if (ctx->demandCount[detIdx] < 0xFFU)
        {
          ctx->demandCount[detIdx]++;
        }
      }

      /* TODO: Accumulate occupancyMs — requires a tick timestamp source.
       * Integrate with the system clock adapter or a free-running HAL timer. */

      ctx->state[detIdx] = newState;
    }
  }
} /* DetectorInputAdapter_UpdateFromCAN */

/* --------------------------------------------------------------------------
 * Port callbacks
 * --------------------------------------------------------------------------*/

static DetectorState_t DetInput_GetState(void *vctx, uint8_t DetectorIndex)
{
  DetectorInputAdapterCtx_t *ctx = (DetectorInputAdapterCtx_t *) vctx;

  if (DetectorIndex >= DETECTORS_MAX)
  {
    return DETECTOR_STATE_BROKEN;
  }

  return ctx->state[DetectorIndex];
}

static uint8_t DetInput_GetDemandCount(void *vctx, uint8_t DetectorIndex)
{
  DetectorInputAdapterCtx_t *ctx = (DetectorInputAdapterCtx_t *) vctx;

  if (DetectorIndex >= DETECTORS_MAX)
  {
    return 0U;
  }

  return ctx->demandCount[DetectorIndex];
}

static void DetInput_ClearDemand(void *vctx, uint8_t DetectorIndex)
{
  DetectorInputAdapterCtx_t *ctx = (DetectorInputAdapterCtx_t *) vctx;

  if (DetectorIndex < DETECTORS_MAX)
  {
    ctx->demandCount[DetectorIndex] = 0U;
  }
}

static uint32_t DetInput_GetOccupancyMs(void *vctx, uint8_t DetectorIndex)
{
  DetectorInputAdapterCtx_t *ctx = (DetectorInputAdapterCtx_t *) vctx;

  if (DetectorIndex >= DETECTORS_MAX)
  {
    return 0U;
  }

  return ctx->occupancyMs[DetectorIndex];
}

static void DetInput_ClearOccupancy(void *vctx, uint8_t DetectorIndex)
{
  DetectorInputAdapterCtx_t *ctx = (DetectorInputAdapterCtx_t *) vctx;

  if (DetectorIndex < DETECTORS_MAX)
  {
    ctx->occupancyMs[DetectorIndex] = 0U;
  }
}
