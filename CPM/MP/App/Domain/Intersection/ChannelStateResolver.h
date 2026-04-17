/* App/Domain/Intersection/ChannelStateResolver.h
 *
 * Maps a 96-bit physical signal-output image (from the field bus) into
 * per-channel R/Y/G state triples using an MP-maintained
 * output-to-channel mapping.
 *
 * The mapping table is provided by ConfigurationService when CP
 * publishes it; the resolver itself holds no runtime state — it is a
 * pure transform.
 */
#ifndef CHANNEL_STATE_RESOLVER_H
#define CHANNEL_STATE_RESOLVER_H

#include <stdint.h>

#include "Intersection/IntersectionConfig.h"
#include "Ports/IFieldBusPort.h"

#define MP_SIGNAL_OUTPUT_COUNT_MAX 96U
#define MP_CHANNEL_COUNT_MAX INTERSECTION_CHANNEL_COUNT_MAX
#define MP_OUTPUT_CHANNEL_UNASSIGNED 0xFFU

typedef enum
{
  CHANNEL_COLOR_NONE = 0U,
  CHANNEL_COLOR_RED = 1U,
  CHANNEL_COLOR_YELLOW = 2U,
  CHANNEL_COLOR_GREEN = 3U
} ChannelColor_t;

typedef struct
{
  uint8_t channelIndex; /* 0..MP_CHANNEL_COUNT_MAX-1 or MP_OUTPUT_CHANNEL_UNASSIGNED */
  ChannelColor_t color;
} OutputChannelMap_t;

typedef struct
{
  OutputChannelMap_t outputs[MP_SIGNAL_OUTPUT_COUNT_MAX];
} ChannelOutputMapping_t;

typedef struct
{
  uint8_t red;
  uint8_t yellow;
  uint8_t green;
} ChannelColorState_t;

typedef struct
{
  ChannelColorState_t channels[MP_CHANNEL_COUNT_MAX];
} ChannelStateImage_t;

void ChannelStateResolverInit(ChannelOutputMapping_t *mapping);
uint8_t ChannelStateResolverResolveCommanded(
  const ChannelOutputMapping_t *mapping,
  const FieldBusLoadSwitchImage_t *
  commandedImage,
  ChannelStateImage_t *outState);

/* Derive per-channel measured R/Y/G from the observed SSM voltage
 * presence bits. The per-SSM voltage bits follow the same output
 * index order as the commanded image, so the mapping is identical. */
uint8_t ChannelStateResolverResolveMeasured(
  const ChannelOutputMapping_t *mapping,
  const FieldBusSsmTelemetry_t *ssm,
  uint8_t ssmCount,
  ChannelStateImage_t *outState);

uint8_t ChannelStateResolverGetOutputBit(const FieldBusLoadSwitchImage_t *image,
                                         uint8_t outputIndex);

#endif /* CHANNEL_STATE_RESOLVER_H */
