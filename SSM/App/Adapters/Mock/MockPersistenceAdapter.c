/**
 ******************************************************************************
 * @file    Adapters/Mock/MockPersistenceAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockPersistenceAdapter.h"

static PersistenceStatus_e MockRead(void *ctx,
                                    PersistenceKey_e eKey,
                                    void *out,
                                    uint16_t size)
{
  MockPersistenceAdapterCtx_t *pC = (MockPersistenceAdapterCtx_t *) ctx;

  pC->readCount++;

  if ((uint32_t) eKey >= (uint32_t) PERSIST_KEY__COUNT)
  {
    return PERSIST_FAIL;
  }

  MockPersistenceSlot_t *slot = &pC->slots[eKey];

  if (slot->forceReadFail != 0U)
  {
    return PERSIST_FAIL;
  }

  if (slot->size == 0U)
  {
    return PERSIST_FAIL;
  }

  if (size > slot->size)
  {
    return PERSIST_FAIL;
  }

  memcpy(out, slot->abBlob, size);

  return PERSIST_OK;
}

static PersistenceStatus_e MockWrite(void *ctx,
                                     PersistenceKey_e eKey,
                                     const void *in,
                                     uint16_t size)
{
  MockPersistenceAdapterCtx_t *pC = (MockPersistenceAdapterCtx_t *) ctx;

  pC->writeCount++;

  if ((uint32_t) eKey >= (uint32_t) PERSIST_KEY__COUNT)
  {
    return PERSIST_FAIL;
  }

  MockPersistenceSlot_t *slot = &pC->slots[eKey];

  if (slot->forceWriteFail != 0U)
  {
    return PERSIST_FAIL;
  }

  if (size > MOCK_PERSIST_MAX_BLOB_SIZE)
  {
    return PERSIST_FAIL;
  }

  memcpy(slot->abBlob, in, size);
  slot->size = size;

  return PERSIST_OK;
}

void MockPersistenceAdapter_Init(MockPersistenceAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IPersistencePort_t MockPersistenceAdapter_CreatePort(
  MockPersistenceAdapterCtx_t *ctx)
{
  IPersistencePort_t port;

  port.ctx = ctx;
  port.Read = MockRead;
  port.Write = MockWrite;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
