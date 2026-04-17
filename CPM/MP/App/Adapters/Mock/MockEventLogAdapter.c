/* App/Adapters/Mock/MockEventLogAdapter.c */

#include "MockEventLogAdapter.h"

#include <string.h>

static uint8_t MockAppend(void *ctx, const EventLogRecord_t *record)
{
  MockEventLogAdapterCtx_t *self = (MockEventLogAdapterCtx_t *) ctx;

  if ((self == NULL) || (record == NULL))
  {
    return 0U;
  }

  self->records[self->head] = *record;
  self->head = (self->head + 1U) % MOCK_EVENT_LOG_CAPACITY;

  if (self->count < MOCK_EVENT_LOG_CAPACITY)
  {
    self->count++;
  }
  else
  {
    self->tail = (self->tail + 1U) % MOCK_EVENT_LOG_CAPACITY;
  }

  return 1U;
}

static uint8_t MockReadNext(void *ctx, EventLogRecord_t *record)
{
  MockEventLogAdapterCtx_t *self = (MockEventLogAdapterCtx_t *) ctx;

  if ((self == NULL) || (record == NULL))
  {
    return 0U;
  }

  if (self->count == 0U)
  {
    return 0U;
  }

  *record = self->records[self->tail];
  self->tail = (self->tail + 1U) % MOCK_EVENT_LOG_CAPACITY;
  self->count--;

  return 1U;
}

static uint8_t MockCount(void *ctx, uint32_t *count)
{
  const MockEventLogAdapterCtx_t *self =
    (const MockEventLogAdapterCtx_t *) ctx;

  if ((self == NULL) || (count == NULL))
  {
    return 0U;
  }

  *count = self->count;

  return 1U;
}

static uint8_t MockClear(void *ctx)
{
  MockEventLogAdapterCtx_t *self = (MockEventLogAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  self->head = 0U;
  self->tail = 0U;
  self->count = 0U;

  return 1U;
}

void MockEventLogAdapterInit(MockEventLogAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
}

IEventLogPort_t MockEventLogAdapterCreatePort(MockEventLogAdapterCtx_t *ctx)
{
  IEventLogPort_t port;

  port.ctx = ctx;
  port.Append = MockAppend;
  port.ReadNext = MockReadNext;
  port.Count = MockCount;
  port.Clear = MockClear;

  return port;
}
