/* App/Domain/Malfunction/ModuleHealthMonitor.h
 *
 * Tracks liveness of CP, all PSMs, and all SSMs via periodic CAN
 * traffic. Each alive pulse (seen from the field or control bus
 * adapter) resets the module's stale counter. A counter that exceeds
 * its per-class threshold raises a missing-module fault.
 *
 * Defaults mirror the legacy MP windows: SSM 150 ms, PSM 1000 ms,
 * CP 500 ms, all at the MP 10 ms tick.
 */
#ifndef MODULE_HEALTH_MONITOR_H
#define MODULE_HEALTH_MONITOR_H

#include <stdint.h>

#include "Malfunction/FaultEmit.h"
#include "Ports/IFieldBusPort.h"

#define MODULE_HEALTH_CP_THRESHOLD_DEFAULT 50U    /* 500 ms */
#define MODULE_HEALTH_PSM_THRESHOLD_DEFAULT 100U  /* 1000 ms */
#define MODULE_HEALTH_SSM_THRESHOLD_DEFAULT 15U   /* 150 ms */

typedef struct
{
  uint16_t cpStaleTicks;
  uint16_t psmStaleTicks[FIELD_BUS_PSM_COUNT];
  uint16_t ssmStaleTicks[FIELD_BUS_SSM_COUNT];
  uint8_t cpMissing;
  uint8_t psmMissing[FIELD_BUS_PSM_COUNT];
  uint8_t ssmMissing[FIELD_BUS_SSM_COUNT];
  uint16_t cpThreshold;
  uint16_t psmThreshold;
  uint16_t ssmThreshold;
} ModuleHealthMonitor_t;

void ModuleHealthMonitorInit(ModuleHealthMonitor_t *monitor);
void ModuleHealthMonitorTick(ModuleHealthMonitor_t *monitor,
                             const FieldBusSnapshot_t *snapshot,
                             uint32_t timestampTicks,
                             const FaultEmit_t *emit);

#endif /* MODULE_HEALTH_MONITOR_H */
