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
  c->readCount++;
  if (c->readResult == 0U)                        { return 0U; }
  if (dst == NULL)                                 { return 0U; }
  if ((address + size) > (uint32_t) MOCK_EEPROM_SIZE) { return 0U; }
  memcpy(dst, &c->buf[address], size);
  return 1U;
}

static uint8_t AdapterWrite(void *ctx, uint32_t address, const void *src, uint32_t size)
{
  MockEepromAdapterCtx_t *c = (MockEepromAdapterCtx_t *) ctx;
  c->writeCount++;
  if (c->writeResult == 0U)                       { return 0U; }
  if (src == NULL)                                 { return 0U; }
  if ((address + size) > (uint32_t) MOCK_EEPROM_SIZE) { return 0U; }
  memcpy(&c->buf[address], src, size);
  return 1U;
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void MockEepromAdapterInit(MockEepromAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->readResult  = 1U;
  ctx->writeResult = 1U;
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
