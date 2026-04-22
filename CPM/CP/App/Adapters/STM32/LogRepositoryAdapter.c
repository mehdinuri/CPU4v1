/* App/Adapters/STM32/LogRepositoryAdapter.c
 *
 * EEPROM-backed 1103 event-log repository.
 */
#include "LogRepositoryAdapter.h"

#include <string.h>

#include "Domain/Services/EventReportService.h"
#include "MSM.h"

typedef struct
{
  uint32_t sequence;
  uint16_t physicalIndex;
} LogRepositoryScanEntry_t;

static uint32_t Crc32Compute(const uint8_t *data, uint32_t length)
{
  uint32_t crc = 0xFFFFFFFFUL;
  uint32_t index;
  uint8_t bit;

  if (data == NULL)
  {
    return 0U;
  }

  for (index = 0U; index < length; index++)
  {
    crc ^= data[index];
    for (bit = 0U; bit < 8U; bit++)
    {
      if ((crc & 1UL) != 0UL)
      {
        crc = (crc >> 1U) ^ 0xEDB88320UL;
      }
      else
      {
        crc >>= 1U;
      }
    }
  }

  return ~crc;
}

static uint32_t StorageAddressForIndex(uint16_t physicalIndex)
{
  return EEPROM_STORAGE_ADDR_LOG
         + ((uint32_t) physicalIndex * sizeof(EventLogStorageRecord_t));
}

static void UpdatePublicWriteIndex(LogRepositoryAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->writeIndex = (ctx->count >= LOG_RECORDS_MAX) ? 0U : ctx->count;
}

static uint8_t ReadPhysicalDirect(IEepromStoragePort_t *eepromPort,
                                  uint16_t physicalIndex,
                                  EventLogStorageRecord_t *record)
{
  if ((eepromPort == NULL) || (record == NULL) || (physicalIndex >= LOG_RECORDS_MAX))
  {
    return FALSE;
  }

  return EepromStorageRead(eepromPort,
                           StorageAddressForIndex(physicalIndex),
                           record,
                           sizeof(*record));
}

static uint8_t ReadPhysicalRuntime(const LogRepositoryAdapterCtx_t *ctx,
                                   uint16_t physicalIndex,
                                   EventLogStorageRecord_t *record)
{
  if ((ctx == NULL) || (record == NULL) || (physicalIndex >= LOG_RECORDS_MAX))
  {
    return FALSE;
  }

  if (ctx->eepromPort != NULL)
  {
    return ReadPhysicalDirect(ctx->eepromPort, physicalIndex, record);
  }

  return MSMRequest(MSM_REQ_EEPROM_READ,
                    StorageAddressForIndex(physicalIndex),
                    record,
                    sizeof(*record));
}

static uint8_t WritePhysical(uint16_t physicalIndex,
                             const EventLogStorageRecord_t *record)
{
  if ((record == NULL) || (physicalIndex >= LOG_RECORDS_MAX))
  {
    return FALSE;
  }

  return MSMRequest(MSM_REQ_EEPROM_WRITE,
                    StorageAddressForIndex(physicalIndex),
                    (void *) record,
                    sizeof(*record));
}

static uint8_t PhysicalRecordIsValid(const EventLogStorageRecord_t *record)
{
  uint32_t expectedCrc;

  if (record == NULL)
  {
    return FALSE;
  }

  if (record->valueLength > EVENT_LOG_VALUE_MAX_LENGTH)
  {
    return FALSE;
  }

  expectedCrc = Crc32Compute((const uint8_t *) record,
                             sizeof(*record) - sizeof(record->crc32));
  return (uint8_t) (expectedCrc == record->crc32);
}

static void EncodeRecord(const EventReportLogRecord_t *source,
                         uint32_t sequence,
                         EventLogStorageRecord_t *target)
{
  if ((source == NULL) || (target == NULL))
  {
    return;
  }

  (void) memset(target, 0, sizeof(*target));
  target->sequence = sequence;
  target->eventLogID = source->eventLogID;
  target->eventLogTime = source->eventLogTime;
  target->eventLogTimeMilliseconds = source->eventLogTimeMilliseconds;
  target->eventLogClass = source->eventLogClass;
  target->valueLength = source->eventLogValueLength;
  if (source->eventLogValueLength > 0U)
  {
    (void) memcpy(&target->eventLogValue[0],
                  &source->eventLogValue[0],
                  source->eventLogValueLength);
  }
  target->crc32 = Crc32Compute((const uint8_t *) target,
                               sizeof(*target) - sizeof(target->crc32));
}

