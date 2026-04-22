/* App/Domain/FaultMonitor/FaultMonitorService.h
 *
 * Aggregates every monitor output into FaultMonitorStatus_t and keeps
 * a rolling FaultMonitorTrace of recent faults. Exposes read + ack
 * semantics so CP can read the image once and the service clears
 * any "new-since-last-read" bits — same shape as IUnitAlarmPort's
 * AcknowledgeUnitAlarmStatus2Read.
 */
#ifndef FAULT_MONITOR_SERVICE_H
#define FAULT_MONITOR_SERVICE_H

#include <stdint.h>

#include "FaultMonitor/FaultMonitorStatus.h"
#include "FaultMonitor/FaultMonitorTrace.h"
#include "Malfunction/FaultCodes.h"

typedef struct
{
  FaultMonitorStatus_t status;
  FaultMonitorStatus_t statusSinceLastRead;
  FaultMonitorTrace_t trace;
  uint32_t totalFaults;
} FaultMonitorService_t;

void FaultMonitorServiceInit(FaultMonitorService_t *service);
void FaultMonitorServiceOnFault(FaultMonitorService_t *service,
                                const FaultEvent_t *event);
void FaultMonitorServiceRead(FaultMonitorService_t *service,
                             FaultMonitorStatus_t *outStatus);
void FaultMonitorServiceReadAndAck(FaultMonitorService_t *service,
                                   FaultMonitorStatus_t *outStatus);
uint32_t FaultMonitorServiceGetTotalFaults(
  const FaultMonitorService_t *service);
uint8_t FaultMonitorServiceGetEventBySequence(
  const FaultMonitorService_t *service,
  uint32_t sequence,
  FaultEvent_t *outEvent);
const FaultMonitorTrace_t *FaultMonitorServiceGetTrace(
  const FaultMonitorService_t *service);

#endif /* FAULT_MONITOR_SERVICE_H */
