/* App/Adapters/STM32/SystemMonitorAdapter.c */

#include "SystemMonitorAdapter.h"

#include <stddef.h>

#include "adc.h"
#include "gpio.h"
#include "main.h"
#include "stm32g4xx_hal.h"

static uint16_t ReadAdcChannel(uint32_t channel)
{
  ADC_ChannelConfTypeDef cfg = { 0 };
  uint16_t result = 0U;

  cfg.Channel = channel;
  cfg.Rank = ADC_REGULAR_RANK_1;
  cfg.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
  cfg.SingleDiff = ADC_SINGLE_ENDED;
  cfg.OffsetNumber = ADC_OFFSET_NONE;
  cfg.Offset = 0U;

  if (HAL_ADC_ConfigChannel(&hadc1, &cfg) != HAL_OK)
  {
    return 0U;
  }

  if (HAL_ADC_Start(&hadc1) != HAL_OK)
  {
    return 0U;
  }

  if (HAL_ADC_PollForConversion(&hadc1, 10U) == HAL_OK)
  {
    result = (uint16_t) HAL_ADC_GetValue(&hadc1);
  }

  (void) HAL_ADC_Stop(&hadc1);

  return result;
}

static uint16_t CountsToVbatMilliVolts(uint16_t counts)
{
  uint32_t mv = ((uint32_t) counts * SYSTEM_MONITOR_ADC_REF_MV)
                / SYSTEM_MONITOR_ADC_MAX;

  mv = (mv * SYSTEM_MONITOR_VBAT_DIVIDER_NUM)
       / SYSTEM_MONITOR_VBAT_DIVIDER_DEN;

  if (mv > 0xFFFFU)
  {
    mv = 0xFFFFU;
  }

  return (uint16_t) mv;
}

static uint8_t AdapterGetBatteryVoltageMilliVolts(void *ctx,
                                                  uint16_t *milliVolts)
{
  SystemMonitorAdapterCtx_t *self = (SystemMonitorAdapterCtx_t *) ctx;

  if ((self == NULL) || (milliVolts == NULL))
  {
    return 0U;
  }

  uint16_t counts = ReadAdcChannel(ADC_CHANNEL_15);

  self->lastVbatMilliVolts = CountsToVbatMilliVolts(counts);
  *milliVolts = self->lastVbatMilliVolts;

  return 1U;
}

static uint8_t AdapterGetThermistorDegC(void *ctx, int16_t *degCelsius)
{
  SystemMonitorAdapterCtx_t *self = (SystemMonitorAdapterCtx_t *) ctx;

  if ((self == NULL) || (degCelsius == NULL))
  {
    return 0U;
  }

  /* Raw counts exposed as deg-C with a placeholder linear curve until
   * the production NTC beta parameters are finalised. A follow-up
   * replaces this with a table lookup. */
  uint16_t counts = ReadAdcChannel(ADC_CHANNEL_3);

  self->lastThermistorDegC = (int16_t) ((counts >> 5U) - 50);
  *degCelsius = self->lastThermistorDegC;

  return 1U;
}

static uint8_t AdapterGetChargerActive(void *ctx, uint8_t *active)
{
  if ((ctx == NULL) || (active == NULL))
  {
    return 0U;
  }

  GPIO_PinState pin = HAL_GPIO_ReadPin(CHRGING_GPIO_Port, CHRGING_Pin);

  *active = (pin == GPIO_PIN_SET) ? 1U : 0U;

  return 1U;
}

static uint8_t AdapterSetChargerEnable(void *ctx, uint8_t enable)
{
  SystemMonitorAdapterCtx_t *self = (SystemMonitorAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  HAL_GPIO_WritePin(CHRGR_CEN_GPIO_Port,
                    CHRGR_CEN_Pin,
                    (enable != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  self->chargerEnabled = (enable != 0U) ? 1U : 0U;

  return 1U;
}

void SystemMonitorAdapterInit(SystemMonitorAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->lastVbatMilliVolts = 0U;
  ctx->lastThermistorDegC = 0;
  ctx->chargerEnabled = 0U;
}

ISystemMonitorPort_t SystemMonitorAdapterCreatePort(
  SystemMonitorAdapterCtx_t *ctx)
{
  ISystemMonitorPort_t port;

  port.ctx = ctx;
  port.GetBatteryVoltageMilliVolts = AdapterGetBatteryVoltageMilliVolts;
  port.GetThermistorDegC = AdapterGetThermistorDegC;
  port.GetChargerActive = AdapterGetChargerActive;
  port.SetChargerEnable = AdapterSetChargerEnable;

  return port;
}
