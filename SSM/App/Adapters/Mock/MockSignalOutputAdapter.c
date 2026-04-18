/**
 ******************************************************************************
 * @file    Adapters/Mock/MockSignalOutputAdapter.c
 * @brief   Test double for ISignalOutputPort.
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockSignalOutputAdapter.h"

static void MockApply(void *pCtx, const tSSignalOutputImage *pImage)
{
  tSMockSignalOutputAdapterCtx *pC = (tSMockSignalOutputAdapterCtx *) pCtx;

  pC->SLastImage = *pImage;
  pC->lApplyCount++;
}

static void MockAllOff(void *pCtx)
{
  tSMockSignalOutputAdapterCtx *pC = (tSMockSignalOutputAdapterCtx *) pCtx;

  memset(&pC->SLastImage, 0, sizeof(pC->SLastImage));
  pC->lAllOffCount++;
}

void MockSignalOutputAdapter_Init(tSMockSignalOutputAdapterCtx *pCtx)
{
  memset(pCtx, 0, sizeof(*pCtx));
}

ISignalOutputPort_t MockSignalOutputAdapter_CreatePort(
  tSMockSignalOutputAdapterCtx *pCtx)
{
  ISignalOutputPort_t port;

  port.pCtx = pCtx;
  port.Apply = MockApply;
  port.AllOff = MockAllOff;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
