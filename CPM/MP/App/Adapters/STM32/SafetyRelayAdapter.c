/* App/Adapters/STM32/SafetyRelayAdapter.c */

#include "SafetyRelayAdapter.h"

#include <stddef.h>

#include "gpio.h"
#include "main.h"
#include "stm32g4xx_hal.h"

static void DrivePins(SafetyRelayState_t state)
{
  GPIO_PinState relay = (state == SAFETY_RELAY_STATE_CLOSED)
                        ? GPIO_PIN_SET : GPIO_PIN_RESET;
  /* TRIAC_Pin is active-low per the legacy wiring. */
  GPIO_PinState triac = (state == SAFETY_RELAY_STATE_CLOSED)
                        ? GPIO_PIN_RESET : GPIO_PIN_SET;

  HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, relay);
  HAL_GPIO_WritePin(TRIAC_GPIO_Port, TRIAC_Pin, triac);
}

static uint8_t AdapterSetState(void *ctx, SafetyRelayState_t state)
{
  SafetyRelayAdapterCtx_t *self = (SafetyRelayAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  DrivePins(state);
  self->commandedState = state;

  return 1U;
}

static uint8_t AdapterGetCommandedState(void *ctx, SafetyRelayState_t *state)
{
  const SafetyRelayAdapterCtx_t *self = (const SafetyRelayAdapterCtx_t *) ctx;

  if ((self == NULL) || (state == NULL))
  {
    return 0U;
  }

  *state = self->commandedState;

  return 1U;
}

static uint8_t AdapterGetActualState(void *ctx, SafetyRelayState_t *state)
{
  if ((ctx == NULL) || (state == NULL))
  {
    return 0U;
  }

  GPIO_PinState pin = HAL_GPIO_ReadPin(RELAY_GPIO_Port, RELAY_Pin);

  *state = (pin == GPIO_PIN_SET) ? SAFETY_RELAY_STATE_CLOSED
           : SAFETY_RELAY_STATE_OPEN;

  return 1U;
}

void SafetyRelayAdapterInit(SafetyRelayAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->commandedState = SAFETY_RELAY_STATE_OPEN;
  DrivePins(SAFETY_RELAY_STATE_OPEN);
}

ISafetyRelayPort_t SafetyRelayAdapterCreatePort(SafetyRelayAdapterCtx_t *ctx)
{
  ISafetyRelayPort_t port;

  port.ctx = ctx;
  port.SetState = AdapterSetState;
  port.GetCommandedState = AdapterGetCommandedState;
  port.GetActualState = AdapterGetActualState;

  return port;
}
