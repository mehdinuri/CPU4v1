/* App/Adapters/Mock/MockStatusLEDAdapter.h */
#ifndef MOCK_STATUS_LED_ADAPTER_H
#define MOCK_STATUS_LED_ADAPTER_H

#include "Ports/IStatusLEDPort.h"

typedef struct
{
  StatusLEDState_t lastState;
  uint32_t setCallCount;
} MockStatusLEDAdapterCtx_t;

void MockStatusLEDAdapterInit(MockStatusLEDAdapterCtx_t *ctx);
IStatusLEDPort_t MockStatusLEDAdapterCreatePort(MockStatusLEDAdapterCtx_t *ctx);

#endif /* MOCK_STATUS_LED_ADAPTER_H */
