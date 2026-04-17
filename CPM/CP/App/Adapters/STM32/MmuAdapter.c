/* App/Adapters/STM32/MmuAdapter.c
 *
 * Conservative MMU adapter. It is a platform-side safety seam, not the final
 * MMU implementation. The current behavior is explicit and testable:
 * pass through normally, or force all-red on every channel.
 */
#include "MmuAdapter.h"

#include <string.h>

static void SetAllRed(OutputDriverImage_t *image)
{
  uint8_t channelIndex;

  if (image == NULL)
  {
    return;
  }

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       ++channelIndex)
  {
    image->channels[channelIndex] = OUTPUT_DRIVER_ASPECT_RED;
    image->channelDimmed[channelIndex] = 0U;
    image->channelDimAlternateHalfCycle[channelIndex] = 0U;
  }
}

static uint8_t FilterOutputImage(void *ctx,
                                 const OutputDriverImage_t *requested,
                                 OutputDriverImage_t *approved)
{
  MmuAdapterCtx_t *adapterCtx = (MmuAdapterCtx_t *) ctx;

  if ((adapterCtx == NULL) || (requested == NULL) || (approved == NULL))
  {
    return 0U;
  }

  adapterCtx->lastRequestedImage = *requested;

  if (adapterCtx->forceAllRed != 0U)
  {
    SetAllRed(approved);
  }
  else
  {
    *approved = *requested;
  }

  adapterCtx->lastApprovedImage = *approved;

  return 1U;
}

static uint8_t SetForceAllRedPort(void *ctx, uint8_t forceAllRed)
{
  MmuAdapterSetForceAllRed((MmuAdapterCtx_t *) ctx, forceAllRed);

  return 1U;
}

void MmuAdapterInit(MmuAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  memset(ctx, 0, sizeof(*ctx));
  SetAllRed(&ctx->lastRequestedImage);
  SetAllRed(&ctx->lastApprovedImage);
}

void MmuAdapterSetForceAllRed(MmuAdapterCtx_t *ctx, uint8_t forceAllRed)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->forceAllRed = (uint8_t) (forceAllRed != 0U);
}

IMmuPort_t MmuAdapterCreatePort(MmuAdapterCtx_t *ctx)
{
  IMmuPort_t port;

  port.ctx = ctx;
  port.SetForceAllRed = SetForceAllRedPort;
  port.FilterOutputImage = FilterOutputImage;

  return port;
}
