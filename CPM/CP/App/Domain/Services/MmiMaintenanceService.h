/* App/Domain/Services/MmiMaintenanceService.h */
#ifndef MMI_MAINTENANCE_SERVICE_H
#define MMI_MAINTENANCE_SERVICE_H

#include <stdint.h>

#include "Domain/Services/MmiLocalSettingsService.h"
#include "Domain/Services/MmiProtocol.h"
#include "Ports/IMmiMaintenancePort.h"
#include "Ports/IModuleBusPort.h"

enum
{
  MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_START = 1U,
  MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_STOP = 2U,
  MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_SELECT = 3U
};

typedef struct
{
  IMmiMaintenancePort_t *maintenancePort;
  IModuleBusPort_t *moduleBusPort;
  MmiLocalSettingsService_t *localSettingsService;
} MmiMaintenanceService_t;

void MmiMaintenanceServiceInit(MmiMaintenanceService_t *service);
void MmiMaintenanceServiceBind(MmiMaintenanceService_t *service,
                               IMmiMaintenancePort_t *maintenancePort,
                               IModuleBusPort_t *moduleBusPort,
                               MmiLocalSettingsService_t *localSettingsService);
MmiProtocolStatus_t MmiMaintenanceServiceExecute(
  MmiMaintenanceService_t *service,
  uint8_t resourceId,
  const uint8_t *payload,
  uint16_t payloadLength);
uint8_t MmiMaintenanceServiceRequestModeControl(
  MmiMaintenanceService_t *service,
  uint8_t requestedControl);
uint8_t MmiMaintenanceServiceRequestRelayState(
  MmiMaintenanceService_t *service,
  uint8_t requestedState);
uint8_t MmiMaintenanceServiceEnterIapMode(MmiMaintenanceService_t *service);
uint8_t MmiMaintenanceServiceFactoryReset(MmiMaintenanceService_t *service);
uint8_t MmiMaintenanceServiceStartOutputTest(MmiMaintenanceService_t *service);
uint8_t MmiMaintenanceServiceStopOutputTest(MmiMaintenanceService_t *service);
uint8_t MmiMaintenanceServiceSelectOutputTest(
  MmiMaintenanceService_t *service,
  uint8_t outputNumber);
uint8_t MmiMaintenanceServiceReadOutputTestStatus(
  MmiMaintenanceService_t *service,
  MmiMaintenanceOutputTestStatus_t *status);

#endif /* MMI_MAINTENANCE_SERVICE_H */
