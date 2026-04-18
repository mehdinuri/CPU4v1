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
  GPIO_TypeDef *pPort;
  uint16_t sPin;
} tSPinRef;

/* Readback pins — active-low. Channel order matches ISignalOutputPort. */
static const tSPinRef SaReadbackPins[SIGNAL_OUTPUT_CHANNEL_COUNT] =
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

static void AdapterSample(void *pCtx, tSSignalInputSnapshot *pOut)
{
  uint8_t i;

  (void) pCtx;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    /* Active-low in hardware: pin low → channel active → logical 1. */
    pOut->aChannels[i] =
      (HAL_GPIO_ReadPin(SaReadbackPins[i].pPort,
                        SaReadbackPins[i].sPin) == GPIO_PIN_RESET)
      ? 1U : 0U;
  }

  pOut->bCardId = GPIOGetCardID();
}

void GpioInputAdapter_Init(tSGpioInputAdapterCtx *pCtx)
{
  pCtx->bReserved = 0U;
}

ISignalInputPort_t GpioInputAdapter_CreatePort(tSGpioInputAdapterCtx *pCtx)
{
  ISignalInputPort_t port;

  port.pCtx = pCtx;
  port.Sample = AdapterSample;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
