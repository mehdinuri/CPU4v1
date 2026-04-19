/* App/Adapters/STM32/GpsConfigAdapter.h
 *
 * Persisted GPS configuration adapter.
 */
#ifndef GPS_CONFIG_ADAPTER_H
#define GPS_CONFIG_ADAPTER_H

#include "Ports/IGpsPort.h"

typedef struct
{
  uint8_t reserved;
} GpsConfigAdapterCtx_t;

void GpsConfigAdapterInit(GpsConfigAdapterCtx_t *ctx);
IGpsPort_t GpsConfigAdapterCreatePort(GpsConfigAdapterCtx_t *ctx);

#endif /* GPS_CONFIG_ADAPTER_H */
