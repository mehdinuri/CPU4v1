/* App/Domain/Services/MmiEventLogService.h
 *
 * Domain service for MMI v2 event-log access over the canonical log
 * repository port.
 */
#ifndef MMI_EVENT_LOG_SERVICE_H
#define MMI_EVENT_LOG_SERVICE_H

#include <stdint.h>

#include "Domain/Services/MmiProtocol.h"
#include "Ports/ILogRepositoryPort.h"

typedef struct
{
  ILogRepositoryPort_t *logRepositoryPort;
} MmiEventLogService_t;

void MmiEventLogServiceInit(MmiEventLogService_t *service);
void MmiEventLogServiceBind(MmiEventLogService_t *service,
                            ILogRepositoryPort_t *logRepositoryPort);
uint8_t MmiEventLogServiceGetLatestIndex(const MmiEventLogService_t *service,
                                         uint16_t *latestIndex);
uint8_t MmiEventLogServiceCanReadFromIndex(const MmiEventLogService_t *service,
                                           uint16_t index);
uint8_t MmiEventLogServiceReadRecord(const MmiEventLogService_t *service,
                                     uint16_t index,
                                     MmiEventRecordV2_t *record);
uint8_t MmiEventLogServiceFindLatestByEventCode(
  const MmiEventLogService_t *service,
  uint8_t eventCode,
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
