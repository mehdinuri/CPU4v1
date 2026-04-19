/* App/Domain/Services/MmiService.h
 *
 * Route catalog and future execution boundary for MMI protocol v2.
 * This service deliberately separates:
 * - runtime snapshots from the new NEMA TS2 engine
 * - standards-backed NTCIP object access
 * - local non-NTCIP settings
 * - vendor diagnostics and maintenance commands
 */
#ifndef MMI_SERVICE_H
#define MMI_SERVICE_H

#include <stdint.h>

#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/Intersection/CpMpLinkService.h"
#include "Domain/Intersection/DetectorReportService.h"
#include "Domain/Intersection/GlobalTimeManagementService.h"
#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/Services/MmiProtocol.h"

typedef enum
{
  MMI_RESOURCE_SOURCE_NONE = 0U,
  MMI_RESOURCE_SOURCE_ENGINE_RUNTIME = 1U,
  MMI_RESOURCE_SOURCE_CONTROLLER_SNAPSHOT = 2U,
  MMI_RESOURCE_SOURCE_NTCIP_OBJECT_DIRECTORY = 3U,
  MMI_RESOURCE_SOURCE_LOCAL_SETTINGS = 4U,
  MMI_RESOURCE_SOURCE_VENDOR_DIAGNOSTICS = 5U,
  MMI_RESOURCE_SOURCE_EVENT_LOG = 6U,
  MMI_RESOURCE_SOURCE_MAINTENANCE = 7U
} MmiResourceSource_t;

typedef enum
{
  MMI_RESOURCE_COUNT_SINGLE = 0U,
  MMI_RESOURCE_COUNT_RINGS = 1U,
  MMI_RESOURCE_COUNT_PHASES = 2U,
  MMI_RESOURCE_COUNT_CHANNELS = 3U,
  MMI_RESOURCE_COUNT_OVERLAPS = 4U,
  MMI_RESOURCE_COUNT_VEHICLE_DETECTORS = 5U,
  MMI_RESOURCE_COUNT_PEDESTRIAN_DETECTORS = 6U,
  MMI_RESOURCE_COUNT_VARIABLE = 7U
} MmiResourceCountSource_t;

typedef struct
{
  uint8_t namespaceId;
  uint8_t resourceId;
  uint8_t readOnly;
  uint8_t supportsSubscription;
  uint8_t requiresTransaction;
  MmiResourceSource_t source;
  MmiResourceCountSource_t countSource;
  uint16_t recordSize;
} MmiResourceDescriptor_t;

typedef struct
{
  ConfigurationService_t *configurationService;
  IntersectionEngine_t *intersectionEngine;
  IntersectionController_t *intersectionController;
  DetectorReportService_t *detectorReportService;
  GlobalTimeManagementService_t *globalTimeManagementService;
  CpMpLinkService_t *cpMpLinkService;
} MmiService_t;

void MmiServiceInit(MmiService_t *service);
void MmiServiceBind(MmiService_t *service,
                    ConfigurationService_t *configurationService,
                    IntersectionEngine_t *intersectionEngine,
                    IntersectionController_t *intersectionController,
                    DetectorReportService_t *detectorReportService,
                    GlobalTimeManagementService_t *globalTimeManagementService,
                    CpMpLinkService_t *cpMpLinkService);
uint8_t MmiServiceLookupResource(const MmiService_t *service,
                                 uint8_t namespaceId,
                                 uint8_t resourceId,
                                 MmiResourceDescriptor_t *descriptor);
uint8_t MmiServiceResolveRecordCount(const MmiService_t *service,
                                     const MmiResourceDescriptor_t *descriptor,
                                     uint16_t *count);

#endif /* MMI_SERVICE_H */
