/* App/Adapters/STM32/EventLogAdapter.c */

#include "EventLogAdapter.h"

#include <stddef.h>
#include <string.h>

static uint8_t AdapterAppend(void *ctx, const EventLogRecord_t *record)
{
  EventLogAdapterCtx_t *self = (EventLogAdapterCtx_t *) ctx;

  if ((self == NULL) || (record == NULL))
  {
    return 0U;
  }

  self->records[self->head] = *record;
  self->head = (self->head + 1U) % EVENT_LOG_ADAPTER_CAPACITY;

  if (self->count < EVENT_LOG_ADAPTER_CAPACITY)
  {
    self->count++;
  }
  else
  {
    self->tail = (self->tail + 1U) % EVENT_LOG_ADAPTER_CAPACITY;
    self->droppedCount++;
  }

  return 1U;
}

static uint8_t AdapterReadNext(void *ctx, EventLogRecord_t *record)
{
  EventLogAdapterCtx_t *self = (EventLogAdapterCtx_t *) ctx;

  if ((self == NULL) || (record == NULL))
  {
    return 0U;
  }

  if (self->count == 0U)
  {
    return 0U;
  }

  *record = self->records[self->tail];
  self->tail = (self->tail + 1U) % EVENT_LOG_ADAPTER_CAPACITY;
  self->count--;

  return 1U;
}

static uint8_t AdapterCount(void *ctx, uint32_t *count)
{
  const EventLogAdapterCtx_t *self = (const EventLogAdapterCtx_t *) ctx;

  if ((self == NULL) || (count == NULL))
  {
    return 0U;
  }

  *count = self->count;

  return 1U;
}

static uint8_t AdapterClear(void *ctx)
{
  EventLogAdapterCtx_t *self = (EventLogAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  self->head = 0U;
  self->tail = 0U;
  self->count = 0U;

  return 1U;
}

void EventLogAdapterInit(EventLogAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
}

IEventLogPort_t EventLogAdapterCreatePort(EventLogAdapterCtx_t *ctx)
{
  IEventLogPort_t port;

  port.ctx = ctx;
  port.Append = AdapterAppend;
  port.ReadNext = AdapterReadNext;
  port.Count = AdapterCount;
  port.Clear = AdapterClear;

  return port;
}
