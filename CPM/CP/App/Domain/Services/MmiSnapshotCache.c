/* App/Domain/Services/MmiSnapshotCache.c */
#include "MmiSnapshotCache.h"

#include <string.h>

static uint8_t SaturateTicksToDeciseconds(uint32_t ticks)
{
  uint32_t deciseconds = ticks / 10U;

  if (deciseconds > 255U)
  {
    return 255U;
  }

  return (uint8_t) deciseconds;
}

static uint8_t ReadRuntime(const MmiSnapshotCache_t *cache,
                           const IntersectionRuntime_t **runtime)
{
  if ((cache == NULL) || (runtime == NULL) || (cache->intersectionEngine == NULL))
  {
    return 0U;
  }

  *runtime = IntersectionEngineGetRuntime(cache->intersectionEngine);
  return (uint8_t) (*runtime != NULL);
}

static uint8_t ReadSnapshot(const MmiSnapshotCache_t *cache,
                            ModuleBusSnapshot_t *snapshot)
{
  if ((cache == NULL) || (snapshot == NULL)
      || (cache->intersectionController == NULL))
  {
    return 0U;
  }

  return IntersectionControllerGetLastSnapshot(cache->intersectionController,
                                               snapshot);
}

static uint8_t GetVehicleCallPhase(const MmiSnapshotCache_t *cache,
                                   uint8_t detectorNumber,
                                   uint8_t *phaseNumber)
{
  if ((cache == NULL) || (phaseNumber == NULL)
      || (cache->configurationService == NULL))
  {
    return 0U;
  }

  return ConfigurationServiceGetVehicleDetectorCallPhase(
    cache->configurationService,
    detectorNumber,
    phaseNumber);
}

static uint8_t GetPedestrianCallPhase(const MmiSnapshotCache_t *cache,
                                      uint8_t detectorNumber,
                                      uint8_t *phaseNumber)
{
  if ((cache == NULL) || (phaseNumber == NULL)
      || (cache->configurationService == NULL))
  {
    return 0U;
  }

  return ConfigurationServiceGetPedestrianDetectorCallPhase(
    cache->configurationService,
    detectorNumber,
    phaseNumber);
}

static uint8_t ReadRequestedAppliedImages(const MmiSnapshotCache_t *cache,
                                          OutputDriverImage_t *requested,
                                          OutputDriverImage_t *applied)
{
  if ((cache == NULL) || (requested == NULL) || (applied == NULL)
      || (cache->intersectionController == NULL)
      || (cache->intersectionController->outputDispatcher == NULL))
  {
    return 0U;
  }

  if (IntersectionOutputDispatcherGetLastRequestedImage(
        cache->intersectionController->outputDispatcher,
        requested) == 0U)
  {
    return 0U;
  }

  if (IntersectionOutputDispatcherGetLastAppliedImage(
        cache->intersectionController->outputDispatcher,
        applied) == 0U)
  {
    return 0U;
  }

  return 1U;
}

