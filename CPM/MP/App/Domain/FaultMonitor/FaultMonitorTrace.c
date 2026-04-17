/* App/Domain/FaultMonitor/FaultMonitorTrace.c */

#include "FaultMonitor/FaultMonitorTrace.h"

#include <stddef.h>
#include <string.h>

void FaultMonitorTraceInit(FaultMonitorTrace_t *trace)
{
  if (trace == NULL)
  {
    return;
  }

  (void) memset(trace, 0, sizeof(*trace));
}

void FaultMonitorTraceAppend(FaultMonitorTrace_t *trace,
                             const FaultEvent_t *event)
{
  if ((trace == NULL) || (event == NULL))
  {
    return;
  }

  trace->entries[trace->head] = *event;
  trace->head = (trace->head + 1U) % FAULT_MONITOR_TRACE_CAPACITY;

  if (trace->size < FAULT_MONITOR_TRACE_CAPACITY)
  {
    trace->size++;
  }
  else
  {
    trace->droppedCount++;
  }
}

uint32_t FaultMonitorTraceSize(const FaultMonitorTrace_t *trace)
{
  return (trace != NULL) ? trace->size : 0U;
}

const FaultEvent_t *FaultMonitorTraceAt(const FaultMonitorTrace_t *trace,
                                        uint32_t reverseIndex)
{
  if ((trace == NULL) || (reverseIndex >= trace->size))
  {
    return NULL;
  }

  uint32_t forwardIndex =
    (trace->head + FAULT_MONITOR_TRACE_CAPACITY - 1U - reverseIndex)
    % FAULT_MONITOR_TRACE_CAPACITY;

  return &trace->entries[forwardIndex];
}
