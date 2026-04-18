/**
 ******************************************************************************
 * @file    Adapters/Mock/MockCanBusAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockCanBusAdapter.h"

static uint8_t MockSendStd(void *pCtx, tECanBusId eBus,
                           const tSCanFrame *pFrame)
{
  tSMockCanBusAdapterCtx *pC = (tSMockCanBusAdapterCtx *) pCtx;

  if (pC->bForceSendFail != 0U)
  {
    return 0U;
  }

  if (pC->bSentCount < MOCK_CAN_MAX_RECORDED)
  {
    pC->aSent[pC->bSentCount].eBus = eBus;
    pC->aSent[pC->bSentCount].SFrame = *pFrame;
    pC->bSentCount++;
  }

  return 1U;
}

void MockCanBusAdapter_Init(tSMockCanBusAdapterCtx *pCtx)
{
  memset(pCtx, 0, sizeof(*pCtx));
}

ICanBusPort_t MockCanBusAdapter_CreatePort(tSMockCanBusAdapterCtx *pCtx)
{
  ICanBusPort_t port;

  port.pCtx = pCtx;
  port.SendStd = MockSendStd;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
