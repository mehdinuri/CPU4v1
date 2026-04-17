/* App/Ports/IEventLogPort.h
 *
 * Port interface for the persistent malfunction event log. Domain code
 * appends EventLogRecord_t entries as faults are detected or cleared;
 * the adapter stores them in NAND / EEPROM. Entries can be read back
 * (FIFO) for forwarding to CP over the control bus.
 */
#ifndef I_EVENT_LOG_PORT_H
#define I_EVENT_LOG_PORT_H

#include <stddef.h>
#include <stdint.h>

#define EVENT_LOG_PARAM_BYTES 8U

typedef struct
{
  uint32_t timestampSeconds;
  uint16_t eventCode;
  uint16_t source;
  uint8_t params[EVENT_LOG_PARAM_BYTES];
} EventLogRecord_t;

typedef struct
{
  void *ctx;

  uint8_t (*Append)(void *ctx, const EventLogRecord_t *record);
  uint8_t (*ReadNext)(void *ctx, EventLogRecord_t *record);
  uint8_t (*Count)(void *ctx, uint32_t *count);
  uint8_t (*Clear)(void *ctx);
} IEventLogPort_t;

static inline uint8_t EventLogAppend(IEventLogPort_t *port,
                                     const EventLogRecord_t *record)
{
  if ((port == NULL) || (port->Append == NULL))
  {
    return 0U;
  }

  return port->Append(port->ctx, record);
}

static inline uint8_t EventLogReadNext(IEventLogPort_t *port,
                                       EventLogRecord_t *record)
{
  if ((port == NULL) || (port->ReadNext == NULL))
  {
    return 0U;
  }

  return port->ReadNext(port->ctx, record);
}

static inline uint8_t EventLogCount(const IEventLogPort_t *port,
                                    uint32_t *count)
{
  if ((port == NULL) || (port->Count == NULL))
  {
    return 0U;
  }

  return port->Count(port->ctx, count);
}

static inline uint8_t EventLogClear(IEventLogPort_t *port)
{
  if ((port == NULL) || (port->Clear == NULL))
  {
    return 0U;
  }

  return port->Clear(port->ctx);
}

#endif /* I_EVENT_LOG_PORT_H */
