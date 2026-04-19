/* App/Adapters/STM32/ModemConfigAdapter.c */
#include "ModemConfigAdapter.h"

#include <string.h>

#include "MCS.h"

static uint8_t GetModemType(void *ctx)
{
  (void) ctx;
  return MCSGetModemType();
}

static void SetModemType(void *ctx, uint8_t modemType)
{
  (void) ctx;
  MCSSetModemType(modemType);
}

static uint8_t SaveConfig(void *ctx)
{
  (void) ctx;
  return MCSWriteConInfo();
}

static uint8_t IsValidModemType(void *ctx, uint8_t modemType)
{
  (void) ctx;
  return (uint8_t) (modemType < (uint8_t) MCS_MODULE_TYPE_MAX);
}

void ModemConfigAdapterInit(ModemConfigAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
  }
}

IModemConfigPort_t ModemConfigAdapterCreatePort(ModemConfigAdapterCtx_t *ctx)
{
  IModemConfigPort_t port;

  port.ctx = ctx;
  port.GetModemType = GetModemType;
  port.SetModemType = SetModemType;
  port.SaveConfig = SaveConfig;
  port.IsValidModemType = IsValidModemType;
  return port;
}
