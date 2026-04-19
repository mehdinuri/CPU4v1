/* App/Domain/Services/MmiSnapshotCache.h
 *
 * Stable read-model cache for MMI v2 runtime panels. The control task refreshes
 * this cache from canonical domain services so UI transports can answer UI
 * requests without touching live engine state directly.
 * without pulling live controller state directly.
 */
#ifndef MMI_SNAPSHOT_CACHE_H
#define MMI_SNAPSHOT_CACHE_H

#include <stdint.h>

#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/Intersection/CpMpLinkService.h"
#include "Domain/Intersection/DetectorReportService.h"
#include "Domain/Intersection/GlobalTimeManagementService.h"
#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/Services/MmiProtocol.h"
#include "Domain/Services/OutputTestService.h"
#include "Domain/Services/RelayControlService.h"
#include "Domain/Services/UiCommsIdentityService.h"
#include "Domain/Services/UiDoorService.h"
#include "Domain/Services/UiPowerService.h"

typedef struct
{
  ConfigurationService_t *configurationService;
  IntersectionEngine_t *intersectionEngine;
  IntersectionController_t *intersectionController;
  DetectorReportService_t *detectorReportService;
  GlobalTimeManagementService_t *globalTimeManagementService;
  CpMpLinkService_t *cpMpLinkService;
  UiPowerService_t *uiPowerService;
  UiCommsIdentityService_t *uiCommsIdentityService;
  UiDoorService_t *uiDoorService;
  RelayControlService_t *relayControlService;
  OutputTestService_t *outputTestService;
  MmiRuntimeSummaryV2_t summary;
  MmiRuntimeRingRecordV2_t rings[INTERSECTION_RING_COUNT_MAX];
  MmiRuntimePhaseRecordV2_t phases[INTERSECTION_PHASE_COUNT_MAX];
  MmiRuntimeChannelRecordV2_t channels[INTERSECTION_CHANNEL_COUNT_MAX];
  MmiRuntimeOverlapRecordV2_t overlaps[INTERSECTION_OVERLAP_COUNT_MAX];
  MmiRuntimeRawInputsV2_t rawInputs;
  MmiRuntimeVehicleDetectorRecordV2_t vehicleDetectors[
    INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX];
  MmiRuntimePedestrianDetectorRecordV2_t pedestrianDetectors[
    INTERSECTION_PED_INPUT_COUNT_MAX];
  MmiRuntimeModuleStatusV2_t moduleStatus;
  MmiRuntimeSafetySummaryV2_t safetySummary;
  MmiRuntimeSafetyChannelRecordV2_t safetyChannels[
    INTERSECTION_CHANNEL_COUNT_MAX];
  MmiRuntimeClockSummaryV2_t clockSummary;
  MmiRuntimePowerSummaryV2_t powerSummary;
  MmiRuntimeCommsSummaryV2_t commsSummary;
  MmiRuntimeRelaySummaryV2_t relaySummary;
  MmiRuntimeOutputTestSummaryV2_t outputTestSummary;
  MmiRuntimeDoorSummaryV2_t doorSummary;
  uint8_t refreshValid;
} MmiSnapshotCache_t;

void MmiSnapshotCacheInit(MmiSnapshotCache_t *cache);
void MmiSnapshotCacheBind(MmiSnapshotCache_t *cache,
                          ConfigurationService_t *configurationService,
                          IntersectionEngine_t *intersectionEngine,
                          IntersectionController_t *intersectionController,
                          DetectorReportService_t *detectorReportService,
                          GlobalTimeManagementService_t *globalTimeManagementService,
                          CpMpLinkService_t *cpMpLinkService);
void MmiSnapshotCacheBindUiPowerService(MmiSnapshotCache_t *cache,
                                        UiPowerService_t *uiPowerService);
void MmiSnapshotCacheBindUiCommsIdentityService(
  MmiSnapshotCache_t *cache,
  UiCommsIdentityService_t *uiCommsIdentityService);
