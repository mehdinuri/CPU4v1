/**
 ******************************************************************************
 * @file    Adapters/Mock/MockIndicatorLEDAdapter.c
 * @brief   Mock adapter for IIndicatorLEDPort.
 ******************************************************************************
 */

#include "MockIndicatorLEDAdapter.h"
#include <string.h>

/* ---------------------------------------------------------------------------
 * Private implementations
 * ---------------------------------------------------------------------------*/
static void AdapterSetState(void *ctx, IndicatorLEDId_t id, uint8_t on)
{
  MockIndicatorLEDAdapterCtx_t *c = (MockIndicatorLEDAdapterCtx_t *) ctx;
  if ((uint8_t) id < MOCK_LED_COUNT)
  {
    c->ledStates[(uint8_t) id] = (on != 0U) ? 1U : 0U;
  }
  c->setCallCount++;
}

static void AdapterToggle(void *ctx, IndicatorLEDId_t id)
{
  MockIndicatorLEDAdapterCtx_t *c = (MockIndicatorLEDAdapterCtx_t *) ctx;
  if ((uint8_t) id < MOCK_LED_COUNT)
  {
    c->ledStates[(uint8_t) id] ^= 1U;
  }
  c->toggleCallCount++;
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void MockIndicatorLEDAdapterInit(MockIndicatorLEDAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IIndicatorLEDPort_t MockIndicatorLEDAdapterCreatePort(MockIndicatorLEDAdapterCtx_t *ctx)
{
  IIndicatorLEDPort_t port;
  port.ctx      = ctx;
  port.SetState = AdapterSetState;
  port.Toggle   = AdapterToggle;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
