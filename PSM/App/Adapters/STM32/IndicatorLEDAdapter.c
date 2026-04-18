/**
 ******************************************************************************
 * @file    Adapters/STM32/IndicatorLEDAdapter.c
 * @brief   STM32 adapter for IIndicatorLEDPort — wraps gpio.h LED functions.
 ******************************************************************************
 */

#include "IndicatorLEDAdapter.h"
#include "gpio.h"

/* ---------------------------------------------------------------------------
 * Private adapter implementations
 * ---------------------------------------------------------------------------*/
static void AdapterSetState(void *ctx, IndicatorLEDId_t id, uint8_t on)
{
  (void) ctx;
  switch (id)
  {
    case LED_ID_DCOK:
      if (on != 0U) { GPIODCOKLEDOn(); }
      else          { GPIODCOKLEDOff(); }
      break;

    case LED_ID_ACOK:
      if (on != 0U) { GPIOACOKLEDOn(); }
      else          { GPIOACOKLEDOff(); }
      break;

    case LED_ID_ERROR:
      if (on != 0U) { GPIOErrorLEDOn(); }
      else          { GPIOErrorLEDOff(); }
      break;

    case LED_ID_COM:
      if (on != 0U) { GPIOComLEDOn(); }
      else          { GPIOComLEDOff(); }
      break;

    default:
      break;
  }
}

static void AdapterToggle(void *ctx, IndicatorLEDId_t id)
{
  (void) ctx;
  if (id == LED_ID_COM)
  {
    GPIOComLEDToggle();
  }
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void IndicatorLEDAdapterInit(IndicatorLEDAdapterCtx_t *ctx)
{
  ctx->bInitialised = 1U;
}

IIndicatorLEDPort_t IndicatorLEDAdapterCreatePort(IndicatorLEDAdapterCtx_t *ctx)
{
  IIndicatorLEDPort_t port;
  port.ctx      = ctx;
  port.SetState = AdapterSetState;
  port.Toggle   = AdapterToggle;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
