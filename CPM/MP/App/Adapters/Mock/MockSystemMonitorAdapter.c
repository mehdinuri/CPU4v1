/* App/Adapters/Mock/MockSystemMonitorAdapter.c */

#include "MockSystemMonitorAdapter.h"

#include <string.h>

static uint8_t MockGetBatteryVoltageMilliVolts(void *ctx,
                                               uint16_t *milliVolts)
{
  const MockSystemMonitorAdapterCtx_t *self =
    (const MockSystemMonitorAdapterCtx_t *) ctx;

  if ((self == NULL) || (milliVolts == NULL))
  {
    return 0U;
  }

  *milliVolts = self->batteryVoltageMilliVolts;

  return 1U;
}

static uint8_t MockGetThermistorDegC(void *ctx, int16_t *degCelsius)
{
  const MockSystemMonitorAdapterCtx_t *self =
    (const MockSystemMonitorAdapterCtx_t *) ctx;

  if ((self == NULL) || (degCelsius == NULL))
  {
    return 0U;
  }

  *degCelsius = self->thermistorDegC;

  return 1U;
}

static uint8_t MockGetChargerActive(void *ctx, uint8_t *active)
{
  const MockSystemMonitorAdapterCtx_t *self =
    (const MockSystemMonitorAdapterCtx_t *) ctx;

  if ((self == NULL) || (active == NULL))
  {
    return 0U;
  }

  *active = self->chargerActive;

  return 1U;
}

static uint8_t MockSetChargerEnable(void *ctx, uint8_t enable)
{
  MockSystemMonitorAdapterCtx_t *self =
    (MockSystemMonitorAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  self->chargerEnabled = (enable != 0U) ? 1U : 0U;

  return 1U;
}

void MockSystemMonitorAdapterInit(MockSystemMonitorAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
  ctx->batteryVoltageMilliVolts = 12000U;
  ctx->thermistorDegC = 25;
  ctx->chargerEnabled = 1U;
}

ISystemMonitorPort_t MockSystemMonitorAdapterCreatePort(
  MockSystemMonitorAdapterCtx_t *ctx)
{
  ISystemMonitorPort_t port;

  port.ctx = ctx;
  port.GetBatteryVoltageMilliVolts = MockGetBatteryVoltageMilliVolts;
  port.GetThermistorDegC = MockGetThermistorDegC;
  port.GetChargerActive = MockGetChargerActive;
  port.SetChargerEnable = MockSetChargerEnable;

  return port;
}
