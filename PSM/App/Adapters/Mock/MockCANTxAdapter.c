/**
 ******************************************************************************
 * @file    Adapters/Mock/MockCANTxAdapter.c
 * @brief   Mock adapter for ICANTxPort.
 ******************************************************************************
 */

#include "MockCANTxAdapter.h"
#include <string.h>

/* ---------------------------------------------------------------------------
 * Private implementation
 * ---------------------------------------------------------------------------*/
static void AdapterSend(void *ctx,
                         uint32_t lID,
                         const uint8_t *baData,
                         uint8_t bDataLen)
{
  MockCANTxAdapterCtx_t *c = (MockCANTxAdapterCtx_t *) ctx;
  uint8_t bCopy            = (bDataLen > MOCK_CAN_BUF_SIZE) ? MOCK_CAN_BUF_SIZE : bDataLen;

  c->lLastID  = lID;
  c->bLastLen = bDataLen;
  memset(c->aLastData, 0, sizeof(c->aLastData));
  if (baData != NULL)
  {
    memcpy(c->aLastData, baData, bCopy);
  }
  c->lSendCount++;
}

static uint8_t AdapterGetOverflowCount(void *ctx)
{
  return ((MockCANTxAdapterCtx_t *) ctx)->bMockOverflow;
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void MockCANTxAdapterInit(MockCANTxAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

ICANTxPort_t MockCANTxAdapterCreatePort(MockCANTxAdapterCtx_t *ctx)
{
  ICANTxPort_t port;
  port.ctx              = ctx;
  port.Send             = AdapterSend;
  port.GetOverflowCount = AdapterGetOverflowCount;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
