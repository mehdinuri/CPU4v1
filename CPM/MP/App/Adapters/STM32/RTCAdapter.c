/* App/Adapters/STM32/RTCAdapter.c */

#include "RTCAdapter.h"

#include <stddef.h>

#include "stm32g4xx_hal.h"

#define SECONDS_PER_MINUTE 60U
#define SECONDS_PER_HOUR   (60U * 60U)
#define SECONDS_PER_DAY    (24U * 60U * 60U)

static uint32_t DaysBeforeMonth(uint16_t year, uint8_t month)
{
  static const uint16_t cumulative[] =
  { 0U, 31U, 59U, 90U, 120U, 151U, 181U, 212U, 243U, 273U, 304U, 334U };

  uint32_t days = cumulative[(month > 0U) ? (month - 1U) : 0U];
  uint8_t isLeap = (uint8_t) (((year % 4U) == 0U)
                              && (((year % 100U) != 0U)
                                  || ((year % 400U) == 0U)));

  if ((isLeap != 0U) && (month > 2U))
  {
    days++;
  }

  return days;
}

static uint32_t ToEpoch(const RealtimeClockTime_t *time)
{
  uint32_t years = (uint32_t) time->year - 1970U;
  uint32_t leapYears = ((uint32_t) time->year / 4U)
                       - ((uint32_t) time->year / 100U)
                       + ((uint32_t) time->year / 400U)
                       - (1970U / 4U - 1970U / 100U + 1970U / 400U);
  uint32_t daysTotal = (years * 365U) + leapYears
                       + DaysBeforeMonth(time->year, time->month)
                       + (uint32_t) time->day - 1U;
  uint32_t seconds = (daysTotal * SECONDS_PER_DAY)
                     + ((uint32_t) time->hour * SECONDS_PER_HOUR)
                     + ((uint32_t) time->minute * SECONDS_PER_MINUTE)
                     + (uint32_t) time->second;

  return seconds;
}

static void FromEpoch(uint32_t epoch, RealtimeClockTime_t *out)
{
  uint32_t days = epoch / SECONDS_PER_DAY;
  uint32_t secondsOfDay = epoch - (days * SECONDS_PER_DAY);
  uint16_t year = 1970U;

  for (;;)
  {
    uint8_t leap = (uint8_t) (((year % 4U) == 0U)
                              && (((year % 100U) != 0U)
                                  || ((year % 400U) == 0U)));
    uint16_t yearDays = (leap != 0U) ? 366U : 365U;

    if (days < yearDays)
    {
      break;
    }

    days -= yearDays;
    year++;
  }

  out->year = year;
  out->hour = (uint8_t) (secondsOfDay / SECONDS_PER_HOUR);
  out->minute = (uint8_t) ((secondsOfDay % SECONDS_PER_HOUR) / 60U);
  out->second = (uint8_t) (secondsOfDay % 60U);
  out->weekday = (uint8_t) (((epoch / SECONDS_PER_DAY) + 4U) % 7U + 1U);

  uint8_t month = 1U;

  for (; month < 12U; month++)
  {
    uint32_t before = DaysBeforeMonth(year, month + 1U);
    uint32_t current = DaysBeforeMonth(year, month);

    if (days < (before - current))
    {
      break;
    }

    days -= (before - current);
  }

  out->month = month;
  out->day = (uint8_t) (days + 1U);
} /* FromEpoch */

static uint32_t CurrentEpochSeconds(const RTCAdapterCtx_t *ctx)
{
  uint32_t now = HAL_GetTick();
  uint32_t elapsedSeconds = (now - ctx->baseTicksMs) / 1000U;

  return ctx->baseEpochSeconds + elapsedSeconds;
}

static uint8_t AdapterGetTime(void *ctx, RealtimeClockTime_t *time)
{
  const RTCAdapterCtx_t *self = (const RTCAdapterCtx_t *) ctx;

  if ((self == NULL) || (time == NULL))
  {
    return 0U;
  }

  FromEpoch(CurrentEpochSeconds(self), time);

  return 1U;
}

static uint8_t AdapterSetTime(void *ctx, const RealtimeClockTime_t *time)
{
  RTCAdapterCtx_t *self = (RTCAdapterCtx_t *) ctx;

  if ((self == NULL) || (time == NULL))
  {
    return 0U;
  }

  self->baseEpochSeconds = ToEpoch(time);
  self->baseTicksMs = HAL_GetTick();

  return 1U;
}

static uint8_t AdapterGetEpochSeconds(void *ctx, uint32_t *epochSeconds)
{
  const RTCAdapterCtx_t *self = (const RTCAdapterCtx_t *) ctx;

  if ((self == NULL) || (epochSeconds == NULL))
  {
    return 0U;
  }

  *epochSeconds = CurrentEpochSeconds(self);

  return 1U;
}

void RTCAdapterInit(RTCAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  /* 2026-01-01 00:00:00 UTC as default until an external time source
   * writes the real clock via SetTime(). */
  RealtimeClockTime_t seed = {
    .year = 2026U, .month = 1U, .day = 1U,
    .hour = 0U, .minute = 0U, .second = 0U, .weekday = 4U
  };

  ctx->baseEpochSeconds = ToEpoch(&seed);
  ctx->baseTicksMs = HAL_GetTick();
}

IRealtimeClockPort_t RTCAdapterCreatePort(RTCAdapterCtx_t *ctx)
{
  IRealtimeClockPort_t port;

  port.ctx = ctx;
  port.GetTime = AdapterGetTime;
  port.SetTime = AdapterSetTime;
  port.GetEpochSeconds = AdapterGetEpochSeconds;

  return port;
}
