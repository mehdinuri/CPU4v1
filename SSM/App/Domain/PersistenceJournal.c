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
  ((uint16_t) offsetof(tSPersistenceJournalRecord, lCrc32))

uint8_t PersistenceJournal_RecordBuild(tSPersistenceJournalRecord *pRecord,
                                       uint32_t lSequence,
                                       const void *pPayload,
                                       uint16_t sPayloadSize)
{
  if ((pRecord == 0)
      || (pPayload == 0)
      || (sPayloadSize == 0U)
      || (sPayloadSize > PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE))
  {
    return 0U;
  }

  memset(pRecord, 0, sizeof(*pRecord));
  pRecord->lMagic = PERSISTENCE_JOURNAL_MAGIC;
  pRecord->lSequence = lSequence;
  pRecord->sPayloadSize = sPayloadSize;
  pRecord->bVersion = PERSISTENCE_JOURNAL_VERSION;
  pRecord->bCommit = PERSISTENCE_JOURNAL_COMMIT;
  memcpy(pRecord->abPayload, pPayload, sPayloadSize);
  pRecord->lCrc32 = Crc32_Compute(pRecord, PERSISTENCE_JOURNAL_CRC_SPAN);

  return 1U;
}

uint8_t PersistenceJournal_RecordIsValid(
  const tSPersistenceJournalRecord *pRecord,
  uint16_t sMaxPayloadSize)
{
  if (pRecord == 0)
  {
    return 0U;
  }

  if ((pRecord->lMagic != PERSISTENCE_JOURNAL_MAGIC)
      || (pRecord->bVersion != PERSISTENCE_JOURNAL_VERSION)
      || (pRecord->bCommit != PERSISTENCE_JOURNAL_COMMIT))
  {
    return 0U;
  }

  if ((pRecord->sPayloadSize == 0U)
      || (pRecord->sPayloadSize > sMaxPayloadSize)
      || (pRecord->sPayloadSize > PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE))
  {
    return 0U;
  }

  return (Crc32_Compute(pRecord, PERSISTENCE_JOURNAL_CRC_SPAN)
          == pRecord->lCrc32) ? 1U : 0U;
}

const tSPersistenceJournalRecord *PersistenceJournal_SelectLatest(
  const tSPersistenceJournalRecord *pA,
  const tSPersistenceJournalRecord *pB,
  uint16_t sMaxPayloadSize)
{
  uint8_t bAValid = PersistenceJournal_RecordIsValid(pA, sMaxPayloadSize);
  uint8_t bBValid = PersistenceJournal_RecordIsValid(pB, sMaxPayloadSize);

  if ((bAValid == 0U) && (bBValid == 0U))
  {
    return 0;
  }

  if (bAValid == 0U)
  {
    return pB;
  }

  if (bBValid == 0U)
  {
    return pA;
  }

  return ((uint32_t) (pB->lSequence - pA->lSequence) < 0x80000000U) ? pB : pA;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
