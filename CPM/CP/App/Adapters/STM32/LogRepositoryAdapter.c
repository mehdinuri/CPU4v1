/* App/Adapters/STM32/LogRepositoryAdapter.c
 *
 * EEPROM-backed circular log repository.
 */
#include "LogRepositoryAdapter.h"

#include <string.h>

#include "MSM.h"
#include "data.h"

static uint8_t MetadataWrite(LogRepositoryAdapterCtx_t *ctx)
{
  if (MSMRequest(MSM_REQ_EEPROM_WRITE,
                 EEPROM_STORAGE_ADDR_LOG_EXISTENCE,
                 &ctx->exists,
                 sizeof(ctx->exists)) == FALSE)
  {
    return FALSE;
  }

  if (MSMRequest(MSM_REQ_EEPROM_WRITE,
                 EEPROM_STORAGE_ADDR_LOG_INDEXES,
                 &ctx->writeIndex,
                 sizeof(ctx->writeIndex)) == FALSE)
  {
    return FALSE;
  }

  return MSMRequest(MSM_REQ_EEPROM_WRITE,
                    EEPROM_STORAGE_ADDR_LOG_RECORD_NUMBER,
                    &ctx->count,
                    sizeof(ctx->count));
}

static void MetadataLoad(LogRepositoryAdapterCtx_t *ctx,
                         IEepromStoragePort_t *eepromPort)
{
  ctx->writeIndex = 0U;
  ctx->count = 0U;
  ctx->exists = FALSE;

  if (eepromPort == NULL)
  {
    return;
  }

  /* Boot-time metadata restore runs before the scheduler starts.
   * Read the EEPROM through the device adapter instead of routing
   * through MSM, otherwise adapter construction depends on the MSM
   * task already existing and running.
   */
  if (EepromStorageRead(eepromPort,
                        EEPROM_STORAGE_ADDR_LOG_EXISTENCE,
                        &ctx->exists,
                        sizeof(ctx->exists)) == FALSE)
  {
    ctx->exists = FALSE;

    return;
  }

  if ((ctx->exists != TRUE) && (ctx->exists != FALSE))
  {
    ctx->exists = FALSE;

    return;
  }

  if (ctx->exists == FALSE)
  {
    return;
  }

  if (EepromStorageRead(eepromPort,
                        EEPROM_STORAGE_ADDR_LOG_INDEXES,
                        &ctx->writeIndex,
                        sizeof(ctx->writeIndex)) == FALSE)
  {
    ctx->writeIndex = 0U;
  }

  if (EepromStorageRead(eepromPort,
                        EEPROM_STORAGE_ADDR_LOG_RECORD_NUMBER,
                        &ctx->count,
                        sizeof(ctx->count)) == FALSE)
  {
    ctx->count = 0U;
  }

  if ((ctx->writeIndex >= LOG_RECORDS_MAX)
      || (ctx->count > LOG_RECORDS_MAX))
  {
    ctx->writeIndex = 0U;
    ctx->count = 0U;
    ctx->exists = FALSE;
  }
} /* MetadataLoad */

static uint8_t AdapterAppend(void *ctx, const void *record,
                             uint32_t recordSize, uint16_t *writtenIndex)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;
  uint16_t currentIndex;

  if ((record == NULL) || (recordSize != sizeof(tSLogRecord)))
  {
    return FALSE;
  }

  currentIndex = c->writeIndex;

  if (MSMRequest(MSM_REQ_EEPROM_WRITE,
                 EEPROM_STORAGE_ADDR_LOG
                 + (currentIndex * sizeof(tSLogRecord)),
                 (void *) record,
                 sizeof(tSLogRecord)) == FALSE)
  {
    return FALSE;
  }

  c->exists = TRUE;

  if (c->count < LOG_RECORDS_MAX)
  {
    c->count++;
  }

  c->writeIndex++;
  c->writeIndex %= LOG_RECORDS_MAX;

  if (MetadataWrite(c) == FALSE)
  {
    return FALSE;
  }

  if (writtenIndex != NULL)
  {
    *writtenIndex = currentIndex;
  }

  return TRUE;
} /* AdapterAppend */

static uint8_t AdapterRead(void *ctx, uint16_t index,
                           void *record, uint32_t recordSize)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;

  if ((record == NULL) || (recordSize != sizeof(tSLogRecord)))
  {
    return FALSE;
  }

  if (((index >= c->count) && (c->count < LOG_RECORDS_MAX))
      || (index >= LOG_RECORDS_MAX))
  {
    return FALSE;
  }

  return MSMRequest(MSM_REQ_EEPROM_READ,
                    EEPROM_STORAGE_ADDR_LOG
                    + (index * sizeof(tSLogRecord)),
                    record,
                    sizeof(tSLogRecord));
}

static uint8_t AdapterClear(void *ctx)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;

  c->writeIndex = 0U;
  c->count = 0U;
  c->exists = FALSE;

  return MetadataWrite(c);
}

static uint8_t AdapterExists(void *ctx)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;

  return c->exists;
}

static uint8_t AdapterIsIndexValid(void *ctx, uint16_t index)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;

  if (index >= LOG_RECORDS_MAX)
  {
    return FALSE;
  }

  if (c->count == LOG_RECORDS_MAX)
  {
    return TRUE;
  }

  return (index < c->count) ? TRUE : FALSE;
}

static uint16_t AdapterGetCount(void *ctx)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;

  return c->count;
}

static uint16_t AdapterGetWriteIndex(void *ctx)
{
  LogRepositoryAdapterCtx_t *c = (LogRepositoryAdapterCtx_t *) ctx;

  return c->writeIndex;
}

void LogRepositoryAdapterInit(LogRepositoryAdapterCtx_t *ctx,
                              IEepromStoragePort_t *eepromPort)
{
  memset(ctx, 0, sizeof(*ctx));
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
