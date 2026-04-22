/* App/Adapters/STM32/LogEventAdapter.c */
#include "LogEventAdapter.h"

static uint8_t AdapterAppend(void *ctx,
                             uint8_t eventCode,
                             uint8_t eventParam,
                             uint16_t eventShortParam,
                             uint32_t eventLongParam)
{
  LogEventAdapterCtx_t *adapterCtx = (LogEventAdapterCtx_t *) ctx;

  if ((adapterCtx == NULL) || (adapterCtx->eventReportService == NULL))
  {
    return 0U;
  }

  EventReportServiceAppendLegacyEvent(adapterCtx->eventReportService,
                                      eventCode,
                                      eventParam,
                                      eventShortParam,
                                      eventLongParam);
  return 1U;
}

void LogEventAdapterInit(LogEventAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    ctx->eventReportService = NULL;
  }
}

void LogEventAdapterBindEventReportService(LogEventAdapterCtx_t *ctx,
                                           EventReportService_t *service)
{
  if (ctx != NULL)
  {
    ctx->eventReportService = service;
  }
}

ILogEventPort_t LogEventAdapterCreatePort(LogEventAdapterCtx_t *ctx)
{
  ILogEventPort_t port;

  port.ctx = ctx;
  port.Append = AdapterAppend;

  return port;
}