static uint8_t ReadVehicleDetectorSample(const MmiSnapshotCache_t *cache,
                                         uint8_t detectorNumber,
                                         DetectorReportVehicleSample_t *sample)
{
  uint8_t detectorIndex;

  if ((cache == NULL) || (sample == NULL) || (cache->detectorReportService == NULL)
      || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  if (DetectorReportServiceGetVehicleSample(cache->detectorReportService,
                                            detectorNumber,
                                            sample) != 0U)
  {
    return 1U;
  }

  detectorIndex = (uint8_t) (detectorNumber - 1U);
  *sample = cache->detectorReportService->vehicleSamples[detectorIndex];
  return 1U;
}

static uint8_t ReadPedestrianDetectorSample(const MmiSnapshotCache_t *cache,
                                            uint8_t detectorNumber,
                                            DetectorReportPedestrianSample_t *sample)
{
  uint8_t detectorIndex;

  if ((cache == NULL) || (sample == NULL) || (cache->detectorReportService == NULL)
      || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  if (DetectorReportServiceGetPedestrianSample(cache->detectorReportService,
                                               detectorNumber,
                                               sample) != 0U)
  {
    return 1U;
  }

  detectorIndex = (uint8_t) (detectorNumber - 1U);
  *sample = cache->detectorReportService->pedestrianSamples[detectorIndex];
  return 1U;
}

static void RefreshSummary(MmiSnapshotCache_t *cache,
                           const IntersectionRuntime_t *runtime)
{
  MmiRuntimeSummaryV2_t *summary = &cache->summary;

  memset(summary, 0, sizeof(*summary));
  summary->mode = runtime->mode;
  summary->localFreeStatus = runtime->localFreeStatus;
  summary->unitControlStatus = runtime->unitControlStatus;
  summary->coordPatternStatus = runtime->coordPatternStatus;
  summary->actionPlanControl = runtime->actionPlanControl;
  summary->timebaseActionStatus = runtime->timebaseActionStatus;
  summary->preemptStatus = runtime->preemptStatus;
  summary->mmuFlashActive = runtime->mmuFlashActive;
  summary->startUpFlashActive = runtime->startUpFlashActive;
  summary->dimmingActive = runtime->dimmingActive;
  summary->coordCycleStatusSeconds = runtime->coordCycleStatusSeconds;
  summary->coordSyncStatusSeconds = runtime->coordSyncStatusSeconds;
  summary->monotonicTicks = runtime->monotonicTicks;

  if (cache->intersectionEngine != NULL)
  {
    summary->activeSequenceNumber = cache->intersectionEngine->activeSequenceNumber;
    summary->configLoaded = cache->intersectionEngine->configLoaded;
  }

  if (cache->cpMpLinkService != NULL)
  {
    summary->safetyAction =
      (uint8_t) CpMpLinkServiceGetEffectiveSafetyAction(cache->cpMpLinkService);
    summary->safetyReasonCode =
      CpMpLinkServiceGetLastSafetyReasonCode(cache->cpMpLinkService);
  }
}

static void RefreshRings(MmiSnapshotCache_t *cache,
                         const IntersectionRuntime_t *runtime)
{
  uint8_t ringIndex;

  for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
  {
    MmiRuntimeRingRecordV2_t *record = &cache->rings[ringIndex];
    const IntersectionRingRuntime_t *source = &runtime->rings[ringIndex];

    memset(record, 0, sizeof(*record));
    record->ringNumber = (uint8_t) (ringIndex + 1U);
    record->activePhaseNumber = (uint8_t) (source->activePhaseIndex + 1U);
    record->stage = (uint8_t) source->stage;
    record->statusCode = (uint8_t) source->statusCode;
    record->terminationReasonBits = source->terminationReasonBits;
    record->barrierWaiting = source->barrierWaiting;
    record->activePosition = source->activePosition;
    record->pendingPosition = source->pendingPosition;
    record->stageElapsedTicks = source->stageElapsedTicks;
  }
}

static void RefreshPhases(MmiSnapshotCache_t *cache,
                          const IntersectionRuntime_t *runtime)
{
  uint8_t phaseIndex;

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    MmiRuntimePhaseRecordV2_t *record = &cache->phases[phaseIndex];
    const IntersectionPhaseRuntime_t *source = &runtime->phases[phaseIndex];

    memset(record, 0, sizeof(*record));
    record->phaseNumber = (uint8_t) (phaseIndex + 1U);
    record->interval = (uint8_t) source->interval;
    record->pedInterval = (uint8_t) source->pedInterval;
    record->detectorActive = source->detectorActive;
    record->callLatched = source->callLatched;
    record->pedInputActive = source->pedInputActive;
    record->pedCallLatched = source->pedCallLatched;
    record->nextPhase = (uint8_t) (source->next + 1U);
    record->intervalElapsedTicks = source->intervalElapsedTicks;
    record->pedIntervalElapsedTicks = source->pedIntervalElapsedTicks;
  }
}

static void RefreshChannels(MmiSnapshotCache_t *cache,
                            const IntersectionRuntime_t *runtime)
{
  OutputDriverImage_t requested;
  OutputDriverImage_t applied;
  uint8_t haveAppliedImages = ReadRequestedAppliedImages(cache,
                                                         &requested,
                                                         &applied);
  uint8_t channelIndex;

  memset(&requested, 0, sizeof(requested));
  memset(&applied, 0, sizeof(applied));
  haveAppliedImages = ReadRequestedAppliedImages(cache, &requested, &applied);

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    MmiRuntimeChannelRecordV2_t *record = &cache->channels[channelIndex];
    const IntersectionChannelRuntime_t *source = &runtime->channels[channelIndex];

    memset(record, 0, sizeof(*record));
    record->channelNumber = (uint8_t) (channelIndex + 1U);
    record->requestedAspect = (uint8_t) source->aspect;
    record->appliedAspect = (uint8_t) source->aspect;
    record->dimmed = source->dimmed;
    record->dimAlternateHalfCycle = source->dimAlternateHalfCycle;

    if (haveAppliedImages != 0U)
    {
      record->requestedAspect = (uint8_t) requested.channels[channelIndex];
      record->appliedAspect = (uint8_t) applied.channels[channelIndex];
      record->dimmed = requested.channelDimmed[channelIndex];
      record->dimAlternateHalfCycle =
        requested.channelDimAlternateHalfCycle[channelIndex];
    }
  }
}

