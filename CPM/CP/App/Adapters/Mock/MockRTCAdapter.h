/* App/Adapters/Mock/MockRTCAdapter.h
 *
 * IRealtimeClockPort in-memory test double.
 */
#ifndef MOCK_RTC_ADAPTER_H
#define MOCK_RTC_ADAPTER_H

#include "Ports/IRealtimeClockPort.h"

typedef struct
{
  RtcSnapshot_t snapshot;
  uint32_t initSignature;
  uint32_t centuryMetadata;
  uint32_t adjustmentCount;
  RtcDstAdjustment_t lastAdjustment;
} MockRTCAdapterCtx_t;

void MockRTCAdapterInit(MockRTCAdapterCtx_t *ctx);
IRealtimeClockPort_t MockRTCAdapterCreatePort(MockRTCAdapterCtx_t *ctx);

#endif /* MOCK_RTC_ADAPTER_H */
