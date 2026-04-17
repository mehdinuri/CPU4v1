/* App/Ports/IOutputDriverPort.h
 *
 * Port interface for applying the controller output image to the cabinet
 * output path.
 */
#ifndef IOUTPUT_DRIVER_PORT_H
#define IOUTPUT_DRIVER_PORT_H

#include <stdint.h>

#include "Domain/Intersection/IntersectionConfig.h"

typedef enum
{
  OUTPUT_DRIVER_ASPECT_RED = 0,
  OUTPUT_DRIVER_ASPECT_YELLOW,
  OUTPUT_DRIVER_ASPECT_GREEN,
  OUTPUT_DRIVER_ASPECT_DARK,
  OUTPUT_DRIVER_ASPECT_FLASH_RED,
  OUTPUT_DRIVER_ASPECT_FLASH_YELLOW
} OutputDriverAspect_t;

typedef struct
{
  OutputDriverAspect_t channels[INTERSECTION_CHANNEL_COUNT_MAX];
  uint8_t channelDimmed[INTERSECTION_CHANNEL_COUNT_MAX];
  uint8_t channelDimAlternateHalfCycle[INTERSECTION_CHANNEL_COUNT_MAX];
} OutputDriverImage_t;

typedef struct
{
  void *ctx;

  uint8_t (*Apply)(void *ctx, const OutputDriverImage_t *image);
  uint8_t (*SetConfigEpoch)(void *ctx, uint16_t configEpoch);
} IOutputDriverPort_t;

static inline uint8_t OutputDriverApply(IOutputDriverPort_t *p,
                                        const OutputDriverImage_t *image)
{
  return p->Apply(p->ctx, image);
}

static inline uint8_t OutputDriverSetConfigEpoch(IOutputDriverPort_t *p,
                                                 uint16_t configEpoch)
{
  if ((p == NULL) || (p->SetConfigEpoch == NULL))
  {
    return 1U;
  }

  return p->SetConfigEpoch(p->ctx, configEpoch);
}

#endif /* IOUTPUT_DRIVER_PORT_H */
