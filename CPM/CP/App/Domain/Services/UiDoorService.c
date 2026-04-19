/* App/Domain/Services/UiDoorService.c */
#include "UiDoorService.h"

#include <string.h>

enum
{
  UI_DOOR_EVENT_OPEN = 64U,
  UI_DOOR_EVENT_CLOSED = 65U
};

void UiDoorServiceInit(UiDoorService_t *service)
{
  if (service != NULL)
  {
    (void) memset(service, 0, sizeof(*service));
    service->latestOpenLogIndex = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
    service->latestCloseLogIndex = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
  }
}

void UiDoorServiceBind(UiDoorService_t *service,
                       IDoorSensorPort_t *doorSensorPort,
                       MmiEventLogService_t *eventLogService)
{
  if (service != NULL)
  {
    service->doorSensorPort = doorSensorPort;
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
  if (MmiEventLogServiceFindLatestByEventCode(service->eventLogService,
                                              UI_DOOR_EVENT_OPEN,
                                              &index) != 0U)
  {
    service->latestOpenLogIndex = index;
  }

  index = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
  if (MmiEventLogServiceFindLatestByEventCode(service->eventLogService,
                                              UI_DOOR_EVENT_CLOSED,
                                              &index) != 0U)
  {
    service->latestCloseLogIndex = index;
  }
}

uint8_t UiDoorServiceStep(UiDoorService_t *service)
{
  uint8_t open;

  if ((service == NULL) || (service->doorSensorPort == NULL))
  {
    return 0U;
  }

  open = DoorSensorIsOpen(service->doorSensorPort);
  service->changed = 0U;

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
    UiDoorServiceRefreshLatestLogIndices(service);
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
