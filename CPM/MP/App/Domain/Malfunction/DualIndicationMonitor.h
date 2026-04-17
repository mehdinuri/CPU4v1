/* App/Domain/Malfunction/DualIndicationMonitor.h
 *
 * Raises FAULT_CODE_DUAL_INDICATION when any channel reports two or
 * more colors lit simultaneously (R+Y, R+G, Y+G, R+Y+G) for longer
 * than DUAL_INDICATION_DWELL_TICKS_DEFAULT. TS2 MMU mandates reaction
 * in < 1 s; at 10 ms tick this defaults to 200 ms (20 ticks).
 */
#ifndef DUAL_INDICATION_MONITOR_H
#define DUAL_INDICATION_MONITOR_H

#include <stdint.h>

#include "Intersection/ChannelStateResolver.h"
#include "Malfunction/FaultEmit.h"

#define DUAL_INDICATION_DWELL_TICKS_DEFAULT 20U

typedef struct
{
  uint16_t dwell[MP_CHANNEL_COUNT_MAX];
  uint16_t threshold;
} DualIndicationMonitor_t;

void DualIndicationMonitorInit(DualIndicationMonitor_t *monitor,
                               uint16_t threshold);
void DualIndicationMonitorTick(DualIndicationMonitor_t *monitor,
                               const ChannelStateImage_t *measured,
                               uint32_t timestampTicks,
                               const FaultEmit_t *emit);

#endif /* DUAL_INDICATION_MONITOR_H */
