/* App/Adapters/Mock/MockLogRepositoryAdapter.c
 *
 * In-memory circular log repository for host-side tests.
 */
#include "MockLogRepositoryAdapter.h"

#include <string.h>

static uint8_t AdapterAppend(void *ctx, const void *record,
                             uint32_t recordSize, uint16_t *writtenIndex)
{
  MockLogRepositoryAdapterCtx_t *c = (MockLogRepositoryAdapterCtx_t *) ctx;
  uint16_t currentIndex;

  if ((record == NULL) || (recordSize > MOCK_LOG_REPOSITORY_MAX_RECORD_SIZE))
  {
    return 0U;
  }

  currentIndex = c->writeIndex;

  memset(c->records[currentIndex], 0, MOCK_LOG_REPOSITORY_MAX_RECORD_SIZE);
  memcpy(c->records[currentIndex], record, recordSize);

  c->exists = 1U;
  c->writeIndex++;
  c->writeIndex %= MOCK_LOG_REPOSITORY_MAX_RECORDS;

  if (c->count < MOCK_LOG_REPOSITORY_MAX_RECORDS)
  {
    c->count++;
  }

  if (writtenIndex != NULL)
  {
    *writtenIndex = currentIndex;
  }

  return 1U;
}

static uint8_t AdapterRead(void *ctx, uint16_t index,
                           void *record, uint32_t recordSize)
{
  MockLogRepositoryAdapterCtx_t *c = (MockLogRepositoryAdapterCtx_t *) ctx;

  if ((record == NULL) || (recordSize > MOCK_LOG_REPOSITORY_MAX_RECORD_SIZE))
  {
    return 0U;
  }

  if ((index >= MOCK_LOG_REPOSITORY_MAX_RECORDS)
      || ((c->count < MOCK_LOG_REPOSITORY_MAX_RECORDS) && (index >= c->count)))
  {
    return 0U;
  }

  memcpy(record, c->records[index], recordSize);

  return 1U;
}

static uint8_t AdapterClear(void *ctx)
{
  MockLogRepositoryAdapterCtx_t *c = (MockLogRepositoryAdapterCtx_t *) ctx;

  memset(c->records, 0, sizeof(c->records));
  c->writeIndex = 0U;
  c->count = 0U;
  c->exists = 0U;

  return 1U;
}

static uint8_t AdapterExists(void *ctx)
{
  MockLogRepositoryAdapterCtx_t *c = (MockLogRepositoryAdapterCtx_t *) ctx;

  return c->exists;
}

static uint8_t AdapterIsIndexValid(void *ctx, uint16_t index)
{
  MockLogRepositoryAdapterCtx_t *c = (MockLogRepositoryAdapterCtx_t *) ctx;

  if (index >= MOCK_LOG_REPOSITORY_MAX_RECORDS)
  {
    return 0U;
  }

  if (c->count == MOCK_LOG_REPOSITORY_MAX_RECORDS)
  {
    return 1U;
  }

  return (index < c->count) ? 1U : 0U;
}

static uint16_t AdapterGetCount(void *ctx)
{
  MockLogRepositoryAdapterCtx_t *c = (MockLogRepositoryAdapterCtx_t *) ctx;

  return c->count;
}

static uint16_t AdapterGetWriteIndex(void *ctx)
{
  MockLogRepositoryAdapterCtx_t *c = (MockLogRepositoryAdapterCtx_t *) ctx;

  return c->writeIndex;
}

void MockLogRepositoryAdapterInit(MockLogRepositoryAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

ILogRepositoryPort_t MockLogRepositoryAdapterCreatePort(
  MockLogRepositoryAdapterCtx_t *ctx)
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
