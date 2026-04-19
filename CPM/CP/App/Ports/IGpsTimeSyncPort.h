/* App/Ports/IGpsTimeSyncPort.h
 *
 * Port for GPS-driven RTC sync status used by local maintenance features.
 */
#ifndef IGPS_TIME_SYNC_PORT_H
#define IGPS_TIME_SYNC_PORT_H

#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*IsGpsAlive)(void *ctx);
  uint8_t (*IsRtcInitialUpdateDone)(void *ctx);
  void (*InvalidateRtcInitialUpdate)(void *ctx);
} IGpsTimeSyncPort_t;

static inline uint8_t GpsTimeSyncPort_IsGpsAlive(IGpsTimeSyncPort_t *p)
{
  return p->IsGpsAlive(p->ctx);
}

static inline uint8_t GpsTimeSyncPort_IsRtcInitialUpdateDone(IGpsTimeSyncPort_t *p)
{
  return p->IsRtcInitialUpdateDone(p->ctx);
}

static inline void GpsTimeSyncPort_InvalidateRtcInitialUpdate(
  IGpsTimeSyncPort_t *p)
{
  p->InvalidateRtcInitialUpdate(p->ctx);
}

#endif /* IGPS_TIME_SYNC_PORT_H */
