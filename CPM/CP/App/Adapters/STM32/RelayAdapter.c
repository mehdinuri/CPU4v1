/* App/Adapters/STM32/RelayAdapter.c
 *
 * Raw CP-side relay GPIO adapter. The MMU layer owns topology/permit logic;
 * this adapter only applies the raw drive level to RELAY_Pin.
 */
#include "RelayAdapter.h"
#include "main.h"
#include "stm32h7xx_hal.h"

/* ------------------------------------------------------------------
 * Static helpers
 * ------------------------------------------------------------------ */
static void AdapterSet(void *ctx, uint8_t on)
{
  RelayAdapterCtx_t *c = (RelayAdapterCtx_t *) ctx;
  GPIO_PinState pinState = (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;

  HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, pinState);
  c->state = (on != 0U) ? 1U : 0U;
}

static uint8_t AdapterGet(void *ctx)
{
  (void) ctx;

  return (HAL_GPIO_ReadPin(RELAY_GPIO_Port,
                           RELAY_Pin) == GPIO_PIN_SET) ? 1U : 0U;
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */
void RelayAdapterInit(RelayAdapterCtx_t *ctx)
{
  ctx->state = 1U;
  HAL_GPIO_WritePin(RELAY_GPIO_Port,
                    RELAY_Pin,
                    GPIO_PIN_SET); /* active-low hardware safe state */
}

IRelayPort_t RelayAdapterCreatePort(RelayAdapterCtx_t *ctx)
{
  IRelayPort_t port;

  port.ctx = ctx;
  port.Set = AdapterSet;
  port.Get = AdapterGet;

  return port;
}