static void RefreshOverlaps(MmiSnapshotCache_t *cache,
                            const IntersectionRuntime_t *runtime)
{
  uint8_t overlapIndex;

  for (overlapIndex = 0U; overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX;
       overlapIndex++)
  {
    MmiRuntimeOverlapRecordV2_t *record = &cache->overlaps[overlapIndex];

    memset(record, 0, sizeof(*record));
    record->overlapNumber = (uint8_t) (overlapIndex + 1U);
    record->aspect = (uint8_t) runtime->overlaps[overlapIndex].aspect;
  }
}

static void RefreshRawInputs(MmiSnapshotCache_t *cache,
                             const ModuleBusSnapshot_t *snapshot)
{
  memset(&cache->rawInputs, 0, sizeof(cache->rawInputs));

  if (snapshot != NULL)
  {
    cache->rawInputs.rawVehicleMask = snapshot->rawVehicleDetectorInputs;
    cache->rawInputs.rawPedestrianMask = snapshot->rawPedestrianInputs;
    cache->rawInputs.preemptInputs = snapshot->preemptInputs;
    cache->rawInputs.preemptControls = snapshot->preemptControls;
    cache->rawInputs.validMask = snapshot->validMask;
    cache->rawInputs.healthMask = snapshot->healthMask;
    cache->rawInputs.staleMask = snapshot->staleMask;
    cache->rawInputs.contextFaultMask = snapshot->contextFaultMask;
    cache->rawInputs.sequenceFaultMask = snapshot->sequenceFaultMask;
    cache->rawInputs.sequence = snapshot->sequence;
    cache->rawInputs.configEpoch = snapshot->configEpoch;
    cache->rawInputs.loadSwitchReds = snapshot->loadSwitchReds;
    cache->rawInputs.loadSwitchYellows = snapshot->loadSwitchYellows;
    cache->rawInputs.loadSwitchGreens = snapshot->loadSwitchGreens;
  }
}

static void RefreshDetectorRecords(MmiSnapshotCache_t *cache,
                                   const IntersectionRuntime_t *runtime,
                                   const ModuleBusSnapshot_t *snapshot)
{
  uint8_t detectorIndex;

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    MmiRuntimeVehicleDetectorRecordV2_t *record =
      &cache->vehicleDetectors[detectorIndex];
    const IntersectionVehicleDetectorRuntime_t *source =
      &runtime->vehicleDetectors[detectorIndex];
    DetectorReportVehicleSample_t sample;
    uint8_t phaseNumber = 0U;

    memset(record, 0, sizeof(*record));
    record->detectorNumber = (uint8_t) (detectorIndex + 1U);
    record->inputActive = source->inputActive;
    record->remoteActuation = source->remoteActuation;
    record->recognitionActive = source->recognitionActive;
    record->delayTimerDeciseconds =
      SaturateTicksToDeciseconds(source->delayTimerTicks);
    record->extendTimerDeciseconds =
      SaturateTicksToDeciseconds(source->extendTimerTicks);

    if (GetVehicleCallPhase(cache, (uint8_t) (detectorIndex + 1U),
                            &phaseNumber) != 0U)
    {
      record->callPhase = phaseNumber;
    }

    if (ReadVehicleDetectorSample(cache,
                                  (uint8_t) (detectorIndex + 1U),
                                  &sample) != 0U)
    {
      record->volume = sample.volume;
      record->occupancy = sample.occupancy;
      record->averageSpeed = sample.averageSpeed;
    }

    if (snapshot != NULL)
    {
      record->alarm = snapshot->vehicleDetectorAlarms[detectorIndex];
      record->reportedAlarm =
        snapshot->vehicleDetectorReportedAlarms[detectorIndex];
    }
  }

  for (detectorIndex = 0U; detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       detectorIndex++)
  {
    MmiRuntimePedestrianDetectorRecordV2_t *record =
      &cache->pedestrianDetectors[detectorIndex];
    const IntersectionPedestrianDetectorRuntime_t *source =
      &runtime->pedestrianDetectors[detectorIndex];
    DetectorReportPedestrianSample_t sample;
    uint8_t phaseNumber = 0U;

    memset(record, 0, sizeof(*record));
    record->detectorNumber = (uint8_t) (detectorIndex + 1U);
    record->inputActive = source->inputActive;
    record->remoteActuation = source->remoteActuation;
    record->alternateTimingRequest = source->alternateTimingRequest;

    if (GetPedestrianCallPhase(cache, (uint8_t) (detectorIndex + 1U),
                               &phaseNumber) != 0U)
    {
      record->callPhase = phaseNumber;
    }

    if (ReadPedestrianDetectorSample(cache,
                                     (uint8_t) (detectorIndex + 1U),
                                     &sample) != 0U)
    {
      record->volume = sample.volume;
      record->actuations = sample.actuations;
      record->services = sample.services;
    }

    if (snapshot != NULL)
    {
      record->alarm = snapshot->pedestrianDetectorAlarms[detectorIndex];
    }
  }
}

