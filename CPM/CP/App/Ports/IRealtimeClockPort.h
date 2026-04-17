/* App/Ports/IRealtimeClockPort.h
 *
 * Port interface for a battery-backed real-time clock with backup
 * registers used for small metadata values.
 */
#ifndef IREALTIME_CLOCK_PORT_H
#define IREALTIME_CLOCK_PORT_H

#include <stdint.h>

typedef struct
{
  uint8_t Century;
  uint8_t Year;
  uint8_t Month;
  uint8_t Date;
  uint8_t WeekDay;
  uint8_t Hours;
  uint8_t Minutes;
  uint8_t Seconds;
} RtcSnapshot_t;

typedef enum
{
  RTC_META_INIT_SIGNATURE = 0,
  RTC_META_CENTURY
} RtcMetadataId_t;

typedef enum
{
  RTC_DST_ADD_ONE_HOUR = 0,
  RTC_DST_SUBTRACT_ONE_HOUR
} RtcDstAdjustment_t;

typedef struct
{
  void *ctx;

  uint8_t (*ReadSnapshot)(void *ctx, RtcSnapshot_t *snapshot);
  uint8_t (*WriteSnapshot)(void *ctx, const RtcSnapshot_t *snapshot);
  uint8_t (*ReadMetadata)(void *ctx, RtcMetadataId_t metadataId,
                          uint32_t *value);
  uint8_t (*WriteMetadata)(void *ctx, RtcMetadataId_t metadataId,
                           uint32_t value);
  uint8_t (*ApplyDstAdjustment)(void *ctx, RtcDstAdjustment_t adjustment);
} IRealtimeClockPort_t;

static inline uint8_t RealtimeClockReadSnapshot(IRealtimeClockPort_t *p,
                                                RtcSnapshot_t *snapshot)
{
  return p->ReadSnapshot(p->ctx, snapshot);
}

static inline uint8_t RealtimeClockWriteSnapshot(IRealtimeClockPort_t *p,
                                                 const RtcSnapshot_t *snapshot)
{
  return p->WriteSnapshot(p->ctx, snapshot);
}

static inline uint8_t RealtimeClockReadMetadata(IRealtimeClockPort_t *p,
                                                RtcMetadataId_t metadataId,
                                                uint32_t *value)
{
  return p->ReadMetadata(p->ctx, metadataId, value);
}

static inline uint8_t RealtimeClockWriteMetadata(IRealtimeClockPort_t *p,
                                                 RtcMetadataId_t metadataId,
                                                 uint32_t value)
{
  return p->WriteMetadata(p->ctx, metadataId, value);
}

static inline uint8_t RealtimeClockApplyDstAdjustment(IRealtimeClockPort_t *p,
                                                      RtcDstAdjustment_t adj)
{
  return p->ApplyDstAdjustment(p->ctx, adj);
}

#endif /* IREALTIME_CLOCK_PORT_H */
