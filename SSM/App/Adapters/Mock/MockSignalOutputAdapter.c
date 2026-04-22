/**
 ******************************************************************************
 * @file    Adapters/Mock/MockSignalOutputAdapter.c
 * @brief   Test double for ISignalOutputPort.
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockSignalOutputAdapter.h"

static void MockApply(void *ctx, const SignalOutputImage_t *image)
{
  MockSignalOutputAdapterCtx_t *pC = (MockSignalOutputAdapterCtx_t *) ctx;

  pC->lastImage = *image;
  pC->applyCount++;
}

static void MockAllOff(void *ctx)
{
  MockSignalOutputAdapterCtx_t *pC = (MockSignalOutputAdapterCtx_t *) ctx;

  memset(&pC->lastImage, 0, sizeof(pC->lastImage));
  pC->allOffCount++;
}

void MockSignalOutputAdapter_Init(MockSignalOutputAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

ISignalOutputPort_t MockSignalOutputAdapter_CreatePort(
  MockSignalOutputAdapterCtx_t *ctx)
{
  ISignalOutputPort_t port;

  port.ctx = ctx;
  port.Apply = MockApply;
  port.AllOff = MockAllOff;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
