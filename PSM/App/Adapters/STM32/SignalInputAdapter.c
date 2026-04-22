/**
 ******************************************************************************
 * @file    Adapters/STM32/SignalInputAdapter.c
 * @brief   STM32 adapter for ISignalInputPort — wraps gpio.h read functions.
 *          Returns 1 (GPIO_PIN_SET) or 0 (GPIO_PIN_RESET) as uint8_t.
 ******************************************************************************
 */

#include "SignalInputAdapter.h"
#include "gpio.h"

/* ---------------------------------------------------------------------------
 * Private adapter implementation
 * ---------------------------------------------------------------------------*/
static uint8_t AdapterGetState(void *ctx, SignalInputId_t id)
{
  (void) ctx;
  switch (id)
  {
    case SIGNAL_IN_ACOK:   return (uint8_t) GPIOACOKStateGet();
    case SIGNAL_IN_DCOK:   return (uint8_t) GPIODCOKStateGet();
    case SIGNAL_IN_COMMOK: return (uint8_t) GPIOCommOKStateGet();
    default:               return 0U;
  }
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void SignalInputAdapterInit(SignalInputAdapterCtx_t *ctx)
{
  ctx->initialised = 1U;
}

ISignalInputPort_t SignalInputAdapterCreatePort(SignalInputAdapterCtx_t *ctx)
{
  ISignalInputPort_t port;
  port.ctx      = ctx;
  port.GetState = AdapterGetState;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
