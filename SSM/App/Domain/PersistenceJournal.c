/**
 ******************************************************************************
 * @file    Domain/PersistenceJournal.c
 *******************************************************************************
 */

#include <stddef.h>
#include <string.h>
#include "Domain/PersistenceJournal.h"
#include "Domain/Crc32.h"

#define PERSISTENCE_JOURNAL_MAGIC   0x4A53534DU
#define PERSISTENCE_JOURNAL_VERSION 1U
#define PERSISTENCE_JOURNAL_COMMIT  0xA5U

#define PERSISTENCE_JOURNAL_CRC_SPAN \
        ((uint16_t) offsetof(PersistenceJournalRecord_t, crc32))

uint8_t PersistenceJournal_RecordBuild(PersistenceJournalRecord_t *record,
                                       uint32_t sequence,
                                       const void *payload,
                                       uint16_t payloadSize)
{
  if ((record == 0)
      || (payload == 0)
      || (payloadSize == 0U)
      || (payloadSize > PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE))
  {
    return 0U;
  }

  memset(record, 0, sizeof(*record));
  record->magic = PERSISTENCE_JOURNAL_MAGIC;
  record->sequence = sequence;
  record->payloadSize = payloadSize;
  record->version = PERSISTENCE_JOURNAL_VERSION;
  record->commit = PERSISTENCE_JOURNAL_COMMIT;
  memcpy(record->payload, payload, payloadSize);
  record->crc32 = Crc32_Compute(record, PERSISTENCE_JOURNAL_CRC_SPAN);

  return 1U;
}

uint8_t PersistenceJournal_RecordIsValid(
  const PersistenceJournalRecord_t *record,
  uint16_t maxPayloadSize)
{
  if (record == 0)
  {
    return 0U;
  }

  if ((record->magic != PERSISTENCE_JOURNAL_MAGIC)
      || (record->version != PERSISTENCE_JOURNAL_VERSION)
      || (record->commit != PERSISTENCE_JOURNAL_COMMIT))
  {
    return 0U;
  }

  if ((record->payloadSize == 0U)
      || (record->payloadSize > maxPayloadSize)
      || (record->payloadSize > PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE))
  {
    return 0U;
  }

  return (Crc32_Compute(record, PERSISTENCE_JOURNAL_CRC_SPAN)
          == record->crc32) ? 1U : 0U;
}

const PersistenceJournalRecord_t *PersistenceJournal_SelectLatest(
  const PersistenceJournalRecord_t *pA,
  const
  PersistenceJournalRecord_t
  *pB,
  uint16_t
  maxPayloadSize)
{
  uint8_t aValid = PersistenceJournal_RecordIsValid(pA, maxPayloadSize);
  uint8_t bValid = PersistenceJournal_RecordIsValid(pB, maxPayloadSize);

  if ((aValid == 0U) && (bValid == 0U))
  {
    return 0;
  }

  if (aValid == 0U)
  {
    return pB;
  }

  if (bValid == 0U)
  {
    return pA;
  }

  return ((uint32_t) (pB->sequence - pA->sequence) < 0x80000000U) ? pB : pA;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
