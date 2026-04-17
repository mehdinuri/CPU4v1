/* App/Adapters/STM32/StatusLEDAdapter.c */

#include "StatusLEDAdapter.h"

#include <stddef.h>

#include "gpio.h"
#include "main.h"
#include "stm32g4xx_hal.h"

static uint32_t PeriodForState(StatusLEDState_t state)
{
  switch (state)
  {
      case STATUS_LED_STATE_BLINK_SLOW:
      {
        return 1000U;
      }

      case STATUS_LED_STATE_BLINK_FAST:
      {
        return 200U;
      }

      case STATUS_LED_STATE_OFF:
      case STATUS_LED_STATE_ON:
      default:
      {
        return 0U;
      }
  }
}

static uint8_t AdapterSetState(void *ctx, StatusLEDState_t state)
{
  StatusLEDAdapterCtx_t *self = (StatusLEDAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  self->state = state;

  if (state == STATUS_LED_STATE_OFF)
  {
    HAL_GPIO_WritePin(COM_LED_GPIO_Port, COM_LED_Pin, GPIO_PIN_RESET);
  }
  else if (state == STATUS_LED_STATE_ON)
  {
    HAL_GPIO_WritePin(COM_LED_GPIO_Port, COM_LED_Pin, GPIO_PIN_SET);
  }
  else
  {
    /* blink states driven by StatusLEDAdapterTick() */
  }

  return 1U;
}

void StatusLEDAdapterInit(StatusLEDAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->state = STATUS_LED_STATE_OFF;
  ctx->lastToggleMs = 0U;
  HAL_GPIO_WritePin(COM_LED_GPIO_Port, COM_LED_Pin, GPIO_PIN_RESET);
}

IStatusLEDPort_t StatusLEDAdapterCreatePort(StatusLEDAdapterCtx_t *ctx)
{
  IStatusLEDPort_t port;

  port.ctx = ctx;
  port.SetState = AdapterSetState;

  return port;
}

void StatusLEDAdapterTick(StatusLEDAdapterCtx_t *ctx, uint32_t nowMs)
{
  if (ctx == NULL)
  {
    return;
  }

  uint32_t period = PeriodForState(ctx->state);

  if (period == 0U)
  {
    return;
  }

  if ((nowMs - ctx->lastToggleMs) >= (period / 2U))
  {
    HAL_GPIO_TogglePin(COM_LED_GPIO_Port, COM_LED_Pin);
    ctx->lastToggleMs = nowMs;
  }
}
