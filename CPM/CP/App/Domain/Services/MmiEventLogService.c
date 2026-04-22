/* App/Domain/Services/MmiEventLogService.c */
#include "MmiEventLogService.h"

#include <string.h>

static uint8_t IsBufferFull(uint16_t count, uint16_t writeIndex)
{
  return (uint8_t) ((count > 0U) && (writeIndex < count));
}

static uint16_t ComputeLatestIndex(uint16_t count, uint16_t writeIndex)
{
  if (count == 0U)
  {
    return MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
  }

  if (IsBufferFull(count, writeIndex) != 0U)
  {
    return (uint16_t) ((writeIndex + count - 1U) % count);
  }

  return (uint16_t) (count - 1U);
}

static void EncodeRecord(const MmiEventLogService_t *service,
                         uint16_t index,
                         const EventReportLogRecord_t *source,
                         MmiEventRecordV2_t *target)
{
  uint8_t eventNumber = 0U;

  if ((source == NULL) || (target == NULL))
  {
    return;
  }

  (void) memset(target, 0, sizeof(*target));
  target->logIndex = index;
  target->eventClass = source->eventLogClass;
  if ((service != NULL) && (service->eventReportService != NULL))
  {
    (void) EventReportServiceGetEventNumberForIndex(service->eventReportService,
                                                    index,
                                                    &eventNumber);
  }
  target->eventNumber = eventNumber;
  target->eventId = source->eventLogID;
  target->eventTime = source->eventLogTime;
  target->eventTimeMilliseconds = source->eventLogTimeMilliseconds;
  target->valueLength = source->eventLogValueLength;
  if (source->eventLogValueLength > 0U)
  {
    (void) memcpy(&target->value[0],
                  &source->eventLogValue[0],
                  source->eventLogValueLength);
  }
}

uint8_t MmiEventLogServiceGetLatestIndex(const MmiEventLogService_t *service,
                                         uint16_t *latestIndex)
{
  uint16_t count;
  uint16_t writeIndex;

  if ((service == NULL) || (service->eventReportService == NULL)
      || (latestIndex == NULL))
  {
    return 0U;
  }

  *latestIndex = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
  if (service->eventReportService->count == 0U)
  {
    return 1U;
  }

  count = service->eventReportService->count;
  writeIndex = service->eventReportService->writeIndex;
  *latestIndex = ComputeLatestIndex(count, writeIndex);
  return 1U;
}

uint8_t MmiEventLogServiceIsIndexValid(const MmiEventLogService_t *service,
                                       uint16_t index)
{
  if ((service == NULL) || (service->eventReportService == NULL))
  {
    return 0U;
  }

  return EventReportServiceCanReadLogIndex(service->eventReportService, index);
}

uint8_t MmiEventLogServiceCanReadFromIndex(const MmiEventLogService_t *service,
                                           uint16_t index)
{
  uint16_t writeIndex;

  if (MmiEventLogServiceIsIndexValid(service, index) == 0U)
  {
    return 0U;
  }

  writeIndex = service->eventReportService->writeIndex;
  return (uint8_t) (writeIndex != index);
}

uint8_t MmiEventLogServiceReadRecord(const MmiEventLogService_t *service,
                                     uint16_t index,
                                     MmiEventRecordV2_t *record)
{
  EventReportLogRecord_t storageRecord;

  if ((service == NULL) || (service->eventReportService == NULL)
      || (record == NULL))
  {
    return 0U;
  }

  if (EventReportServiceReadLogRecord(service->eventReportService,
                                      index,
                                      &storageRecord) == 0U)
  {
    return 0U;
  }

  EncodeRecord(service, index, &storageRecord, record);
  return 1U;
}

uint8_t MmiEventLogServiceFindLatestByEventId(
  const MmiEventLogService_t *service,
  uint16_t eventId,
  uint16_t *index)
{
  if ((service == NULL) || (service->eventReportService == NULL)
      || (index == NULL))
  {
    return 0U;
  }

  return EventReportServiceFindLatestEventId(service->eventReportService,
                                             eventId,
                                             index);
}

