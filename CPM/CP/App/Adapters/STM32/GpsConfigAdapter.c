/* App/Adapters/STM32/GpsConfigAdapter.c */
#include "GpsConfigAdapter.h"

#include <string.h>

#include "gps.h"

static uint8_t GetPortType(void *ctx)
{
  (void) ctx;
  return GpsPortGet();
}

static void SetPortType(void *ctx, uint8_t type)
{
  (void) ctx;
  GpsPortSet(type);
}

static uint8_t GetBaudRateIndex(void *ctx)
{
  (void) ctx;
  return GpsBaudRateIndexGet();
}

static void SetBaudRateIndex(void *ctx, uint8_t index)
{
  (void) ctx;
  GpsBaudRateIndexSet(index);
}

static uint32_t IndexToBaudRate(void *ctx, uint8_t index)
{
  (void) ctx;
  return GpsIndexToBaudRate(index);
}

static uint8_t SaveConfig(void *ctx)
{
  uint8_t ok;

  (void) ctx;
  ok = GpsPortWrite();
  ok = (uint8_t) (ok && GpsBaudRateIndexWrite());
  return ok;
}

static uint8_t IsValidPortType(void *ctx, uint8_t type)
{
  (void) ctx;
  return (uint8_t) (type <= GPS_PORT_TYPE_MAX);
}

static uint8_t IsValidBaudRateIndex(void *ctx, uint8_t index)
{
  (void) ctx;
  return (uint8_t) ((index >= GPS_MIN_BAUD_RATE_INDEX)
                    && (index <= GPS_MAX_BAUD_RATE_INDEX));
}

void GpsConfigAdapterInit(GpsConfigAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
  }
}

IGpsPort_t GpsConfigAdapterCreatePort(GpsConfigAdapterCtx_t *ctx)
{
  IGpsPort_t port;

  port.ctx = ctx;
  port.GetPortType = GetPortType;
  port.SetPortType = SetPortType;
  port.GetBaudRateIndex = GetBaudRateIndex;
  port.SetBaudRateIndex = SetBaudRateIndex;
  port.IndexToBaudRate = IndexToBaudRate;
  port.SaveConfig = SaveConfig;
  port.IsValidPortType = IsValidPortType;
  port.IsValidBaudRateIndex = IsValidBaudRateIndex;
  return port;
}
