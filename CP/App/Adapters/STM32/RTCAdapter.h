#pragma once

/*
 * App/Adapters/STM32/RTCAdapter.h
 *
 * ISystemClockPort concrete implementation for STM32H743.
 * Wraps the HAL RTC peripheral.  The epoch is maintained as a combination
 * of a stored base offset and elapsed time derived from the RTC registers.
 * DST offset is a runtime-settable signed integer (seconds).
 */
#include "Ports/ISystemClockPort.h"

#ifdef STM32H743xx
#include "stm32h7xx_hal.h"
#endif

typedef struct
{
  uint32_t epochOffset;          /* Epoch value stored at last setEpoch() call    */
  uint32_t rtcBaseSeconds;       /* RTC elapsed seconds at the time of setEpoch() */
  int32_t dstOffsetSeconds;      /* DST offset currently in force (signed, s)     */

  #ifdef STM32H743xx
  RTC_HandleTypeDef *hrtc;       /* HAL RTC peripheral handle                     */
  #else
  void *hrtc;                    /* Placeholder for host builds                   */
  #endif
} RTCAdapterCtx_t;

/**
 * Initialise the adapter, storing the RTC handle.
 * epochOffset is set to 0 (call setEpoch() to prime from GPS or SNMP).
 */
#ifdef STM32H743xx
void RTCAdapter_Init(RTCAdapterCtx_t *ctx, RTC_HandleTypeDef *hrtc);

#else
void RTCAdapter_Init(RTCAdapterCtx_t *ctx, void *hrtc);

#endif

/** Build an ISystemClockPort_t wired to ctx. */
ISystemClockPort_t RTCAdapter_CreatePort(RTCAdapterCtx_t *ctx);