static void RefreshModuleStatus(MmiSnapshotCache_t *cache,
                                const ModuleBusSnapshot_t *snapshot)
{
  memset(&cache->moduleStatus, 0, sizeof(cache->moduleStatus));

  if (snapshot != NULL)
  {
    cache->moduleStatus.validMask = snapshot->validMask;
    cache->moduleStatus.healthMask = snapshot->healthMask;
    cache->moduleStatus.staleMask = snapshot->staleMask;
    cache->moduleStatus.contextFaultMask = snapshot->contextFaultMask;
    cache->moduleStatus.sequenceFaultMask = snapshot->sequenceFaultMask;
    cache->moduleStatus.sequence = snapshot->sequence;
    cache->moduleStatus.configEpoch = snapshot->configEpoch;
    cache->moduleStatus.loadSwitchReds = snapshot->loadSwitchReds;
    cache->moduleStatus.loadSwitchYellows = snapshot->loadSwitchYellows;
    cache->moduleStatus.loadSwitchGreens = snapshot->loadSwitchGreens;
  }
}

static void RefreshSafety(MmiSnapshotCache_t *cache,
                          const IntersectionRuntime_t *runtime)
{
  CpMpFaultStatusImageV1_t faultStatus;
  uint8_t haveFaultStatus = 0U;
  uint8_t channelIndex;

  memset(&cache->safetySummary, 0, sizeof(cache->safetySummary));
  memset(&cache->safetyChannels, 0, sizeof(cache->safetyChannels));

  if (cache->cpMpLinkService != NULL)
  {
    cache->safetySummary.peerHealthy =
      CpMpLinkServicePeerHealthy(cache->cpMpLinkService);
    cache->safetySummary.authorityReady =
      CpMpLinkServiceAuthorityReady(cache->cpMpLinkService);
    cache->safetySummary.safetyAction =
      (uint8_t) CpMpLinkServiceGetEffectiveSafetyAction(cache->cpMpLinkService);
    cache->safetySummary.safetyReasonCode =
      CpMpLinkServiceGetLastSafetyReasonCode(cache->cpMpLinkService);
    haveFaultStatus = CpMpLinkServiceGetFaultStatus(cache->cpMpLinkService,
                                                    &faultStatus);
  }

  cache->safetySummary.mmuFlashActive = runtime->mmuFlashActive;
  cache->safetySummary.startUpFlashActive = runtime->startUpFlashActive;

  if (haveFaultStatus != 0U)
  {
    cache->safetySummary.configState = faultStatus.configState;
    cache->safetySummary.faultSequence = faultStatus.sequence;
    cache->safetySummary.globalFaultFlags = faultStatus.globalFlags;

    for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
         channelIndex++)
    {
      cache->safetyChannels[channelIndex].channelNumber =
        (uint8_t) (channelIndex + 1U);
      cache->safetyChannels[channelIndex].faultFlags =
        faultStatus.channelFlags[channelIndex];
    }
  }
  else
  {
    for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
         channelIndex++)
    {
      cache->safetyChannels[channelIndex].channelNumber =
        (uint8_t) (channelIndex + 1U);
    }
  }
}

