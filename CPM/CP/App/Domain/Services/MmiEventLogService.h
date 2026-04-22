/* App/Domain/Services/MmiEventLogService.h
 *
 * Domain service for MMI v2 event-log access over the CP-owned 1103 event
 * report service.
 */
#ifndef MMI_EVENT_LOG_SERVICE_H
#define MMI_EVENT_LOG_SERVICE_H

#include <stdint.h>

#include "Domain/Services/MmiProtocol.h"
#include "Domain/Services/EventReportService.h"

typedef struct
{
  EventReportService_t *eventReportService;
} MmiEventLogService_t;

void MmiEventLogServiceInit(MmiEventLogService_t *service);
void MmiEventLogServiceBind(MmiEventLogService_t *service,
                            EventReportService_t *eventReportService);
uint8_t MmiEventLogServiceGetLatestIndex(const MmiEventLogService_t *service,
                                         uint16_t *latestIndex);
uint8_t MmiEventLogServiceIsIndexValid(const MmiEventLogService_t *service,
                                       uint16_t index);
uint8_t MmiEventLogServiceCanReadFromIndex(const MmiEventLogService_t *service,
                                           uint16_t index);
uint8_t MmiEventLogServiceReadRecord(const MmiEventLogService_t *service,
                                     uint16_t index,
                                     MmiEventRecordV2_t *record);
uint8_t MmiEventLogServiceFindLatestByEventId(
  const MmiEventLogService_t *service,
  uint16_t eventId,
  uint16_t *index);
MmiProtocolStatus_t MmiEventLogServiceRead(
  const MmiEventLogService_t *service,
  uint8_t resourceId,
  const uint8_t *requestPayload,
  uint16_t requestPayloadLength,
  uint8_t *responsePayload,
  uint16_t responsePayloadCapacity,
  uint16_t *responsePayloadLength);

#endif /* MMI_EVENT_LOG_SERVICE_H */
