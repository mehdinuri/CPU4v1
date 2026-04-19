/* App/Domain/Services/MmiMaintenanceService.c */
#include "MmiMaintenanceService.h"

#include <string.h>

void MmiMaintenanceServiceInit(MmiMaintenanceService_t *service)
{
  if (service != NULL)
  {
    (void) memset(service, 0, sizeof(*service));
  }
}

void MmiMaintenanceServiceBind(MmiMaintenanceService_t *service,
                               IControllerModeControlPort_t *controllerModePort,
                               IModuleBusPort_t *moduleBusPort,
                               MmiLocalSettingsService_t *localSettingsService,
                               IFactoryResetPort_t *factoryResetPort)
{
  if (service != NULL)
  {
    service->controllerModePort = controllerModePort;
    service->moduleBusPort = moduleBusPort;
    service->localSettingsService = localSettingsService;
    service->factoryResetPort = factoryResetPort;
    service->relayControlService = NULL;
    service->outputTestService = NULL;
  }
}

void MmiMaintenanceServiceBindRelayControlService(
  MmiMaintenanceService_t *service,
  RelayControlService_t *relayControlService)
{
  if (service != NULL)
  {
    service->relayControlService = relayControlService;
  }
}

void MmiMaintenanceServiceBindOutputTestService(
  MmiMaintenanceService_t *service,
  OutputTestService_t *outputTestService)
{
  if (service != NULL)
  {
    service->outputTestService = outputTestService;
  }
}

uint8_t MmiMaintenanceServiceRequestModeControl(
  MmiMaintenanceService_t *service,
  uint8_t requestedControl)
{
  return (service == NULL) ? 0U
         : ControllerModeControlPortRequest(service->controllerModePort,
                                            requestedControl);
}

uint8_t MmiMaintenanceServiceRequestRelayState(
  MmiMaintenanceService_t *service,
  uint8_t requestedState)
{
  return ((service == NULL) || (service->relayControlService == NULL)) ? 0U
         : RelayControlServiceSetUserOutputPowerEnabled(
           service->relayControlService,
           requestedState);
}

uint8_t MmiMaintenanceServiceFactoryReset(MmiMaintenanceService_t *service)
{
  return (service == NULL) ? 0U
         : FactoryResetPortRequest(service->factoryResetPort);
}

uint8_t MmiMaintenanceServiceStartOutputTest(MmiMaintenanceService_t *service)
{
  return ((service == NULL) || (service->outputTestService == NULL)) ? 0U
         : OutputTestServiceSetEnabled(service->outputTestService, 1U);
}

uint8_t MmiMaintenanceServiceStopOutputTest(MmiMaintenanceService_t *service)
{
  return ((service == NULL) || (service->outputTestService == NULL)) ? 0U
         : OutputTestServiceSetEnabled(service->outputTestService, 0U);
}

uint8_t MmiMaintenanceServiceSelectOutputTest(
  MmiMaintenanceService_t *service,
  uint8_t outputNumber)
{
  return ((service == NULL) || (service->outputTestService == NULL)) ? 0U
         : OutputTestServiceSetChannelAspect(service->outputTestService,
                                             outputNumber,
                                             OUTPUT_DRIVER_ASPECT_GREEN);
}

uint8_t MmiMaintenanceServiceReadOutputTestStatus(
  MmiMaintenanceService_t *service,
  MmiMaintenanceOutputTestStatus_t *status)
{
  uint8_t channelIndex;
  uint16_t forcedMask;
  OutputDriverAspect_t aspect = OUTPUT_DRIVER_ASPECT_DARK;

  if (service == NULL)
  {
    return 0U;
  }

  if ((service->outputTestService == NULL) || (status == NULL))
  {
    return 0U;
  }

  (void) memset(status, 0, sizeof(*status));
  forcedMask = OutputTestServiceGetForcedMask(service->outputTestService);

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    if ((forcedMask & (uint16_t) (1U << channelIndex)) != 0U)
    {
      status->outputNumber = (uint8_t) (channelIndex + 1U);
      if (OutputTestServiceGetChannelAspect(service->outputTestService,
                                            status->outputNumber,
                                            &aspect) != 0U)
      {
        status->state = (uint8_t) aspect;
      }
      break;
    }
  }

  return 1U;
}

