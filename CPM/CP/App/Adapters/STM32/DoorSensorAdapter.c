/* App/Adapters/STM32/DoorSensorAdapter.c
 *
 * IDoorSensorPort concrete implementation.
 * Reads DOOR_Pin (PC2) directly via HAL.
 * The pin is configured as a pull-down input by MX_GPIO_Init;
 * no additional setup is required here.
 */
#include "DoorSensorAdapter.h"
#include "main.h"
#include "stm32h7xx_hal.h"

/* ------------------------------------------------------------------
 * Static helpers
 * ------------------------------------------------------------------ */
static uint8_t AdapterIsOpen(void *ctx)
{
  (void) ctx;

  return (HAL_GPIO_ReadPin(DOOR_GPIO_Port, DOOR_Pin) == GPIO_PIN_SET) ? 1U : 0U;
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */
void DoorSensorAdapterInit(DoorSensorAdapterCtx_t *ctx)
{
  ctx->reserved = 0U;
  /* Pin already configured by MX_GPIO_Init — nothing to do. */
}

IDoorSensorPort_t DoorSensorAdapterCreatePort(DoorSensorAdapterCtx_t *ctx)
{
  IDoorSensorPort_t port;

  port.ctx = ctx;
  port.IsOpen = AdapterIsOpen;

  return port;
}
