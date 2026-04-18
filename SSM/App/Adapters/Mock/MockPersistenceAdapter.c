/**
 ******************************************************************************
 * @file    Adapters/Mock/MockPersistenceAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/Mock/MockPersistenceAdapter.h"

static tEPersistenceStatus MockRead(void *pCtx,
                                    tEPersistenceKey eKey,
                                    void *pOut,
                                    uint16_t sSize)
{
  tSMockPersistenceAdapterCtx *pC = (tSMockPersistenceAdapterCtx *) pCtx;

  pC->lReadCount++;

  if ((uint32_t) eKey >= (uint32_t) PERSIST_KEY__COUNT)
  {
    return PERSIST_FAIL;
  }

  tSMockPersistenceSlot *pSlot = &pC->SaSlots[eKey];

  if (pSlot->bForceReadFail != 0U)
  {
    return PERSIST_FAIL;
  }

  if (pSlot->sSize == 0U)
  {
    return PERSIST_FAIL;
  }

  if (sSize > pSlot->sSize)
  {
    return PERSIST_FAIL;
  }

  memcpy(pOut, pSlot->abBlob, sSize);

  return PERSIST_OK;
}

static tEPersistenceStatus MockWrite(void *pCtx,
                                     tEPersistenceKey eKey,
                                     const void *pIn,
                                     uint16_t sSize)
{
  tSMockPersistenceAdapterCtx *pC = (tSMockPersistenceAdapterCtx *) pCtx;

  pC->lWriteCount++;

  if ((uint32_t) eKey >= (uint32_t) PERSIST_KEY__COUNT)
  {
    return PERSIST_FAIL;
  }

  tSMockPersistenceSlot *pSlot = &pC->SaSlots[eKey];

  if (pSlot->bForceWriteFail != 0U)
  {
    return PERSIST_FAIL;
  }

  if (sSize > MOCK_PERSIST_MAX_BLOB_SIZE)
  {
    return PERSIST_FAIL;
  }

  memcpy(pSlot->abBlob, pIn, sSize);
  pSlot->sSize = sSize;

  return PERSIST_OK;
}

void MockPersistenceAdapter_Init(tSMockPersistenceAdapterCtx *pCtx)
{
  memset(pCtx, 0, sizeof(*pCtx));
}

IPersistencePort_t MockPersistenceAdapter_CreatePort(
  tSMockPersistenceAdapterCtx *pCtx)
{
  IPersistencePort_t port;

  port.pCtx = pCtx;
  port.Read = MockRead;
  port.Write = MockWrite;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
