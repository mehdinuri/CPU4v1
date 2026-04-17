/* App/Domain/Malfunction/DarkChannelMonitor.h
 *
 * Raises FAULT_CODE_DARK_CHANNEL when a channel was commanded to an
 * active (R, Y, or G) state but the measured state reports all three
 * colors off for longer than DARK_CHANNEL_DWELL_TICKS_DEFAULT.
 */
#ifndef DARK_CHANNEL_MONITOR_H
#define DARK_CHANNEL_MONITOR_H

#include <stdint.h>

#include "Intersection/ChannelStateResolver.h"
#include "Malfunction/FaultEmit.h"

#define DARK_CHANNEL_DWELL_TICKS_DEFAULT 50U /* 500 ms */

typedef struct
{
  uint16_t dwell[MP_CHANNEL_COUNT_MAX];
  uint16_t threshold;
} DarkChannelMonitor_t;

void DarkChannelMonitorInit(DarkChannelMonitor_t *monitor,
                            uint16_t threshold);
void DarkChannelMonitorTick(DarkChannelMonitor_t *monitor,
                            const ChannelStateImage_t *commanded,
                            const ChannelStateImage_t *measured,
                            uint32_t timestampTicks,
                            const FaultEmit_t *emit);

#endif /* DARK_CHANNEL_MONITOR_H */
