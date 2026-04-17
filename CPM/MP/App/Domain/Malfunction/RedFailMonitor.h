/* App/Domain/Malfunction/RedFailMonitor.h
 *
 * NEMA TS2 red-fail: if a channel is expected to show red (every
 * channel must rest in red between service cycles) but the measured
 * red is absent — and the channel is not in flash — emit
 * FAULT_CODE_RED_FAIL.
 */
#ifndef RED_FAIL_MONITOR_H
#define RED_FAIL_MONITOR_H

#include <stdint.h>

#include "Intersection/ChannelStateResolver.h"
#include "Malfunction/FaultEmit.h"

#define RED_FAIL_DWELL_TICKS_DEFAULT 100U /* 1 s */

typedef struct
{
  uint16_t dwell[MP_CHANNEL_COUNT_MAX];
  uint16_t threshold;
} RedFailMonitor_t;

void RedFailMonitorInit(RedFailMonitor_t *monitor, uint16_t threshold);
void RedFailMonitorTick(RedFailMonitor_t *monitor,
                        const ChannelStateImage_t *commanded,
                        const ChannelStateImage_t *measured,
                        uint32_t timestampTicks,
                        const FaultEmit_t *emit);

#endif /* RED_FAIL_MONITOR_H */
