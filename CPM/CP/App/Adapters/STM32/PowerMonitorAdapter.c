/* App/Adapters/STM32/PowerMonitorAdapter.c */
#include "PowerMonitorAdapter.h"

#include <stddef.h>

#include "data.h"

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

  converted = (uint32_t) (((float) GetPowerSupplyNet(0U) * 0.73029f) + 0.5f);
  if (converted > 6000U)
  {
    converted = 6000U;
  }

  *lineVoltage = (uint16_t) converted;

  return 1U;
}

void PowerMonitorAdapterInit(PowerMonitorAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->primarySource = POWER_MONITOR_SOURCE_AC_LINE;
}

IPowerMonitorPort_t PowerMonitorAdapterCreatePort(PowerMonitorAdapterCtx_t *ctx)
{
  IPowerMonitorPort_t port;

  port.ctx = ctx;
  port.GetPrimarySource = AdapterGetPrimarySource;
  port.GetLineVoltageTenthsVrms = AdapterGetLineVoltageTenthsVrms;

  return port;
}
