/**
 ******************************************************************************
 * @file    Adapters/Mock/MockEepromAdapter.c
 * @brief   Mock adapter for IEepromPort.
 ******************************************************************************
 */

#include "MockEepromAdapter.h"
#include <string.h>

/* ---------------------------------------------------------------------------
 * Private implementations
 * ---------------------------------------------------------------------------*/
static uint8_t AdapterRead(void *ctx, uint32_t address, void *dst, uint32_t size)
{
  MockEepromAdapterCtx_t *c = (MockEepromAdapterCtx_t *) ctx;
  c->lReadCount++;
  if (c->bReadResult == 0U)                        { return 0U; }
  if (dst == NULL)                                 { return 0U; }
  if ((address + size) > (uint32_t) MOCK_EEPROM_SIZE) { return 0U; }
  memcpy(dst, &c->aBuf[address], size);
  return 1U;
}

static uint8_t AdapterWrite(void *ctx, uint32_t address, const void *src, uint32_t size)
{
  MockEepromAdapterCtx_t *c = (MockEepromAdapterCtx_t *) ctx;
  c->lWriteCount++;
  if (c->bWriteResult == 0U)                       { return 0U; }
  if (src == NULL)                                 { return 0U; }
  if ((address + size) > (uint32_t) MOCK_EEPROM_SIZE) { return 0U; }
  memcpy(&c->aBuf[address], src, size);
  return 1U;
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void MockEepromAdapterInit(MockEepromAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->bReadResult  = 1U;
  ctx->bWriteResult = 1U;
}

IEepromPort_t MockEepromAdapterCreatePort(MockEepromAdapterCtx_t *ctx)
{
  IEepromPort_t port;
  port.ctx   = ctx;
  port.Read  = AdapterRead;
  port.Write = AdapterWrite;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
