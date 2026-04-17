/* App/Domain/FaultMonitor/FaultMonitorTrace.h
 *
 * TS2 MMU §6.4-style trace log: rolling ring of the most recent
 * FaultEvent_t records, readable by CP for post-incident diagnosis.
 */
#ifndef FAULT_MONITOR_TRACE_H
#define FAULT_MONITOR_TRACE_H

#include <stdint.h>

#include "Malfunction/FaultCodes.h"

#define FAULT_MONITOR_TRACE_CAPACITY 32U

typedef struct
{
  FaultEvent_t entries[FAULT_MONITOR_TRACE_CAPACITY];
  uint32_t head;
  uint32_t size;
  uint32_t droppedCount;
} FaultMonitorTrace_t;

void FaultMonitorTraceInit(FaultMonitorTrace_t *trace);
void FaultMonitorTraceAppend(FaultMonitorTrace_t *trace,
                             const FaultEvent_t *event);
uint32_t FaultMonitorTraceSize(const FaultMonitorTrace_t *trace);
const FaultEvent_t *FaultMonitorTraceAt(const FaultMonitorTrace_t *trace,
                                        uint32_t reverseIndex);

#endif /* FAULT_MONITOR_TRACE_H */
