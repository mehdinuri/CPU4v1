/* App/Domain/Services/UiDoorService.h */
#ifndef UI_DOOR_SERVICE_H
#define UI_DOOR_SERVICE_H

#include <stdint.h>

#include "Domain/Services/MmiEventLogService.h"
#include "Ports/IDoorSensorPort.h"

typedef struct
{
  IDoorSensorPort_t *doorSensorPort;
  MmiEventLogService_t *eventLogService;
  uint8_t initialized;
  uint8_t open;
  uint8_t changed;
  uint16_t latestOpenLogIndex;
  uint16_t latestCloseLogIndex;
  uint32_t changeSequence;
} UiDoorService_t;

void UiDoorServiceInit(UiDoorService_t *service);
void UiDoorServiceBind(UiDoorService_t *service,
                       IDoorSensorPort_t *doorSensorPort,
                       MmiEventLogService_t *eventLogService);
uint8_t UiDoorServiceStep(UiDoorService_t *service);
void UiDoorServiceRefreshLatestLogIndices(UiDoorService_t *service);
uint8_t UiDoorServiceIsOpen(const UiDoorService_t *service);
uint8_t UiDoorServiceConsumeChanged(UiDoorService_t *service);
uint16_t UiDoorServiceGetLatestOpenLogIndex(const UiDoorService_t *service);
uint16_t UiDoorServiceGetLatestCloseLogIndex(const UiDoorService_t *service);
uint32_t UiDoorServiceGetChangeSequence(const UiDoorService_t *service);

#endif /* UI_DOOR_SERVICE_H */