static MmiProtocolStatus_t ReadCursor(
  const MmiEventLogService_t *service,
  uint8_t *responsePayload,
  uint16_t responsePayloadCapacity,
  uint16_t *responsePayloadLength)
{
  uint16_t cursor = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;

  if ((service == NULL) || (service->eventReportService == NULL)
      || (responsePayload == NULL) || (responsePayloadLength == NULL)
      || (responsePayloadCapacity < sizeof(cursor)))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (MmiEventLogServiceGetLatestIndex(service, &cursor) == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  (void) memcpy(responsePayload, &cursor, sizeof(cursor));
  *responsePayloadLength = (uint16_t) sizeof(cursor);
  return MMI_PROTOCOL_V2_STATUS_OK;
}

static MmiProtocolStatus_t ReadPage(
  const MmiEventLogService_t *service,
  const uint8_t *requestPayload,
  uint16_t requestPayloadLength,
  uint8_t *responsePayload,
  uint16_t responsePayloadCapacity,
  uint16_t *responsePayloadLength)
{
  MmiEventPageRequestV2_t request;
  MmiEventPageHeaderV2_t header;
  uint16_t count;
  uint16_t writeIndex;
  uint16_t maxRecordsInBuffer;
  uint16_t currentIndex;
  uint16_t recordOffset;
  uint8_t recordCount;
  uint8_t fullBuffer;

  if ((service == NULL) || (service->eventReportService == NULL)
      || (requestPayload == NULL) || (responsePayload == NULL)
      || (responsePayloadLength == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (requestPayloadLength != sizeof(request))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  if (responsePayloadCapacity < sizeof(header))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  (void) memcpy(&request, requestPayload, sizeof(request));
  if (request.maxCount == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  maxRecordsInBuffer = (uint16_t) ((responsePayloadCapacity - sizeof(header))
                                   / sizeof(MmiEventRecordV2_t));
  if (maxRecordsInBuffer == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }
  if (maxRecordsInBuffer > MMI_PROTOCOL_V2_EVENT_PAGE_MAX_RECORDS)
  {
    maxRecordsInBuffer = MMI_PROTOCOL_V2_EVENT_PAGE_MAX_RECORDS;
  }

  (void) memset(&header, 0, sizeof(header));
  header.startIndex = request.startIndex;

  count = service->eventReportService->count;
  writeIndex = service->eventReportService->writeIndex;
  if (count == 0U)
  {
    (void) memcpy(responsePayload, &header, sizeof(header));
    *responsePayloadLength = (uint16_t) sizeof(header);
    return MMI_PROTOCOL_V2_STATUS_OK;
  }

  if (request.startIndex >= count)
  {
    return MMI_PROTOCOL_V2_STATUS_BAD_INDEX;
  }

  fullBuffer = IsBufferFull(count, writeIndex);
  currentIndex = request.startIndex;
  recordOffset = (uint16_t) sizeof(header);
  recordCount = request.maxCount;
  if ((uint16_t) recordCount > maxRecordsInBuffer)
  {
    recordCount = (uint8_t) maxRecordsInBuffer;
  }

  header.count = 0U;
  while (header.count < recordCount)
  {
    EventReportLogRecord_t storageRecord;
    MmiEventRecordV2_t encodedRecord;

    if (EventReportServiceReadLogRecord(service->eventReportService,
                                        currentIndex,
                                        &storageRecord) == 0U)
    {
      break;
    }

    EncodeRecord(service, currentIndex, &storageRecord, &encodedRecord);
    (void) memcpy(&responsePayload[recordOffset],
                  &encodedRecord,
                  sizeof(encodedRecord));
    recordOffset = (uint16_t) (recordOffset + sizeof(encodedRecord));
    header.count++;

    if (fullBuffer != 0U)
    {
      currentIndex = (uint16_t) ((currentIndex + 1U) % count);
      if (currentIndex == request.startIndex)
      {
        break;
      }
    }
    else
    {
      currentIndex++;
      if (currentIndex >= count)
      {
        break;
      }
    }
  }

  if (fullBuffer != 0U)
  {
    header.moreAvailable = (uint8_t) (count > header.count);
  }
  else
  {
    header.moreAvailable =
      (uint8_t) ((request.startIndex + header.count) < count);
  }

  (void) memcpy(responsePayload, &header, sizeof(header));
  *responsePayloadLength = recordOffset;
  return MMI_PROTOCOL_V2_STATUS_OK;
}

void MmiEventLogServiceInit(MmiEventLogService_t *service)
{
  if (service != NULL)
  {
    service->eventReportService = NULL;
  }
}

void MmiEventLogServiceBind(MmiEventLogService_t *service,
                            EventReportService_t *eventReportService)
{
  if (service != NULL)
  {
    service->eventReportService = eventReportService;
  }
}

MmiProtocolStatus_t MmiEventLogServiceRead(
  const MmiEventLogService_t *service,
  uint8_t resourceId,
  const uint8_t *requestPayload,
  uint16_t requestPayloadLength,
  uint8_t *responsePayload,
  uint16_t responsePayloadCapacity,
  uint16_t *responsePayloadLength)
{
  switch ((MmiProtocolEventResource_t) resourceId)
  {
      case MMI_PROTOCOL_V2_EVENT_RESOURCE_PAGE:
      {
        return ReadPage(service,
                        requestPayload,
                        requestPayloadLength,
                        responsePayload,
                        responsePayloadCapacity,
                        responsePayloadLength);
      }

      case MMI_PROTOCOL_V2_EVENT_RESOURCE_CURSOR:
      {
        if (requestPayloadLength != 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
        }

        return ReadCursor(service,
                          responsePayload,
                          responsePayloadCapacity,
                          responsePayloadLength);
      }

      default:
      {
        return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
      }
  }
}
