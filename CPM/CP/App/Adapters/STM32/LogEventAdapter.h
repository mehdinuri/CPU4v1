/* App/Adapters/STM32/LogEventAdapter.h
 *
 * ILogEventPort concrete implementation backed by the legacy MLM log append
 * entrypoint.
 */
#ifndef LOG_EVENT_ADAPTER_H
#define LOG_EVENT_ADAPTER_H

#include "Ports/ILogEventPort.h"

typedef struct
{
  uint8_t reserved;
} LogEventAdapterCtx_t;

void LogEventAdapterInit(LogEventAdapterCtx_t *ctx);
ILogEventPort_t LogEventAdapterCreatePort(LogEventAdapterCtx_t *ctx);

#endif /* LOG_EVENT_ADAPTER_H */
