/* App/Ports/IRealtimeClockPort.h
 *
 * Port interface for the MP real-time clock. Used for fault
 * timestamping. Broken-down time is UTC; the adapter handles
 * local-time and DST if required downstream.
 */
#ifndef I_REALTIME_CLOCK_PORT_H
#define I_REALTIME_CLOCK_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  uint16_t year;   /* full year, e.g. 2026 */
  uint8_t month;   /* 1-12 */
  uint8_t day;     /* 1-31 */
  uint8_t hour;    /* 0-23 */
  uint8_t minute;  /* 0-59 */
  uint8_t second;  /* 0-59 */
  uint8_t weekday; /* 1-7, 1=Monday */
} RealtimeClockTime_t;

typedef struct
{
  void *ctx;

  uint8_t (*GetTime)(void *ctx, RealtimeClockTime_t *time);
  uint8_t (*SetTime)(void *ctx, const RealtimeClockTime_t *time);
  uint8_t (*GetEpochSeconds)(void *ctx, uint32_t *epochSeconds);
} IRealtimeClockPort_t;

static inline uint8_t RealtimeClockGetTime(const IRealtimeClockPort_t *port,
                                           RealtimeClockTime_t *time)
{
  if ((port == NULL) || (port->GetTime == NULL))
  {
    return 0U;
  }

  return port->GetTime(port->ctx, time);
}

static inline uint8_t RealtimeClockSetTime(IRealtimeClockPort_t *port,
                                           const RealtimeClockTime_t *time)
{
  if ((port == NULL) || (port->SetTime == NULL))
  {
    return 0U;
  }

  return port->SetTime(port->ctx, time);
}

static inline uint8_t RealtimeClockGetEpochSeconds(
  const IRealtimeClockPort_t *port,
  uint32_t *epochSeconds)
{
  if ((port == NULL) || (port->GetEpochSeconds == NULL))
  {
    return 0U;
  }

  return port->GetEpochSeconds(port->ctx, epochSeconds);
}

#endif /* I_REALTIME_CLOCK_PORT_H */
