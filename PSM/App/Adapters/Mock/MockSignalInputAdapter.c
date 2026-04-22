/**
 ******************************************************************************
 * @file    Adapters/Mock/MockSignalInputAdapter.c
 * @brief   Mock adapter for ISignalInputPort.
 ******************************************************************************
 */

#include "MockSignalInputAdapter.h"
#include <string.h>

/* ---------------------------------------------------------------------------
 * Private implementation
 * ---------------------------------------------------------------------------*/
static uint8_t AdapterGetState(void *ctx, SignalInputId_t id)
{
  MockSignalInputAdapterCtx_t *c = (MockSignalInputAdapterCtx_t *) ctx;
  if ((uint8_t) id < MOCK_INPUT_COUNT)
  {
    return c->inputStates[(uint8_t) id];
  }
  return 0U;
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void MockSignalInputAdapterInit(MockSignalInputAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

ISignalInputPort_t MockSignalInputAdapterCreatePort(MockSignalInputAdapterCtx_t *ctx)
{
  ISignalInputPort_t port;
  port.ctx      = ctx;
  port.GetState = AdapterGetState;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
