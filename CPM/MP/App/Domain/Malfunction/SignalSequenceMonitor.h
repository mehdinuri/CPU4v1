/* App/Domain/Malfunction/SignalSequenceMonitor.h
 *
 * Enforces legal per-channel color transitions: R -> G -> Y -> R
 * (and all-dark / flash as legal idle states). Any other edge raises
 * FAULT_CODE_SIGNAL_SEQUENCE immediately.
 */
#ifndef SIGNAL_SEQUENCE_MONITOR_H
#define SIGNAL_SEQUENCE_MONITOR_H

#include <stdint.h>

#include "Intersection/ChannelStateResolver.h"
#include "Malfunction/FaultEmit.h"

typedef struct
{
  ChannelColor_t lastColor[MP_CHANNEL_COUNT_MAX];
  uint8_t initialized;
} SignalSequenceMonitor_t;

void SignalSequenceMonitorInit(SignalSequenceMonitor_t *monitor);
void SignalSequenceMonitorTick(SignalSequenceMonitor_t *monitor,
                               const ChannelStateImage_t *measured,
                               uint32_t timestampTicks,
                               const FaultEmit_t *emit);

#endif /* SIGNAL_SEQUENCE_MONITOR_H */
