/* App/Adapters/Mock/MockSystemMonitorAdapter.h */
#ifndef MOCK_SYSTEM_MONITOR_ADAPTER_H
#define MOCK_SYSTEM_MONITOR_ADAPTER_H

#include "Ports/ISystemMonitorPort.h"

typedef struct
{
  uint16_t batteryVoltageMilliVolts;
  int16_t thermistorDegC;
  uint8_t chargerActive;
  uint8_t chargerEnabled;
} MockSystemMonitorAdapterCtx_t;

void MockSystemMonitorAdapterInit(MockSystemMonitorAdapterCtx_t *ctx);
ISystemMonitorPort_t MockSystemMonitorAdapterCreatePort(
  MockSystemMonitorAdapterCtx_t *ctx);

#endif /* MOCK_SYSTEM_MONITOR_ADAPTER_H */