static void RefreshClock(MmiSnapshotCache_t *cache)
{
  uint32_t value32;
  int32_t diff32;
  uint8_t value8;

  memset(&cache->clockSummary, 0, sizeof(cache->clockSummary));

  if (cache->globalTimeManagementService == NULL)
  {
    return;
  }

  if (GlobalTimeManagementServiceGetGlobalTime(
        cache->globalTimeManagementService,
        &value32) != 0U)
  {
    cache->clockSummary.globalTimeSeconds = value32;
  }

  if (GlobalTimeManagementServiceGetControllerLocalTime(
        cache->globalTimeManagementService,
        &value32) != 0U)
  {
    cache->clockSummary.localTimeSeconds = value32;
  }

  if (GlobalTimeManagementServiceGetGlobalLocalTimeDifferential(
        cache->globalTimeManagementService,
        &diff32) != 0U)
  {
    cache->clockSummary.globalLocalDifferentialSeconds = diff32;
  }

  if (GlobalTimeManagementServiceGetScheduleStatus(
        cache->globalTimeManagementService,
        &value8) != 0U)
  {
    cache->clockSummary.scheduleStatus = value8;
  }

  if (GlobalTimeManagementServiceGetDayPlanStatus(
        cache->globalTimeManagementService,
        &value8) != 0U)
  {
    cache->clockSummary.dayPlanStatus = value8;
  }

  cache->clockSummary.lastAppliedActionNumber =
    cache->globalTimeManagementService->lastAppliedActionNumber;
}

void MmiSnapshotCacheInit(MmiSnapshotCache_t *cache)
{
  if (cache != NULL)
  {
    memset(cache, 0, sizeof(*cache));
  }
}

void MmiSnapshotCacheBind(MmiSnapshotCache_t *cache,
                          ConfigurationService_t *configurationService,
                          IntersectionEngine_t *intersectionEngine,
                          IntersectionController_t *intersectionController,
                          DetectorReportService_t *detectorReportService,
                          GlobalTimeManagementService_t *globalTimeManagementService,
                          CpMpLinkService_t *cpMpLinkService)
{
  if (cache != NULL)
  {
    cache->configurationService = configurationService;
    cache->intersectionEngine = intersectionEngine;
    cache->intersectionController = intersectionController;
    cache->detectorReportService = detectorReportService;
    cache->globalTimeManagementService = globalTimeManagementService;
    cache->cpMpLinkService = cpMpLinkService;
  }
}

uint8_t MmiSnapshotCacheRefresh(MmiSnapshotCache_t *cache)
{
  const IntersectionRuntime_t *runtime = NULL;
  ModuleBusSnapshot_t snapshot;
  ModuleBusSnapshot_t *snapshotPtr = NULL;

  if ((cache == NULL) || (ReadRuntime(cache, &runtime) == 0U))
  {
    return 0U;
  }

  if (ReadSnapshot(cache, &snapshot) != 0U)
  {
    snapshotPtr = &snapshot;
  }

  RefreshSummary(cache, runtime);
  RefreshRings(cache, runtime);
  RefreshPhases(cache, runtime);
  RefreshChannels(cache, runtime);
  RefreshOverlaps(cache, runtime);
  RefreshRawInputs(cache, snapshotPtr);
  RefreshDetectorRecords(cache, runtime, snapshotPtr);
  RefreshModuleStatus(cache, snapshotPtr);
  RefreshSafety(cache, runtime);
  RefreshClock(cache);
  cache->refreshValid = 1U;

  return 1U;
}

uint8_t MmiSnapshotCacheGetSummary(const MmiSnapshotCache_t *cache,
                                   MmiRuntimeSummaryV2_t *summary)
{
  if ((cache == NULL) || (summary == NULL) || (cache->refreshValid == 0U))
  {
    return 0U;
  }

  *summary = cache->summary;
  return 1U;
}

uint8_t MmiSnapshotCacheGetRingRecord(const MmiSnapshotCache_t *cache,
                                      uint8_t ringNumber,
                                      MmiRuntimeRingRecordV2_t *record)
{
  if ((cache == NULL) || (record == NULL) || (cache->refreshValid == 0U)
      || (ringNumber == 0U) || (ringNumber > INTERSECTION_RING_COUNT_MAX))
  {
    return 0U;
  }

  *record = cache->rings[ringNumber - 1U];
  return 1U;
}

