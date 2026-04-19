/* App/Adapters/STM32/PowerMonitorAdapter.c */
#include "PowerMonitorAdapter.h"

#include <stddef.h>

enum
{
  POWER_MONITOR_SOURCE_UNKNOWN = 1,
  POWER_MONITOR_SOURCE_OTHER = 2,
  POWER_MONITOR_SOURCE_AC_LINE = 3
};

static uint8_t AdapterGetPrimarySource(void *ctx, uint8_t *powerSource)
{
  PowerMonitorAdapterCtx_t *adapter = (PowerMonitorAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (powerSource == NULL))
  {
    return 0U;
  }

  *powerSource = adapter->primarySource;

  return 1U;
}

static uint8_t AdapterGetLineVoltageTenthsVrms(void *ctx, uint16_t *lineVoltage)
{
  PowerMonitorAdapterCtx_t *adapter = (PowerMonitorAdapterCtx_t *) ctx;
  uint32_t converted;

  if ((adapter == NULL) || (lineVoltage == NULL))
  {
    return 0U;
  }

  if (adapter->primarySource != POWER_MONITOR_SOURCE_AC_LINE)
  {
    *lineVoltage = 3001U;

    return 1U;
  }

  if ((adapter->powerService == NULL)
      || (UiPowerServiceGetLineVoltageTenthsVrms(adapter->powerService,
                                                 1U,
                                                 lineVoltage) == 0U))
  {
    *lineVoltage = 3001U;
    return 1U;
  }

  converted = *lineVoltage;
  if (converted > 6000U)
  {
    *lineVoltage = 6000U;
  }

  return 1U;
}

void PowerMonitorAdapterInit(PowerMonitorAdapterCtx_t *ctx,
                             UiPowerService_t *powerService)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->primarySource = POWER_MONITOR_SOURCE_AC_LINE;
  ctx->powerService = powerService;
}

IPowerMonitorPort_t PowerMonitorAdapterCreatePort(PowerMonitorAdapterCtx_t *ctx)
{
  IPowerMonitorPort_t port;

  port.ctx = ctx;
  port.GetPrimarySource = AdapterGetPrimarySource;
  port.GetLineVoltageTenthsVrms = AdapterGetLineVoltageTenthsVrms;

  return port;
}
