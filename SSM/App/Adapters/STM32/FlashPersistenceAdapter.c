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
  uint32_t slotAddresses[2];
  uint16_t maxPayloadSize;
} KeySlot_t;

/* Key → dual-slot flash journal. Each slot lives at the start of one reserved
 * flash page so a new commit never erases the previously valid record first.
 */
static const KeySlot_t keyTable[PERSIST_KEY__COUNT] =
{
  [PERSIST_KEY_SIGNAL_OUTPUTS_FLASH] = {
    { FLASH_ADDR_USER_SLOT0, FLASH_ADDR_USER_SLOT1 }, 12U
  }
};

/* The payload we memcpy out of a record is bounded by
 * sizeof(PersistenceJournalRecord_t::payload). Every key's maxPayloadSize
 * must respect that bound, otherwise a Read with size == maxPayloadSize
 * could walk past the source buffer. Add new keys here if the table grows.
 */
_Static_assert(12U <= PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE,
               "SIGNAL_OUTPUTS_FLASH payload exceeds journal record capacity");

static void RecordReadOrZero(uint32_t address,
                             PersistenceJournalRecord_t *out)
{
  if (StorageRequest(STORAGE_REQ_FLASH_READ,
                     address,
                     out,
                     (uint32_t) sizeof(*out)) == 0U)
  {
    memset(out, 0, sizeof(*out));
  }
}

static PersistenceStatus_e AdapterRead(void *ctx,
                                       PersistenceKey_e eKey,
                                       void *out,
                                       uint16_t size)
{
  (void) ctx;

  if (((uint32_t) eKey >= (uint32_t) PERSIST_KEY__COUNT)
      || (out == 0)
      || (size == 0U)
      || (size > keyTable[eKey].maxPayloadSize))
  {
    return PERSIST_FAIL;
  }

  PersistenceJournalRecord_t records[2];
  const PersistenceJournalRecord_t *latest;

  RecordReadOrZero(keyTable[eKey].slotAddresses[0], &records[0]);
  RecordReadOrZero(keyTable[eKey].slotAddresses[1], &records[1]);

  latest = PersistenceJournal_SelectLatest(&records[0],
                                           &records[1],
                                           keyTable[eKey].maxPayloadSize);
  if ((latest == 0) || (size > latest->payloadSize))
  {
    return PERSIST_FAIL;
  }

  memcpy(out, latest->payload, size);

  return PERSIST_OK;
}

static PersistenceStatus_e AdapterWrite(void *ctx,
                                        PersistenceKey_e eKey,
                                        const void *in,
                                        uint16_t size)
{
  (void) ctx;

  if (((uint32_t) eKey >= (uint32_t) PERSIST_KEY__COUNT)
      || (in == 0)
      || (size == 0U)
      || (size > keyTable[eKey].maxPayloadSize))
  {
    return PERSIST_FAIL;
  }

  PersistenceJournalRecord_t records[2];
  const PersistenceJournalRecord_t *latest;
  PersistenceJournalRecord_t newRecord;
  uint8_t targetSlot = 0U;
  uint32_t nextSequence = 1U;

  RecordReadOrZero(keyTable[eKey].slotAddresses[0], &records[0]);
  RecordReadOrZero(keyTable[eKey].slotAddresses[1], &records[1]);

  latest = PersistenceJournal_SelectLatest(&records[0],
                                           &records[1],
                                           keyTable[eKey].maxPayloadSize);
  if (latest == &records[0])
  {
    targetSlot = 1U;
    nextSequence = records[0].sequence + 1U;
  }
  else if (latest == &records[1])
  {
    targetSlot = 0U;
    nextSequence = records[1].sequence + 1U;
  }

  if (PersistenceJournal_RecordBuild(&newRecord,
                                     nextSequence,
                                     in,
                                     size) == 0U)
  {
    return PERSIST_FAIL;
  }

  return (StorageRequest(STORAGE_REQ_FLASH_WRITE,
                         keyTable[eKey].slotAddresses[targetSlot],
                         &newRecord,
                         (uint32_t) sizeof(newRecord)) != 0U)
         ? PERSIST_OK : PERSIST_FAIL;
} /* AdapterWrite */

void FlashPersistenceAdapter_Init(FlashPersistenceAdapterCtx_t *ctx)
{
  ctx->reserved = 0U;
}

IPersistencePort_t FlashPersistenceAdapter_CreatePort(
  FlashPersistenceAdapterCtx_t *ctx)
{
  IPersistencePort_t port;

  port.ctx = ctx;
  port.Read = AdapterRead;
  port.Write = AdapterWrite;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
