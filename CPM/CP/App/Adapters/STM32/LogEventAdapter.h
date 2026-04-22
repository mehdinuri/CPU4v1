/* App/Adapters/STM32/LogEventAdapter.h
 *
 * ILogEventPort concrete implementation that forwards legacy runtime events
 * into the CP-owned 1103 event-report service.
 */
#ifndef LOG_EVENT_ADAPTER_H
#define LOG_EVENT_ADAPTER_H

#include "Domain/Services/EventReportService.h"
#include "Ports/ILogEventPort.h"

typedef struct
{
  EventReportService_t *eventReportService;
} LogEventAdapterCtx_t;

void LogEventAdapterInit(LogEventAdapterCtx_t *ctx);
void LogEventAdapterBindEventReportService(LogEventAdapterCtx_t *ctx,
                                           EventReportService_t *service);
ILogEventPort_t LogEventAdapterCreatePort(LogEventAdapterCtx_t *ctx);

#endif /* LOG_EVENT_ADAPTER_H */