uint8_t MmiSnapshotCacheGetPhaseRecord(const MmiSnapshotCache_t *cache,
                                       uint8_t phaseNumber,
                                       MmiRuntimePhaseRecordV2_t *record)
{
  if ((cache == NULL) || (record == NULL) || (cache->refreshValid == 0U)
      || (phaseNumber == 0U) || (phaseNumber > INTERSECTION_PHASE_COUNT_MAX))
  {
    return 0U;
  }

  *record = cache->phases[phaseNumber - 1U];
  return 1U;
}

uint8_t MmiSnapshotCacheGetChannelRecord(const MmiSnapshotCache_t *cache,
                                         uint8_t channelNumber,
                                         MmiRuntimeChannelRecordV2_t *record)
{
  if ((cache == NULL) || (record == NULL) || (cache->refreshValid == 0U)
      || (channelNumber == 0U)
      || (channelNumber > INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  *record = cache->channels[channelNumber - 1U];
  return 1U;
}

uint8_t MmiSnapshotCacheGetOverlapRecord(const MmiSnapshotCache_t *cache,
                                         uint8_t overlapNumber,
                                         MmiRuntimeOverlapRecordV2_t *record)
{
  if ((cache == NULL) || (record == NULL) || (cache->refreshValid == 0U)
      || (overlapNumber == 0U)
      || (overlapNumber > INTERSECTION_OVERLAP_COUNT_MAX))
  {
    return 0U;
  }

  *record = cache->overlaps[overlapNumber - 1U];
  return 1U;
}

uint8_t MmiSnapshotCacheGetRawInputs(const MmiSnapshotCache_t *cache,
                                     MmiRuntimeRawInputsV2_t *rawInputs)
{
  if ((cache == NULL) || (rawInputs == NULL) || (cache->refreshValid == 0U))
  {
    return 0U;
  }

  *rawInputs = cache->rawInputs;
  return 1U;
}

uint8_t MmiSnapshotCacheGetVehicleDetectorRecord(
  const MmiSnapshotCache_t *cache,
  uint8_t detectorNumber,
  MmiRuntimeVehicleDetectorRecordV2_t *record)
{
  if ((cache == NULL) || (record == NULL) || (cache->refreshValid == 0U)
      || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  *record = cache->vehicleDetectors[detectorNumber - 1U];
  return 1U;
}

uint8_t MmiSnapshotCacheGetPedestrianDetectorRecord(
  const MmiSnapshotCache_t *cache,
  uint8_t detectorNumber,
  MmiRuntimePedestrianDetectorRecordV2_t *record)
{
  if ((cache == NULL) || (record == NULL) || (cache->refreshValid == 0U)
      || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  *record = cache->pedestrianDetectors[detectorNumber - 1U];
  return 1U;
}

uint8_t MmiSnapshotCacheGetModuleStatus(const MmiSnapshotCache_t *cache,
                                        MmiRuntimeModuleStatusV2_t *moduleStatus)
{
  if ((cache == NULL) || (moduleStatus == NULL)
      || (cache->refreshValid == 0U))
  {
    return 0U;
  }

  *moduleStatus = cache->moduleStatus;
  return 1U;
}

uint8_t MmiSnapshotCacheGetSafetySummary(const MmiSnapshotCache_t *cache,
                                         MmiRuntimeSafetySummaryV2_t *summary)
{
  if ((cache == NULL) || (summary == NULL) || (cache->refreshValid == 0U))
  {
    return 0U;
  }

  *summary = cache->safetySummary;
  return 1U;
}

uint8_t MmiSnapshotCacheGetSafetyChannelRecord(
  const MmiSnapshotCache_t *cache,
  uint8_t channelNumber,
  MmiRuntimeSafetyChannelRecordV2_t *record)
{
  if ((cache == NULL) || (record == NULL) || (cache->refreshValid == 0U)
      || (channelNumber == 0U)
      || (channelNumber > INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  *record = cache->safetyChannels[channelNumber - 1U];
  return 1U;
}

uint8_t MmiSnapshotCacheGetClockSummary(const MmiSnapshotCache_t *cache,
                                        MmiRuntimeClockSummaryV2_t *summary)
{
  if ((cache == NULL) || (summary == NULL) || (cache->refreshValid == 0U))
  {
    return 0U;
  }

  *summary = cache->clockSummary;
  return 1U;
}