static void DecodeRecord(const EventLogStorageRecord_t *source,
                         EventReportLogRecord_t *target)
{
  if ((source == NULL) || (target == NULL))
  {
    return;
  }

  (void) memset(target, 0, sizeof(*target));
  target->eventLogID = source->eventLogID;
  target->eventLogTime = source->eventLogTime;
  target->eventLogTimeMilliseconds = source->eventLogTimeMilliseconds;
  target->eventLogClass = source->eventLogClass;
  target->eventLogValueLength = source->valueLength;
  if (source->valueLength > 0U)
  {
    (void) memcpy(&target->eventLogValue[0],
                  &source->eventLogValue[0],
                  source->valueLength);
  }
}

static void InsertScanEntry(LogRepositoryScanEntry_t *entries,
                            uint16_t *count,
                            uint32_t sequence,
                            uint16_t physicalIndex)
{
  uint16_t insertAt;
  uint16_t moveIndex;

  if ((entries == NULL) || (count == NULL) || (*count >= LOG_RECORDS_MAX))
  {
    return;
  }

  insertAt = *count;
  while ((insertAt > 0U) && (entries[insertAt - 1U].sequence > sequence))
  {
    insertAt--;
  }

  for (moveIndex = *count; moveIndex > insertAt; moveIndex--)
  {
    entries[moveIndex] = entries[moveIndex - 1U];
  }

  entries[insertAt].sequence = sequence;
  entries[insertAt].physicalIndex = physicalIndex;
  (*count)++;
}

static void MetadataLoad(LogRepositoryAdapterCtx_t *ctx,
                         IEepromStoragePort_t *eepromPort)
{
  LogRepositoryScanEntry_t entries[LOG_RECORDS_MAX];
  EventLogStorageRecord_t record;
  uint16_t entryCount = 0U;
  uint16_t physicalIndex;

  if (ctx == NULL)
  {
    return;
  }

  ctx->eepromPort = eepromPort;
  ctx->writeIndex = 0U;
  ctx->physicalWriteIndex = 0U;
  ctx->count = 0U;
  ctx->nextSequence = 0U;
  ctx->exists = FALSE;
  (void) memset(&ctx->logicalToPhysical[0], 0, sizeof(ctx->logicalToPhysical));

  if (eepromPort == NULL)
  {
    return;
  }

  for (physicalIndex = 0U; physicalIndex < LOG_RECORDS_MAX; physicalIndex++)
  {
    if (ReadPhysicalDirect(eepromPort, physicalIndex, &record) == FALSE)
    {
      continue;
    }

    if (PhysicalRecordIsValid(&record) == FALSE)
    {
      continue;
    }

    InsertScanEntry(&entries[0], &entryCount, record.sequence, physicalIndex);
  }

  if (entryCount == 0U)
  {
    return;
  }

  for (physicalIndex = 0U; physicalIndex < entryCount; physicalIndex++)
  {
    ctx->logicalToPhysical[physicalIndex] = entries[physicalIndex].physicalIndex;
  }

  ctx->count = entryCount;
  ctx->exists = TRUE;
  ctx->nextSequence = entries[entryCount - 1U].sequence + 1U;
  ctx->physicalWriteIndex =
    (uint16_t) ((entries[entryCount - 1U].physicalIndex + 1U) % LOG_RECORDS_MAX);
  UpdatePublicWriteIndex(ctx);
}

