/* App/Domain/Malfunction/MinYellowMonitor.h
 *
 * NEMA TS2 minimum yellow change: each channel's yellow interval must
 * be at least phaseYellowChange seconds (stored in the phase config as
 * deciseconds). Monitor tracks per-channel Y-on duration; on Y->R
 * edge, if duration < configured minimum, emit FAULT_CODE_MIN_YELLOW_SHORT.
 */
#ifndef MIN_YELLOW_MONITOR_H
#define MIN_YELLOW_MONITOR_H

#include <stdint.h>

#include "Intersection/ChannelStateResolver.h"
#include "Intersection/ConfigurationService.h"
#include "Malfunction/FaultEmit.h"

/* At 10 ms tick, one decisecond is 10 ticks. */
#define MIN_YELLOW_TICKS_PER_DECISECOND 10U

typedef struct
{
  uint16_t yellowTicks[MP_CHANNEL_COUNT_MAX];
  uint8_t inYellow[MP_CHANNEL_COUNT_MAX];
} MinYellowMonitor_t;

void MinYellowMonitorInit(MinYellowMonitor_t *monitor);
void MinYellowMonitorTick(MinYellowMonitor_t *monitor,
                          const ConfigurationService_t *config,
                          const ChannelStateImage_t *measured,
                          uint32_t timestampTicks,
                          const FaultEmit_t *emit);

#endif /* MIN_YELLOW_MONITOR_H */
