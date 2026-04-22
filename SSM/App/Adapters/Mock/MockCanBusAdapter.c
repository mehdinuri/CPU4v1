/**
 ******************************************************************************
 * @file    Adapters/Mock/MockCanBusAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockCanBusAdapter.h"

static uint8_t MockSendStd(void *ctx, CanBusId_e eBus,
                           const CanFrame_t *frame)
{
  MockCanBusAdapterCtx_t *pC = (MockCanBusAdapterCtx_t *) ctx;

  if (pC->forceSendFail != 0U)
  {
    return 0U;
  }

  if (pC->sentCount < MOCK_CAN_MAX_RECORDED)
  {
    pC->sent[pC->sentCount].eBus = eBus;
    pC->sent[pC->sentCount].frame = *frame;
    pC->sentCount++;
  }

  return 1U;
}

void MockCanBusAdapter_Init(MockCanBusAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

ICanBusPort_t MockCanBusAdapter_CreatePort(MockCanBusAdapterCtx_t *ctx)
{
  ICanBusPort_t port;

  port.ctx = ctx;
  port.SendStd = MockSendStd;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
