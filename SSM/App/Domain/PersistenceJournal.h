/**
 ******************************************************************************
 * @file    Domain/PersistenceJournal.h
 * @brief   Pure validation/build helpers for dual-slot flash journal records.
 *******************************************************************************
 */

#ifndef DOMAIN_PERSISTENCE_JOURNAL_H
#define DOMAIN_PERSISTENCE_JOURNAL_H

#include <stdint.h>

#define PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE 16U

typedef struct
{
  uint32_t lMagic;
  uint32_t lSequence;
  uint16_t sPayloadSize;
  uint8_t bVersion;
  uint8_t bCommit;
  uint8_t abPayload[PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE];
  uint32_t lCrc32;
} tSPersistenceJournalRecord;

uint8_t PersistenceJournal_RecordBuild(tSPersistenceJournalRecord *pRecord,
                                       uint32_t lSequence,
                                       const void *pPayload,
                                       uint16_t sPayloadSize);
uint8_t PersistenceJournal_RecordIsValid(
  const tSPersistenceJournalRecord *pRecord,
  uint16_t sMaxPayloadSize);
const tSPersistenceJournalRecord *PersistenceJournal_SelectLatest(
  const tSPersistenceJournalRecord *pA,
  const
  tSPersistenceJournalRecord
  *pB,
  uint16_t
  sMaxPayloadSize);

#endif /* DOMAIN_PERSISTENCE_JOURNAL_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
