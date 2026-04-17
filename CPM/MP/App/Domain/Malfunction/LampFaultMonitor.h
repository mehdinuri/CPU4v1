/* App/Domain/Malfunction/LampFaultMonitor.h
 *
 * Cross-checks the commanded load-switch image against the measured
 * voltage-presence bits reported by each SSM. Four cases:
 *
 *   commanded OFF + measured OFF  -> nominal
 *   commanded OFF + measured ON   -> lamp driven externally or short
 *                                     to mains (FAULT_CODE_LAMP_DRIVEN_EXTERNALLY)
 *   commanded ON  + measured OFF  -> open circuit / burnt lamp
 *                                     (FAULT_CODE_LAMP_OPEN_CIRCUIT)
 *   commanded ON  + measured ON   -> nominal
 *
 * Current-based open / short discrimination (legacy DetermineSOLampsState)
 * is a follow-up once SSM current scaling is finalised; for now we
 * emit coarse-grained open/driven faults with a dwell threshold to
 * avoid alarming on transient edges around CP broadcast cycles.
 */
#ifndef LAMP_FAULT_MONITOR_H
#define LAMP_FAULT_MONITOR_H

#include <stdint.h>

#include "Intersection/ChannelStateResolver.h"
#include "Malfunction/FaultEmit.h"

#define LAMP_FAULT_DWELL_TICKS_DEFAULT 30U /* 300 ms */

typedef struct
{
  uint16_t openDwell[MP_SIGNAL_OUTPUT_COUNT_MAX];
  uint16_t externalDwell[MP_SIGNAL_OUTPUT_COUNT_MAX];
  uint16_t threshold;
} LampFaultMonitor_t;

void LampFaultMonitorInit(LampFaultMonitor_t *monitor, uint16_t threshold);
void LampFaultMonitorTick(LampFaultMonitor_t *monitor,
                          const FieldBusLoadSwitchImage_t *commanded,
                          const FieldBusSsmTelemetry_t *ssm,
                          uint8_t ssmCount,
                          uint32_t timestampTicks,
                          const FaultEmit_t *emit);

#endif /* LAMP_FAULT_MONITOR_H */
