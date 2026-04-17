/* App/Adapters/Mock/MockEventLogAdapter.h
 *
 * IEventLogPort in-memory test double — fixed-size ring buffer.
 */
#ifndef MOCK_EVENT_LOG_ADAPTER_H
#define MOCK_EVENT_LOG_ADAPTER_H

#include "Ports/IEventLogPort.h"

#define MOCK_EVENT_LOG_CAPACITY 64U

typedef struct
{
  EventLogRecord_t records[MOCK_EVENT_LOG_CAPACITY];
  uint32_t head;
  uint32_t tail;
  uint32_t count;
} MockEventLogAdapterCtx_t;

void MockEventLogAdapterInit(MockEventLogAdapterCtx_t *ctx);
IEventLogPort_t MockEventLogAdapterCreatePort(MockEventLogAdapterCtx_t *ctx);

#endif /* MOCK_EVENT_LOG_ADAPTER_H */
