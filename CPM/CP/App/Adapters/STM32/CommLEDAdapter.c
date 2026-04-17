/* App/Adapters/STM32/CommLEDAdapter.c
 *
 * IStatusLEDPort concrete implementation for the comm activity LED (PA4).
 * Drives COM_LED_Pin directly via HAL.
 */
#include "CommLEDAdapter.h"
#include "main.h"
#include "stm32h7xx_hal.h"

/* ------------------------------------------------------------------
 * Static helpers
 * ------------------------------------------------------------------ */
static void AdapterToggle(void *ctx)
{
  CommLEDAdapterCtx_t *c = (CommLEDAdapterCtx_t *) ctx;

  HAL_GPIO_TogglePin(COM_LED_GPIO_Port, COM_LED_Pin);
  c->state ^= 1U;
}

static void AdapterSetState(void *ctx, uint8_t on)
{
  CommLEDAdapterCtx_t *c = (CommLEDAdapterCtx_t *) ctx;
  GPIO_PinState pinState = (on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;

  HAL_GPIO_WritePin(COM_LED_GPIO_Port, COM_LED_Pin, pinState);
  c->state = (on != 0U) ? 1U : 0U;
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */
void CommLEDAdapterInit(CommLEDAdapterCtx_t *ctx)
{
  ctx->state = 0U;
  /* Pin configured by MX_GPIO_Init — nothing to do. */
}

IStatusLEDPort_t CommLEDAdapterCreatePort(CommLEDAdapterCtx_t *ctx)
{
  IStatusLEDPort_t port;

  port.ctx = ctx;
  port.Toggle = AdapterToggle;
  port.SetState = AdapterSetState;

  return port;
}
