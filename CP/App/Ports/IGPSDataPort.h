#pragma once

/*
 * App/Ports/IGPSDataPort.h
 *
 * GNSS position and time data. The concrete adapter parses NMEA sentences
 * from UART5; the mock injects fix data for time-sync tests.
 */
#include <stdint.h>
#include <stdbool.h>

typedef struct GpsFix
{
  bool isValid;           /* True when receiver has a 3D fix */
  int32_t latitudeE7;     /* Degrees × 10^7 (e.g. 41.0082°N = 410082000) */
  int32_t longitudeE7;    /* Degrees × 10^7 */
  int32_t altitudeM;      /* Altitude in metres */
  uint32_t epoch;         /* UTC epoch derived from NMEA GPRMC sentence */
  uint8_t satellites;     /* Number of satellites used in fix */
} GpsFix_t;

typedef struct IGPSDataPort
{
  void *ctx;

  /* Copy the most recent fix into [outFix].
   * outFix->isValid is false until the first valid NMEA sentence arrives. */
  void (*getLastFix)(void *ctx, GpsFix_t *outFix);
} IGPSDataPort_t;

static inline void GPS_GetLastFix(IGPSDataPort_t *p, GpsFix_t *out)
{
  p->getLastFix(p->ctx, out);
}
