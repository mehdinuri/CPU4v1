/**
 ******************************************************************************
 * @file    Adapters/STM32/GpioOutputAdapter.c
 * @brief   STM32 adapter for ISignalOutputPort.
 ******************************************************************************
 */

#include "Adapters/STM32/GpioOutputAdapter.h"
#include "main.h"

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
} PinRef_t;

/* Channel order must match ISignalOutputPort.h */
static const PinRef_t pinTable[SIGNAL_OUTPUT_CHANNEL_COUNT] =
{
  { R1_GPIO_Port, R1_Pin }, { Y1_GPIO_Port, Y1_Pin }, { G1_GPIO_Port, G1_Pin },
  { R2_GPIO_Port, R2_Pin }, { Y2_GPIO_Port, Y2_Pin }, { G2_GPIO_Port, G2_Pin },
  { R3_GPIO_Port, R3_Pin }, { Y3_GPIO_Port, Y3_Pin }, { G3_GPIO_Port, G3_Pin },
  { R4_GPIO_Port, R4_Pin }, { Y4_GPIO_Port, Y4_Pin }, { G4_GPIO_Port, G4_Pin }
};

static void AdapterApply(void *ctx, const SignalOutputImage_t *image)
{
  uint8_t i;

  (void) ctx;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    GPIO_PinState state = (image->channels[i]
                           != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    HAL_GPIO_WritePin(pinTable[i].port, pinTable[i].pin, state);
  }
}

static void AdapterAllOff(void *ctx)
{
  uint8_t i;

  (void) ctx;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    HAL_GPIO_WritePin(pinTable[i].port, pinTable[i].pin, GPIO_PIN_RESET);
  }
}

void GpioOutputAdapter_Init(GpioOutputAdapterCtx_t *ctx)
{
  ctx->reserved = 0U;

  /* MX_GPIO_Init() is called by CubeMX before the scheduler starts;
   * the pins are already configured as push-pull outputs by that point.
   */
}

ISignalOutputPort_t GpioOutputAdapter_CreatePort(GpioOutputAdapterCtx_t *ctx)
{
  ISignalOutputPort_t port;

  port.ctx = ctx;
  port.Apply = AdapterApply;
  port.AllOff = AdapterAllOff;

  return port;
}

void GpioOutputAdapter_ForceAllOff(void)
{
  uint8_t i;

  /* Context-free. No vtable dispatch, no port instance, no RTOS primitives.
   * Single source of truth for the pin map — shares pinTable with the
   * regular Apply/AllOff path.
   */
  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    HAL_GPIO_WritePin(pinTable[i].port, pinTable[i].pin, GPIO_PIN_RESET);
  }
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
