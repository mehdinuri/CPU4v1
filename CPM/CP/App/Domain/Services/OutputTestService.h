/* App/Domain/Services/OutputTestService.h */
#ifndef OUTPUT_TEST_SERVICE_H
#define OUTPUT_TEST_SERVICE_H

#include <stdint.h>

#include "Ports/IOutputDriverPort.h"

typedef struct
{
  uint8_t enabled;
  uint32_t forcedMask;
  OutputDriverAspect_t forcedAspects[INTERSECTION_CHANNEL_COUNT_MAX];
  uint32_t changeSequence;
} OutputTestService_t;

void OutputTestServiceInit(OutputTestService_t *service);
uint8_t OutputTestServiceSetEnabled(OutputTestService_t *service,
                                    uint8_t enabled);
uint8_t OutputTestServiceSetChannelAspect(OutputTestService_t *service,
                                          uint8_t channelNumber,
                                          OutputDriverAspect_t aspect);
uint8_t OutputTestServiceClearChannel(OutputTestService_t *service,
                                      uint8_t channelNumber);
uint8_t OutputTestServiceApply(const OutputTestService_t *service,
                               const OutputDriverImage_t *requested,
                               OutputDriverImage_t *target);
uint8_t OutputTestServiceIsEnabled(const OutputTestService_t *service);
uint32_t OutputTestServiceGetForcedMask(const OutputTestService_t *service);
uint8_t OutputTestServiceGetChannelAspect(const OutputTestService_t *service,
                                          uint8_t channelNumber,
                                          OutputDriverAspect_t *aspect);
uint32_t OutputTestServiceGetChangeSequence(
  const OutputTestService_t *service);

#endif /* OUTPUT_TEST_SERVICE_H */
