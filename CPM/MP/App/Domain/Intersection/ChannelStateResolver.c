/* App/Domain/Intersection/ChannelStateResolver.c */

#include "Intersection/ChannelStateResolver.h"

#include <stddef.h>
#include <string.h>

void ChannelStateResolverInit(ChannelOutputMapping_t *mapping)
{
  uint32_t i;

  if (mapping == NULL)
  {
    return;
  }

  for (i = 0U; i < MP_SIGNAL_OUTPUT_COUNT_MAX; i++)
  {
    mapping->outputs[i].channelIndex = MP_OUTPUT_CHANNEL_UNASSIGNED;
    mapping->outputs[i].color = CHANNEL_COLOR_NONE;
  }
}

uint8_t ChannelStateResolverGetOutputBit(const FieldBusLoadSwitchImage_t *image,
                                         uint8_t outputIndex)
{
  uint8_t byteIndex;
  uint8_t bitOffset;

  if ((image == NULL) || (outputIndex >= MP_SIGNAL_OUTPUT_COUNT_MAX))
  {
    return 0U;
  }

  byteIndex = outputIndex >> 3U;
  bitOffset = outputIndex & 0x07U;

  return (uint8_t) ((image->bits[byteIndex] >> bitOffset) & 0x01U);
}

static void ApplyChannelBit(ChannelColorState_t *state,
                            ChannelColor_t color,
                            uint8_t on)
{
  switch (color)
  {
      case CHANNEL_COLOR_RED:
      {
        state->red = on;
        break;
      }

      case CHANNEL_COLOR_YELLOW:
      {
        state->yellow = on;
        break;
      }

      case CHANNEL_COLOR_GREEN:
      {
        state->green = on;
        break;
      }

      case CHANNEL_COLOR_NONE:
      default:
      {
        /* unassigned lamps contribute nothing */
        break;
      }
  }
}

uint8_t ChannelStateResolverResolveCommanded(
  const ChannelOutputMapping_t *mapping,
  const FieldBusLoadSwitchImage_t *
  commandedImage,
  ChannelStateImage_t *outState)
{
  uint32_t i;

  if ((mapping == NULL) || (commandedImage == NULL) || (outState == NULL))
  {
    return 0U;
  }

  (void) memset(outState, 0, sizeof(*outState));

  for (i = 0U; i < MP_SIGNAL_OUTPUT_COUNT_MAX; i++)
  {
    const OutputChannelMap_t *m = &mapping->outputs[i];

    if (m->channelIndex >= MP_CHANNEL_COUNT_MAX)
    {
      continue;
    }

    uint8_t on =
      ChannelStateResolverGetOutputBit(commandedImage, (uint8_t) i);

    ApplyChannelBit(&outState->channels[m->channelIndex], m->color, on);
  }

  return 1U;
}

uint8_t ChannelStateResolverResolveMeasured(
  const ChannelOutputMapping_t *mapping,
  const FieldBusSsmTelemetry_t *ssm,
  uint8_t ssmCount,
  ChannelStateImage_t *outState)
{
  uint32_t moduleIdx;
  uint32_t slot;
  uint32_t outputIdx;

  if ((mapping == NULL) || (ssm == NULL) || (outState == NULL))
  {
    return 0U;
  }

  if (ssmCount > FIELD_BUS_SSM_COUNT)
  {
    return 0U;
  }

  (void) memset(outState, 0, sizeof(*outState));

  for (moduleIdx = 0U; moduleIdx < ssmCount; moduleIdx++)
  {
    for (slot = 0U; slot < FIELD_BUS_SSM_OUTPUTS_PER_MODULE; slot++)
    {
      outputIdx = (moduleIdx * FIELD_BUS_SSM_OUTPUTS_PER_MODULE) + slot;

      if (outputIdx >= MP_SIGNAL_OUTPUT_COUNT_MAX)
      {
        break;
      }

      const OutputChannelMap_t *m = &mapping->outputs[outputIdx];

      if (m->channelIndex >= MP_CHANNEL_COUNT_MAX)
      {
        continue;
      }

      uint8_t on =
        (uint8_t) ((ssm[moduleIdx].voltagePresenceBits >> slot) & 0x01U);

      ApplyChannelBit(&outState->channels[m->channelIndex], m->color, on);
    }
  }

  return 1U;
} /* ChannelStateResolverResolveMeasured */
