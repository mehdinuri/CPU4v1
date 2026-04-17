/* App/Adapters/STM32/HeaterAdapter.c
 *
 * IHeaterPort concrete implementation.
 * Drives PE10 (HEAT pin) directly via HAL.
 */
#include "HeaterAdapter.h"
#include "main.h"
#include "stm32h7xx_hal.h"

/* ------------------------------------------------------------------
 * Static helpers
 * ------------------------------------------------------------------ */
static void AdapterEnable(void *ctx)
{
  HeaterAdapterCtx_t *c = (HeaterAdapterCtx_t *) ctx;

  HAL_GPIO_WritePin(HEAT_GPIO_Port, HEAT_Pin, GPIO_PIN_SET);
  c->enabled = 1U;
}

static void AdapterDisable(void *ctx)
{
  HeaterAdapterCtx_t *c = (HeaterAdapterCtx_t *) ctx;

  HAL_GPIO_WritePin(HEAT_GPIO_Port, HEAT_Pin, GPIO_PIN_RESET);
  c->enabled = 0U;
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */
void HeaterAdapterInit(HeaterAdapterCtx_t *ctx)
{
  ctx->enabled = 0U;
  HAL_GPIO_WritePin(HEAT_GPIO_Port, HEAT_Pin, GPIO_PIN_RESET); /* safe default */
}

IHeaterPort_t HeaterAdapterCreatePort(HeaterAdapterCtx_t *ctx)
{
  IHeaterPort_t port;

  port.ctx = ctx;
  port.Enable = AdapterEnable;
  port.Disable = AdapterDisable;

  return port;
}
