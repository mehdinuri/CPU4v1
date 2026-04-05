/*
 * App/Adapters/STM32/RTCAdapter.c
 *
 * ISystemClockPort implementation — STM32H743 RTC peripheral.
 *
 * Epoch derivation strategy:
 *   At setEpoch(e):
 *     ctx->epochOffset    = e
 *     ctx->rtcBaseSeconds = RTC_ReadElapsedSeconds()   (BCD → binary)
 *
 *   At getEpoch():
 *     elapsed = RTC_ReadElapsedSeconds() - ctx->rtcBaseSeconds
 *     return ctx->epochOffset + elapsed
 *
 * NOTE: The BCD → binary helper is a stub (returns 0) until the HAL
 * calls are wired in.  Replace with real HAL_RTC_GetTime / HAL_RTC_GetDate
 * calls inside the #ifdef STM32H743xx guards below.
 */
#include "RTCAdapter.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------*/
static uint32_t RTC_GetEpoch(void *ctx);
static bool RTC_SetEpoch(void *ctx, uint32_t epoch);
static int32_t RTC_GetDstOffsetSeconds(void *ctx);

/* --------------------------------------------------------------------------
 * Internal helper: read RTC as elapsed seconds since last setEpoch().
 * Returns 0 on host builds (no HAL available).
 * --------------------------------------------------------------------------*/
static uint32_t RTC_ReadElapsedSeconds(RTCAdapterCtx_t *ctx)
{
  #ifdef STM32H743xx

  /* TODO: HAL impl — read BCD time/date registers and convert to seconds.
   *
   * RTC_TimeTypeDef sTime = {0};
   * RTC_DateTypeDef sDate = {0};
   * HAL_RTC_GetTime(ctx->hrtc, &sTime, RTC_FORMAT_BIN);
   * HAL_RTC_GetDate(ctx->hrtc, &sDate, RTC_FORMAT_BIN);
   *
   * Convert sDate.Year/Month/Date + sTime.Hours/Minutes/Seconds to epoch.
   * A simple approach: use a local mktime-equivalent operating on a
   * fixed epoch base (e.g. 2000-01-01 = 946684800).
   *
   * return computed_epoch;
   */
  (void) ctx;

  return 0U;
  #else
  (void) ctx;

  return 0U;
  #endif
}

/* --------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------*/

#ifdef STM32H743xx
void RTCAdapter_Init(RTCAdapterCtx_t *ctx, RTC_HandleTypeDef *hrtc)
#else
void RTCAdapter_Init(RTCAdapterCtx_t *ctx, void *hrtc)
#endif
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->hrtc = hrtc;
  ctx->epochOffset = 0U;
  ctx->rtcBaseSeconds = 0U;
  ctx->dstOffsetSeconds = 0;
}

ISystemClockPort_t RTCAdapter_CreatePort(RTCAdapterCtx_t *ctx)
{
  ISystemClockPort_t port;

  port.ctx = ctx;
  port.getEpoch = RTC_GetEpoch;
  port.setEpoch = RTC_SetEpoch;
  port.getDstOffsetSeconds = RTC_GetDstOffsetSeconds;

  return port;
}

/* --------------------------------------------------------------------------
 * Port callbacks
 * --------------------------------------------------------------------------*/

static uint32_t RTC_GetEpoch(void *vctx)
{
  RTCAdapterCtx_t *ctx = (RTCAdapterCtx_t *) vctx;
  uint32_t elapsed = RTC_ReadElapsedSeconds(ctx);

  /* Guard against underflow if RTC has been reset. */
  if (elapsed < ctx->rtcBaseSeconds)
  {
    return ctx->epochOffset;
  }

  return ctx->epochOffset + (elapsed - ctx->rtcBaseSeconds);
}

static bool RTC_SetEpoch(void *vctx, uint32_t epoch)
{
  RTCAdapterCtx_t *ctx = (RTCAdapterCtx_t *) vctx;

  ctx->epochOffset = epoch;
  ctx->rtcBaseSeconds = RTC_ReadElapsedSeconds(ctx);

  #ifdef STM32H743xx

  /* TODO: HAL impl — Program the new time into the RTC peripheral.
   *
   * Convert epoch to year/month/day/hour/min/sec, then:
   * RTC_TimeTypeDef sTime = { .Hours = ..., .Minutes = ..., .Seconds = ... };
   * RTC_DateTypeDef sDate = { .Year = ..., .Month = ..., .Date = ... };
   * HAL_RTC_SetTime(ctx->hrtc, &sTime, RTC_FORMAT_BIN);
   * HAL_RTC_SetDate(ctx->hrtc, &sDate, RTC_FORMAT_BIN);
   */
  #endif

  return true;
}

static int32_t RTC_GetDstOffsetSeconds(void *vctx)
{
  RTCAdapterCtx_t *ctx = (RTCAdapterCtx_t *) vctx;

  return ctx->dstOffsetSeconds;
}
