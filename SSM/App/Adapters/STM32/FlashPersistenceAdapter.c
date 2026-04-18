/**
 ******************************************************************************
 * @file    Adapters/STM32/FlashPersistenceAdapter.c
 *******************************************************************************
 */

#include <string.h>
#include "Adapters/STM32/FlashPersistenceAdapter.h"
#include "Domain/PersistenceJournal.h"
#include "flash.h"
#include "storage.h"

typedef struct
{
  uint32_t aSlotAddresses[2];
  uint16_t sMaxPayloadSize;
} tSKeySlot;

/* Key → dual-slot flash journal. Each slot lives at the start of one reserved
 * flash page so a new commit never erases the previously valid record first.
 */
static const tSKeySlot SaKeyTable[PERSIST_KEY__COUNT] =
{
  [PERSIST_KEY_SIGNAL_OUTPUTS_FLASH] = {
    { FLASH_ADDR_USER_SLOT0, FLASH_ADDR_USER_SLOT1 }, 12U
  }
};

static void RecordReadOrZero(uint32_t lAddress, tSPersistenceJournalRecord *pOut)
{
  if (StorageRequest(STORAGE_REQ_FLASH_READ,
                     lAddress,
                     pOut,
                     (uint32_t) sizeof(*pOut)) == 0U)
  {
    memset(pOut, 0, sizeof(*pOut));
  }
}

static tEPersistenceStatus AdapterRead(void *pCtx,
                                       tEPersistenceKey eKey,
                                       void *pOut,
                                       uint16_t sSize)
{
  (void) pCtx;

  if (((uint32_t) eKey >= (uint32_t) PERSIST_KEY__COUNT)
      || (pOut == 0)
      || (sSize == 0U)
      || (sSize > SaKeyTable[eKey].sMaxPayloadSize))
  {
    return PERSIST_FAIL;
  }

  tSPersistenceJournalRecord SaRecords[2];
  const tSPersistenceJournalRecord *pSLatest;

  RecordReadOrZero(SaKeyTable[eKey].aSlotAddresses[0], &SaRecords[0]);
  RecordReadOrZero(SaKeyTable[eKey].aSlotAddresses[1], &SaRecords[1]);

  pSLatest = PersistenceJournal_SelectLatest(&SaRecords[0],
                                             &SaRecords[1],
                                             SaKeyTable[eKey].sMaxPayloadSize);
  if ((pSLatest == 0) || (sSize > pSLatest->sPayloadSize))
  {
    return PERSIST_FAIL;
  }

  memcpy(pOut, pSLatest->abPayload, sSize);

  return PERSIST_OK;
}

static tEPersistenceStatus AdapterWrite(void *pCtx,
                                        tEPersistenceKey eKey,
                                        const void *pIn,
                                        uint16_t sSize)
{
  (void) pCtx;

  if (((uint32_t) eKey >= (uint32_t) PERSIST_KEY__COUNT)
      || (pIn == 0)
      || (sSize == 0U)
      || (sSize > SaKeyTable[eKey].sMaxPayloadSize))
  {
    return PERSIST_FAIL;
  }

  tSPersistenceJournalRecord SaRecords[2];
  const tSPersistenceJournalRecord *pSLatest;
  tSPersistenceJournalRecord SNewRecord;
  uint8_t bTargetSlot = 0U;
  uint32_t lNextSequence = 1U;

  RecordReadOrZero(SaKeyTable[eKey].aSlotAddresses[0], &SaRecords[0]);
  RecordReadOrZero(SaKeyTable[eKey].aSlotAddresses[1], &SaRecords[1]);

  pSLatest = PersistenceJournal_SelectLatest(&SaRecords[0],
                                             &SaRecords[1],
                                             SaKeyTable[eKey].sMaxPayloadSize);
  if (pSLatest == &SaRecords[0])
  {
    bTargetSlot = 1U;
    lNextSequence = SaRecords[0].lSequence + 1U;
  }
  else if (pSLatest == &SaRecords[1])
  {
    bTargetSlot = 0U;
    lNextSequence = SaRecords[1].lSequence + 1U;
  }

  if (PersistenceJournal_RecordBuild(&SNewRecord,
                                     lNextSequence,
                                     pIn,
                                     sSize) == 0U)
  {
    return PERSIST_FAIL;
  }

  return (StorageRequest(STORAGE_REQ_FLASH_WRITE,
                         SaKeyTable[eKey].aSlotAddresses[bTargetSlot],
                         &SNewRecord,
                         (uint32_t) sizeof(SNewRecord)) != 0U)
         ? PERSIST_OK : PERSIST_FAIL;
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
