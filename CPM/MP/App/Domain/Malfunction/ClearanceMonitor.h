/* App/Domain/Malfunction/ClearanceMonitor.h
 *
 * NEMA TS2 red clearance: after a channel terminates yellow, its
 * configured phaseRedClear interval must elapse before any conflicting
 * channel can legally become active (R->G or steady Y).
 *
 * Per channel we maintain a clearance countdown (ticks remaining);
 * while it is non-zero, any conflicting channel becoming lit raises
 * FAULT_CODE_CLEARANCE_SHORT.
 */
#ifndef CLEARANCE_MONITOR_H
#define CLEARANCE_MONITOR_H

#include <stdint.h>

#include "Intersection/ChannelStateResolver.h"
#include "Intersection/ConfigurationService.h"
#include "Malfunction/FaultEmit.h"

#define CLEARANCE_TICKS_PER_DECISECOND 10U

typedef struct
{
  uint16_t clearanceRemainingTicks[MP_CHANNEL_COUNT_MAX];
  ChannelColor_t lastColor[MP_CHANNEL_COUNT_MAX];
  uint8_t initialized;
} ClearanceMonitor_t;

void ClearanceMonitorInit(ClearanceMonitor_t *monitor);
void ClearanceMonitorTick(ClearanceMonitor_t *monitor,
                          const ConfigurationService_t *config,
                          const ChannelStateImage_t *measured,
                          uint32_t timestampTicks,
                          const FaultEmit_t *emit);

#endif /* CLEARANCE_MONITOR_H */
