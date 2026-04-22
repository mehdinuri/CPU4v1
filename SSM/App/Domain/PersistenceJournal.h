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
  uint32_t magic;
  uint32_t sequence;
  uint16_t payloadSize;
  uint8_t version;
  uint8_t commit;
  uint8_t payload[PERSISTENCE_JOURNAL_MAX_PAYLOAD_SIZE];
  uint32_t crc32;
} PersistenceJournalRecord_t;

uint8_t PersistenceJournal_RecordBuild(PersistenceJournalRecord_t *record,
                                       uint32_t sequence,
                                       const void *payload,
                                       uint16_t payloadSize);
uint8_t PersistenceJournal_RecordIsValid(
  const PersistenceJournalRecord_t *record,
  uint16_t maxPayloadSize);
const PersistenceJournalRecord_t *PersistenceJournal_SelectLatest(
  const PersistenceJournalRecord_t *pA,
  const
  PersistenceJournalRecord_t
  *pB,
  uint16_t
  maxPayloadSize);

#endif /* DOMAIN_PERSISTENCE_JOURNAL_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
