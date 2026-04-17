/* App/Domain/Malfunction/ConflictMonitor.h
 *
 * TS2-style inter-channel conflict detector. For every pair of
 * conflicting channels it watches the measured channel-state image
 * each tick; if a forbidden combination (G-G, G-Y, Y-Y) persists for
 * longer than CONFLICT_DWELL_TICKS_DEFAULT it emits a conflict fault.
 *
 * Dwell counters are stored one-sided (only the (a,b) with a<b pair).
 */
#ifndef CONFLICT_MONITOR_H
#define CONFLICT_MONITOR_H

#include <stdint.h>

#include "Intersection/ChannelStateResolver.h"
#include "Intersection/ConfigurationService.h"
#include "Malfunction/FaultEmit.h"

/* 400 ms at 10 ms tick — matches legacy MIN_CONFLICT. */
#define CONFLICT_DWELL_TICKS_DEFAULT 40U

typedef struct
{
  uint16_t dwell[MP_CHANNEL_COUNT_MAX][MP_CHANNEL_COUNT_MAX];
  uint16_t threshold;
} ConflictMonitor_t;

void ConflictMonitorInit(ConflictMonitor_t *monitor, uint16_t threshold);
void ConflictMonitorTick(ConflictMonitor_t *monitor,
                         const ConfigurationService_t *config,
                         const ChannelStateImage_t *measured,
                         uint32_t timestampTicks,
                         const FaultEmit_t *emit);

#endif /* CONFLICT_MONITOR_H */
