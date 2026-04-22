/* App/Domain/Malfunction/PowerSupplyMonitor.h
 *
 * Monitors PSM telemetry (line voltage, line frequency, +24 V rails,
 * +5 V rails) against configurable bounds with symmetric hysteresis.
 * Default limits mirror the legacy MP values (-20 % /
 * +20 % of nominal line voltage at 220 V, ±4 % frequency).
 */
#ifndef POWER_SUPPLY_MONITOR_H
#define POWER_SUPPLY_MONITOR_H

#include <stdint.h>

#include "Malfunction/FaultEmit.h"
#include "Ports/IFieldBusPort.h"

typedef struct
{
  uint16_t lineVoltageLower;
  uint16_t lineVoltageUpper;
  uint16_t lineVoltageHysteresis;
  uint16_t lineFrequencyLower;
  uint16_t lineFrequencyUpper;
  uint16_t rail24VMin;
  uint16_t rail5VMin;
} PowerSupplyMonitorLimits_t;

typedef struct
{
  PowerSupplyMonitorLimits_t limits;
  uint8_t lineVoltageFaultLatched[FIELD_BUS_PSM_COUNT];
  uint8_t frequencyFaultLatched[FIELD_BUS_PSM_COUNT];
  uint8_t rail24FaultLatched[FIELD_BUS_PSM_COUNT];
  uint8_t rail5FaultLatched[FIELD_BUS_PSM_COUNT];
} PowerSupplyMonitor_t;

void PowerSupplyMonitorInit(PowerSupplyMonitor_t *monitor,
                            const PowerSupplyMonitorLimits_t *limits);
void PowerSupplyMonitorTick(PowerSupplyMonitor_t *monitor,
                            const FieldBusSnapshot_t *snapshot,
                            uint32_t timestampTicks,
                            const FaultEmit_t *emit);

/* Matches legacy MP constants translated to ADC counts. */
extern const PowerSupplyMonitorLimits_t PowerSupplyMonitorDefaultLimits;

#endif /* POWER_SUPPLY_MONITOR_H */
