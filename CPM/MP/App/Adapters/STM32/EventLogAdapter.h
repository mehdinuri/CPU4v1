/* App/Adapters/STM32/EventLogAdapter.h
 *
 * IEventLogPort backed by an in-RAM ring buffer for now. A follow-up
 * replaces the storage with the KIOXIA NAND on SPI2 (FLASH_WP / HOLD
 * lines already wired in main.h). The port contract stays identical.
 */
#ifndef EVENT_LOG_ADAPTER_H
#define EVENT_LOG_ADAPTER_H

#include "Ports/IEventLogPort.h"

#define EVENT_LOG_ADAPTER_CAPACITY 256U

typedef struct
{
  EventLogRecord_t records[EVENT_LOG_ADAPTER_CAPACITY];
  uint32_t head;
  uint32_t tail;
  uint32_t count;
  uint32_t droppedCount;
} EventLogAdapterCtx_t;

void EventLogAdapterInit(EventLogAdapterCtx_t *ctx);
IEventLogPort_t EventLogAdapterCreatePort(EventLogAdapterCtx_t *ctx);

#endif /* EVENT_LOG_ADAPTER_H */
