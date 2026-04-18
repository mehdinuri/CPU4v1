/* App/Adapters/Mock/MockPowerMonitorAdapter.h */
#ifndef MOCK_POWER_MONITOR_ADAPTER_H
#define MOCK_POWER_MONITOR_ADAPTER_H

#include "Ports/IPowerMonitorPort.h"

typedef struct
{
  uint8_t primarySource;
  uint16_t lineVoltageTenthsVrms;
  uint32_t primarySourceReadCount;
  uint32_t lineVoltageReadCount;
} MockPowerMonitorAdapterCtx_t;

void MockPowerMonitorAdapterInit(MockPowerMonitorAdapterCtx_t *ctx);
IPowerMonitorPort_t MockPowerMonitorAdapterCreatePort(
  MockPowerMonitorAdapterCtx_t *ctx);

#endif /* MOCK_POWER_MONITOR_ADAPTER_H */
