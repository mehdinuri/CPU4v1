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
                         uint32_t id,
                         const uint8_t *data,
                         uint8_t dataLen)
{
  MockCANTxAdapterCtx_t *c = (MockCANTxAdapterCtx_t *) ctx;
  uint8_t copy            = (dataLen > MOCK_CAN_BUF_SIZE) ? MOCK_CAN_BUF_SIZE : dataLen;

  c->lastID  = id;
  c->lastLen = dataLen;
  memset(c->lastData, 0, sizeof(c->lastData));
  if (data != NULL)
  {
    memcpy(c->lastData, data, copy);
  }
  c->sendCount++;
}

static uint8_t AdapterGetOverflowCount(void *ctx)
{
  return ((MockCANTxAdapterCtx_t *) ctx)->mockOverflow;
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
