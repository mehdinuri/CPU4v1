/**
 ******************************************************************************
 * @file    Adapters/Mock/MockSignalInputAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockSignalInputAdapter.h"

static void MockSample(void *pCtx, tSSignalInputSnapshot *pOut)
{
  tSMockSignalInputAdapterCtx *pC = (tSMockSignalInputAdapterCtx *) pCtx;

  *pOut = pC->SCanned;
  pC->lSampleCount++;
}

void MockSignalInputAdapter_Init(tSMockSignalInputAdapterCtx *pCtx)
{
  memset(pCtx, 0, sizeof(*pCtx));
}

void MockSignalInputAdapter_SetSnapshot(tSMockSignalInputAdapterCtx *pCtx,
                                        const tSSignalInputSnapshot *pSnap)
{
  pCtx->SCanned = *pSnap;
}

ISignalInputPort_t MockSignalInputAdapter_CreatePort(
  tSMockSignalInputAdapterCtx *pCtx)
{
  ISignalInputPort_t port;

  port.pCtx = pCtx;
  port.Sample = MockSample;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
