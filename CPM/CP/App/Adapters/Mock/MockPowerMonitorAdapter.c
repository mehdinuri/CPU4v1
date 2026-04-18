/* App/Adapters/Mock/MockPowerMonitorAdapter.c */
#include "MockPowerMonitorAdapter.h"

#include <stddef.h>

static uint8_t AdapterGetPrimarySource(void *ctx, uint8_t *powerSource)
{
  MockPowerMonitorAdapterCtx_t *adapter = (MockPowerMonitorAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (powerSource == NULL))
  {
    return 0U;
  }

  adapter->primarySourceReadCount++;
  *powerSource = adapter->primarySource;

  return 1U;
}

static uint8_t AdapterGetLineVoltageTenthsVrms(void *ctx, uint16_t *lineVoltage)
{
  MockPowerMonitorAdapterCtx_t *adapter = (MockPowerMonitorAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (lineVoltage == NULL))
  {
    return 0U;
  }

  adapter->lineVoltageReadCount++;
  *lineVoltage = adapter->lineVoltageTenthsVrms;

  return 1U;
}

void MockPowerMonitorAdapterInit(MockPowerMonitorAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->primarySource = 3U;
  ctx->lineVoltageTenthsVrms = 6001U;
  ctx->primarySourceReadCount = 0U;
  ctx->lineVoltageReadCount = 0U;
}

IPowerMonitorPort_t MockPowerMonitorAdapterCreatePort(
  MockPowerMonitorAdapterCtx_t *ctx)
{
  IPowerMonitorPort_t port;

  port.ctx = ctx;
  port.GetPrimarySource = AdapterGetPrimarySource;
  port.GetLineVoltageTenthsVrms = AdapterGetLineVoltageTenthsVrms;

  return port;
}
