/* App/Domain/Services/UiDoorService.c */
#include "UiDoorService.h"

#include <string.h>

#include "Domain/Services/EventReportService.h"

enum
{
  UI_DOOR_EVENT_OPEN = 64U,
  UI_DOOR_EVENT_CLOSED = 65U,
  UI_DOOR_POLL_INTERVAL_MS = 1000U
};

void UiDoorServiceInit(UiDoorService_t *service)
{
  if (service != NULL)
  {
    (void) memset(service, 0, sizeof(*service));
    service->latestOpenLogIndex = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
    service->latestCloseLogIndex = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
    service->pollIntervalMs = UI_DOOR_POLL_INTERVAL_MS;
  }
}

void UiDoorServiceBind(UiDoorService_t *service,
                       IDoorSensorPort_t *doorSensorPort,
                       ILogEventPort_t *eventPort,
                       MmiEventLogService_t *eventLogService)
{
  if (service != NULL)
  {
    service->doorSensorPort = doorSensorPort;
    service->eventPort = eventPort;
    service->eventLogService = eventLogService;
  }
}

void UiDoorServiceRefreshLatestLogIndices(UiDoorService_t *service)
{
  uint16_t index;

  if ((service == NULL) || (service->eventLogService == NULL))
  {
    return;
  }

  index = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
  service->latestOpenLogIndex = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
  if (MmiEventLogServiceFindLatestByEventId(
        service->eventLogService,
        EVENT_REPORT_EVENT_ID_DOOR_OPEN,
        &index) != 0U)
  {
    service->latestOpenLogIndex = index;
  }

  index = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
  service->latestCloseLogIndex = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
  if (MmiEventLogServiceFindLatestByEventId(
        service->eventLogService,
        EVENT_REPORT_EVENT_ID_DOOR_CLOSED,
        &index) != 0U)
  {
    service->latestCloseLogIndex = index;
  }
}

uint8_t UiDoorServiceStep(UiDoorService_t *service, uint32_t nowTicks)
{
  uint32_t elapsedTicks;
  uint8_t open;
  uint8_t eventCode;

  if ((service == NULL) || (service->doorSensorPort == NULL))
  {
    return 0U;
  }

  elapsedTicks = nowTicks - service->lastPollTick;
  if ((service->initialized != 0U) && (elapsedTicks < service->pollIntervalMs))
  {
    return 1U;
  }

  open = DoorSensorIsOpen(service->doorSensorPort);
  service->lastPollTick = nowTicks;

  if (service->initialized == 0U)
  {
    service->open = open;
    service->initialized = 1U;
    return 1U;
  }

  if (service->open != open)
  {
    service->open = open;
    service->changed = 1U;
    service->changeSequence++;
    eventCode = (open != 0U) ? UI_DOOR_EVENT_OPEN : UI_DOOR_EVENT_CLOSED;

    if ((service->eventPort != NULL)
        && (LogEventAppend(service->eventPort, eventCode, 0U, 0U, 0U) != 0U))
    {
      UiDoorServiceRefreshLatestLogIndices(service);
    }
  }

  return 1U;
}

uint8_t UiDoorServiceIsOpen(const UiDoorService_t *service)
{
  return (service == NULL) ? 0U : service->open;
}

uint8_t UiDoorServiceConsumeChanged(UiDoorService_t *service)
{
  uint8_t changed;

  if (service == NULL)
  {
    return 0U;
  }

  changed = service->changed;
  service->changed = 0U;
  return changed;
}

uint16_t UiDoorServiceGetLatestOpenLogIndex(const UiDoorService_t *service)
{
  return (service == NULL) ? MMI_PROTOCOL_V2_EVENT_CURSOR_NONE
         : service->latestOpenLogIndex;
}

uint16_t UiDoorServiceGetLatestCloseLogIndex(const UiDoorService_t *service)
{
  return (service == NULL) ? MMI_PROTOCOL_V2_EVENT_CURSOR_NONE
         : service->latestCloseLogIndex;
}

uint32_t UiDoorServiceGetChangeSequence(const UiDoorService_t *service)
{
  return (service == NULL) ? 0U : service->changeSequence;
}
