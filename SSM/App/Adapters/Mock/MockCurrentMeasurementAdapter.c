/**
 ******************************************************************************
 * @file    Adapters/Mock/MockCurrentMeasurementAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockCurrentMeasurementAdapter.h"

static void MockGetLatest(void *pCtx, tSCurrentMeasurementSnapshot *pOut)
{
  tSMockCurrentMeasurementAdapterCtx *pC =
    (tSMockCurrentMeasurementAdapterCtx *) pCtx;

  *pOut = pC->SStored;
  pC->lGetLatestCount++;
}

void MockCurrentMeasurementAdapter_Init(
  tSMockCurrentMeasurementAdapterCtx *pCtx)
{
  memset(pCtx, 0, sizeof(*pCtx));
}

void MockCurrentMeasurementAdapter_SetSnapshot(
  tSMockCurrentMeasurementAdapterCtx *pCtx,
  const
  tSCurrentMeasurementSnapshot *
  pSnap)
{
  pCtx->SStored = *pSnap;
}

ICurrentMeasurementPort_t MockCurrentMeasurementAdapter_CreatePort(
  tSMockCurrentMeasurementAdapterCtx *pCtx)
{
  ICurrentMeasurementPort_t port;

  port.pCtx = pCtx;
  port.GetLatest = MockGetLatest;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
