/* App/Adapters/STM32/LogEventAdapter.c */
#include "LogEventAdapter.h"

#include "MLM.h"

static uint8_t AdapterAppend(void *ctx,
                             uint8_t eventCode,
                             uint8_t eventParam,
                             uint16_t eventShortParam,
                             uint32_t eventLongParam)
{
  (void) ctx;

  return LogRequest(LOG_REQ_APPEND_ASYNCH,
                    NULL,
                    eventCode,
                    eventParam,
                    eventShortParam,
                    eventLongParam,
                    0U);
}

void LogEventAdapterInit(LogEventAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    ctx->reserved = 0U;
  }
}

ILogEventPort_t LogEventAdapterCreatePort(LogEventAdapterCtx_t *ctx)
{
  ILogEventPort_t port;

  port.ctx = ctx;
  port.Append = AdapterAppend;

  return port;
}
