/* App/Adapters/STM32/WatchdogAdapter.h
 *
 * IWatchdogPort wrapper over STM32 IWDG. The IWDG peripheral handle
 * (hiwdg) is owned by CubeMX in iwdg.c; this adapter just refreshes
 * it on Feed().
 */
#ifndef WATCHDOG_ADAPTER_H
#define WATCHDOG_ADAPTER_H

#include "Ports/IWatchdogPort.h"

typedef struct
{
  uint32_t feedCount;
} WatchdogAdapterCtx_t;

void WatchdogAdapterInit(WatchdogAdapterCtx_t *ctx);
IWatchdogPort_t WatchdogAdapterCreatePort(WatchdogAdapterCtx_t *ctx);

#endif /* WATCHDOG_ADAPTER_H */
