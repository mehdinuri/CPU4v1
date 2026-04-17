/* App/Adapters/STM32/UnitInputAdapter.c
 *
 * Local unit inputs for controller runtime behavior. The GPIO pins are
 * configured by MX_GPIO_Init().
 */
#include "UnitInputAdapter.h"

#include "main.h"
#include "stm32h7xx_hal.h"

static uint8_t AdapterGetDimmingInputActive(void *ctx)
{
  (void) ctx;

  return (HAL_GPIO_ReadPin(DIMMING_GPIO_Port, DIMMING_Pin) == GPIO_PIN_SET)
           ? 1U
           : 0U;
}

static uint8_t AdapterGetInterconnectCommand(void *ctx)
{
  (void) ctx;

  return 0U;
}

static uint8_t AdapterGetInterconnectInputsValid(void *ctx)
{
  (void) ctx;

  return 1U;
}

void UnitInputAdapterInit(UnitInputAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->reserved = 0U;
}

IUnitInputPort_t UnitInputAdapterCreatePort(UnitInputAdapterCtx_t *ctx)
{
  IUnitInputPort_t port;

  port.ctx = ctx;
  port.GetDimmingInputActive = AdapterGetDimmingInputActive;
  port.GetInterconnectCommand = AdapterGetInterconnectCommand;
  port.GetInterconnectInputsValid = AdapterGetInterconnectInputsValid;

  return port;
}
