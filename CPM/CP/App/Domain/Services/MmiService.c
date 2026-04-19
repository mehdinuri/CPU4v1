/* App/Domain/Services/MmiService.c */
#include "MmiService.h"

#include <stddef.h>

static const MmiResourceDescriptor_t kRuntimeResources[] =
{
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_SUMMARY,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_ENGINE_RUNTIME,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiRuntimeSummaryV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_RINGS,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_ENGINE_RUNTIME,
    MMI_RESOURCE_COUNT_RINGS,
    (uint16_t) sizeof(MmiRuntimeRingRecordV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_PHASES,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_ENGINE_RUNTIME,
    MMI_RESOURCE_COUNT_PHASES,
    (uint16_t) sizeof(MmiRuntimePhaseRecordV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_CHANNELS,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_ENGINE_RUNTIME,
    MMI_RESOURCE_COUNT_CHANNELS,
    (uint16_t) sizeof(MmiRuntimeChannelRecordV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_OVERLAPS,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_ENGINE_RUNTIME,
    MMI_RESOURCE_COUNT_OVERLAPS,
    (uint16_t) sizeof(MmiRuntimeOverlapRecordV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_RAW_INPUTS,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_CONTROLLER_SNAPSHOT,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiRuntimeRawInputsV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_VEHICLE_DETECTORS,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_ENGINE_RUNTIME,
    MMI_RESOURCE_COUNT_VEHICLE_DETECTORS,
    (uint16_t) sizeof(MmiRuntimeVehicleDetectorRecordV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_PEDESTRIAN_DETECTORS,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_ENGINE_RUNTIME,
    MMI_RESOURCE_COUNT_PEDESTRIAN_DETECTORS,
    (uint16_t) sizeof(MmiRuntimePedestrianDetectorRecordV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_MODULE_STATUS,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_CONTROLLER_SNAPSHOT,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiRuntimeModuleStatusV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_SAFETY_SUMMARY,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_VENDOR_DIAGNOSTICS,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiRuntimeSafetySummaryV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_SAFETY_CHANNELS,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_VENDOR_DIAGNOSTICS,
    MMI_RESOURCE_COUNT_CHANNELS,
    (uint16_t) sizeof(MmiRuntimeSafetyChannelRecordV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_CLOCK,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_ENGINE_RUNTIME,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiRuntimeClockSummaryV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_POWER,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_CONTROLLER_SNAPSHOT,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiRuntimePowerSummaryV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_COMMS,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_LOCAL_SETTINGS,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiRuntimeCommsSummaryV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_RELAY,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_MAINTENANCE,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiRuntimeRelaySummaryV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_OUTPUT_TEST,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_MAINTENANCE,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiRuntimeOutputTestSummaryV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
    MMI_PROTOCOL_V2_RUNTIME_TOPIC_DOOR,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_CONTROLLER_SNAPSHOT,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiRuntimeDoorSummaryV2_t) }
};

static const MmiResourceDescriptor_t kStandardResources[] =
{
  { MMI_PROTOCOL_V2_NAMESPACE_STANDARD_OBJECT,
    MMI_PROTOCOL_V2_STANDARD_RESOURCE_NTCIP_OBJECT,
    0U, 0U, 1U,
    MMI_RESOURCE_SOURCE_NTCIP_OBJECT_DIRECTORY,
    MMI_RESOURCE_COUNT_VARIABLE,
    0U }
};

static const MmiResourceDescriptor_t kLocalResources[] =
{
  { MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS,
    MMI_PROTOCOL_V2_LOCAL_RESOURCE_MODEM,
    0U, 0U, 0U,
    MMI_RESOURCE_SOURCE_LOCAL_SETTINGS,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiLocalModemSettingsV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS,
    MMI_PROTOCOL_V2_LOCAL_RESOURCE_GPS,
    0U, 0U, 0U,
    MMI_RESOURCE_SOURCE_LOCAL_SETTINGS,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiLocalGpsSettingsV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS,
    MMI_PROTOCOL_V2_LOCAL_RESOURCE_USER_FLAGS,
    0U, 0U, 0U,
    MMI_RESOURCE_SOURCE_LOCAL_SETTINGS,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiLocalUserFlagsV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS,
    MMI_PROTOCOL_V2_LOCAL_RESOURCE_BROKEN_INPUT,
    0U, 0U, 0U,
    MMI_RESOURCE_SOURCE_LOCAL_SETTINGS,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiLocalBrokenInputSettingsV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS,
    MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN,
    1U, 0U, 0U,
    MMI_RESOURCE_SOURCE_LOCAL_SETTINGS,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiLocalAdminInfoV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS,
    MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN_PASSWORD_CHANGE,
    0U, 0U, 0U,
    MMI_RESOURCE_SOURCE_LOCAL_SETTINGS,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiLocalAdminPasswordChangeV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS,
    MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS,
    0U, 0U, 0U,
    MMI_RESOURCE_SOURCE_LOCAL_SETTINGS,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiLocalClockSettingsV2_t) }
};

static const MmiResourceDescriptor_t kVendorResources[] =
{
  { MMI_PROTOCOL_V2_NAMESPACE_VENDOR_PRIVATE,
    MMI_PROTOCOL_V2_VENDOR_RESOURCE_CPMP_LINK,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_VENDOR_DIAGNOSTICS,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiRuntimeSafetySummaryV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_VENDOR_PRIVATE,
    MMI_PROTOCOL_V2_VENDOR_RESOURCE_CPMP_FAULT_SUMMARY,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_VENDOR_DIAGNOSTICS,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiRuntimeSafetySummaryV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_VENDOR_PRIVATE,
    MMI_PROTOCOL_V2_VENDOR_RESOURCE_CPMP_FAULT_CHANNELS,
    1U, 1U, 0U,
    MMI_RESOURCE_SOURCE_VENDOR_DIAGNOSTICS,
    MMI_RESOURCE_COUNT_CHANNELS,
    (uint16_t) sizeof(MmiRuntimeSafetyChannelRecordV2_t) }
};

static const MmiResourceDescriptor_t kEventResources[] =
{
  { MMI_PROTOCOL_V2_NAMESPACE_EVENT_LOG,
    MMI_PROTOCOL_V2_EVENT_RESOURCE_PAGE,
    1U, 0U, 0U,
    MMI_RESOURCE_SOURCE_EVENT_LOG,
    MMI_RESOURCE_COUNT_VARIABLE,
    0U },
  { MMI_PROTOCOL_V2_NAMESPACE_EVENT_LOG,
    MMI_PROTOCOL_V2_EVENT_RESOURCE_CURSOR,
    1U, 0U, 0U,
    MMI_RESOURCE_SOURCE_EVENT_LOG,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(uint16_t) }
};

static const MmiResourceDescriptor_t kMaintenanceResources[] =
{
  { MMI_PROTOCOL_V2_NAMESPACE_MAINTENANCE,
    MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_MODE_CONTROL,
    0U, 0U, 0U,
    MMI_RESOURCE_SOURCE_MAINTENANCE,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiMaintenanceModeCommandV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_MAINTENANCE,
    MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_TIME_SET,
    0U, 0U, 0U,
    MMI_RESOURCE_SOURCE_MAINTENANCE,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiMaintenanceTimeSetCommandV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_MAINTENANCE,
    MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_DETECTOR_RESET,
    0U, 0U, 0U,
    MMI_RESOURCE_SOURCE_MAINTENANCE,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiMaintenanceDetectorResetCommandV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_MAINTENANCE,
    MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_OUTPUT_TEST,
    0U, 0U, 0U,
    MMI_RESOURCE_SOURCE_MAINTENANCE,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiMaintenanceOutputTestCommandV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_MAINTENANCE,
    MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_FACTORY_RESET,
    0U, 0U, 0U,
    MMI_RESOURCE_SOURCE_MAINTENANCE,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiMaintenanceFactoryResetCommandV2_t) },
  { MMI_PROTOCOL_V2_NAMESPACE_MAINTENANCE,
    MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_RELAY_COMMAND,
    0U, 0U, 0U,
    MMI_RESOURCE_SOURCE_MAINTENANCE,
    MMI_RESOURCE_COUNT_SINGLE,
    (uint16_t) sizeof(MmiMaintenanceRelayCommandV2_t) }
};

static uint8_t LookupInTable(const MmiResourceDescriptor_t *table,
                             uint16_t tableCount,
                             uint8_t namespaceId,
                             uint8_t resourceId,
                             MmiResourceDescriptor_t *descriptor)
{
  uint16_t index;

  if ((table == NULL) || (descriptor == NULL))
  {
    return 0U;
  }

  for (index = 0U; index < tableCount; index++)
  {
    if ((table[index].namespaceId == namespaceId)
        && (table[index].resourceId == resourceId))
    {
      *descriptor = table[index];
      return 1U;
    }
  }

  return 0U;
}

void MmiServiceInit(MmiService_t *service)
{
  if (service != NULL)
  {
    service->configurationService = NULL;
    service->intersectionEngine = NULL;
    service->intersectionController = NULL;
    service->detectorReportService = NULL;
    service->globalTimeManagementService = NULL;
    service->cpMpLinkService = NULL;
    service->uiPowerService = NULL;
    service->uiCommsIdentityService = NULL;
    service->uiDoorService = NULL;
    service->relayControlService = NULL;
    service->outputTestService = NULL;
  }
}

void MmiServiceBind(MmiService_t *service,
                    ConfigurationService_t *configurationService,
                    IntersectionEngine_t *intersectionEngine,
                    IntersectionController_t *intersectionController,
                    DetectorReportService_t *detectorReportService,
                    GlobalTimeManagementService_t *globalTimeManagementService,
                    CpMpLinkService_t *cpMpLinkService)
{
  if (service != NULL)
  {
    service->configurationService = configurationService;
    service->intersectionEngine = intersectionEngine;
    service->intersectionController = intersectionController;
    service->detectorReportService = detectorReportService;
    service->globalTimeManagementService = globalTimeManagementService;
    service->cpMpLinkService = cpMpLinkService;
  }
}

void MmiServiceBindUiPowerService(MmiService_t *service,
                                  UiPowerService_t *uiPowerService)
{
  if (service != NULL)
  {
    service->uiPowerService = uiPowerService;
  }
}

void MmiServiceBindUiCommsIdentityService(
  MmiService_t *service,
  UiCommsIdentityService_t *uiCommsIdentityService)
{
  if (service != NULL)
  {
    service->uiCommsIdentityService = uiCommsIdentityService;
  }
}

void MmiServiceBindUiDoorService(MmiService_t *service,
                                 UiDoorService_t *uiDoorService)
{
  if (service != NULL)
  {
    service->uiDoorService = uiDoorService;
  }
}

void MmiServiceBindRelayControlService(MmiService_t *service,
                                       RelayControlService_t *relayControlService)
{
  if (service != NULL)
  {
    service->relayControlService = relayControlService;
  }
}

void MmiServiceBindOutputTestService(MmiService_t *service,
                                     OutputTestService_t *outputTestService)
{
  if (service != NULL)
  {
    service->outputTestService = outputTestService;
  }
}

uint8_t MmiServiceLookupResource(const MmiService_t *service,
                                 uint8_t namespaceId,
                                 uint8_t resourceId,
                                 MmiResourceDescriptor_t *descriptor)
{
  (void) service;

  if (descriptor == NULL)
  {
    return 0U;
  }

  switch (namespaceId)
  {
    case MMI_PROTOCOL_V2_NAMESPACE_RUNTIME:
    {
      return LookupInTable(kRuntimeResources,
                           (uint16_t) (sizeof(kRuntimeResources)
                                       / sizeof(kRuntimeResources[0])),
                           namespaceId,
                           resourceId,
                           descriptor);
    }

    case MMI_PROTOCOL_V2_NAMESPACE_STANDARD_OBJECT:
    {
      return LookupInTable(kStandardResources,
                           (uint16_t) (sizeof(kStandardResources)
                                       / sizeof(kStandardResources[0])),
                           namespaceId,
                           resourceId,
                           descriptor);
    }

    case MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS:
    {
      return LookupInTable(kLocalResources,
                           (uint16_t) (sizeof(kLocalResources)
                                       / sizeof(kLocalResources[0])),
                           namespaceId,
                           resourceId,
                           descriptor);
    }

    case MMI_PROTOCOL_V2_NAMESPACE_VENDOR_PRIVATE:
    {
      return LookupInTable(kVendorResources,
                           (uint16_t) (sizeof(kVendorResources)
                                       / sizeof(kVendorResources[0])),
                           namespaceId,
                           resourceId,
                           descriptor);
    }

    case MMI_PROTOCOL_V2_NAMESPACE_EVENT_LOG:
    {
      return LookupInTable(kEventResources,
                           (uint16_t) (sizeof(kEventResources)
                                       / sizeof(kEventResources[0])),
                           namespaceId,
                           resourceId,
                           descriptor);
    }

    case MMI_PROTOCOL_V2_NAMESPACE_MAINTENANCE:
    {
      return LookupInTable(kMaintenanceResources,
                           (uint16_t) (sizeof(kMaintenanceResources)
                                       / sizeof(kMaintenanceResources[0])),
                           namespaceId,
                           resourceId,
                           descriptor);
    }

    default:
    {
      return 0U;
    }
  }
}

uint8_t MmiServiceResolveRecordCount(const MmiService_t *service,
                                     const MmiResourceDescriptor_t *descriptor,
                                     uint16_t *count)
{
  uint8_t value8;

  if ((service == NULL) || (descriptor == NULL) || (count == NULL))
  {
    return 0U;
  }

  switch (descriptor->countSource)
  {
    case MMI_RESOURCE_COUNT_SINGLE:
    {
      *count = 1U;
      return 1U;
    }

    case MMI_RESOURCE_COUNT_RINGS:
    {
      if (service->configurationService == NULL)
      {
        return 0U;
      }

      value8 = ConfigurationServiceGetRingCount(service->configurationService);
      *count = value8;
      return 1U;
    }

    case MMI_RESOURCE_COUNT_PHASES:
    {
      if (service->configurationService == NULL)
      {
        return 0U;
      }

      value8 = ConfigurationServiceGetPhaseCount(service->configurationService);
      *count = value8;
      return 1U;
    }

    case MMI_RESOURCE_COUNT_CHANNELS:
    {
      if (service->configurationService == NULL)
      {
        return 0U;
      }

      value8 = ConfigurationServiceGetChannelCount(service->configurationService);
      *count = value8;
      return 1U;
    }

    case MMI_RESOURCE_COUNT_OVERLAPS:
    {
      if (service->configurationService == NULL)
      {
        return 0U;
      }

      value8 = ConfigurationServiceGetOverlapCount(service->configurationService);
      *count = value8;
      return 1U;
    }

    case MMI_RESOURCE_COUNT_VEHICLE_DETECTORS:
    {
      if (service->configurationService == NULL)
      {
        return 0U;
      }

      value8 =
        ConfigurationServiceGetVehicleDetectorCount(service->configurationService);
      *count = value8;
      return 1U;
    }

    case MMI_RESOURCE_COUNT_PEDESTRIAN_DETECTORS:
    {
      if (service->configurationService == NULL)
      {
        return 0U;
      }

      value8 = ConfigurationServiceGetPedestrianDetectorCount(
        service->configurationService);
      *count = value8;
      return 1U;
    }

    case MMI_RESOURCE_COUNT_VARIABLE:
    default:
    {
      return 0U;
    }
  }
}
