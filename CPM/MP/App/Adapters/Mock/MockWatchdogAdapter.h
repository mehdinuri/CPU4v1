/* App/Adapters/Mock/MockWatchdogAdapter.h */
#ifndef MOCK_WATCHDOG_ADAPTER_H
#define MOCK_WATCHDOG_ADAPTER_H

#include "Ports/IWatchdogPort.h"

typedef struct
{
  uint32_t feedCount;
} MockWatchdogAdapterCtx_t;

void MockWatchdogAdapterInit(MockWatchdogAdapterCtx_t *ctx);
IWatchdogPort_t MockWatchdogAdapterCreatePort(MockWatchdogAdapterCtx_t *ctx);

#endif /* MOCK_WATCHDOG_ADAPTER_H */
