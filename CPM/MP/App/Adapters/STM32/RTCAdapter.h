/* App/Adapters/STM32/RTCAdapter.h
 *
 * IRealtimeClockPort implementation. STM32G473 on the MP board does
 * not have the RTC peripheral enabled in CubeMX at the time of this
 * port; we service the port from HAL_GetTick() + a base-epoch stored
 * in RAM. When RTC is enabled, the Init path can be extended to copy
 * the calendar from RTC_TimeTypeDef / RTC_DateTypeDef.
 */
#ifndef RTC_ADAPTER_H
#define RTC_ADAPTER_H

#include "Ports/IRealtimeClockPort.h"

typedef struct
{
  uint32_t baseEpochSeconds;
  uint32_t baseTicksMs;
} RTCAdapterCtx_t;

void RTCAdapterInit(RTCAdapterCtx_t *ctx);
IRealtimeClockPort_t RTCAdapterCreatePort(RTCAdapterCtx_t *ctx);

#endif /* RTC_ADAPTER_H */
