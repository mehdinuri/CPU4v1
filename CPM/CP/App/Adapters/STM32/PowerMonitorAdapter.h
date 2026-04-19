/* App/Adapters/STM32/PowerMonitorAdapter.h
 *
 * IPowerMonitorPort concrete implementation backed by the STM32 cabinet power
 * measurements.
 */
#ifndef POWER_MONITOR_ADAPTER_H
#define POWER_MONITOR_ADAPTER_H

#include "Domain/Services/UiPowerService.h"
#include "Ports/IPowerMonitorPort.h"

typedef struct
{
  uint8_t primarySource;
  UiPowerService_t *powerService;
} PowerMonitorAdapterCtx_t;

void PowerMonitorAdapterInit(PowerMonitorAdapterCtx_t *ctx,
                             UiPowerService_t *powerService);
IPowerMonitorPort_t PowerMonitorAdapterCreatePort(PowerMonitorAdapterCtx_t *ctx);

#endif /* POWER_MONITOR_ADAPTER_H */
