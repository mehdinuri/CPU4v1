/* App/Adapters/STM32/PowerMonitorAdapter.h
 *
 * IPowerMonitorPort concrete implementation backed by the STM32 cabinet power
 * measurements.
 */
#ifndef POWER_MONITOR_ADAPTER_H
#define POWER_MONITOR_ADAPTER_H

#include "Ports/IPowerMonitorPort.h"

typedef struct
{
  uint8_t primarySource;
} PowerMonitorAdapterCtx_t;

void PowerMonitorAdapterInit(PowerMonitorAdapterCtx_t *ctx);
IPowerMonitorPort_t PowerMonitorAdapterCreatePort(PowerMonitorAdapterCtx_t *ctx);

#endif /* POWER_MONITOR_ADAPTER_H */
