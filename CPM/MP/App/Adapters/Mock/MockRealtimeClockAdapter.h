/* App/Adapters/Mock/MockRealtimeClockAdapter.h */
#ifndef MOCK_REALTIME_CLOCK_ADAPTER_H
#define MOCK_REALTIME_CLOCK_ADAPTER_H

#include "Ports/IRealtimeClockPort.h"

typedef struct
{
  RealtimeClockTime_t time;
  uint32_t epochSeconds;
} MockRealtimeClockAdapterCtx_t;

void MockRealtimeClockAdapterInit(MockRealtimeClockAdapterCtx_t *ctx);
IRealtimeClockPort_t MockRealtimeClockAdapterCreatePort(
  MockRealtimeClockAdapterCtx_t *ctx);

#endif /* MOCK_REALTIME_CLOCK_ADAPTER_H */
