/* App/Ports/ILogEventPort.h
 *
 * Event-log append port.
 * Used by domain services that need to append controller events without
 * depending on the legacy log-management implementation.
 */
#ifndef ILOG_EVENT_PORT_H
#define ILOG_EVENT_PORT_H

#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*Append)(void *ctx,
                    uint8_t eventCode,
                    uint8_t eventParam,
                    uint16_t eventShortParam,
                    uint32_t eventLongParam);
} ILogEventPort_t;

static inline uint8_t LogEventAppend(ILogEventPort_t *p,
                                     uint8_t eventCode,
                                     uint8_t eventParam,
                                     uint16_t eventShortParam,
                                     uint32_t eventLongParam)
{
  return p->Append(p->ctx,
                   eventCode,
                   eventParam,
                   eventShortParam,
                   eventLongParam);
}

#endif /* ILOG_EVENT_PORT_H */