void MmiSnapshotCacheBindUiDoorService(MmiSnapshotCache_t *cache,
                                       UiDoorService_t *uiDoorService);
void MmiSnapshotCacheBindRelayControlService(
  MmiSnapshotCache_t *cache,
  RelayControlService_t *relayControlService);
void MmiSnapshotCacheBindOutputTestService(MmiSnapshotCache_t *cache,
                                           OutputTestService_t *outputTestService);
uint8_t MmiSnapshotCacheRefresh(MmiSnapshotCache_t *cache);
uint8_t MmiSnapshotCacheGetSummary(const MmiSnapshotCache_t *cache,
                                   MmiRuntimeSummaryV2_t *summary);
uint8_t MmiSnapshotCacheGetRingRecord(const MmiSnapshotCache_t *cache,
                                      uint8_t ringNumber,
                                      MmiRuntimeRingRecordV2_t *record);
uint8_t MmiSnapshotCacheGetPhaseRecord(const MmiSnapshotCache_t *cache,
                                       uint8_t phaseNumber,
                                       MmiRuntimePhaseRecordV2_t *record);
uint8_t MmiSnapshotCacheGetChannelRecord(const MmiSnapshotCache_t *cache,
                                         uint8_t channelNumber,
                                         MmiRuntimeChannelRecordV2_t *record);
uint8_t MmiSnapshotCacheGetOverlapRecord(const MmiSnapshotCache_t *cache,
                                         uint8_t overlapNumber,
                                         MmiRuntimeOverlapRecordV2_t *record);
uint8_t MmiSnapshotCacheGetRawInputs(const MmiSnapshotCache_t *cache,
                                     MmiRuntimeRawInputsV2_t *rawInputs);
uint8_t MmiSnapshotCacheGetVehicleDetectorRecord(
  const MmiSnapshotCache_t *cache,
  uint8_t detectorNumber,
  MmiRuntimeVehicleDetectorRecordV2_t *record);
uint8_t MmiSnapshotCacheGetPedestrianDetectorRecord(
  const MmiSnapshotCache_t *cache,
  uint8_t detectorNumber,
  MmiRuntimePedestrianDetectorRecordV2_t *record);
uint8_t MmiSnapshotCacheGetModuleStatus(const MmiSnapshotCache_t *cache,
                                        MmiRuntimeModuleStatusV2_t *moduleStatus);
uint8_t MmiSnapshotCacheGetSafetySummary(const MmiSnapshotCache_t *cache,
                                         MmiRuntimeSafetySummaryV2_t *summary);
uint8_t MmiSnapshotCacheGetSafetyChannelRecord(
  const MmiSnapshotCache_t *cache,
  uint8_t channelNumber,
  MmiRuntimeSafetyChannelRecordV2_t *record);
uint8_t MmiSnapshotCacheGetClockSummary(const MmiSnapshotCache_t *cache,
                                        MmiRuntimeClockSummaryV2_t *summary);
uint8_t MmiSnapshotCacheGetPowerSummary(const MmiSnapshotCache_t *cache,
                                        MmiRuntimePowerSummaryV2_t *summary);
uint8_t MmiSnapshotCacheGetCommsSummary(const MmiSnapshotCache_t *cache,
                                        MmiRuntimeCommsSummaryV2_t *summary);
uint8_t MmiSnapshotCacheGetRelaySummary(const MmiSnapshotCache_t *cache,
                                        MmiRuntimeRelaySummaryV2_t *summary);
uint8_t MmiSnapshotCacheGetOutputTestSummary(
  const MmiSnapshotCache_t *cache,
  MmiRuntimeOutputTestSummaryV2_t *summary);
uint8_t MmiSnapshotCacheGetDoorSummary(const MmiSnapshotCache_t *cache,
                                       MmiRuntimeDoorSummaryV2_t *summary);

#endif /* MMI_SNAPSHOT_CACHE_H */
