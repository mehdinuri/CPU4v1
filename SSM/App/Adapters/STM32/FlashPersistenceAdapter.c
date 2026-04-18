/**
 ******************************************************************************
 * @file    Adapters/STM32/FlashPersistenceAdapter.c
 ******************************************************************************
 */

#include "Adapters/STM32/FlashPersistenceAdapter.h"
#include "flash.h"
#include "storage.h"

typedef struct
{
  uint32_t lAddress;
  uint16_t sMaxSize;
} tSKeySlot;

/* Key → flash slot table. Single slot today; grow as keys are added.
 * Slot size 12 = tSWireFormat (magic + data + reserved + CRC). FlashWrite
 * pads to the next double-word boundary automatically.
 */
static const tSKeySlot SaKeyTable[PERSIST_KEY__COUNT] =
{
  [PERSIST_KEY_SIGNAL_OUTPUTS_FLASH] = { FLASH_ADDR_USER_BASE, 12U }
};

static tEPersistenceStatus AdapterRead(void *pCtx,
                                       tEPersistenceKey eKey,
                                       void *pOut,
                                       uint16_t sSize)
{
  (void) pCtx;

  if ((uint32_t) eKey >= (uint32_t) PERSIST_KEY__COUNT)
  {
    return PERSIST_FAIL;
  }

  if (sSize > SaKeyTable[eKey].sMaxSize)
  {
    return PERSIST_FAIL;
  }

  uint8_t bOk = StorageRequest(STORAGE_REQ_FLASH_READ,
                               SaKeyTable[eKey].lAddress,
                               pOut,
                               (uint32_t) sSize);

  return (bOk != 0U) ? PERSIST_OK : PERSIST_FAIL;
}

static tEPersistenceStatus AdapterWrite(void *pCtx,
                                        tEPersistenceKey eKey,
                                        const void *pIn,
                                        uint16_t sSize)
{
  (void) pCtx;

  if ((uint32_t) eKey >= (uint32_t) PERSIST_KEY__COUNT)
  {
    return PERSIST_FAIL;
  }

  if (sSize > SaKeyTable[eKey].sMaxSize)
  {
    return PERSIST_FAIL;
  }

  /* StorageRequest's pvData is non-const for historical reasons; the
   * FLASH_WRITE path never mutates the buffer. Cast is safe here.
   */
  uint8_t bOk = StorageRequest(STORAGE_REQ_FLASH_WRITE,
                               SaKeyTable[eKey].lAddress,
                               (void *) pIn,
                               (uint32_t) sSize);

  return (bOk != 0U) ? PERSIST_OK : PERSIST_FAIL;
}

void FlashPersistenceAdapter_Init(tSFlashPersistenceAdapterCtx *pCtx)
{
  pCtx->bReserved = 0U;
}

IPersistencePort_t FlashPersistenceAdapter_CreatePort(
  tSFlashPersistenceAdapterCtx *pCtx)
{
  IPersistencePort_t port;

  port.pCtx = pCtx;
  port.Read = AdapterRead;
  port.Write = AdapterWrite;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
