/* App/Domain/Intersection/IntersectionOutputDispatcher.c
 *
 * Bridges the logical engine output image to the cabinet output path.
 */
#include "IntersectionOutputDispatcher.h"

#include <string.h>

static OutputDriverAspect_t ConvertAspect(IntersectionOutputAspect_t aspect)
{
  switch (aspect)
  {
      case INTERSECTION_OUTPUT_ASPECT_RED:
      {
        return OUTPUT_DRIVER_ASPECT_RED;
      }

      case INTERSECTION_OUTPUT_ASPECT_YELLOW:
      {
        return OUTPUT_DRIVER_ASPECT_YELLOW;
      }

      case INTERSECTION_OUTPUT_ASPECT_GREEN:
      {
        return OUTPUT_DRIVER_ASPECT_GREEN;
      }

      case INTERSECTION_OUTPUT_ASPECT_DARK:
      {
        return OUTPUT_DRIVER_ASPECT_DARK;
      }

      case INTERSECTION_OUTPUT_ASPECT_FLASH_RED:
      {
        return OUTPUT_DRIVER_ASPECT_FLASH_RED;
      }

      case INTERSECTION_OUTPUT_ASPECT_FLASH_GREEN:
      {
        return OUTPUT_DRIVER_ASPECT_FLASH_GREEN;
      }

      case INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW:
      default:
      {
        return OUTPUT_DRIVER_ASPECT_FLASH_YELLOW;
      }
  }
}

static void ConvertOutputIntentImage(
  const IntersectionOutputIntentImage_t *source,
  OutputDriverImage_t *target)
{
  uint8_t channelIndex;

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    target->channels[channelIndex] =
      ConvertAspect(source->channels[channelIndex]);
    target->channelDimmed[channelIndex] = source->channelDimmed[channelIndex];
    target->channelDimAlternateHalfCycle[channelIndex] =
      source->channelDimAlternateHalfCycle[channelIndex];
  }
}

void IntersectionOutputDispatcherInit(
  IntersectionOutputDispatcher_t *dispatcher)
{
  if (dispatcher == NULL)
  {
    return;
  }

  memset(dispatcher, 0, sizeof(*dispatcher));
}

void IntersectionOutputDispatcherBind(
  IntersectionOutputDispatcher_t *dispatcher,
  const IntersectionEngine_t *engine,
  IMmuPort_t *mmuPort,
  IOutputDriverPort_t *outputDriverPort)
{
  if (dispatcher == NULL)
  {
    return;
  }

  dispatcher->engine = engine;
  dispatcher->mmuPort = mmuPort;
  dispatcher->outputDriverPort = outputDriverPort;
  dispatcher->lastDispatchOk = 0U;
}

uint8_t IntersectionOutputDispatcherDispatch(
  IntersectionOutputDispatcher_t *dispatcher)
{
  IntersectionOutputIntentImage_t outputIntentImage;

  if ((dispatcher == NULL) || (dispatcher->engine == NULL)
      || (dispatcher->outputDriverPort == NULL))
  {
    return 0U;
  }

  if (IntersectionEngineGetOutputIntentImage(dispatcher->engine,
                                             &outputIntentImage) == 0U)
  {
    dispatcher->lastDispatchOk = 0U;

    return 0U;
  }

  ConvertOutputIntentImage(&outputIntentImage, &dispatcher->lastRequestedImage);

  if (MmuFilterOutputImage(dispatcher->mmuPort,
                           &dispatcher->lastRequestedImage,
                           &dispatcher->lastAppliedImage) == 0U)
  {
    dispatcher->lastDispatchOk = 0U;

    return 0U;
  }

  dispatcher->lastDispatchOk = OutputDriverApply(dispatcher->outputDriverPort,
                                                 &dispatcher->lastAppliedImage);

  return dispatcher->lastDispatchOk;
}

uint8_t IntersectionOutputDispatcherGetLastRequestedImage(
  const IntersectionOutputDispatcher_t *dispatcher,
  OutputDriverImage_t *
  image)
{
  if ((dispatcher == NULL) || (image == NULL))
  {
    return 0U;
  }

  *image = dispatcher->lastRequestedImage;

  return 1U;
}

uint8_t IntersectionOutputDispatcherGetLastAppliedImage(
  const IntersectionOutputDispatcher_t *dispatcher,
  OutputDriverImage_t *
  image)
{
  if ((dispatcher == NULL) || (image == NULL))
  {
    return 0U;
  }

  *image = dispatcher->lastAppliedImage;

  return 1U;
}