static uint8_t AdapterAppend(void *ctx,
                             const void *record,
                             uint32_t recordSize,
                             uint16_t *writtenIndex)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;
  const EventReportLogRecord_t *source = (const EventReportLogRecord_t *) record;
  EventLogStorageRecord_t storageRecord;
  uint16_t logicalIndex;
  uint16_t moveIndex;

  if ((c == NULL) || (source == NULL)
      || (recordSize != sizeof(EventReportLogRecord_t))
      || (source->eventLogClass == 0U)
      || (source->eventLogValueLength > EVENT_LOG_VALUE_MAX_LENGTH))
  {
    return FALSE;
  }

  EncodeRecord(source, c->nextSequence, &storageRecord);
  if (WritePhysical(c->physicalWriteIndex, &storageRecord) == FALSE)
  {
    return FALSE;
  }

  if (c->count < LOG_RECORDS_MAX)
  {
    logicalIndex = c->count;
    c->logicalToPhysical[logicalIndex] = c->physicalWriteIndex;
    c->count++;
  }
  else
  {
    logicalIndex = (uint16_t) (LOG_RECORDS_MAX - 1U);
    for (moveIndex = 0U; moveIndex < logicalIndex; moveIndex++)
    {
      c->logicalToPhysical[moveIndex] = c->logicalToPhysical[moveIndex + 1U];
    }
    c->logicalToPhysical[logicalIndex] = c->physicalWriteIndex;
  }

  c->exists = TRUE;
  c->nextSequence++;
  c->physicalWriteIndex = (uint16_t) ((c->physicalWriteIndex + 1U)
                                      % LOG_RECORDS_MAX);
  UpdatePublicWriteIndex(c);

  if (writtenIndex != NULL)
  {
    *writtenIndex = logicalIndex;
  }

  return TRUE;
}

static uint8_t AdapterRead(void *ctx,
                           uint16_t index,
                           void *record,
                           uint32_t recordSize)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;
  EventLogStorageRecord_t storageRecord;

  if ((c == NULL) || (record == NULL)
      || (recordSize != sizeof(EventReportLogRecord_t))
      || (index >= c->count))
  {
    return FALSE;
  }

  if (ReadPhysicalRuntime(c, c->logicalToPhysical[index], &storageRecord) == FALSE)
  {
    return FALSE;
  }

  if (PhysicalRecordIsValid(&storageRecord) == FALSE)
  {
    return FALSE;
  }

  DecodeRecord(&storageRecord, (EventReportLogRecord_t *) record);
  return TRUE;
}

static uint8_t AdapterClear(void *ctx)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;
  EventLogStorageRecord_t emptyRecord;
  uint16_t physicalIndex;

  if (c == NULL)
  {
    return FALSE;
  }

  (void) memset(&emptyRecord, 0, sizeof(emptyRecord));
  for (physicalIndex = 0U; physicalIndex < LOG_RECORDS_MAX; physicalIndex++)
  {
    if (WritePhysical(physicalIndex, &emptyRecord) == FALSE)
    {
      return FALSE;
    }
  }

  (void) memset(&c->logicalToPhysical[0], 0, sizeof(c->logicalToPhysical));
  c->writeIndex = 0U;
  c->physicalWriteIndex = 0U;
  c->count = 0U;
  c->nextSequence = 0U;
  c->exists = FALSE;
  return TRUE;
}

static uint8_t AdapterExists(void *ctx)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;

  return (c == NULL) ? FALSE : c->exists;
}

static uint8_t AdapterIsIndexValid(void *ctx, uint16_t index)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;

  if (c == NULL)
  {
    return FALSE;
  }

  return (uint8_t) (index < c->count);
}

static uint16_t AdapterGetCount(void *ctx)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;

  return (c == NULL) ? 0U : c->count;
}

static uint16_t AdapterGetWriteIndex(void *ctx)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;

  return (c == NULL) ? 0U : c->writeIndex;
}

void LogRepositoryAdapterInit(LogRepositoryAdapterCtx_t *ctx,
                              IEepromStoragePort_t *eepromPort)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
  MetadataLoad(ctx, eepromPort);
}

ILogRepositoryPort_t LogRepositoryAdapterCreatePort(
  LogRepositoryAdapterCtx_t *ctx)
{
  ILogRepositoryPort_t port;

  port.ctx = ctx;
  port.Append = AdapterAppend;
  port.Read = AdapterRead;
  port.Clear = AdapterClear;
  port.Exists = AdapterExists;
  port.IsIndexValid = AdapterIsIndexValid;
  port.GetCount = AdapterGetCount;
  port.GetWriteIndex = AdapterGetWriteIndex;

  return port;
}
