/* App/Adapters/STM32/RTCAdapter.h
 *
 * STM32 RTC adapter backed by the HAL RTC driver and RTC backup
 * registers.
 */
#ifndef RTC_ADAPTER_H
#define RTC_ADAPTER_H

#include "Ports/IRealtimeClockPort.h"

typedef struct
{
  uint8_t initialised;
} RTCAdapterCtx_t;

void RTCAdapterInit(RTCAdapterCtx_t *ctx);
IRealtimeClockPort_t RTCAdapterCreatePort(RTCAdapterCtx_t *ctx);

#endif /* RTC_ADAPTER_H */
