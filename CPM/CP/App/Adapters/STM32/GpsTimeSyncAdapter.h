/* App/Adapters/STM32/GpsTimeSyncAdapter.h */
#ifndef GPS_TIME_SYNC_ADAPTER_H
#define GPS_TIME_SYNC_ADAPTER_H

#include "Ports/IGpsTimeSyncPort.h"

typedef struct
{
  uint8_t reserved;
} GpsTimeSyncAdapterCtx_t;

void GpsTimeSyncAdapterInit(GpsTimeSyncAdapterCtx_t *ctx);
IGpsTimeSyncPort_t GpsTimeSyncAdapterCreatePort(GpsTimeSyncAdapterCtx_t *ctx);

#endif /* GPS_TIME_SYNC_ADAPTER_H */
