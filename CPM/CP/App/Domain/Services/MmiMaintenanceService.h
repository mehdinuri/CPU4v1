/* App/Domain/Services/MmiMaintenanceService.h */
#ifndef MMI_MAINTENANCE_SERVICE_H
#define MMI_MAINTENANCE_SERVICE_H

#include <stdint.h>

#include "Domain/Services/OutputTestService.h"
#include "Domain/Services/RelayControlService.h"
#include "Domain/Services/MmiLocalSettingsService.h"
#include "Domain/Services/MmiProtocol.h"
#include "Ports/IControllerModeControlPort.h"
#include "Ports/IFactoryResetPort.h"
#include "Ports/IModuleBusPort.h"

enum
{
  MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_START = 1U,
  MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_STOP = 2U,
  MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_SELECT = 3U,
  MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_FORCE = 4U
};

typedef struct
{
  uint8_t outputNumber;
  uint8_t powerNet;
  uint8_t power;
  uint8_t state;
  uint8_t net;
  uint16_t currentNow;
  uint16_t currentMin;
  uint16_t currentMax;
} MmiMaintenanceOutputTestStatus_t;

typedef struct
{
  IControllerModeControlPort_t *controllerModePort;
  IFactoryResetPort_t *factoryResetPort;
  IModuleBusPort_t *moduleBusPort;
  MmiLocalSettingsService_t *localSettingsService;
  RelayControlService_t *relayControlService;
  OutputTestService_t *outputTestService;
} MmiMaintenanceService_t;

void MmiMaintenanceServiceInit(MmiMaintenanceService_t *service);
void MmiMaintenanceServiceBind(MmiMaintenanceService_t *service,
                               IControllerModeControlPort_t *controllerModePort,
                               IModuleBusPort_t *moduleBusPort,
                               MmiLocalSettingsService_t *localSettingsService,
                               IFactoryResetPort_t *factoryResetPort);
void MmiMaintenanceServiceBindRelayControlService(
  MmiMaintenanceService_t *service,
  RelayControlService_t *relayControlService);
void MmiMaintenanceServiceBindOutputTestService(
  MmiMaintenanceService_t *service,
  OutputTestService_t *outputTestService);
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
