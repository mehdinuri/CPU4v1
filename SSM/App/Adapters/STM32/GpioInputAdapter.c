/**
 ******************************************************************************
 * @file    Adapters/STM32/GpioInputAdapter.c
 ******************************************************************************
 */

#include "Adapters/STM32/GpioInputAdapter.h"
#include "main.h"
#include "gpio.h"

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
} PinRef_t;

/* Readback pins — active-low. Channel order matches ISignalOutputPort. */
static const PinRef_t readbackPins[SIGNAL_OUTPUT_CHANNEL_COUNT] =
{
  { R1_V_GPIO_Port, R1_V_Pin }, { Y1_V_GPIO_Port, Y1_V_Pin },
  { G1_V_GPIO_Port, G1_V_Pin },
  { R2_V_GPIO_Port, R2_V_Pin }, { Y2_V_GPIO_Port, Y2_V_Pin },
  { G2_V_GPIO_Port, G2_V_Pin },
  { R3_V_GPIO_Port, R3_V_Pin }, { Y3_V_GPIO_Port, Y3_V_Pin },
  { G3_V_GPIO_Port, G3_V_Pin },
  { R4_V_GPIO_Port, R4_V_Pin }, { Y4_V_GPIO_Port, Y4_V_Pin },
  { G4_V_GPIO_Port, G4_V_Pin }
};

static void AdapterSample(void *ctx, SignalInputSnapshot_t *out)
{
  uint8_t i;

  (void) ctx;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    /* Active-low in hardware: pin low → channel active → logical 1. */
    out->channels[i] =
      (HAL_GPIO_ReadPin(readbackPins[i].port,
                        readbackPins[i].pin) == GPIO_PIN_RESET)
      ? 1U : 0U;
  }

  out->cardId = GPIOGetCardID();
}

void GpioInputAdapter_Init(GpioInputAdapterCtx_t *ctx)
{
  ctx->reserved = 0U;
}

ISignalInputPort_t GpioInputAdapter_CreatePort(GpioInputAdapterCtx_t *ctx)
{
  ISignalInputPort_t port;

  port.ctx = ctx;
  port.Sample = AdapterSample;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
