/* App/Domain/Services/OutputTestService.c */
#include "OutputTestService.h"

#include <string.h>

static void SetAllDark(OutputDriverImage_t *image)
{
  uint8_t channelIndex;

  if (image == NULL)
  {
    return;
  }

  (void) memset(image, 0, sizeof(*image));
  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    image->channels[channelIndex] = OUTPUT_DRIVER_ASPECT_DARK;
  }
}

void OutputTestServiceInit(OutputTestService_t *service)
{
  uint8_t channelIndex;

  if (service == NULL)
  {
    return;
  }

  (void) memset(service, 0, sizeof(*service));
  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    service->forcedAspects[channelIndex] = OUTPUT_DRIVER_ASPECT_DARK;
  }
}

uint8_t OutputTestServiceSetEnabled(OutputTestService_t *service,
                                    uint8_t enabled)
{
  uint8_t normalized;

  if (service == NULL)
  {
    return 0U;
  }

  normalized = (uint8_t) (enabled != 0U);
  if (service->enabled != normalized)
  {
    service->enabled = normalized;
    service->changeSequence++;
  }

  return 1U;
}

uint8_t OutputTestServiceSetChannelAspect(OutputTestService_t *service,
                                          uint8_t channelNumber,
                                          OutputDriverAspect_t aspect)
{
  uint8_t index;
  uint16_t mask;

  if ((service == NULL) || (channelNumber == 0U)
      || (channelNumber > INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  index = (uint8_t) (channelNumber - 1U);
  mask = (uint16_t) (1U << index);
  service->enabled = 1U;
  service->forcedMask |= mask;
  service->forcedAspects[index] = aspect;
  service->changeSequence++;
  return 1U;
}

uint8_t OutputTestServiceClearChannel(OutputTestService_t *service,
                                      uint8_t channelNumber)
{
  uint8_t index;
  uint16_t mask;

  if ((service == NULL) || (channelNumber == 0U)
      || (channelNumber > INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  index = (uint8_t) (channelNumber - 1U);
  mask = (uint16_t) (1U << index);
  service->forcedMask &= (uint16_t) ~mask;
  service->forcedAspects[index] = OUTPUT_DRIVER_ASPECT_DARK;
  service->changeSequence++;
  return 1U;
}

uint8_t OutputTestServiceApply(const OutputTestService_t *service,
                               const OutputDriverImage_t *requested,
                               OutputDriverImage_t *target)
{
  uint8_t channelIndex;

  if ((service == NULL) || (requested == NULL) || (target == NULL))
  {
    return 0U;
  }

  if (service->enabled == 0U)
  {
    *target = *requested;
    return 1U;
  }

  SetAllDark(target);

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    uint16_t mask = (uint16_t) (1U << channelIndex);

    if ((service->forcedMask & mask) != 0U)
    {
      target->channels[channelIndex] = service->forcedAspects[channelIndex];
    }
  }

  return 1U;
}

uint8_t OutputTestServiceIsEnabled(const OutputTestService_t *service)
{
  return (service == NULL) ? 0U : service->enabled;
}

uint16_t OutputTestServiceGetForcedMask(const OutputTestService_t *service)
{
  return (service == NULL) ? 0U : service->forcedMask;
}

uint8_t OutputTestServiceGetChannelAspect(const OutputTestService_t *service,
                                          uint8_t channelNumber,
                                          OutputDriverAspect_t *aspect)
{
  uint8_t index;
  uint16_t mask;

  if ((service == NULL) || (aspect == NULL) || (channelNumber == 0U)
      || (channelNumber > INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  index = (uint8_t) (channelNumber - 1U);
  mask = (uint16_t) (1U << index);
  if ((service->forcedMask & mask) == 0U)
  {
    return 0U;
  }

  *aspect = service->forcedAspects[index];
  return 1U;
}

uint32_t OutputTestServiceGetChangeSequence(
  const OutputTestService_t *service)
{
  return (service == NULL) ? 0U : service->changeSequence;
}
