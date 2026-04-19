/* App/Adapters/STM32/GpsTimeSyncAdapter.c */
#include "GpsTimeSyncAdapter.h"

#include <string.h>

#include "gps.h"

static uint8_t AdapterIsGpsAlive(void *ctx)
{
  (void) ctx;
  return GpsModemAliveGet();
}

static uint8_t AdapterIsRtcInitialUpdateDone(void *ctx)
{
  (void) ctx;
  return GpsRTCInitialUpdateDoneGet();
}

static void AdapterInvalidateRtcInitialUpdate(void *ctx)
{
  (void) ctx;
  GpsRTCInitialUpdateDoneSet(0U);
}

void GpsTimeSyncAdapterInit(GpsTimeSyncAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
  }
}

IGpsTimeSyncPort_t GpsTimeSyncAdapterCreatePort(GpsTimeSyncAdapterCtx_t *ctx)
{
  IGpsTimeSyncPort_t port;

  port.ctx = ctx;
  port.IsGpsAlive = AdapterIsGpsAlive;
  port.IsRtcInitialUpdateDone = AdapterIsRtcInitialUpdateDone;
  port.InvalidateRtcInitialUpdate = AdapterInvalidateRtcInitialUpdate;
  return port;
}
