/* App/Adapters/STM32/SafetyRelayAdapter.c */

#include "SafetyRelayAdapter.h"

#include <stddef.h>

#include "gpio.h"
#include "main.h"
#include "stm32g4xx_hal.h"

static void DrivePins(SafetyRelayAdapterCtx_t *ctx, SafetyRelayState_t state)
{
  uint8_t relayDrive = (state == SAFETY_RELAY_STATE_CLOSED) ? 1U : 0U;
  uint8_t triacDrive = (state == SAFETY_RELAY_STATE_CLOSED) ? 0U : 1U;
  GPIO_PinState relay;
  GPIO_PinState triac;

  if (ctx == NULL)
  {
    return;
  }

  if (ctx->topology == SAFETY_RELAY_TOPOLOGY_ECO_ACTIVE_HIGH_TRIP)
  {
    relayDrive = (uint8_t) (relayDrive == 0U);
  }

  relay = (relayDrive != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;
  triac = (triacDrive != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;

  HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, relay);
  HAL_GPIO_WritePin(TRIAC_GPIO_Port, TRIAC_Pin, triac);

  ctx->lastRelayDrive = relayDrive;
  ctx->lastTriacDrive = triacDrive;
}

static uint8_t AdapterSetState(void *ctx, SafetyRelayState_t state)
{
  SafetyRelayAdapterCtx_t *self = (SafetyRelayAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  DrivePins(self, state);
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
  const SafetyRelayAdapterCtx_t *self = (const SafetyRelayAdapterCtx_t *) ctx;
  GPIO_PinState pin;
  uint8_t relayDrive;

  if ((self == NULL) || (state == NULL))
  {
    return 0U;
  }

  pin = HAL_GPIO_ReadPin(RELAY_GPIO_Port, RELAY_Pin);
  relayDrive = (pin == GPIO_PIN_SET) ? 1U : 0U;

  if (self->topology == SAFETY_RELAY_TOPOLOGY_ECO_ACTIVE_HIGH_TRIP)
  {
    *state = (relayDrive != 0U) ? SAFETY_RELAY_STATE_OPEN
             : SAFETY_RELAY_STATE_CLOSED;
  }
  else
  {
    *state = (relayDrive != 0U) ? SAFETY_RELAY_STATE_CLOSED
             : SAFETY_RELAY_STATE_OPEN;
  }

  return 1U;
}

void SafetyRelayAdapterInit(SafetyRelayAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->commandedState = SAFETY_RELAY_STATE_OPEN;
  ctx->topology = SAFETY_RELAY_TOPOLOGY_LEGACY_ACTIVE_HIGH_CLOSE;
  ctx->lastRelayDrive = 0U;
  ctx->lastTriacDrive = 1U;
  DrivePins(ctx, SAFETY_RELAY_STATE_OPEN);
}

void SafetyRelayAdapterSetTopology(SafetyRelayAdapterCtx_t *ctx,
                                   SafetyRelayTopology_t topology)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->topology = topology;
  DrivePins(ctx, ctx->commandedState);
}

uint8_t SafetyRelayAdapterGetLastRelayDrive(const SafetyRelayAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return 0U;
  }

  return ctx->lastRelayDrive;
}

uint8_t SafetyRelayAdapterGetLastTriacDrive(const SafetyRelayAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return 0U;
  }

  return ctx->lastTriacDrive;
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