static MmiProtocolStatus_t ExecuteTimeSet(MmiMaintenanceService_t *service,
                                          const uint8_t *payload,
                                          uint16_t payloadLength)
{
  MmiMaintenanceTimeSetCommandV2_t request;
  MmiLocalClockSettingsV2_t clockSettings;
  uint8_t buffer[sizeof(clockSettings)];
  uint16_t bufferLength = 0U;

  if ((service == NULL) || (service->localSettingsService == NULL)
      || (payload == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (payloadLength != sizeof(request))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  (void) memcpy(&request, payload, sizeof(request));
  if (MmiLocalSettingsServiceRead(service->localSettingsService,
                                  MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS,
                                  &buffer[0],
                                  &bufferLength) != MMI_PROTOCOL_V2_STATUS_OK)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (bufferLength != sizeof(clockSettings))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  (void) memcpy(&clockSettings, &buffer[0], sizeof(clockSettings));
  clockSettings.second = request.second;
  clockSettings.minute = request.minute;
  clockSettings.hour = request.hour;
  clockSettings.day = request.day;
  clockSettings.month = request.month;
  clockSettings.year = request.year;

  return MmiLocalSettingsServiceWrite(service->localSettingsService,
                                      MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS,
                                      (const uint8_t *) &clockSettings,
                                      sizeof(clockSettings));
}

static MmiProtocolStatus_t ExecuteDetectorReset(MmiMaintenanceService_t *service,
                                                const uint8_t *payload,
                                                uint16_t payloadLength)
{
  MmiMaintenanceDetectorResetCommandV2_t request;
  ModuleBusDetectorClass_t detectorClass;

  if ((service == NULL) || (payload == NULL) || (service->moduleBusPort == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (payloadLength != sizeof(request))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  (void) memcpy(&request, payload, sizeof(request));
  if (request.detectorClass == 1U)
  {
    detectorClass = MODULE_BUS_DETECTOR_CLASS_VEHICLE;
  }
  else if (request.detectorClass == 2U)
  {
    detectorClass = MODULE_BUS_DETECTOR_CLASS_PEDESTRIAN;
  }
  else
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  return (ModuleBusCommandDetectorReset(service->moduleBusPort,
                                        detectorClass,
                                        request.detectorNumber) != 0U)
         ? MMI_PROTOCOL_V2_STATUS_OK
         : MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
}

static MmiProtocolStatus_t ExecuteOutputTest(MmiMaintenanceService_t *service,
                                             const uint8_t *payload,
                                             uint16_t payloadLength)
{
  MmiMaintenanceOutputTestCommandV2_t request;
  uint8_t ok = 0U;

  if ((service == NULL) || (payload == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (payloadLength != sizeof(request))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  (void) memcpy(&request, payload, sizeof(request));
  switch (request.command)
  {
      case MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_START:
      {
        ok = MmiMaintenanceServiceStartOutputTest(service);
        break;
      }

      case MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_STOP:
      {
        ok = MmiMaintenanceServiceStopOutputTest(service);
        break;
      }

      case MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_SELECT:
      {
        ok = MmiMaintenanceServiceSelectOutputTest(service,
                                                   request.outputNumber);
        break;
      }

      case MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_FORCE:
      {
        if (service->outputTestService == NULL)
        {
          return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
        }

        ok = OutputTestServiceSetChannelAspect(service->outputTestService,
                                               request.outputNumber,
                                               (OutputDriverAspect_t)
                                               request.aspect);
        break;
      }

      default:
      {
        return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
      }
  }

  return (ok != 0U) ? MMI_PROTOCOL_V2_STATUS_OK
         : MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
}

MmiProtocolStatus_t MmiMaintenanceServiceExecute(
  MmiMaintenanceService_t *service,
  uint8_t resourceId,
  const uint8_t *payload,
  uint16_t payloadLength)
{
  if (service == NULL)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  switch ((MmiProtocolMaintenanceResource_t) resourceId)
  {
      case MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_MODE_CONTROL:
      {
        MmiMaintenanceModeCommandV2_t request;

        if ((payload == NULL) || (payloadLength != sizeof(request)))
        {
          return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
        }

        (void) memcpy(&request, payload, sizeof(request));
        return (MmiMaintenanceServiceRequestModeControl(service,
                                                       request.requestedControl)
                != 0U)
               ? MMI_PROTOCOL_V2_STATUS_OK
               : MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
      }

      case MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_TIME_SET:
      {
        return ExecuteTimeSet(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_DETECTOR_RESET:
      {
        return ExecuteDetectorReset(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_OUTPUT_TEST:
      {
        return ExecuteOutputTest(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_FACTORY_RESET:
      {
        MmiMaintenanceFactoryResetCommandV2_t request;

        if ((payload == NULL) || (payloadLength != sizeof(request)))
        {
          return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
        }

        (void) memcpy(&request, payload, sizeof(request));
        if ((request.magic0 != 0xA5U) || (request.magic1 != 0x5AU))
        {
          return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
        }

        return (MmiMaintenanceServiceFactoryReset(service) != 0U)
               ? MMI_PROTOCOL_V2_STATUS_OK
               : MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
      }

      case MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_RELAY_COMMAND:
      {
        MmiMaintenanceRelayCommandV2_t request;

        if ((payload == NULL) || (payloadLength != sizeof(request)))
        {
          return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
        }

        (void) memcpy(&request, payload, sizeof(request));
        return (MmiMaintenanceServiceRequestRelayState(service,
                                                       request.requestedState)
                != 0U)
               ? MMI_PROTOCOL_V2_STATUS_OK
               : MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
      }

      default:
      {
        return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
      }
  }
}
