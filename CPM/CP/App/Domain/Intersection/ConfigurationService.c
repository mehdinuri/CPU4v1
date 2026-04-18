/* App/Domain/Intersection/ConfigurationService.c
 *
 * Transactional immutable-slot persistence and candidate overlay handling for
 * the new controller configuration model.
 */
#include "ConfigurationService.h"

#include <stddef.h>
#include <string.h>

typedef struct
{
  uint8_t valid;
  ConfigurationSlotId_t slotId;
  uint32_t generation;
  IntersectionConfig_t config;
} LoadedSlot_t;

#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V2 2UL
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V2 148U
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V3 3UL
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V3 \
        (4U + (INTERSECTION_PHASE_COUNT_MAX * 16U) \
         + (INTERSECTION_RING_COUNT_MAX * 8U) \
         + (INTERSECTION_CHANNEL_COUNT_MAX * 24U) \
         + (INTERSECTION_OVERLAP_COUNT_MAX * 38U))
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V4 4UL
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V4 \
        (4U + (INTERSECTION_PHASE_COUNT_MAX * 24U) \
         + (INTERSECTION_RING_COUNT_MAX * 8U) \
         + (INTERSECTION_CHANNEL_COUNT_MAX * 24U) \
         + (INTERSECTION_OVERLAP_COUNT_MAX * 38U))
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V5 5UL
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V5 \
        (4U + (INTERSECTION_PHASE_COUNT_MAX * 24U) \
         + (INTERSECTION_RING_COUNT_MAX * 8U) \
         + 8U \
         + (INTERSECTION_PATTERN_COUNT_MAX * 8U) \
         + (INTERSECTION_SPLIT_COUNT_MAX * INTERSECTION_PHASE_COUNT_MAX * 4U) \
         + (INTERSECTION_CHANNEL_COUNT_MAX * 24U) \
         + (INTERSECTION_OVERLAP_COUNT_MAX * 38U))
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V6 6UL
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V6 \
        (4U + (INTERSECTION_PHASE_COUNT_MAX * 24U) \
         + (INTERSECTION_RING_COUNT_MAX * 8U) \
         + 8U \
         + (INTERSECTION_PATTERN_COUNT_MAX * 8U) \
         + (INTERSECTION_SPLIT_COUNT_MAX * INTERSECTION_PHASE_COUNT_MAX * 4U) \
         + (INTERSECTION_PREEMPT_COUNT_MAX * 124U) \
         + (INTERSECTION_CHANNEL_COUNT_MAX * 24U) \
         + (INTERSECTION_OVERLAP_COUNT_MAX * 38U))
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V7 7UL
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V7 \
        (4U + (INTERSECTION_PHASE_COUNT_MAX * 24U) \
         + (INTERSECTION_RING_COUNT_MAX * 8U) \
         + 8U \
         + (INTERSECTION_PATTERN_COUNT_MAX * 8U) \
         + (INTERSECTION_SPLIT_COUNT_MAX * INTERSECTION_PHASE_COUNT_MAX * 4U) \
         + (INTERSECTION_PREEMPT_COUNT_MAX * 124U) \
         + (INTERSECTION_PREEMPT_COUNT_MAX \
            * INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX * 2U) \
         + (INTERSECTION_PREEMPT_GATE_COUNT_MAX \
            * (1U + INTERSECTION_PREEMPT_GATE_DESCRIPTION_MAX)) \
         + (INTERSECTION_CHANNEL_COUNT_MAX * 24U) \
         + (INTERSECTION_OVERLAP_COUNT_MAX * 38U))
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V8 8UL
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V8 \
        (4U + (INTERSECTION_PHASE_COUNT_MAX * 24U) \
         + (INTERSECTION_RING_COUNT_MAX * 8U) \
         + 8U \
         + (INTERSECTION_PATTERN_COUNT_MAX * 8U) \
         + (INTERSECTION_SPLIT_COUNT_MAX * INTERSECTION_PHASE_COUNT_MAX * 4U) \
         + (INTERSECTION_PREEMPT_COUNT_MAX * 124U) \
         + (INTERSECTION_PREEMPT_COUNT_MAX \
            * INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX * 2U) \
         + (INTERSECTION_PREEMPT_GATE_COUNT_MAX \
            * (1U + INTERSECTION_PREEMPT_GATE_DESCRIPTION_MAX)) \
         + (INTERSECTION_PHASE_COUNT_MAX * 2U) \
         + (INTERSECTION_PREEMPT_COUNT_MAX * 2U) \
         + (INTERSECTION_CHANNEL_COUNT_MAX * 24U) \
         + (INTERSECTION_OVERLAP_COUNT_MAX * 38U))
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V9 9UL
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V9 \
        (4U + (INTERSECTION_PHASE_COUNT_MAX * CONFIGURATION_PHASE_IMAGE_SIZE) \
         + (INTERSECTION_RING_COUNT_MAX * 8U) \
         + 8U \
         + (INTERSECTION_PATTERN_COUNT_MAX * 8U) \
         + (INTERSECTION_SPLIT_COUNT_MAX * INTERSECTION_PHASE_COUNT_MAX * 4U) \
         + (INTERSECTION_PREEMPT_COUNT_MAX * 124U) \
         + (INTERSECTION_PREEMPT_COUNT_MAX \
            * INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX * 2U) \
         + (INTERSECTION_PREEMPT_GATE_COUNT_MAX \
            * (1U + INTERSECTION_PREEMPT_GATE_DESCRIPTION_MAX)) \
         + (INTERSECTION_PHASE_COUNT_MAX * 2U) \
         + (INTERSECTION_PREEMPT_COUNT_MAX * 2U) \
         + (INTERSECTION_CHANNEL_COUNT_MAX * 24U) \
         + (INTERSECTION_OVERLAP_COUNT_MAX * 38U))
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V10 10UL
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V11 11UL
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V12 12UL
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V13 13UL
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V14 14UL
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V15 15UL
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V16 16UL
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V17 17UL
#define CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V18 18UL
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V18 \
        (CONFIGURATION_IMAGE_PAYLOAD_SIZE \
         - CONFIGURATION_CABINET_ENVIRONMENT_IMAGE_SIZE)
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V17 \
        (CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V18 \
         - CONFIGURATION_GLOBAL_TIME_MANAGEMENT_IMAGE_SIZE)
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V16 \
        (CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V17 \
         - ((INTERSECTION_SEQUENCE_COUNT_MAX - 1U) \
            * INTERSECTION_RING_COUNT_MAX * 8U))
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V14 \
        (CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V16 \
         - 4U \
         - (INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX \
            * CONFIGURATION_USER_DEFINED_BACKUP_CONTENT_IMAGE_SIZE))
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V13 \
        (CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V14 - 4U)
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V12 \
        (CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V13 - 4U)
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V11 \
        (CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V12 \
         - (2U + (INTERSECTION_TIMEBASE_ACTION_COUNT_MAX \
                  * CONFIGURATION_TIMEBASE_ACTION_IMAGE_SIZE)))
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V10 \
        (CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V11 - 8U)

static uint8_t GetCandidateInputMapping(ConfigurationService_t *service,
                                        IntersectionInputMappingConfig_t *
                                        inputMapping);
static uint8_t GetCandidateVehicleDetector(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  IntersectionVehicleDetectorConfig_t *detector);
static uint8_t GetCandidatePedestrianDetector(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  IntersectionPedestrianDetectorConfig_t *detector);
static uint8_t GetCandidateUserDefinedBackupContent(
  ConfigurationService_t *service,
  uint8_t contentIndex,
  IntersectionUserDefinedBackupContentConfig_t *content);
static uint8_t GetCandidateDetectorReportConfig(
  ConfigurationService_t *service,
  IntersectionDetectorReportConfig_t *detectorReportConfig);
static uint8_t GetCandidateTimebase(
  ConfigurationService_t *service,
  IntersectionTimebaseConfig_t *timebase);
static uint8_t GetCandidateGlobalTimeManagement(
  ConfigurationService_t *service,
  IntersectionGlobalTimeManagementConfig_t *globalTimeManagement);
static uint8_t GetCandidateCabinetEnvironment(
  ConfigurationService_t *service,
  IntersectionCabinetEnvironmentConfig_t *cabinetEnvironment);
static uint8_t GetCandidateUnit(ConfigurationService_t *service,
                                IntersectionUnitConfig_t *unit);
static uint8_t GetCandidateRingPlan(ConfigurationService_t *service,
                                    uint8_t ringIndex,
                                    IntersectionRingPlan_t *ringPlan);
static uint8_t GetCandidateSequenceRingPlan(ConfigurationService_t *service,
                                            uint8_t sequenceNumber,
                                            uint8_t ringIndex,
                                            IntersectionRingPlan_t *ringPlan);

static uint8_t SequenceNumberToIndex(uint8_t sequenceNumber,
                                     uint8_t *sequenceIndex)
{
  if ((sequenceNumber == 0U)
      || (sequenceNumber > INTERSECTION_SEQUENCE_COUNT_MAX))
  {
    return 0U;
  }

  if (sequenceIndex != NULL)
  {
    *sequenceIndex = (uint8_t) (sequenceNumber - 1U);
  }

  return 1U;
}

static uint8_t NormalizeVehicleDetectorOptions(uint8_t options)
{
  if ((options & (uint8_t) (VEHICLE_DETECTOR_OPTIONS_YELLOW_LOCK
                            | VEHICLE_DETECTOR_OPTIONS_RED_LOCK))
      == (uint8_t) (VEHICLE_DETECTOR_OPTIONS_YELLOW_LOCK
                    | VEHICLE_DETECTOR_OPTIONS_RED_LOCK))
  {
    options &= (uint8_t) (~VEHICLE_DETECTOR_OPTIONS_RED_LOCK);
  }

  return options;
}

static void RebuildLegacyInputMapping(IntersectionConfig_t *config)
{
  uint8_t phaseIndex;
  uint8_t detectorIndex;
  uint8_t pedIndex;

  if (config == NULL)
  {
    return;
  }

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    config->inputMapping.phaseDetectors[phaseIndex] = 0U;
    config->inputMapping.phasePedInputs[phaseIndex] = 0U;
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    uint8_t phaseNumber = config->vehicleDetectors[detectorIndex].callPhase;

    if ((phaseNumber != 0U)
        && (phaseNumber <= INTERSECTION_PHASE_COUNT_MAX)
        && (config->inputMapping.phaseDetectors[phaseNumber - 1U] == 0U))
    {
      config->inputMapping.phaseDetectors[phaseNumber - 1U] =
        (uint8_t) (detectorIndex + 1U);
    }
  }

  for (pedIndex = 0U; pedIndex < INTERSECTION_PED_INPUT_COUNT_MAX; pedIndex++)
  {
    uint8_t phaseNumber = config->pedestrianDetectors[pedIndex].callPhase;

    if ((phaseNumber != 0U)
        && (phaseNumber <= INTERSECTION_PHASE_COUNT_MAX)
        && (config->inputMapping.phasePedInputs[phaseNumber - 1U] == 0U))
    {
      config->inputMapping.phasePedInputs[phaseNumber - 1U] =
        (uint8_t) (pedIndex + 1U);
    }
  }
}

static void ReplicateBaseSequencePlans(IntersectionConfig_t *config)
{
  uint8_t sequenceIndex;
  uint8_t ringIndex;

  if (config == NULL)
  {
    return;
  }

  for (sequenceIndex = 1U;
       sequenceIndex < INTERSECTION_SEQUENCE_COUNT_MAX;
       sequenceIndex++)
  {
    for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
    {
      config->sequencePlans[sequenceIndex][ringIndex] = config->rings[ringIndex];
    }
  }
}

static void WriteLe16(uint8_t *dst, uint16_t value)
{
  dst[0] = (uint8_t) (value & 0xFFU);
  dst[1] = (uint8_t) ((value >> 8) & 0xFFU);
}

static void WriteLe32(uint8_t *dst, uint32_t value)
{
  dst[0] = (uint8_t) (value & 0xFFU);
  dst[1] = (uint8_t) ((value >> 8) & 0xFFU);
  dst[2] = (uint8_t) ((value >> 16) & 0xFFU);
  dst[3] = (uint8_t) ((value >> 24) & 0xFFU);
}

static uint16_t ReadLe16(const uint8_t *src)
{
  return (uint16_t) src[0] | ((uint16_t) src[1] << 8);
}

static uint32_t ReadLe32(const uint8_t *src)
{
  return (uint32_t) src[0]
         | ((uint32_t) src[1] << 8)
         | ((uint32_t) src[2] << 16)
         | ((uint32_t) src[3] << 24);
}

static uint16_t PhaseOptionsFromLegacyFlags(uint8_t enabled,
                                            uint8_t vehicleRecall)
{
  uint16_t options = 0U;

  if (enabled != 0U)
  {
    options |= PHASE_OPTIONS_ENABLED;
  }

  if (vehicleRecall != 0U)
  {
    options |= PHASE_OPTIONS_MIN_RECALL;
  }

  return options;
}

static uint32_t Crc32Compute(const uint8_t *data, uint32_t length)
{
  uint32_t crc = 0xFFFFFFFFUL;
  uint32_t i;
  uint32_t j;

  for (i = 0U; i < length; i++)
  {
    crc ^= data[i];

    for (j = 0U; j < 8U; j++)
    {
      if ((crc & 1UL) != 0UL)
      {
        crc = (crc >> 1) ^ 0xEDB88320UL;
      }
      else
      {
        crc >>= 1;
      }
    }
  }

  return ~crc;
}

static void CandidateClear(ConfigurationService_t *service)
{
  memset(&service->candidate, 0, sizeof(service->candidate));
}

static void SetError(ConfigurationService_t *service,
                     IntersectionConfigError_t type,
                     uint16_t objectIndex)
{
  service->lastError.type = type;
  service->lastError.objectIndex = objectIndex;
}

static void CopyPhaseReferenceList(IntersectionPhaseReferenceList_t *dst,
                                   const uint8_t *values,
                                   uint8_t length)
{
  uint8_t index;

  dst->length = length;

  for (index = 0U; index < INTERSECTION_PHASE_COUNT_MAX; index++)
  {
    dst->values[index] = (index < length) ? values[index] : 0U;
  }
}

static void CopyChannelReferenceList(IntersectionChannelReferenceList_t *dst,
                                     const uint8_t *values,
                                     uint8_t length)
{
  uint8_t index;

  dst->length = length;

  for (index = 0U; index < INTERSECTION_CHANNEL_COUNT_MAX; index++)
  {
    dst->values[index] = (index < length) ? values[index] : 0U;
  }
}

static void CopyOverlapReferenceList(IntersectionOverlapReferenceList_t *dst,
                                     const uint8_t *values,
                                     uint8_t length)
{
  uint8_t index;

  dst->length = length;

  for (index = 0U; index < INTERSECTION_OVERLAP_COUNT_MAX; index++)
  {
    dst->values[index] = (index < length) ? values[index] : 0U;
  }
}

static void PayloadSerialize(const IntersectionConfig_t *config,
                             uint8_t *payload)
{
  uint32_t offset = 0U;
  uint8_t phaseIndex;
  uint8_t sequenceIndex;
  uint8_t ringIndex;
  uint8_t patternIndex;
  uint8_t splitIndex;
  uint8_t preemptIndex;
  uint8_t detectorIndex;
  uint8_t pedDetectorIndex;
  uint8_t channelIndex;
  uint8_t overlapIndex;
  uint8_t itemIndex;

  payload[offset++] = config->phaseCount;
  payload[offset++] = config->ringCount;
  payload[offset++] = config->barrierCount;
  payload[offset++] = config->reserved;

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    const IntersectionPhaseConfig_t *phase = &config->phases[phaseIndex];

    WriteLe16(&payload[offset], phase->phaseOptions);
    offset += 2U;
    payload[offset++] = phase->ring;
    payload[offset++] = phase->startup;
    WriteLe16(&payload[offset], phase->walkSeconds);
    offset += 2U;
    WriteLe16(&payload[offset], phase->pedClearSeconds);
    offset += 2U;
    WriteLe16(&payload[offset], phase->minGreenDs);
    offset += 2U;
    WriteLe16(&payload[offset], phase->phaseMaximum2Ds);
    offset += 2U;
    WriteLe16(&payload[offset], phase->maxGreenDs);
    offset += 2U;
    WriteLe16(&payload[offset], phase->phaseMaximum3Ds);
    offset += 2U;
    WriteLe16(&payload[offset], phase->passageDs);
    offset += 2U;
    WriteLe16(&payload[offset], phase->maxInitialDs);
    offset += 2U;
    WriteLe16(&payload[offset], phase->yellowChangeDs);
    offset += 2U;
    WriteLe16(&payload[offset], phase->redClearDs);
    offset += 2U;
    payload[offset++] = phase->redRevertDs;
    payload[offset++] = phase->addedInitialDs;
    payload[offset++] = phase->timeBeforeReductionSec;
    payload[offset++] = phase->carsBeforeReduction;
    payload[offset++] = phase->timeToReduceSec;
    payload[offset++] = phase->reduceByDs;
    payload[offset++] = phase->minimumGapDs;
    payload[offset++] = phase->dynamicMaxLimitSeconds;
    payload[offset++] = phase->dynamicMaxStepDs;
    payload[offset++] = phase->concurrency.length;

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      payload[offset++] = phase->concurrency.values[itemIndex];
    }

    payload[offset++] = phase->yellowRedBeforeEndPedClearDs;
    payload[offset++] = phase->pedWalkService;
    payload[offset++] = phase->dontWalkRevertDs;
    WriteLe16(&payload[offset], phase->pedAlternateClearSeconds);
    offset += 2U;
    WriteLe16(&payload[offset], phase->pedAlternateWalkSeconds);
    offset += 2U;
    WriteLe16(&payload[offset], phase->pedAdvanceWalkDs);
    offset += 2U;
    WriteLe16(&payload[offset], phase->pedDelayDs);
    offset += 2U;
    payload[offset++] = phase->advWarnGrnStartTimeDs;
    payload[offset++] = phase->advWarnRedStartTimeDs;
    payload[offset++] = phase->altMinTimeTransitionSeconds;
  }

  for (sequenceIndex = 0U;
       sequenceIndex < INTERSECTION_SEQUENCE_COUNT_MAX;
       sequenceIndex++)
  {
    for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
    {
      const IntersectionRingPlan_t *ringPlan =
        &config->sequencePlans[sequenceIndex][ringIndex];
      uint8_t serviceIndex;

      payload[offset++] = ringPlan->phaseCount;
      payload[offset++] = ringPlan->barrierPhaseCount;
      payload[offset++] = ringPlan->reserved0;
      payload[offset++] = ringPlan->reserved1;

      for (serviceIndex = 0U;
           serviceIndex < INTERSECTION_RING_PHASE_COUNT_MAX;
           serviceIndex++)
      {
        payload[offset++] = ringPlan->phaseOrder[serviceIndex];
      }
    }
  }

  payload[offset++] = config->coordination.operationalMode;
  payload[offset++] = config->coordination.correctionMode;
  payload[offset++] = config->coordination.maximumMode;
  payload[offset++] = config->coordination.forceMode;
  payload[offset++] = config->coordination.unitCoordSyncPoint;
  payload[offset++] = config->coordination.reserved0;
  payload[offset++] = config->coordination.reserved1;
  payload[offset++] = config->coordination.reserved2;

  for (patternIndex = 0U;
       patternIndex < INTERSECTION_PATTERN_COUNT_MAX;
       patternIndex++)
  {
    const IntersectionPatternConfig_t *pattern =
      &config->coordination.patterns[patternIndex];

    payload[offset++] = pattern->cycleTimeSeconds;
    payload[offset++] = pattern->offsetTimeSeconds;
    payload[offset++] = pattern->splitNumber;
    payload[offset++] = pattern->sequenceNumber;
    payload[offset++] = pattern->coordSyncPoint;
    payload[offset++] = pattern->options;
    payload[offset++] = pattern->reserved0;
    payload[offset++] = pattern->reserved1;
  }

  for (splitIndex = 0U; splitIndex < INTERSECTION_SPLIT_COUNT_MAX; splitIndex++)
  {
    for (phaseIndex = 0U;
         phaseIndex < INTERSECTION_PHASE_COUNT_MAX;
         phaseIndex++)
    {
      const IntersectionSplitPhaseConfig_t *split =
        &config->coordination.splits[splitIndex][phaseIndex];

      payload[offset++] = split->timeSeconds;
      payload[offset++] = split->mode;
      payload[offset++] = split->coordPhase;
      payload[offset++] = split->options;
    }
  }

  for (preemptIndex = 0U;
       preemptIndex < INTERSECTION_PREEMPT_COUNT_MAX;
       preemptIndex++)
  {
    const IntersectionPreemptConfig_t *preempt =
      &config->preempts[preemptIndex];

    payload[offset++] = preempt->control;
    payload[offset++] = preempt->link;
    WriteLe16(&payload[offset], preempt->delaySeconds);
    offset += 2U;
    WriteLe16(&payload[offset], preempt->minimumDurationSeconds);
    offset += 2U;
    payload[offset++] = preempt->minimumGreenSeconds;
    payload[offset++] = preempt->minimumWalkSeconds;
    payload[offset++] = preempt->enterPedClearSeconds;
    payload[offset++] = preempt->trackGreenSeconds;
    payload[offset++] = preempt->dwellGreenSeconds;
    WriteLe16(&payload[offset], preempt->maximumPresenceSeconds);
    offset += 2U;
    payload[offset++] = preempt->trackPhases.length;

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      payload[offset++] = preempt->trackPhases.values[itemIndex];
    }

    payload[offset++] = preempt->dwellPhases.length;

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      payload[offset++] = preempt->dwellPhases.values[itemIndex];
    }

    payload[offset++] = preempt->dwellPeds.length;

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      payload[offset++] = preempt->dwellPeds.values[itemIndex];
    }

    payload[offset++] = preempt->exitPhases.length;

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      payload[offset++] = preempt->exitPhases.values[itemIndex];
    }

    payload[offset++] = preempt->trackOverlaps.length;

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_OVERLAP_COUNT_MAX;
         itemIndex++)
    {
      payload[offset++] = preempt->trackOverlaps.values[itemIndex];
    }

    payload[offset++] = preempt->dwellOverlaps.length;

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_OVERLAP_COUNT_MAX;
         itemIndex++)
    {
      payload[offset++] = preempt->dwellOverlaps.values[itemIndex];
    }

    payload[offset++] = preempt->cyclingPhases.length;

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      payload[offset++] = preempt->cyclingPhases.values[itemIndex];
    }

    payload[offset++] = preempt->cyclingPeds.length;

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      payload[offset++] = preempt->cyclingPeds.values[itemIndex];
    }

    payload[offset++] = preempt->cyclingOverlaps.length;

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_OVERLAP_COUNT_MAX;
         itemIndex++)
    {
      payload[offset++] = preempt->cyclingOverlaps.values[itemIndex];
    }

    payload[offset++] = preempt->enterYellowChangeDs;
    payload[offset++] = preempt->enterRedClearDs;
    payload[offset++] = preempt->trackYellowChangeDs;
    payload[offset++] = preempt->trackRedClearDs;
    payload[offset++] = preempt->sequenceNumber;
    payload[offset++] = preempt->exitType;
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    const IntersectionVehicleDetectorConfig_t *detector =
      &config->vehicleDetectors[detectorIndex];

    payload[offset++] = NormalizeVehicleDetectorOptions(detector->options);
    payload[offset++] = detector->callPhase;
    payload[offset++] = detector->switchPhase;
    WriteLe16(&payload[offset], detector->delayDs);
    offset += 2U;
    payload[offset++] = detector->extendDs;
    payload[offset++] = detector->queueLimitSeconds;
    payload[offset++] = detector->noActivityMinutes;
    payload[offset++] = detector->maxPresenceMinutes;
    payload[offset++] = detector->erraticCountsPerMinute;
    payload[offset++] = detector->failTimeSeconds;
    payload[offset++] = detector->options2;
    payload[offset++] = detector->pairedDetector;
    WriteLe16(&payload[offset], detector->pairedDetectorSpacingCm);
    offset += 2U;
    WriteLe16(&payload[offset], detector->avgVehicleLengthCm);
    offset += 2U;
    WriteLe16(&payload[offset], detector->detectorLengthCm);
    offset += 2U;
    payload[offset++] = detector->travelMode;
  }

  for (pedDetectorIndex = 0U;
       pedDetectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       pedDetectorIndex++)
  {
    const IntersectionPedestrianDetectorConfig_t *detector =
      &config->pedestrianDetectors[pedDetectorIndex];

    payload[offset++] = detector->callPhase;
    payload[offset++] = detector->noActivityMinutes;
    payload[offset++] = detector->maxPresenceMinutes;
    payload[offset++] = detector->erraticCountsPerMinute;
    payload[offset++] = detector->apsMinimumActuationDs;
    payload[offset++] = detector->options;
  }

  payload[offset++] = config->detectorReports.volumeOccupancyPeriodSeconds;
  WriteLe16(&payload[offset], config->detectorReports.volumeOccupancyPeriodV3Seconds);
  offset += 2U;
  WriteLe16(&payload[offset], config->detectorReports.pedestrianDetectorPeriodSeconds);
  offset += 2U;
  payload[offset++] = config->detectorReports.reserved0;
  payload[offset++] = config->detectorReports.reserved1;
  payload[offset++] = config->detectorReports.reserved2;

  for (preemptIndex = 0U;
       preemptIndex < INTERSECTION_PREEMPT_COUNT_MAX;
       preemptIndex++)
  {
    for (itemIndex = 0U; itemIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
         itemIndex++)
    {
      WriteLe16(&payload[offset],
                config->preemptQueueDelayWeights[preemptIndex][itemIndex]);
      offset += 2U;
    }
  }

  for (preemptIndex = 0U; preemptIndex < INTERSECTION_PREEMPT_GATE_COUNT_MAX;
       preemptIndex++)
  {
    const IntersectionPreemptGateConfig_t *gate =
      &config->preemptGates[preemptIndex];

    payload[offset++] = gate->descriptionLength;

    for (itemIndex = 0U; itemIndex < INTERSECTION_PREEMPT_GATE_DESCRIPTION_MAX;
         itemIndex++)
    {
      payload[offset++] = gate->description[itemIndex];
    }
  }

  for (channelIndex = 0U;
       channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    const IntersectionChannelConfig_t *channel =
      &config->channels[channelIndex];

    payload[offset++] = channel->controlSource;
    payload[offset++] = channel->controlType;
    payload[offset++] = channel->flashMask;
    payload[offset++] = channel->dimMask;
    payload[offset++] = channel->greenType;
    payload[offset++] = channel->greenIncluded.length;

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_CHANNEL_COUNT_MAX;
         itemIndex++)
    {
      payload[offset++] = channel->greenIncluded.values[itemIndex];
    }

    WriteLe16(&payload[offset], channel->intersectionId);
    offset += 2U;
  }

  for (overlapIndex = 0U;
       overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX;
       overlapIndex++)
  {
    const IntersectionOverlapConfig_t *overlap =
      &config->overlaps[overlapIndex];

    payload[offset++] = overlap->type;
    payload[offset++] = overlap->includedPhases.length;

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      payload[offset++] = overlap->includedPhases.values[itemIndex];
    }

    payload[offset++] = overlap->modifierPhases.length;

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      payload[offset++] = overlap->modifierPhases.values[itemIndex];
    }

    WriteLe16(&payload[offset], overlap->trailGreenDs);
    offset += 2U;
    WriteLe16(&payload[offset], overlap->trailYellowDs);
    offset += 2U;
    WriteLe16(&payload[offset], overlap->trailRedDs);
    offset += 2U;
    WriteLe16(&payload[offset], overlap->walkSeconds);
    offset += 2U;
    WriteLe16(&payload[offset], overlap->pedClearSeconds);
    offset += 2U;
    payload[offset++] = overlap->conflictingPedPhases.length;

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      payload[offset++] = overlap->conflictingPedPhases.values[itemIndex];
    }
  }

  for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
  {
    payload[offset++] = config->inputMapping.phaseDetectors[itemIndex];
  }

  for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
  {
    payload[offset++] = config->inputMapping.phasePedInputs[itemIndex];
  }

  for (itemIndex = 0U; itemIndex < INTERSECTION_PREEMPT_COUNT_MAX; itemIndex++)
  {
    payload[offset++] = config->inputMapping.preemptInputs[itemIndex];
  }

  for (itemIndex = 0U; itemIndex < INTERSECTION_PREEMPT_COUNT_MAX; itemIndex++)
  {
    payload[offset++] = config->inputMapping.preemptControls[itemIndex];
  }

  WriteLe16(&payload[offset], config->timebase.patternSyncMinutes);
  offset += 2U;

  for (itemIndex = 0U; itemIndex < INTERSECTION_TIMEBASE_ACTION_COUNT_MAX;
       itemIndex++)
  {
    const IntersectionTimebaseActionConfig_t *action =
      &config->timebase.actions[itemIndex];

    payload[offset++] = action->pattern;
    payload[offset++] = action->auxiliaryFunction;
    payload[offset++] = action->specialFunction;
    payload[offset++] = action->enabledLane;
  }

  payload[offset++] = config->globalTimeManagement.globalDaylightSaving;
  WriteLe32(&payload[offset],
            (uint32_t)
            config->globalTimeManagement.controllerStandardTimeZoneSeconds);
  offset += 4U;

  for (itemIndex = 0U; itemIndex < INTERSECTION_TIMEBASE_SCHEDULE_COUNT_MAX;
       itemIndex++)
  {
    const IntersectionTimebaseScheduleEntryConfig_t *schedule =
      &config->globalTimeManagement.schedules[itemIndex];

    WriteLe16(&payload[offset], schedule->monthMask);
    offset += 2U;
    payload[offset++] = schedule->dayMask;
    payload[offset++] = schedule->dayPlanNumber;
    WriteLe32(&payload[offset], schedule->dateMask);
    offset += 4U;
  }

  for (itemIndex = 0U; itemIndex < INTERSECTION_DAY_PLAN_COUNT_MAX;
       itemIndex++)
  {
    uint8_t eventIndex;

    for (eventIndex = 0U; eventIndex < INTERSECTION_DAY_PLAN_EVENT_COUNT_MAX;
         eventIndex++)
    {
      const IntersectionDayPlanEventConfig_t *event =
        &config->globalTimeManagement.dayPlans[itemIndex][eventIndex];

      payload[offset++] = event->hour;
      payload[offset++] = event->minute;
      payload[offset++] = event->actionNumber;
    }
  }

  for (itemIndex = 0U;
       itemIndex < INTERSECTION_DAYLIGHT_SAVING_ENTRY_COUNT_MAX;
       itemIndex++)
  {
    const IntersectionDaylightSavingEntryConfig_t *entry =
      &config->globalTimeManagement.daylightSavingEntries[itemIndex];

    payload[offset++] = entry->beginMonth;
    payload[offset++] = entry->beginOccurrences;
    payload[offset++] = entry->beginDayOfWeek;
    payload[offset++] = entry->beginDayOfMonth;
    WriteLe32(&payload[offset], entry->beginSecondsToTransition);
    offset += 4U;
    payload[offset++] = entry->endMonth;
    payload[offset++] = entry->endOccurrences;
    payload[offset++] = entry->endDayOfWeek;
    payload[offset++] = entry->endDayOfMonth;
    WriteLe32(&payload[offset], entry->endSecondsToTransition);
    offset += 4U;
    WriteLe32(&payload[offset], entry->secondsToAdjust);
    offset += 4U;
  }

  payload[offset++] = config->unit.startUpFlashSeconds;
  payload[offset++] = config->unit.autoPedestrianClear;
  WriteLe16(&payload[offset], config->unit.backupTimeSeconds);
  offset += 2U;
  payload[offset++] = config->unit.redRevertDs;
  payload[offset++] = config->unit.startUpFlashMode;
  payload[offset++] = config->unit.timeSourceCommanded;
  payload[offset++] = config->unit.elevationOffsetMeters;

  WriteLe32(&payload[offset], config->unit.userDefinedBackupTimeSeconds);
  offset += 4U;

  for (itemIndex = 0U;
       itemIndex < INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX;
       itemIndex++)
  {
    const IntersectionUserDefinedBackupContentConfig_t *content =
      &config->userDefinedBackupContents[itemIndex];
    uint8_t oidIndex;
    uint8_t descriptionIndex;

    payload[offset++] = content->oidLength;
    payload[offset++] = content->descriptionLength;

    for (oidIndex = 0U;
         oidIndex < INTERSECTION_USER_DEFINED_BACKUP_OID_COMPONENT_COUNT_MAX;
         oidIndex++)
    {
      WriteLe32(&payload[offset], content->oid[oidIndex]);
      offset += 4U;
    }

    for (descriptionIndex = 0U;
         descriptionIndex < INTERSECTION_USER_DEFINED_BACKUP_DESCRIPTION_MAX;
         descriptionIndex++)
    {
      payload[offset++] = content->description[descriptionIndex];
    }
  }

  payload[offset++] = config->cabinetEnvironment.atccLedMode;

  for (itemIndex = 0U;
       itemIndex < INTERSECTION_CABINET_ENVIRONMENT_DEVICE_COUNT_MAX;
       itemIndex++)
  {
    uint8_t descriptionIndex;
    const IntersectionCabinetEnvironmentDeviceConfig_t *device =
      &config->cabinetEnvironment.devices[itemIndex];

    payload[offset++] = device->type;

    for (descriptionIndex = 0U;
         descriptionIndex < INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX;
         descriptionIndex++)
    {
      payload[offset++] = device->description[descriptionIndex];
    }
  }

  for (itemIndex = 0U;
       itemIndex < INTERSECTION_CABINET_TEMP_SENSOR_COUNT_MAX;
       itemIndex++)
  {
    uint8_t descriptionIndex;
    const IntersectionCabinetTemperatureSensorConfig_t *sensor =
      &config->cabinetEnvironment.temperatureSensors[itemIndex];

    for (descriptionIndex = 0U;
         descriptionIndex < INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX;
         descriptionIndex++)
    {
      payload[offset++] = sensor->description[descriptionIndex];
    }

    payload[offset++] = (uint8_t) sensor->highThreshold;
    payload[offset++] = (uint8_t) sensor->lowThreshold;
  }

  for (itemIndex = 0U;
       itemIndex < INTERSECTION_CABINET_HUMIDITY_SENSOR_COUNT_MAX;
       itemIndex++)
  {
    uint8_t descriptionIndex;
    const IntersectionCabinetHumiditySensorConfig_t *sensor =
      &config->cabinetEnvironment.humiditySensors[itemIndex];

    for (descriptionIndex = 0U;
         descriptionIndex < INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX;
         descriptionIndex++)
    {
      payload[offset++] = sensor->description[descriptionIndex];
    }

    payload[offset++] = sensor->threshold;
  }
} /* PayloadSerialize */

static void PayloadDeserializeLegacyV2(const uint8_t *payload,
                                       IntersectionConfig_t *config)
{
  uint32_t offset = 0U;
  uint8_t phaseIndex;
  uint8_t ringIndex;

  IntersectionConfigInitDefaults(config);

  config->phaseCount = payload[offset++];
  config->ringCount = payload[offset++];
  config->barrierCount = payload[offset++];
  config->reserved = payload[offset++];

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    IntersectionPhaseConfig_t *phase = &config->phases[phaseIndex];
    uint8_t legacyEnabled;
    uint8_t legacyRecall;

    legacyEnabled = payload[offset++];
    phase->ring = payload[offset++];
    legacyRecall = payload[offset++];
    phase->startup = (uint8_t) ((legacyEnabled != 0U)
                                ? INTERSECTION_PHASE_STARTUP_PHASE_NOT_ON
                                : INTERSECTION_PHASE_STARTUP_OTHER);
    phase->phaseOptions = PhaseOptionsFromLegacyFlags(legacyEnabled,
                                                      legacyRecall);
    offset++;
    phase->minGreenDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->maxGreenDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->phaseMaximum2Ds = phase->maxGreenDs;
    phase->phaseMaximum3Ds = phase->maxGreenDs;
    phase->yellowChangeDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->redClearDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->passageDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->maxInitialDs = ReadLe16(&payload[offset]);
    offset += 2U;
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
  {
    IntersectionRingPlan_t *ringPlan = &config->rings[ringIndex];
    uint8_t serviceIndex;

    ringPlan->phaseCount = payload[offset++];
    ringPlan->barrierPhaseCount = payload[offset++];
    ringPlan->reserved0 = payload[offset++];
    ringPlan->reserved1 = payload[offset++];

    for (serviceIndex = 0U;
         serviceIndex < INTERSECTION_RING_PHASE_COUNT_MAX;
         serviceIndex++)
    {
      ringPlan->phaseOrder[serviceIndex] = payload[offset++];
    }
  }
} /* PayloadDeserializeLegacyV2 */

static void PayloadDeserializeLegacyV3(const uint8_t *payload,
                                       IntersectionConfig_t *config)
{
  uint32_t offset = 0U;
  uint8_t phaseIndex;
  uint8_t ringIndex;
  uint8_t channelIndex;
  uint8_t overlapIndex;
  uint8_t itemIndex;

  IntersectionConfigInitDefaults(config);

  config->phaseCount = payload[offset++];
  config->ringCount = payload[offset++];
  config->barrierCount = payload[offset++];
  config->reserved = payload[offset++];

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    IntersectionPhaseConfig_t *phase = &config->phases[phaseIndex];
    uint8_t legacyEnabled;
    uint8_t legacyRecall;

    legacyEnabled = payload[offset++];
    phase->ring = payload[offset++];
    legacyRecall = payload[offset++];
    phase->startup = (uint8_t) ((legacyEnabled != 0U)
                                ? INTERSECTION_PHASE_STARTUP_PHASE_NOT_ON
                                : INTERSECTION_PHASE_STARTUP_OTHER);
    phase->phaseOptions = PhaseOptionsFromLegacyFlags(legacyEnabled,
                                                      legacyRecall);
    offset++;
    phase->minGreenDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->maxGreenDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->phaseMaximum2Ds = phase->maxGreenDs;
    phase->phaseMaximum3Ds = phase->maxGreenDs;
    phase->yellowChangeDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->redClearDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->passageDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->maxInitialDs = ReadLe16(&payload[offset]);
    offset += 2U;
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
  {
    IntersectionRingPlan_t *ringPlan = &config->rings[ringIndex];
    uint8_t serviceIndex;

    ringPlan->phaseCount = payload[offset++];
    ringPlan->barrierPhaseCount = payload[offset++];
    ringPlan->reserved0 = payload[offset++];
    ringPlan->reserved1 = payload[offset++];

    for (serviceIndex = 0U;
         serviceIndex < INTERSECTION_RING_PHASE_COUNT_MAX;
         serviceIndex++)
    {
      ringPlan->phaseOrder[serviceIndex] = payload[offset++];
    }
  }

  for (channelIndex = 0U;
       channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    IntersectionChannelConfig_t *channel = &config->channels[channelIndex];

    channel->controlSource = payload[offset++];
    channel->controlType = payload[offset++];
    channel->flashMask = payload[offset++];
    channel->dimMask = payload[offset++];
    channel->greenType = payload[offset++];
    channel->greenIncluded.length = payload[offset++];

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_CHANNEL_COUNT_MAX;
         itemIndex++)
    {
      channel->greenIncluded.values[itemIndex] = payload[offset++];
    }

    channel->intersectionId = ReadLe16(&payload[offset]);
    offset += 2U;
  }

  for (overlapIndex = 0U;
       overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX;
       overlapIndex++)
  {
    IntersectionOverlapConfig_t *overlap = &config->overlaps[overlapIndex];

    overlap->type = payload[offset++];
    overlap->includedPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      overlap->includedPhases.values[itemIndex] = payload[offset++];
    }

    overlap->modifierPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      overlap->modifierPhases.values[itemIndex] = payload[offset++];
    }

    overlap->trailGreenDs = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->trailYellowDs = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->trailRedDs = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->walkSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->pedClearSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->conflictingPedPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      overlap->conflictingPedPhases.values[itemIndex] = payload[offset++];
    }
  }
} /* PayloadDeserializeLegacyV3 */

static void PayloadDeserializeLegacyV4(const uint8_t *payload,
                                       IntersectionConfig_t *config)
{
  uint32_t offset = 0U;
  uint8_t phaseIndex;
  uint8_t ringIndex;
  uint8_t channelIndex;
  uint8_t overlapIndex;
  uint8_t itemIndex;

  IntersectionConfigInitDefaults(config);

  config->phaseCount = payload[offset++];
  config->ringCount = payload[offset++];
  config->barrierCount = payload[offset++];
  config->reserved = payload[offset++];

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    IntersectionPhaseConfig_t *phase = &config->phases[phaseIndex];
    uint8_t legacyEnabled;
    uint8_t legacyRecall;

    legacyEnabled = payload[offset++];
    phase->ring = payload[offset++];
    legacyRecall = payload[offset++];
    phase->startup = (uint8_t) ((legacyEnabled != 0U)
                                ? INTERSECTION_PHASE_STARTUP_PHASE_NOT_ON
                                : INTERSECTION_PHASE_STARTUP_OTHER);
    phase->phaseOptions = PhaseOptionsFromLegacyFlags(legacyEnabled,
                                                      legacyRecall);
    offset++;
    phase->walkSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->pedClearSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->minGreenDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->maxGreenDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->phaseMaximum2Ds = phase->maxGreenDs;
    phase->phaseMaximum3Ds = phase->maxGreenDs;
    phase->yellowChangeDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->redClearDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->passageDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->maxInitialDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->pedAdvanceWalkDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->pedDelayDs = ReadLe16(&payload[offset]);
    offset += 2U;
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
  {
    IntersectionRingPlan_t *ringPlan = &config->rings[ringIndex];
    uint8_t serviceIndex;

    ringPlan->phaseCount = payload[offset++];
    ringPlan->barrierPhaseCount = payload[offset++];
    ringPlan->reserved0 = payload[offset++];
    ringPlan->reserved1 = payload[offset++];

    for (serviceIndex = 0U;
         serviceIndex < INTERSECTION_RING_PHASE_COUNT_MAX;
         serviceIndex++)
    {
      ringPlan->phaseOrder[serviceIndex] = payload[offset++];
    }
  }

  for (channelIndex = 0U;
       channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    IntersectionChannelConfig_t *channel = &config->channels[channelIndex];

    channel->controlSource = payload[offset++];
    channel->controlType = payload[offset++];
    channel->flashMask = payload[offset++];
    channel->dimMask = payload[offset++];
    channel->greenType = payload[offset++];
    channel->greenIncluded.length = payload[offset++];

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_CHANNEL_COUNT_MAX;
         itemIndex++)
    {
      channel->greenIncluded.values[itemIndex] = payload[offset++];
    }

    channel->intersectionId = ReadLe16(&payload[offset]);
    offset += 2U;
  }

  for (overlapIndex = 0U;
       overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX;
       overlapIndex++)
  {
    IntersectionOverlapConfig_t *overlap = &config->overlaps[overlapIndex];

    overlap->type = payload[offset++];
    overlap->includedPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      overlap->includedPhases.values[itemIndex] = payload[offset++];
    }

    overlap->modifierPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      overlap->modifierPhases.values[itemIndex] = payload[offset++];
    }

    overlap->trailGreenDs = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->trailYellowDs = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->trailRedDs = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->walkSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->pedClearSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->conflictingPedPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      overlap->conflictingPedPhases.values[itemIndex] = payload[offset++];
    }
  }
} /* PayloadDeserializeLegacyV4 */

static void PayloadDeserializeLegacyV5(const uint8_t *payload,
                                       IntersectionConfig_t *config)
{
  uint32_t offset = 0U;
  uint8_t phaseIndex;
  uint8_t ringIndex;
  uint8_t patternIndex;
  uint8_t splitIndex;
  uint8_t channelIndex;
  uint8_t overlapIndex;
  uint8_t itemIndex;

  IntersectionConfigInitDefaults(config);

  config->phaseCount = payload[offset++];
  config->ringCount = payload[offset++];
  config->barrierCount = payload[offset++];
  config->reserved = payload[offset++];

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    IntersectionPhaseConfig_t *phase = &config->phases[phaseIndex];
    uint8_t legacyEnabled;
    uint8_t legacyRecall;

    legacyEnabled = payload[offset++];
    phase->ring = payload[offset++];
    legacyRecall = payload[offset++];
    phase->startup = (uint8_t) ((legacyEnabled != 0U)
                                ? INTERSECTION_PHASE_STARTUP_PHASE_NOT_ON
                                : INTERSECTION_PHASE_STARTUP_OTHER);
    phase->phaseOptions = PhaseOptionsFromLegacyFlags(legacyEnabled,
                                                      legacyRecall);
    offset++;
    phase->walkSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->pedClearSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->minGreenDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->maxGreenDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->phaseMaximum2Ds = phase->maxGreenDs;
    phase->phaseMaximum3Ds = phase->maxGreenDs;
    phase->yellowChangeDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->redClearDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->passageDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->maxInitialDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->pedAdvanceWalkDs = ReadLe16(&payload[offset]);
    offset += 2U;
    phase->pedDelayDs = ReadLe16(&payload[offset]);
    offset += 2U;
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
  {
    IntersectionRingPlan_t *ringPlan = &config->rings[ringIndex];
    uint8_t serviceIndex;

    ringPlan->phaseCount = payload[offset++];
    ringPlan->barrierPhaseCount = payload[offset++];
    ringPlan->reserved0 = payload[offset++];
    ringPlan->reserved1 = payload[offset++];

    for (serviceIndex = 0U;
         serviceIndex < INTERSECTION_RING_PHASE_COUNT_MAX;
         serviceIndex++)
    {
      ringPlan->phaseOrder[serviceIndex] = payload[offset++];
    }
  }

  config->coordination.operationalMode = payload[offset++];
  config->coordination.correctionMode = payload[offset++];
  config->coordination.maximumMode = payload[offset++];
  config->coordination.forceMode = payload[offset++];
  config->coordination.unitCoordSyncPoint = payload[offset++];
  config->coordination.reserved0 = payload[offset++];
  config->coordination.reserved1 = payload[offset++];
  config->coordination.reserved2 = payload[offset++];

  for (patternIndex = 0U;
       patternIndex < INTERSECTION_PATTERN_COUNT_MAX;
       patternIndex++)
  {
    IntersectionPatternConfig_t *pattern =
      &config->coordination.patterns[patternIndex];

    pattern->cycleTimeSeconds = payload[offset++];
    pattern->offsetTimeSeconds = payload[offset++];
    pattern->splitNumber = payload[offset++];
    pattern->sequenceNumber = payload[offset++];
    pattern->coordSyncPoint = payload[offset++];
    pattern->options = payload[offset++];
    pattern->reserved0 = payload[offset++];
    pattern->reserved1 = payload[offset++];
  }

  for (splitIndex = 0U; splitIndex < INTERSECTION_SPLIT_COUNT_MAX; splitIndex++)
  {
    for (phaseIndex = 0U;
         phaseIndex < INTERSECTION_PHASE_COUNT_MAX;
         phaseIndex++)
    {
      IntersectionSplitPhaseConfig_t *split =
        &config->coordination.splits[splitIndex][phaseIndex];

      split->timeSeconds = payload[offset++];
      split->mode = payload[offset++];
      split->coordPhase = payload[offset++];
      split->options = payload[offset++];
    }
  }

  for (channelIndex = 0U;
       channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    IntersectionChannelConfig_t *channel = &config->channels[channelIndex];

    channel->controlSource = payload[offset++];
    channel->controlType = payload[offset++];
    channel->flashMask = payload[offset++];
    channel->dimMask = payload[offset++];
    channel->greenType = payload[offset++];
    channel->greenIncluded.length = payload[offset++];

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_CHANNEL_COUNT_MAX;
         itemIndex++)
    {
      channel->greenIncluded.values[itemIndex] = payload[offset++];
    }

    channel->intersectionId = ReadLe16(&payload[offset]);
    offset += 2U;
  }

  for (overlapIndex = 0U;
       overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX;
       overlapIndex++)
  {
    IntersectionOverlapConfig_t *overlap = &config->overlaps[overlapIndex];

    overlap->type = payload[offset++];
    overlap->includedPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      overlap->includedPhases.values[itemIndex] = payload[offset++];
    }

    overlap->modifierPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      overlap->modifierPhases.values[itemIndex] = payload[offset++];
    }

    overlap->trailGreenDs = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->trailYellowDs = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->trailRedDs = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->walkSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->pedClearSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->conflictingPedPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      overlap->conflictingPedPhases.values[itemIndex] = payload[offset++];
    }
  }
} /* PayloadDeserializeLegacyV5 */

static void PayloadDeserialize(const uint8_t *payload,
                               uint32_t payloadLength,
                               IntersectionConfig_t *config)
{
  uint32_t offset = 0U;
  uint8_t phaseIndex;
  uint8_t sequenceIndex;
  uint8_t ringIndex;
  uint8_t patternIndex;
  uint8_t splitIndex;
  uint8_t preemptIndex;
  uint8_t detectorIndex;
  uint8_t pedDetectorIndex;
  uint8_t channelIndex;
  uint8_t overlapIndex;
  uint8_t itemIndex;
  uint8_t currentLayout;
  uint8_t currentOrLegacyV18Layout;
  uint8_t extendedPhaseLayout;
  uint8_t currentOrLegacyV16Layout;
  uint8_t multiSequenceLayout;

  if ((payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V2)
      && (payloadLength != CONFIGURATION_IMAGE_PAYLOAD_SIZE))
  {
    PayloadDeserializeLegacyV2(payload, config);

    return;
  }

  if ((payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V3)
      && (payloadLength != CONFIGURATION_IMAGE_PAYLOAD_SIZE))
  {
    PayloadDeserializeLegacyV3(payload, config);

    return;
  }

  if ((payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V4)
      && (payloadLength != CONFIGURATION_IMAGE_PAYLOAD_SIZE))
  {
    PayloadDeserializeLegacyV4(payload, config);

    return;
  }

  if ((payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V5)
      && (payloadLength != CONFIGURATION_IMAGE_PAYLOAD_SIZE))
  {
    PayloadDeserializeLegacyV5(payload, config);

    return;
  }

  IntersectionConfigInitDefaults(config);
  currentLayout = (uint8_t) (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE);
  currentOrLegacyV18Layout = (uint8_t) ((currentLayout != 0U)
                                        || (payloadLength
                                            == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V18));
  currentOrLegacyV16Layout = (uint8_t) ((currentOrLegacyV18Layout != 0U)
                                        || (payloadLength
                                            == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V17)
                                        || (payloadLength
                                            == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V16));
  multiSequenceLayout = (uint8_t) ((currentOrLegacyV18Layout != 0U)
                                   || (payloadLength
                                       == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V17));
  extendedPhaseLayout = (uint8_t) ((currentOrLegacyV16Layout != 0U)
                                   || (payloadLength
                                       == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V14)
                                   || (payloadLength
                                       == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V13)
                                   || (payloadLength
                                       == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V12)
                                   || (payloadLength
                                       == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V11)
                                   || (payloadLength
                                       == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V10)
                                   || (payloadLength
                                       == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V9));

  config->phaseCount = payload[offset++];
  config->ringCount = payload[offset++];
  config->barrierCount = payload[offset++];
  config->reserved = payload[offset++];

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    IntersectionPhaseConfig_t *phase = &config->phases[phaseIndex];
    if (extendedPhaseLayout != 0U)
    {
      phase->phaseOptions = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->ring = payload[offset++];
      phase->startup = payload[offset++];
      phase->walkSeconds = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->pedClearSeconds = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->minGreenDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->phaseMaximum2Ds = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->maxGreenDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->phaseMaximum3Ds = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->passageDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->maxInitialDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->yellowChangeDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->redClearDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->redRevertDs = payload[offset++];
      phase->addedInitialDs = payload[offset++];
      phase->timeBeforeReductionSec = payload[offset++];
      phase->carsBeforeReduction = payload[offset++];
      phase->timeToReduceSec = payload[offset++];
      phase->reduceByDs = payload[offset++];
      phase->minimumGapDs = payload[offset++];
      phase->dynamicMaxLimitSeconds = payload[offset++];
      phase->dynamicMaxStepDs = payload[offset++];
      phase->concurrency.length = payload[offset++];

      for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
      {
        phase->concurrency.values[itemIndex] = payload[offset++];
      }

      phase->yellowRedBeforeEndPedClearDs = payload[offset++];
      phase->pedWalkService = payload[offset++];
      phase->dontWalkRevertDs = payload[offset++];
      phase->pedAlternateClearSeconds = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->pedAlternateWalkSeconds = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->pedAdvanceWalkDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->pedDelayDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->advWarnGrnStartTimeDs = payload[offset++];
      phase->advWarnRedStartTimeDs = payload[offset++];
      phase->altMinTimeTransitionSeconds = payload[offset++];
    }
    else
    {
      uint8_t legacyEnabled;
      uint8_t legacyRecall;

      legacyEnabled = payload[offset++];
      phase->ring = payload[offset++];
      legacyRecall = payload[offset++];
      phase->startup = (uint8_t) ((legacyEnabled != 0U)
                                  ? INTERSECTION_PHASE_STARTUP_PHASE_NOT_ON
                                  : INTERSECTION_PHASE_STARTUP_OTHER);
      phase->phaseOptions = PhaseOptionsFromLegacyFlags(legacyEnabled,
                                                        legacyRecall);
      offset++;
      phase->walkSeconds = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->pedClearSeconds = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->minGreenDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->maxGreenDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->phaseMaximum2Ds = phase->maxGreenDs;
      phase->phaseMaximum3Ds = phase->maxGreenDs;
      phase->yellowChangeDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->redClearDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->passageDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->maxInitialDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->pedAdvanceWalkDs = ReadLe16(&payload[offset]);
      offset += 2U;
      phase->pedDelayDs = ReadLe16(&payload[offset]);
      offset += 2U;
    }
  }

  if (multiSequenceLayout != 0U)
  {
    for (sequenceIndex = 0U;
         sequenceIndex < INTERSECTION_SEQUENCE_COUNT_MAX;
         sequenceIndex++)
    {
      for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
      {
        IntersectionRingPlan_t *ringPlan =
          &config->sequencePlans[sequenceIndex][ringIndex];
        uint8_t serviceIndex;

        ringPlan->phaseCount = payload[offset++];
        ringPlan->barrierPhaseCount = payload[offset++];
        ringPlan->reserved0 = payload[offset++];
        ringPlan->reserved1 = payload[offset++];

        for (serviceIndex = 0U;
             serviceIndex < INTERSECTION_RING_PHASE_COUNT_MAX;
             serviceIndex++)
        {
          ringPlan->phaseOrder[serviceIndex] = payload[offset++];
        }
      }
    }
  }
  else
  {
    for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
    {
      IntersectionRingPlan_t *ringPlan = &config->rings[ringIndex];
      uint8_t serviceIndex;

      ringPlan->phaseCount = payload[offset++];
      ringPlan->barrierPhaseCount = payload[offset++];
      ringPlan->reserved0 = payload[offset++];
      ringPlan->reserved1 = payload[offset++];

      for (serviceIndex = 0U;
           serviceIndex < INTERSECTION_RING_PHASE_COUNT_MAX;
           serviceIndex++)
      {
        ringPlan->phaseOrder[serviceIndex] = payload[offset++];
      }
    }
  }

  config->coordination.operationalMode = payload[offset++];
  config->coordination.correctionMode = payload[offset++];
  config->coordination.maximumMode = payload[offset++];
  config->coordination.forceMode = payload[offset++];
  config->coordination.unitCoordSyncPoint = payload[offset++];
  config->coordination.reserved0 = payload[offset++];
  config->coordination.reserved1 = payload[offset++];
  config->coordination.reserved2 = payload[offset++];

  for (patternIndex = 0U;
       patternIndex < INTERSECTION_PATTERN_COUNT_MAX;
       patternIndex++)
  {
    IntersectionPatternConfig_t *pattern =
      &config->coordination.patterns[patternIndex];

    pattern->cycleTimeSeconds = payload[offset++];
    pattern->offsetTimeSeconds = payload[offset++];
    pattern->splitNumber = payload[offset++];
    pattern->sequenceNumber = payload[offset++];
    pattern->coordSyncPoint = payload[offset++];
    pattern->options = payload[offset++];
    pattern->reserved0 = payload[offset++];
    pattern->reserved1 = payload[offset++];
  }

  for (splitIndex = 0U; splitIndex < INTERSECTION_SPLIT_COUNT_MAX; splitIndex++)
  {
    for (phaseIndex = 0U;
         phaseIndex < INTERSECTION_PHASE_COUNT_MAX;
         phaseIndex++)
    {
      IntersectionSplitPhaseConfig_t *split =
        &config->coordination.splits[splitIndex][phaseIndex];

      split->timeSeconds = payload[offset++];
      split->mode = payload[offset++];
      split->coordPhase = payload[offset++];
      split->options = payload[offset++];
    }
  }

  for (preemptIndex = 0U;
       preemptIndex < INTERSECTION_PREEMPT_COUNT_MAX;
       preemptIndex++)
  {
    IntersectionPreemptConfig_t *preempt = &config->preempts[preemptIndex];

    preempt->control = payload[offset++];
    preempt->link = payload[offset++];
    preempt->delaySeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    preempt->minimumDurationSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    preempt->minimumGreenSeconds = payload[offset++];
    preempt->minimumWalkSeconds = payload[offset++];
    preempt->enterPedClearSeconds = payload[offset++];
    preempt->trackGreenSeconds = payload[offset++];
    preempt->dwellGreenSeconds = payload[offset++];
    preempt->maximumPresenceSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    preempt->trackPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      preempt->trackPhases.values[itemIndex] = payload[offset++];
    }

    preempt->dwellPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      preempt->dwellPhases.values[itemIndex] = payload[offset++];
    }

    preempt->dwellPeds.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      preempt->dwellPeds.values[itemIndex] = payload[offset++];
    }

    preempt->exitPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      preempt->exitPhases.values[itemIndex] = payload[offset++];
    }

    preempt->trackOverlaps.length = payload[offset++];

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_OVERLAP_COUNT_MAX;
         itemIndex++)
    {
      preempt->trackOverlaps.values[itemIndex] = payload[offset++];
    }

    preempt->dwellOverlaps.length = payload[offset++];

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_OVERLAP_COUNT_MAX;
         itemIndex++)
    {
      preempt->dwellOverlaps.values[itemIndex] = payload[offset++];
    }

    preempt->cyclingPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      preempt->cyclingPhases.values[itemIndex] = payload[offset++];
    }

    preempt->cyclingPeds.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      preempt->cyclingPeds.values[itemIndex] = payload[offset++];
    }

    preempt->cyclingOverlaps.length = payload[offset++];

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_OVERLAP_COUNT_MAX;
         itemIndex++)
    {
      preempt->cyclingOverlaps.values[itemIndex] = payload[offset++];
    }

    preempt->enterYellowChangeDs = payload[offset++];
    preempt->enterRedClearDs = payload[offset++];
    preempt->trackYellowChangeDs = payload[offset++];
    preempt->trackRedClearDs = payload[offset++];
    preempt->sequenceNumber = payload[offset++];
    preempt->exitType = payload[offset++];
  }

  if ((currentOrLegacyV16Layout != 0U)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V14)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V13)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V12)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V11)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V10))
  {
    for (detectorIndex = 0U;
         detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
         detectorIndex++)
    {
      IntersectionVehicleDetectorConfig_t *detector =
        &config->vehicleDetectors[detectorIndex];

      detector->options = NormalizeVehicleDetectorOptions(payload[offset++]);
      detector->callPhase = payload[offset++];
      detector->switchPhase = payload[offset++];
      detector->delayDs = ReadLe16(&payload[offset]);
      offset += 2U;
      detector->extendDs = payload[offset++];
      detector->queueLimitSeconds = payload[offset++];
      detector->noActivityMinutes = payload[offset++];
      detector->maxPresenceMinutes = payload[offset++];
      detector->erraticCountsPerMinute = payload[offset++];
      detector->failTimeSeconds = payload[offset++];
      detector->options2 = payload[offset++];
      detector->pairedDetector = payload[offset++];
      detector->pairedDetectorSpacingCm = ReadLe16(&payload[offset]);
      offset += 2U;
      detector->avgVehicleLengthCm = ReadLe16(&payload[offset]);
      offset += 2U;
      detector->detectorLengthCm = ReadLe16(&payload[offset]);
      offset += 2U;
      detector->travelMode = payload[offset++];
    }

    for (pedDetectorIndex = 0U;
         pedDetectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
         pedDetectorIndex++)
    {
      IntersectionPedestrianDetectorConfig_t *detector =
        &config->pedestrianDetectors[pedDetectorIndex];

      detector->callPhase = payload[offset++];
      detector->noActivityMinutes = payload[offset++];
      detector->maxPresenceMinutes = payload[offset++];
      detector->erraticCountsPerMinute = payload[offset++];
      detector->apsMinimumActuationDs = payload[offset++];
      detector->options = payload[offset++];
    }
  }

  if ((currentOrLegacyV16Layout != 0U)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V14)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V13)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V12)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V11))
  {
    config->detectorReports.volumeOccupancyPeriodSeconds = payload[offset++];
    config->detectorReports.volumeOccupancyPeriodV3Seconds =
      ReadLe16(&payload[offset]);
    offset += 2U;
    config->detectorReports.pedestrianDetectorPeriodSeconds =
      ReadLe16(&payload[offset]);
    offset += 2U;
    config->detectorReports.reserved0 = payload[offset++];
    config->detectorReports.reserved1 = payload[offset++];
    config->detectorReports.reserved2 = payload[offset++];
  }

  if ((currentOrLegacyV16Layout != 0U)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V14)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V13)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V12)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V11)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V10)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V9)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V8)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V7))
  {
    for (preemptIndex = 0U; preemptIndex < INTERSECTION_PREEMPT_COUNT_MAX;
         preemptIndex++)
    {
      for (itemIndex = 0U; itemIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
           itemIndex++)
      {
        config->preemptQueueDelayWeights[preemptIndex][itemIndex] =
          ReadLe16(&payload[offset]);
        offset += 2U;
      }
    }

    for (preemptIndex = 0U; preemptIndex < INTERSECTION_PREEMPT_GATE_COUNT_MAX;
         preemptIndex++)
    {
      IntersectionPreemptGateConfig_t *gate =
        &config->preemptGates[preemptIndex];

      gate->descriptionLength = payload[offset++];

      for (itemIndex = 0U;
           itemIndex < INTERSECTION_PREEMPT_GATE_DESCRIPTION_MAX;
           itemIndex++)
      {
        gate->description[itemIndex] = payload[offset++];
      }
    }
  }

  for (channelIndex = 0U;
       channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    IntersectionChannelConfig_t *channel = &config->channels[channelIndex];

    channel->controlSource = payload[offset++];
    channel->controlType = payload[offset++];
    channel->flashMask = payload[offset++];
    channel->dimMask = payload[offset++];
    channel->greenType = payload[offset++];
    channel->greenIncluded.length = payload[offset++];

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_CHANNEL_COUNT_MAX;
         itemIndex++)
    {
      channel->greenIncluded.values[itemIndex] = payload[offset++];
    }

    channel->intersectionId = ReadLe16(&payload[offset]);
    offset += 2U;
  }

  for (overlapIndex = 0U;
       overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX;
       overlapIndex++)
  {
    IntersectionOverlapConfig_t *overlap = &config->overlaps[overlapIndex];

    overlap->type = payload[offset++];
    overlap->includedPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      overlap->includedPhases.values[itemIndex] = payload[offset++];
    }

    overlap->modifierPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      overlap->modifierPhases.values[itemIndex] = payload[offset++];
    }

    overlap->trailGreenDs = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->trailYellowDs = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->trailRedDs = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->walkSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->pedClearSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    overlap->conflictingPedPhases.length = payload[offset++];

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      overlap->conflictingPedPhases.values[itemIndex] = payload[offset++];
    }
  }

  if ((currentOrLegacyV16Layout != 0U)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V14)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V13)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V12)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V11)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V10)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V9)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V8))
  {
    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      config->inputMapping.phaseDetectors[itemIndex] = payload[offset++];
    }

    for (itemIndex = 0U; itemIndex < INTERSECTION_PHASE_COUNT_MAX; itemIndex++)
    {
      config->inputMapping.phasePedInputs[itemIndex] = payload[offset++];
    }

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_PREEMPT_COUNT_MAX;
         itemIndex++)
    {
      config->inputMapping.preemptInputs[itemIndex] = payload[offset++];
    }

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_PREEMPT_COUNT_MAX;
         itemIndex++)
    {
      config->inputMapping.preemptControls[itemIndex] = payload[offset++];
    }
  }

  if ((payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V9)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V8))
  {
    for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
    {
      uint8_t detectorNumber = config->inputMapping.phaseDetectors[phaseIndex];
      uint8_t pedNumber = config->inputMapping.phasePedInputs[phaseIndex];

      if ((detectorNumber != 0U)
          && (detectorNumber <= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
      {
        config->vehicleDetectors[detectorNumber - 1U].callPhase =
          (uint8_t) (phaseIndex + 1U);
      }

      if ((pedNumber != 0U) && (pedNumber <= INTERSECTION_PED_INPUT_COUNT_MAX))
      {
        config->pedestrianDetectors[pedNumber - 1U].callPhase =
          (uint8_t) (phaseIndex + 1U);
      }
    }
  }

  if ((currentOrLegacyV16Layout != 0U)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V14)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V13)
      || (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V12))
  {
    config->timebase.patternSyncMinutes = ReadLe16(&payload[offset]);
    offset += 2U;

    for (itemIndex = 0U; itemIndex < INTERSECTION_TIMEBASE_ACTION_COUNT_MAX;
         itemIndex++)
    {
      IntersectionTimebaseActionConfig_t *action =
        &config->timebase.actions[itemIndex];

      action->pattern = payload[offset++];
      action->auxiliaryFunction = payload[offset++];
      action->specialFunction = payload[offset++];
      action->enabledLane = payload[offset++];
    }
  }

  if (currentOrLegacyV18Layout != 0U)
  {
    config->globalTimeManagement.globalDaylightSaving = payload[offset++];
    config->globalTimeManagement.controllerStandardTimeZoneSeconds =
      (int32_t) ReadLe32(&payload[offset]);
    offset += 4U;

    for (itemIndex = 0U; itemIndex < INTERSECTION_TIMEBASE_SCHEDULE_COUNT_MAX;
         itemIndex++)
    {
      IntersectionTimebaseScheduleEntryConfig_t *schedule =
        &config->globalTimeManagement.schedules[itemIndex];

      schedule->monthMask = ReadLe16(&payload[offset]);
      offset += 2U;
      schedule->dayMask = payload[offset++];
      schedule->dayPlanNumber = payload[offset++];
      schedule->dateMask = ReadLe32(&payload[offset]);
      offset += 4U;
    }

    for (itemIndex = 0U; itemIndex < INTERSECTION_DAY_PLAN_COUNT_MAX;
         itemIndex++)
    {
      uint8_t eventIndex;

      for (eventIndex = 0U; eventIndex < INTERSECTION_DAY_PLAN_EVENT_COUNT_MAX;
           eventIndex++)
      {
        IntersectionDayPlanEventConfig_t *event =
          &config->globalTimeManagement.dayPlans[itemIndex][eventIndex];

        event->hour = payload[offset++];
        event->minute = payload[offset++];
        event->actionNumber = payload[offset++];
        event->reserved0 = 0U;
      }
    }

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_DAYLIGHT_SAVING_ENTRY_COUNT_MAX;
         itemIndex++)
    {
      IntersectionDaylightSavingEntryConfig_t *entry =
        &config->globalTimeManagement.daylightSavingEntries[itemIndex];

      entry->beginMonth = payload[offset++];
      entry->beginOccurrences = payload[offset++];
      entry->beginDayOfWeek = payload[offset++];
      entry->beginDayOfMonth = payload[offset++];
      entry->beginSecondsToTransition = ReadLe32(&payload[offset]);
      offset += 4U;
      entry->endMonth = payload[offset++];
      entry->endOccurrences = payload[offset++];
      entry->endDayOfWeek = payload[offset++];
      entry->endDayOfMonth = payload[offset++];
      entry->endSecondsToTransition = ReadLe32(&payload[offset]);
      offset += 4U;
      entry->secondsToAdjust = ReadLe32(&payload[offset]);
      offset += 4U;
    }

    config->globalTimeManagement.reserved0 = 0U;
    config->globalTimeManagement.reserved1 = 0U;
    config->globalTimeManagement.reserved2 = 0U;
  }

  if (currentOrLegacyV16Layout != 0U)
  {
    config->unit.startUpFlashSeconds = payload[offset++];
    config->unit.autoPedestrianClear = payload[offset++];
    config->unit.backupTimeSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    config->unit.redRevertDs = payload[offset++];
    config->unit.startUpFlashMode = payload[offset++];
    config->unit.timeSourceCommanded = payload[offset++];
    config->unit.elevationOffsetMeters = payload[offset++];
    config->unit.userDefinedBackupTimeSeconds = ReadLe32(&payload[offset]);
    offset += 4U;

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX;
         itemIndex++)
    {
      IntersectionUserDefinedBackupContentConfig_t *content =
        &config->userDefinedBackupContents[itemIndex];
      uint8_t oidIndex;
      uint8_t descriptionIndex;

      content->oidLength = payload[offset++];
      content->descriptionLength = payload[offset++];
      content->reserved0 = 0U;
      content->reserved1 = 0U;

      for (oidIndex = 0U;
           oidIndex < INTERSECTION_USER_DEFINED_BACKUP_OID_COMPONENT_COUNT_MAX;
           oidIndex++)
      {
        content->oid[oidIndex] = ReadLe32(&payload[offset]);
        offset += 4U;
      }

      for (descriptionIndex = 0U;
           descriptionIndex < INTERSECTION_USER_DEFINED_BACKUP_DESCRIPTION_MAX;
           descriptionIndex++)
      {
        content->description[descriptionIndex] = payload[offset++];
      }
    }
  }
  else if (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V14)
  {
    config->unit.startUpFlashSeconds = payload[offset++];
    config->unit.autoPedestrianClear = payload[offset++];
    config->unit.backupTimeSeconds = ReadLe16(&payload[offset]);
    offset += 2U;
    config->unit.redRevertDs = payload[offset++];
    config->unit.startUpFlashMode = payload[offset++];
    config->unit.timeSourceCommanded = payload[offset++];
    config->unit.elevationOffsetMeters = payload[offset++];
  }
  else if (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V13)
  {
    config->unit.timeSourceCommanded = payload[offset++];
    config->unit.elevationOffsetMeters = payload[offset++];
    offset += 2U;
  }

  if (payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE)
  {
    config->cabinetEnvironment.atccLedMode = payload[offset++];

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_CABINET_ENVIRONMENT_DEVICE_COUNT_MAX;
         itemIndex++)
    {
      uint8_t descriptionIndex;
      IntersectionCabinetEnvironmentDeviceConfig_t *device =
        &config->cabinetEnvironment.devices[itemIndex];

      device->type = payload[offset++];

      for (descriptionIndex = 0U;
           descriptionIndex < INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX;
           descriptionIndex++)
      {
        device->description[descriptionIndex] = payload[offset++];
      }
    }

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_CABINET_TEMP_SENSOR_COUNT_MAX;
         itemIndex++)
    {
      uint8_t descriptionIndex;
      IntersectionCabinetTemperatureSensorConfig_t *sensor =
        &config->cabinetEnvironment.temperatureSensors[itemIndex];

      for (descriptionIndex = 0U;
           descriptionIndex < INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX;
           descriptionIndex++)
      {
        sensor->description[descriptionIndex] = payload[offset++];
      }

      sensor->highThreshold = (int8_t) payload[offset++];
      sensor->lowThreshold = (int8_t) payload[offset++];
    }

    for (itemIndex = 0U;
         itemIndex < INTERSECTION_CABINET_HUMIDITY_SENSOR_COUNT_MAX;
         itemIndex++)
    {
      uint8_t descriptionIndex;
      IntersectionCabinetHumiditySensorConfig_t *sensor =
        &config->cabinetEnvironment.humiditySensors[itemIndex];

      for (descriptionIndex = 0U;
           descriptionIndex < INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX;
           descriptionIndex++)
      {
        sensor->description[descriptionIndex] = payload[offset++];
      }

      sensor->threshold = payload[offset++];
    }
  }

  if (multiSequenceLayout == 0U)
  {
    for (detectorIndex = 0U;
         detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
         detectorIndex++)
    {
      config->vehicleDetectors[detectorIndex].options =
        NormalizeVehicleDetectorOptions(
          config->vehicleDetectors[detectorIndex].options);
    }
  }

  if (multiSequenceLayout == 0U)
  {
    ReplicateBaseSequencePlans(config);
  }

  RebuildLegacyInputMapping(config);
} /* PayloadDeserialize */

static void HeaderEncode(const ConfigurationImageHeader_t *header,
                         uint8_t *bytes)
{
  WriteLe32(&bytes[0], header->magic);
  WriteLe32(&bytes[4], header->schemaVersion);
  WriteLe32(&bytes[8], header->payloadLength);
  WriteLe32(&bytes[12], header->generation);
  WriteLe32(&bytes[16], header->payloadCrc32);
  WriteLe32(&bytes[20], header->headerCrc32);
  WriteLe32(&bytes[24], header->state);
  WriteLe32(&bytes[28], header->migrationSourceVersion);
  WriteLe32(&bytes[32], header->migrationSourceCrc32);
  WriteLe32(&bytes[36], header->reserved);
}

static void HeaderDecode(const uint8_t *bytes,
                         ConfigurationImageHeader_t *header)
{
  header->magic = ReadLe32(&bytes[0]);
  header->schemaVersion = ReadLe32(&bytes[4]);
  header->payloadLength = ReadLe32(&bytes[8]);
  header->generation = ReadLe32(&bytes[12]);
  header->payloadCrc32 = ReadLe32(&bytes[16]);
  header->headerCrc32 = ReadLe32(&bytes[20]);
  header->state = ReadLe32(&bytes[24]);
  header->migrationSourceVersion = ReadLe32(&bytes[28]);
  header->migrationSourceCrc32 = ReadLe32(&bytes[32]);
  header->reserved = ReadLe32(&bytes[36]);
}

static uint32_t HeaderCrc(const ConfigurationImageHeader_t *header)
{
  uint8_t bytes[CONFIGURATION_IMAGE_HEADER_SIZE];

  HeaderEncode(header, bytes);
  memset(&bytes[20], 0, 8U);

  return Crc32Compute(bytes, CONFIGURATION_IMAGE_HEADER_SIZE);
}

static void JournalEncode(const ConfigurationMigrationJournal_t *journal,
                          uint8_t *bytes)
{
  WriteLe32(&bytes[0], journal->magic);
  WriteLe32(&bytes[4], journal->state);
  WriteLe32(&bytes[8], journal->sourceVersion);
  WriteLe32(&bytes[12], journal->sourceCrc32);
  WriteLe32(&bytes[16], journal->targetSlot);
  WriteLe32(&bytes[20], journal->targetGeneration);
}

static void JournalDecode(const uint8_t *bytes,
                          ConfigurationMigrationJournal_t *journal)
{
  journal->magic = ReadLe32(&bytes[0]);
  journal->state = ReadLe32(&bytes[4]);
  journal->sourceVersion = ReadLe32(&bytes[8]);
  journal->sourceCrc32 = ReadLe32(&bytes[12]);
  journal->targetSlot = ReadLe32(&bytes[16]);
  journal->targetGeneration = ReadLe32(&bytes[20]);
}

static ConfigRepositoryObjectId_t SlotToObjectId(ConfigurationSlotId_t slotId)
{
  switch (slotId)
  {
      case CONFIGURATION_SLOT_A:
      {
        return CONFIG_REPOSITORY_OBJECT_SLOT_A;
      }

      case CONFIGURATION_SLOT_B:
      {
        return CONFIG_REPOSITORY_OBJECT_SLOT_B;
      }

      case CONFIGURATION_SLOT_NONE:
      default:
      {
        return CONFIG_REPOSITORY_OBJECT_NONE;
      }
  }
}

static uint8_t HeaderBytesAreBlank(const uint8_t *bytes)
{
  uint32_t i;

  for (i = 0U; i < CONFIGURATION_IMAGE_HEADER_SIZE; i++)
  {
    if (bytes[i] != 0xFFU)
    {
      return 0U;
    }
  }

  return 1U;
}

static uint8_t MaterializeCandidate(const ConfigurationService_t *service,
                                    IntersectionConfig_t *config)
{
  uint8_t detectorIndex;
  uint8_t pedDetectorIndex;
  uint8_t gateIndex;
  uint8_t contentIndex;
  uint8_t phaseIndex;
  uint8_t sequenceIndex;
  uint8_t ringIndex;
  uint8_t preemptIndex;
  uint8_t channelIndex;
  uint8_t overlapIndex;

  if ((service == NULL) || (config == NULL))
  {
    return 0U;
  }

  *config = service->activeConfig;

  if (service->candidate.phaseCountValid != 0U)
  {
    config->phaseCount = service->candidate.phaseCount;
  }

  if (service->candidate.ringCountValid != 0U)
  {
    config->ringCount = service->candidate.ringCount;
  }

  if (service->candidate.barrierCountValid != 0U)
  {
    config->barrierCount = service->candidate.barrierCount;
  }

  if (service->candidate.coordinationValid != 0U)
  {
    config->coordination = service->candidate.coordination;
  }

  if (service->candidate.timebaseValid != 0U)
  {
    config->timebase = service->candidate.timebase;
  }

  if (service->candidate.globalTimeManagementValid != 0U)
  {
    config->globalTimeManagement = service->candidate.globalTimeManagement;
  }

  if (service->candidate.cabinetEnvironmentValid != 0U)
  {
    config->cabinetEnvironment = service->candidate.cabinetEnvironment;
  }

  if (service->candidate.unitValid != 0U)
  {
    config->unit = service->candidate.unit;
  }

  if (service->candidate.inputMappingValid != 0U)
  {
    config->inputMapping = service->candidate.inputMapping;
  }

  if (service->candidate.detectorReportsValid != 0U)
  {
    config->detectorReports = service->candidate.detectorReports;
  }

  for (preemptIndex = 0U;
       preemptIndex < INTERSECTION_PREEMPT_COUNT_MAX;
       preemptIndex++)
  {
    if (service->candidate.preemptValid[preemptIndex] != 0U)
    {
      config->preempts[preemptIndex] =
        service->candidate.preempts[preemptIndex];
    }

    for (detectorIndex = 0U;
         detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
         detectorIndex++)
    {
      if (service->candidate.preemptQueueDelayWeightValid[preemptIndex][
            detectorIndex]
          != 0U)
      {
        config->preemptQueueDelayWeights[preemptIndex][detectorIndex] =
          service->candidate.preemptQueueDelayWeights[preemptIndex][
            detectorIndex];
      }
    }
  }

  for (gateIndex = 0U;
       gateIndex < INTERSECTION_PREEMPT_GATE_COUNT_MAX;
       gateIndex++)
  {
    if (service->candidate.preemptGateValid[gateIndex] != 0U)
    {
      config->preemptGates[gateIndex] =
        service->candidate.preemptGates[gateIndex];
    }
  }

  for (contentIndex = 0U;
       contentIndex < INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX;
       contentIndex++)
  {
    if (service->candidate.userDefinedBackupContentValid[contentIndex] != 0U)
    {
      config->userDefinedBackupContents[contentIndex] =
        service->candidate.userDefinedBackupContents[contentIndex];
    }
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    if (service->candidate.vehicleDetectorValid[detectorIndex] != 0U)
    {
      config->vehicleDetectors[detectorIndex] =
        service->candidate.vehicleDetectors[detectorIndex];
    }
  }

  for (pedDetectorIndex = 0U;
       pedDetectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       pedDetectorIndex++)
  {
    if (service->candidate.pedestrianDetectorValid[pedDetectorIndex] != 0U)
    {
      config->pedestrianDetectors[pedDetectorIndex] =
        service->candidate.pedestrianDetectors[pedDetectorIndex];
    }
  }

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    if (service->candidate.phaseValid[phaseIndex] != 0U)
    {
      config->phases[phaseIndex] = service->candidate.phases[phaseIndex];
    }
  }

  for (sequenceIndex = 0U;
       sequenceIndex < INTERSECTION_SEQUENCE_COUNT_MAX;
       sequenceIndex++)
  {
    for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
    {
      if (service->candidate.sequencePlanValid[sequenceIndex][ringIndex] != 0U)
      {
        config->sequencePlans[sequenceIndex][ringIndex] =
          service->candidate.sequencePlans[sequenceIndex][ringIndex];
      }
    }
  }

  for (channelIndex = 0U;
       channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    if (service->candidate.channelValid[channelIndex] != 0U)
    {
      config->channels[channelIndex] =
        service->candidate.channels[channelIndex];
    }
  }

  for (overlapIndex = 0U;
       overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX;
       overlapIndex++)
  {
    if (service->candidate.overlapValid[overlapIndex] != 0U)
    {
      config->overlaps[overlapIndex] =
        service->candidate.overlaps[overlapIndex];
    }
  }

  RebuildLegacyInputMapping(config);

  return 1U;
} /* MaterializeCandidate */

static uint8_t LoadSlot(ConfigurationService_t *service,
                        ConfigurationSlotId_t slotId,
                        LoadedSlot_t *loadedSlot)
{
  uint8_t headerBytes[CONFIGURATION_IMAGE_HEADER_SIZE];
  uint8_t payloadBytes[CONFIGURATION_IMAGE_PAYLOAD_SIZE];
  ConfigurationImageHeader_t header;
  IntersectionConfigErrorInfo_t errorInfo;
  ConfigRepositoryObjectId_t objectId = SlotToObjectId(slotId);
  uint32_t capacity;

  memset(loadedSlot, 0, sizeof(*loadedSlot));
  loadedSlot->slotId = slotId;

  if ((service->repositoryPort == NULL)
      || (objectId == CONFIG_REPOSITORY_OBJECT_NONE))
  {
    return 0U;
  }

  capacity = ConfigRepositoryGetCapacity(service->repositoryPort, objectId);

  if (capacity
      < (CONFIGURATION_IMAGE_HEADER_SIZE
         + CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V2))
  {
    return 0U;
  }

  if (ConfigRepositoryRead(service->repositoryPort,
                           objectId,
                           0U,
                           headerBytes,
                           sizeof(headerBytes)) == 0U)
  {
    return 0U;
  }

  if (HeaderBytesAreBlank(headerBytes) != 0U)
  {
    return 0U;
  }

  HeaderDecode(headerBytes, &header);

  if ((header.magic != CONFIGURATION_IMAGE_MAGIC)
      || (header.state != CONFIGURATION_SLOT_STATE_VALID)
      || (HeaderCrc(&header) != header.headerCrc32))
  {
    return 0U;
  }

  if (!(((header.schemaVersion == CONFIGURATION_IMAGE_SCHEMA_VERSION)
         && (header.payloadLength == CONFIGURATION_IMAGE_PAYLOAD_SIZE))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V18)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V18))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V17)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V17))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V16)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V16))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V15)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V16))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V14)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V14))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V13)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V13))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V12)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V12))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V11)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V11))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V10)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V10))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V9)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V9))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V8)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V8))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V7)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V7))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V6)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V6))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V4)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V4))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V5)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V5))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V3)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V3))
        || ((header.schemaVersion
             == CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V2)
            && (header.payloadLength
                == CONFIGURATION_IMAGE_PAYLOAD_SIZE_LEGACY_V2))))
  {
    return 0U;
  }

  if (ConfigRepositoryRead(service->repositoryPort,
                           objectId,
                           CONFIGURATION_IMAGE_HEADER_SIZE,
                           payloadBytes,
                           header.payloadLength) == 0U)
  {
    return 0U;
  }

  if (Crc32Compute(payloadBytes, header.payloadLength) != header.payloadCrc32)
  {
    return 0U;
  }

  PayloadDeserialize(payloadBytes, header.payloadLength, &loadedSlot->config);

  if (header.schemaVersion < CONFIGURATION_IMAGE_SCHEMA_VERSION_LEGACY_V16)
  {
    loadedSlot->config.unit.elevationOffsetMeters =
      INTERSECTION_UNIT_ELEVATION_OFFSET_UNKNOWN;
  }

  if (IntersectionConfigValidate(&loadedSlot->config, &errorInfo) == 0U)
  {
    return 0U;
  }

  loadedSlot->generation = header.generation;
  loadedSlot->valid = 1U;

  return 1U;
} /* LoadSlot */

static ConfigurationSlotId_t SelectCommitTarget(
  const ConfigurationService_t *service)
{
  if (service->activeSlot == CONFIGURATION_SLOT_A)
  {
    return CONFIGURATION_SLOT_B;
  }

  if (service->activeSlot == CONFIGURATION_SLOT_B)
  {
    return CONFIGURATION_SLOT_A;
  }

  return CONFIGURATION_SLOT_B;
}

static uint8_t UpdatePhase(ConfigurationService_t *service,
                           uint8_t phaseIndex,
                           const IntersectionPhaseConfig_t *phase)
{
  if ((service == NULL) || (phase == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  service->candidate.phaseValid[phaseIndex] = 1U;
  service->candidate.phases[phaseIndex] = *phase;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdateChannel(ConfigurationService_t *service,
                             uint8_t channelIndex,
                             const IntersectionChannelConfig_t *channel)
{
  if ((service == NULL) || (channel == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if (channelIndex >= INTERSECTION_CHANNEL_COUNT_MAX)
  {
    return 0U;
  }

  service->candidate.channelValid[channelIndex] = 1U;
  service->candidate.channels[channelIndex] = *channel;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdateVehicleDetector(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  const IntersectionVehicleDetectorConfig_t *detector)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (detector == NULL)
      || (service->candidate.inUse == 0U)
      || (detectorIndex >= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
      || (MaterializeCandidate(service, &candidateConfig) == 0U))
  {
    return 0U;
  }

  candidateConfig.vehicleDetectors[detectorIndex] = *detector;
  RebuildLegacyInputMapping(&candidateConfig);
  service->candidate.vehicleDetectorValid[detectorIndex] = 1U;
  service->candidate.vehicleDetectors[detectorIndex] =
    candidateConfig.vehicleDetectors[detectorIndex];
  service->candidate.inputMappingValid = 1U;
  service->candidate.inputMapping = candidateConfig.inputMapping;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdatePedestrianDetector(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  const IntersectionPedestrianDetectorConfig_t *detector)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (detector == NULL)
      || (service->candidate.inUse == 0U)
      || (detectorIndex >= INTERSECTION_PED_INPUT_COUNT_MAX)
      || (MaterializeCandidate(service, &candidateConfig) == 0U))
  {
    return 0U;
  }

  candidateConfig.pedestrianDetectors[detectorIndex] = *detector;
  RebuildLegacyInputMapping(&candidateConfig);
  service->candidate.pedestrianDetectorValid[detectorIndex] = 1U;
  service->candidate.pedestrianDetectors[detectorIndex] =
    candidateConfig.pedestrianDetectors[detectorIndex];
  service->candidate.inputMappingValid = 1U;
  service->candidate.inputMapping = candidateConfig.inputMapping;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdateOverlap(ConfigurationService_t *service,
                             uint8_t overlapIndex,
                             const IntersectionOverlapConfig_t *overlap)
{
  if ((service == NULL) || (overlap == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if (overlapIndex >= INTERSECTION_OVERLAP_COUNT_MAX)
  {
    return 0U;
  }

  service->candidate.overlapValid[overlapIndex] = 1U;
  service->candidate.overlaps[overlapIndex] = *overlap;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdateCoordination(ConfigurationService_t *service,
                                  const IntersectionCoordinationConfig_t *
                                  coordination)
{
  if ((service == NULL) || (coordination == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  service->candidate.coordinationValid = 1U;
  service->candidate.coordination = *coordination;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdateTimebase(ConfigurationService_t *service,
                              const IntersectionTimebaseConfig_t *timebase)
{
  if ((service == NULL) || (timebase == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  service->candidate.timebaseValid = 1U;
  service->candidate.timebase = *timebase;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdateGlobalTimeManagement(
  ConfigurationService_t *service,
  const IntersectionGlobalTimeManagementConfig_t *globalTimeManagement)
{
  if ((service == NULL) || (globalTimeManagement == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  service->candidate.globalTimeManagementValid = 1U;
  service->candidate.globalTimeManagement = *globalTimeManagement;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdateCabinetEnvironment(
  ConfigurationService_t *service,
  const IntersectionCabinetEnvironmentConfig_t *cabinetEnvironment)
{
  if ((service == NULL) || (cabinetEnvironment == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  service->candidate.cabinetEnvironmentValid = 1U;
  service->candidate.cabinetEnvironment = *cabinetEnvironment;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdateUnit(ConfigurationService_t *service,
                          const IntersectionUnitConfig_t *unit)
{
  if ((service == NULL) || (unit == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  service->candidate.unitValid = 1U;
  service->candidate.unit = *unit;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdateUserDefinedBackupContent(
  ConfigurationService_t *service,
  uint8_t contentIndex,
  const IntersectionUserDefinedBackupContentConfig_t *content)
{
  if ((service == NULL) || (content == NULL)
      || (service->candidate.inUse == 0U)
      || (contentIndex >= INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX))
  {
    return 0U;
  }

  service->candidate.userDefinedBackupContentValid[contentIndex] = 1U;
  service->candidate.userDefinedBackupContents[contentIndex] = *content;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdateInputMapping(ConfigurationService_t *service,
                                  const IntersectionInputMappingConfig_t *
                                  inputMapping)
{
  if ((service == NULL) || (inputMapping == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  service->candidate.inputMappingValid = 1U;
  service->candidate.inputMapping = *inputMapping;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdatePreempt(ConfigurationService_t *service,
                             uint8_t preemptIndex,
                             const IntersectionPreemptConfig_t *preempt)
{
  if ((service == NULL) || (preempt == NULL)
      || (service->candidate.inUse == 0U)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  service->candidate.preemptValid[preemptIndex] = 1U;
  service->candidate.preempts[preemptIndex] = *preempt;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdatePreemptQueueDelayWeight(ConfigurationService_t *service,
                                             uint8_t preemptIndex,
                                             uint8_t detectorIndex,
                                             uint16_t detectorWeight)
{
  if ((service == NULL) || (service->candidate.inUse == 0U)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (detectorIndex >= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  service->candidate.preemptQueueDelayWeightValid[preemptIndex][detectorIndex] =
    1U;
  service->candidate.preemptQueueDelayWeights[preemptIndex][detectorIndex] =
    detectorWeight;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdatePreemptGate(ConfigurationService_t *service,
                                 uint8_t gateIndex,
                                 const IntersectionPreemptGateConfig_t *gate)
{
  if ((service == NULL) || (gate == NULL)
      || (service->candidate.inUse == 0U)
      || (gateIndex >= INTERSECTION_PREEMPT_GATE_COUNT_MAX))
  {
    return 0U;
  }

  service->candidate.preemptGateValid[gateIndex] = 1U;
  service->candidate.preemptGates[gateIndex] = *gate;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t UpdateRingPlan(ConfigurationService_t *service,
                              uint8_t sequenceNumber,
                              uint8_t ringIndex,
                              const IntersectionRingPlan_t *ringPlan)
{
  uint8_t sequenceIndex = 0U;

  if ((service == NULL) || (ringPlan == NULL)
      || (service->candidate.inUse == 0U)
      || (ringIndex >= service->activeConfig.ringCount)
      || (SequenceNumberToIndex(sequenceNumber, &sequenceIndex) == 0U))
  {
    return 0U;
  }

  service->candidate.sequencePlanValid[sequenceIndex][ringIndex] = 1U;
  service->candidate.sequencePlans[sequenceIndex][ringIndex] = *ringPlan;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t RingPlanFindPhase(const IntersectionRingPlan_t *ringPlan,
                                 uint8_t phaseIndex,
                                 uint8_t *position)
{
  uint8_t serviceIndex;

  if ((ringPlan == NULL) || (position == NULL))
  {
    return 0U;
  }

  for (serviceIndex = 0U; serviceIndex < ringPlan->phaseCount; serviceIndex++)
  {
    if (ringPlan->phaseOrder[serviceIndex] == phaseIndex)
    {
      *position = serviceIndex;

      return 1U;
    }
  }

  return 0U;
}

static uint8_t RingPlanRemovePhase(IntersectionRingPlan_t *ringPlan,
                                   uint8_t phaseIndex)
{
  uint8_t position;
  uint8_t serviceIndex;

  if ((ringPlan == NULL) || (RingPlanFindPhase(ringPlan, phaseIndex,
                                               &position) == 0U))
  {
    return 1U;
  }

  for (serviceIndex = position;
       serviceIndex + 1U < ringPlan->phaseCount;
       serviceIndex++)
  {
    ringPlan->phaseOrder[serviceIndex] = ringPlan->phaseOrder[serviceIndex
                                                              + 1U];
  }

  if (ringPlan->phaseCount != 0U)
  {
    ringPlan->phaseCount--;
  }

  if (ringPlan->barrierPhaseCount > ringPlan->phaseCount)
  {
    ringPlan->barrierPhaseCount = ringPlan->phaseCount;
  }

  return 1U;
}

static uint8_t RingPlanAppendPhase(IntersectionRingPlan_t *ringPlan,
                                   uint8_t phaseIndex)
{
  uint8_t existingPosition;

  if (ringPlan == NULL)
  {
    return 0U;
  }

  if (RingPlanFindPhase(ringPlan, phaseIndex, &existingPosition) != 0U)
  {
    return 1U;
  }

  if (ringPlan->phaseCount >= INTERSECTION_RING_PHASE_COUNT_MAX)
  {
    return 0U;
  }

  ringPlan->phaseOrder[ringPlan->phaseCount] = phaseIndex;
  ringPlan->phaseCount++;

  if (ringPlan->barrierPhaseCount == 0U)
  {
    ringPlan->barrierPhaseCount = 1U;
  }

  return 1U;
}

static uint8_t CommitPhaseMutation(ConfigurationService_t *service,
                                   uint8_t phaseIndex,
                                   const IntersectionConfig_t *candidateConfig,
                                   uint8_t ringIndexA,
                                   uint8_t ringIndexB)
{
  uint8_t sequenceIndex;

  if ((service == NULL) || (candidateConfig == NULL))
  {
    return 0U;
  }

  service->candidate.phaseValid[phaseIndex] = 1U;
  service->candidate.phases[phaseIndex] = candidateConfig->phases[phaseIndex];

  for (sequenceIndex = 0U;
       sequenceIndex < INTERSECTION_SEQUENCE_COUNT_MAX;
       sequenceIndex++)
  {
    if (ringIndexA < INTERSECTION_RING_COUNT_MAX)
    {
      service->candidate.sequencePlanValid[sequenceIndex][ringIndexA] = 1U;
      service->candidate.sequencePlans[sequenceIndex][ringIndexA] =
        candidateConfig->sequencePlans[sequenceIndex][ringIndexA];
    }

    if ((ringIndexB < INTERSECTION_RING_COUNT_MAX) && (ringIndexB != ringIndexA))
    {
      service->candidate.sequencePlanValid[sequenceIndex][ringIndexB] = 1U;
      service->candidate.sequencePlans[sequenceIndex][ringIndexB] =
        candidateConfig->sequencePlans[sequenceIndex][ringIndexB];
    }
  }

  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

void ConfigurationServiceInit(ConfigurationService_t *service,
                              IConfigRepositoryPort_t *repositoryPort)
{
  if (service == NULL)
  {
    return;
  }

  memset(service, 0, sizeof(*service));
  service->repositoryPort = repositoryPort;
  IntersectionConfigInitDefaults(&service->activeConfig);
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);
  CandidateClear(service);
  (void) ConfigurationServiceLoad(service);
}

uint8_t ConfigurationServiceLoad(ConfigurationService_t *service)
{
  LoadedSlot_t slotA;
  LoadedSlot_t slotB;
  const LoadedSlot_t *selectedSlot = NULL;

  if (service == NULL)
  {
    return 0U;
  }

  CandidateClear(service);
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  if ((LoadSlot(service, CONFIGURATION_SLOT_A, &slotA) != 0U)
      && ((selectedSlot == NULL)
          || (slotA.generation > selectedSlot->generation)))
  {
    selectedSlot = &slotA;
  }

  if ((LoadSlot(service, CONFIGURATION_SLOT_B, &slotB) != 0U)
      && ((selectedSlot == NULL)
          || (slotB.generation > selectedSlot->generation)))
  {
    selectedSlot = &slotB;
  }

  if (selectedSlot == NULL)
  {
    IntersectionConfigInitDefaults(&service->activeConfig);
    service->activeGeneration = 0U;
    service->activeSlot = CONFIGURATION_SLOT_NONE;
    service->loadedFromRepository = 0U;

    return 0U;
  }

  service->activeConfig = selectedSlot->config;
  service->activeGeneration = selectedSlot->generation;
  service->activeSlot = selectedSlot->slotId;
  service->loadedFromRepository = 1U;

  return 1U;
} /* ConfigurationServiceLoad */

const IntersectionConfig_t *ConfigurationServiceGetActiveConfig(
  const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return NULL;
  }

  return &service->activeConfig;
}

uint8_t ConfigurationServiceGetActivePhaseConfig(
  const ConfigurationService_t *service,
  uint8_t phaseIndex,
  IntersectionPhaseConfig_t *
  phaseConfig)
{
  if ((service == NULL) || (phaseConfig == NULL)
      || (phaseIndex >= service->activeConfig.phaseCount))
  {
    return 0U;
  }

  *phaseConfig = service->activeConfig.phases[phaseIndex];

  return 1U;
}

uint8_t ConfigurationServiceGetPhaseCount(const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  return service->activeConfig.phaseCount;
}

uint8_t ConfigurationServiceGetRingCount(const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  return service->activeConfig.ringCount;
}

uint8_t ConfigurationServiceGetSequenceCount(
  const ConfigurationService_t *service)
{
  (void) service;

  return INTERSECTION_SEQUENCE_COUNT_MAX;
}

uint8_t ConfigurationServiceGetActiveRingPlan(
  const ConfigurationService_t *service,
  uint8_t ringIndex,
  IntersectionRingPlan_t *ringPlan)
{
  return ConfigurationServiceGetActiveSequenceRingPlan(service,
                                                       1U,
                                                       ringIndex,
                                                       ringPlan);
}

uint8_t ConfigurationServiceGetActiveSequenceRingPlan(
  const ConfigurationService_t *service,
  uint8_t sequenceNumber,
  uint8_t ringIndex,
  IntersectionRingPlan_t *ringPlan)
{
  uint8_t sequenceIndex = 0U;

  if ((service == NULL) || (ringPlan == NULL)
      || (ringIndex >= service->activeConfig.ringCount)
      || (SequenceNumberToIndex(sequenceNumber, &sequenceIndex) == 0U))
  {
    return 0U;
  }

  *ringPlan = service->activeConfig.sequencePlans[sequenceIndex][ringIndex];

  return 1U;
}

uint8_t ConfigurationServiceGetCandidateRingPlan(
  ConfigurationService_t *service,
  uint8_t ringIndex,
  IntersectionRingPlan_t *ringPlan)
{
  return GetCandidateRingPlan(service, ringIndex, ringPlan);
}

uint8_t ConfigurationServiceGetCandidateSequenceRingPlan(
  ConfigurationService_t *service,
  uint8_t sequenceNumber,
  uint8_t ringIndex,
  IntersectionRingPlan_t *ringPlan)
{
  return GetCandidateSequenceRingPlan(service,
                                      sequenceNumber,
                                      ringIndex,
                                      ringPlan);
}

uint8_t ConfigurationServiceGetCandidateGlobalTimeManagementConfig(
  ConfigurationService_t *service,
  IntersectionGlobalTimeManagementConfig_t *globalTimeManagementConfig)
{
  return GetCandidateGlobalTimeManagement(service, globalTimeManagementConfig);
}

uint8_t ConfigurationServiceGetCandidateCabinetEnvironmentConfig(
  ConfigurationService_t *service,
  IntersectionCabinetEnvironmentConfig_t *cabinetEnvironmentConfig)
{
  return GetCandidateCabinetEnvironment(service, cabinetEnvironmentConfig);
}

uint8_t ConfigurationServiceGetActiveChannelConfig(
  const ConfigurationService_t *service,
  uint8_t channelIndex,
  IntersectionChannelConfig_t *
  channelConfig)
{
  if ((service == NULL) || (channelConfig == NULL)
      || (channelIndex >= INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  *channelConfig = service->activeConfig.channels[channelIndex];

  return 1U;
}

uint8_t ConfigurationServiceGetChannelCount(
  const ConfigurationService_t *service)
{
  (void) service;

  return INTERSECTION_CHANNEL_COUNT_MAX;
}

uint8_t ConfigurationServiceGetActiveOverlapConfig(
  const ConfigurationService_t *service,
  uint8_t overlapIndex,
  IntersectionOverlapConfig_t *
  overlapConfig)
{
  if ((service == NULL) || (overlapConfig == NULL)
      || (overlapIndex >= INTERSECTION_OVERLAP_COUNT_MAX))
  {
    return 0U;
  }

  *overlapConfig = service->activeConfig.overlaps[overlapIndex];

  return 1U;
}

uint8_t ConfigurationServiceGetOverlapCount(
  const ConfigurationService_t *service)
{
  (void) service;

  return INTERSECTION_OVERLAP_COUNT_MAX;
}

uint8_t ConfigurationServiceGetActiveCoordinationConfig(
  const ConfigurationService_t *service,
  IntersectionCoordinationConfig_t
  *coordinationConfig)
{
  if ((service == NULL) || (coordinationConfig == NULL))
  {
    return 0U;
  }

  *coordinationConfig = service->activeConfig.coordination;

  return 1U;
}

uint8_t ConfigurationServiceGetActiveTimebaseConfig(
  const ConfigurationService_t *service,
  IntersectionTimebaseConfig_t *timebaseConfig)
{
  if ((service == NULL) || (timebaseConfig == NULL))
  {
    return 0U;
  }

  *timebaseConfig = service->activeConfig.timebase;

  return 1U;
}

uint8_t ConfigurationServiceGetActiveGlobalTimeManagementConfig(
  const ConfigurationService_t *service,
  IntersectionGlobalTimeManagementConfig_t *globalTimeManagementConfig)
{
  if ((service == NULL) || (globalTimeManagementConfig == NULL))
  {
    return 0U;
  }

  *globalTimeManagementConfig = service->activeConfig.globalTimeManagement;

  return 1U;
}

uint8_t ConfigurationServiceGetActiveCabinetEnvironmentConfig(
  const ConfigurationService_t *service,
  IntersectionCabinetEnvironmentConfig_t *cabinetEnvironmentConfig)
{
  if ((service == NULL) || (cabinetEnvironmentConfig == NULL))
  {
    return 0U;
  }

  *cabinetEnvironmentConfig = service->activeConfig.cabinetEnvironment;

  return 1U;
}

uint8_t ConfigurationServiceGetActiveUnitConfig(
  const ConfigurationService_t *service,
  IntersectionUnitConfig_t *unitConfig)
{
  if ((service == NULL) || (unitConfig == NULL))
  {
    return 0U;
  }

  *unitConfig = service->activeConfig.unit;

  return 1U;
}

uint8_t ConfigurationServiceGetUserDefinedBackupContentCount(
  const ConfigurationService_t *service)
{
  (void) service;

  return INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX;
}

uint8_t ConfigurationServiceGetActiveUserDefinedBackupContentConfig(
  const ConfigurationService_t *service,
  uint8_t contentIndex,
  IntersectionUserDefinedBackupContentConfig_t *contentConfig)
{
  if ((service == NULL) || (contentConfig == NULL)
      || (contentIndex >= INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX))
  {
    return 0U;
  }

  *contentConfig = service->activeConfig.userDefinedBackupContents[contentIndex];

  return 1U;
}

uint8_t ConfigurationServiceMatchesUserDefinedBackupContentOid(
  const ConfigurationService_t *service,
  const uint32_t *oid,
  uint8_t oidLength)
{
  uint8_t contentIndex;
  uint8_t oidIndex;

  if ((service == NULL) || (oid == NULL) || (oidLength == 0U))
  {
    return 0U;
  }

  for (contentIndex = 0U;
       contentIndex < INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX;
       contentIndex++)
  {
    const IntersectionUserDefinedBackupContentConfig_t *content =
      &service->activeConfig.userDefinedBackupContents[contentIndex];

    if (content->oidLength != oidLength)
    {
      continue;
    }

    for (oidIndex = 0U; oidIndex < oidLength; oidIndex++)
    {
      if (content->oid[oidIndex] != oid[oidIndex])
      {
        break;
      }
    }

    if (oidIndex == oidLength)
    {
      return 1U;
    }
  }

  return 0U;
}

uint8_t ConfigurationServiceGetActivePatternConfig(
  const ConfigurationService_t *service,
  uint8_t patternIndex,
  IntersectionPatternConfig_t *
  patternConfig)
{
  if ((service == NULL) || (patternConfig == NULL)
      || (patternIndex >= INTERSECTION_PATTERN_COUNT_MAX))
  {
    return 0U;
  }

  *patternConfig = service->activeConfig.coordination.patterns[patternIndex];

  return 1U;
}

uint8_t ConfigurationServiceGetPatternCount(
  const ConfigurationService_t *service)
{
  (void) service;

  return INTERSECTION_PATTERN_COUNT_MAX;
}

uint8_t ConfigurationServiceGetActiveSplitPhaseConfig(
  const ConfigurationService_t *service,
  uint8_t splitIndex,
  uint8_t phaseIndex,
  IntersectionSplitPhaseConfig_t
  *splitConfig)
{
  if ((service == NULL) || (splitConfig == NULL)
      || (splitIndex >= INTERSECTION_SPLIT_COUNT_MAX)
      || (phaseIndex >= service->activeConfig.phaseCount))
  {
    return 0U;
  }

  *splitConfig =
    service->activeConfig.coordination.splits[splitIndex][phaseIndex];

  return 1U;
}

uint8_t ConfigurationServiceGetSplitCount(const ConfigurationService_t *service)
{
  (void) service;

  return INTERSECTION_SPLIT_COUNT_MAX;
}

uint8_t ConfigurationServiceGetActiveTimebaseActionConfig(
  const ConfigurationService_t *service,
  uint8_t actionIndex,
  IntersectionTimebaseActionConfig_t *actionConfig)
{
  if ((service == NULL) || (actionConfig == NULL)
      || (actionIndex >= INTERSECTION_TIMEBASE_ACTION_COUNT_MAX))
  {
    return 0U;
  }

  *actionConfig = service->activeConfig.timebase.actions[actionIndex];

  return 1U;
}

uint8_t ConfigurationServiceGetTimebaseActionCount(
  const ConfigurationService_t *service)
{
  (void) service;

  return INTERSECTION_TIMEBASE_ACTION_COUNT_MAX;
}

uint8_t ConfigurationServiceGetActivePreemptConfig(
  const ConfigurationService_t *service,
  uint8_t preemptIndex,
  IntersectionPreemptConfig_t *
  preemptConfig)
{
  if ((service == NULL) || (preemptConfig == NULL)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  *preemptConfig = service->activeConfig.preempts[preemptIndex];

  return 1U;
}

uint8_t ConfigurationServiceGetPreemptCount(
  const ConfigurationService_t *service)
{
  (void) service;

  return INTERSECTION_PREEMPT_COUNT_MAX;
}

uint8_t ConfigurationServiceGetVehicleDetectorCount(
  const ConfigurationService_t *service)
{
  (void) service;

  return INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
}

uint8_t ConfigurationServiceGetPedestrianDetectorCount(
  const ConfigurationService_t *service)
{
  (void) service;

  return INTERSECTION_PED_INPUT_COUNT_MAX;
}

uint8_t ConfigurationServiceGetActiveVehicleDetectorConfig(
  const ConfigurationService_t *service,
  uint8_t detectorIndex,
  IntersectionVehicleDetectorConfig_t *detectorConfig)
{
  if ((service == NULL) || (detectorConfig == NULL)
      || (detectorIndex >= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  *detectorConfig = service->activeConfig.vehicleDetectors[detectorIndex];

  return 1U;
}

uint8_t ConfigurationServiceGetActivePedestrianDetectorConfig(
  const ConfigurationService_t *service,
  uint8_t detectorIndex,
  IntersectionPedestrianDetectorConfig_t *detectorConfig)
{
  if ((service == NULL) || (detectorConfig == NULL)
      || (detectorIndex >= INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  *detectorConfig = service->activeConfig.pedestrianDetectors[detectorIndex];

  return 1U;
}

uint8_t ConfigurationServiceGetActiveDetectorReportConfig(
  const ConfigurationService_t *service,
  IntersectionDetectorReportConfig_t *detectorReportConfig)
{
  if ((service == NULL) || (detectorReportConfig == NULL))
  {
    return 0U;
  }

  *detectorReportConfig = service->activeConfig.detectorReports;

  return 1U;
}

uint8_t ConfigurationServiceGetActiveInputMappingConfig(
  const ConfigurationService_t *service,
  IntersectionInputMappingConfig_t
  *inputMapping)
{
  if ((service == NULL) || (inputMapping == NULL))
  {
    return 0U;
  }

  *inputMapping = service->activeConfig.inputMapping;

  return 1U;
}

uint8_t ConfigurationServiceGetVehicleDetectorCallPhase(
  const ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t *phaseNumber)
{
  if ((service == NULL) || (phaseNumber == NULL)
      || (detectorIndex >= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  *phaseNumber = service->activeConfig.vehicleDetectors[detectorIndex].callPhase;

  return 1U;
}

uint8_t ConfigurationServiceGetPedestrianDetectorCallPhase(
  const ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t *phaseNumber)
{
  if ((service == NULL) || (phaseNumber == NULL)
      || (detectorIndex >= INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  *phaseNumber =
    service->activeConfig.pedestrianDetectors[detectorIndex].callPhase;

  return 1U;
}

uint8_t ConfigurationServiceGetPreemptQueueDelayWeight(
  const ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t detectorIndex,
  uint16_t *detectorWeight)
{
  if ((service == NULL) || (detectorWeight == NULL)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (detectorIndex >= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  *detectorWeight =
    service->activeConfig.preemptQueueDelayWeights[preemptIndex][detectorIndex];

  return 1U;
}

uint8_t ConfigurationServiceGetPreemptGateCount(
  const ConfigurationService_t *service)
{
  (void) service;

  return INTERSECTION_PREEMPT_GATE_COUNT_MAX;
}

uint8_t ConfigurationServiceGetActivePreemptGateConfig(
  const ConfigurationService_t *service,
  uint8_t gateIndex,
  IntersectionPreemptGateConfig_t
  *gateConfig)
{
  if ((service == NULL) || (gateConfig == NULL)
      || (gateIndex >= INTERSECTION_PREEMPT_GATE_COUNT_MAX))
  {
    return 0U;
  }

  *gateConfig = service->activeConfig.preemptGates[gateIndex];

  return 1U;
}

uint16_t ConfigurationServiceGetActiveSetId(
  const ConfigurationService_t *service)
{
  uint8_t payloadBytes[CONFIGURATION_IMAGE_PAYLOAD_SIZE];

  if (service == NULL)
  {
    return 0U;
  }

  PayloadSerialize(&service->activeConfig, payloadBytes);

  return (uint16_t) (Crc32Compute(payloadBytes,
                                  sizeof(payloadBytes)) & 0xFFFFU);
}

uint32_t ConfigurationServiceGetActiveGeneration(
  const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  return service->activeGeneration;
}

ConfigurationSlotId_t ConfigurationServiceGetActiveSlot(
  const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return CONFIGURATION_SLOT_NONE;
  }

  return service->activeSlot;
}

uint8_t ConfigurationServiceIsLoadedFromRepository(
  const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  return service->loadedFromRepository;
}

uint8_t ConfigurationServiceHasOpenTransaction(
  const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  return service->candidate.inUse;
}

uint8_t ConfigurationServiceHasPendingChanges(
  const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((service->candidate.inUse != 0U)
                    && (service->candidate.dirty != 0U));
}

ConfigurationVerifyStatus_t ConfigurationServiceGetVerifyStatus(
  const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return CONFIGURATION_VERIFY_STATUS_IDLE;
  }

  return service->verifyStatus;
}

IntersectionConfigErrorInfo_t ConfigurationServiceGetLastError(
  const ConfigurationService_t *service)
{
  IntersectionConfigErrorInfo_t errorInfo;

  errorInfo.type = INTERSECTION_CONFIG_ERROR_NONE;
  errorInfo.objectIndex = 0U;

  if (service == NULL)
  {
    return errorInfo;
  }

  return service->lastError;
}

uint8_t ConfigurationServiceCreateTransaction(ConfigurationService_t *service)
{
  if ((service == NULL) || (service->candidate.inUse != 0U))
  {
    return 0U;
  }

  CandidateClear(service);
  service->candidate.inUse = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

void ConfigurationServiceRollback(ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return;
  }

  CandidateClear(service);
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);
}

uint8_t ConfigurationServiceVerify(ConfigurationService_t *service)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    if (service != NULL)
    {
      service->verifyStatus = CONFIGURATION_VERIFY_STATUS_FAILED;
      SetError(service, INTERSECTION_CONFIG_ERROR_TRANSACTION_STATE, 0U);
    }

    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    service->verifyStatus = CONFIGURATION_VERIFY_STATUS_FAILED;
    SetError(service, INTERSECTION_CONFIG_ERROR_STORAGE, 0U);

    return 0U;
  }

  if ((IntersectionConfigValidate(&candidateConfig, &service->lastError) == 0U)
      || (IntersectionConfigValidateRuntimeSupport(&candidateConfig,
                                                   &service->lastError)
          == 0U))
  {
    service->verifyStatus = CONFIGURATION_VERIFY_STATUS_FAILED;

    return 0U;
  }

  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_SUCCESS;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

uint8_t ConfigurationServiceCommit(ConfigurationService_t *service)
{
  ConfigurationSlotId_t targetSlot;
  ConfigRepositoryObjectId_t targetObject;
  ConfigurationImageHeader_t header;
  uint8_t headerBytes[CONFIGURATION_IMAGE_HEADER_SIZE];
  uint8_t payloadBytes[CONFIGURATION_IMAGE_PAYLOAD_SIZE];
  uint8_t verifyPayload[CONFIGURATION_IMAGE_PAYLOAD_SIZE];
  uint32_t capacity;
  IntersectionConfig_t candidateConfig;
  uint32_t stateValue;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    if (service != NULL)
    {
      service->verifyStatus = CONFIGURATION_VERIFY_STATUS_COMMIT_FAILED;
      SetError(service, INTERSECTION_CONFIG_ERROR_TRANSACTION_STATE, 0U);
    }

    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    service->verifyStatus = CONFIGURATION_VERIFY_STATUS_COMMIT_FAILED;
    SetError(service, INTERSECTION_CONFIG_ERROR_STORAGE, 0U);

    return 0U;
  }

  if (IntersectionConfigValidate(&candidateConfig, &service->lastError) == 0U)
  {
    service->verifyStatus = CONFIGURATION_VERIFY_STATUS_FAILED;

    return 0U;
  }

  targetSlot = SelectCommitTarget(service);
  targetObject = SlotToObjectId(targetSlot);

  if ((service->repositoryPort == NULL)
      || (targetObject == CONFIG_REPOSITORY_OBJECT_NONE))
  {
    service->verifyStatus = CONFIGURATION_VERIFY_STATUS_COMMIT_FAILED;
    SetError(service, INTERSECTION_CONFIG_ERROR_STORAGE, 0U);

    return 0U;
  }

  capacity = ConfigRepositoryGetCapacity(service->repositoryPort, targetObject);

  if (capacity
      < (CONFIGURATION_IMAGE_HEADER_SIZE + CONFIGURATION_IMAGE_PAYLOAD_SIZE))
  {
    service->verifyStatus = CONFIGURATION_VERIFY_STATUS_COMMIT_FAILED;
    SetError(service,
             INTERSECTION_CONFIG_ERROR_STORAGE,
             0U);

    return 0U;
  }

  PayloadSerialize(&candidateConfig, payloadBytes);

  header.magic = CONFIGURATION_IMAGE_MAGIC;
  header.schemaVersion = CONFIGURATION_IMAGE_SCHEMA_VERSION;
  header.payloadLength = CONFIGURATION_IMAGE_PAYLOAD_SIZE;
  header.generation = service->activeGeneration + 1U;
  header.payloadCrc32 = Crc32Compute(payloadBytes, sizeof(payloadBytes));
  header.headerCrc32 = 0U;
  header.state = CONFIGURATION_SLOT_STATE_WRITING;
  header.migrationSourceVersion = 0U;
  header.migrationSourceCrc32 = 0U;
  header.reserved = 0U;
  header.headerCrc32 = HeaderCrc(&header);

  HeaderEncode(&header, headerBytes);

  if ((ConfigRepositoryErase(service->repositoryPort, targetObject, 0U,
                             capacity) == 0U)
      || (ConfigRepositoryWrite(service->repositoryPort,
                                targetObject,
                                0U,
                                headerBytes,
                                sizeof(headerBytes)) == 0U)
      || (ConfigRepositoryWrite(service->repositoryPort,
                                targetObject,
                                CONFIGURATION_IMAGE_HEADER_SIZE,
                                payloadBytes,
                                sizeof(payloadBytes)) == 0U)
      || (ConfigRepositoryRead(service->repositoryPort,
                               targetObject,
                               CONFIGURATION_IMAGE_HEADER_SIZE,
                               verifyPayload,
                               sizeof(verifyPayload)) == 0U)
      || (memcmp(payloadBytes, verifyPayload, sizeof(payloadBytes)) != 0))
  {
    service->verifyStatus = CONFIGURATION_VERIFY_STATUS_COMMIT_FAILED;
    SetError(service, INTERSECTION_CONFIG_ERROR_STORAGE, 0U);

    return 0U;
  }

  stateValue = CONFIGURATION_SLOT_STATE_VALID;
  WriteLe32(headerBytes, stateValue);

  if ((ConfigRepositoryWrite(service->repositoryPort,
                             targetObject,
                             24U,
                             headerBytes,
                             sizeof(stateValue)) == 0U)
      || (ConfigRepositoryRead(service->repositoryPort,
                               targetObject,
                               24U,
                               headerBytes,
                               sizeof(stateValue)) == 0U)
      || (ReadLe32(headerBytes) != CONFIGURATION_SLOT_STATE_VALID))
  {
    service->verifyStatus = CONFIGURATION_VERIFY_STATUS_COMMIT_FAILED;
    SetError(service, INTERSECTION_CONFIG_ERROR_STORAGE, 0U);

    return 0U;
  }

  service->activeConfig = candidateConfig;
  service->activeGeneration = header.generation;
  service->activeSlot = targetSlot;
  service->loadedFromRepository = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_SUCCESS;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);
  CandidateClear(service);

  return 1U;
} /* ConfigurationServiceCommit */

uint8_t ConfigurationServiceSetPhaseMinGreenDs(ConfigurationService_t *service,
                                               uint8_t phaseIndex,
                                               uint16_t valueDs)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  phase.minGreenDs = valueDs;

  return UpdatePhase(service, phaseIndex, &phase);
}

uint8_t ConfigurationServiceSetPhaseWalkSeconds(ConfigurationService_t *service,
                                                uint8_t phaseIndex,
                                                uint16_t walkSeconds)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  phase.walkSeconds = walkSeconds;

  return UpdatePhase(service, phaseIndex, &phase);
}

uint8_t ConfigurationServiceSetPhasePedClearSeconds(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint16_t pedClearSeconds)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  phase.pedClearSeconds = pedClearSeconds;

  return UpdatePhase(service, phaseIndex, &phase);
}

uint8_t ConfigurationServiceSetPhaseMaxGreenDs(ConfigurationService_t *service,
                                               uint8_t phaseIndex,
                                               uint16_t valueDs)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  phase.maxGreenDs = valueDs;

  return UpdatePhase(service, phaseIndex, &phase);
}

uint8_t ConfigurationServiceSetPhaseYellowChangeDs(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint16_t valueDs)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  phase.yellowChangeDs = valueDs;

  return UpdatePhase(service, phaseIndex, &phase);
}

uint8_t ConfigurationServiceSetPhaseRedClearDs(ConfigurationService_t *service,
                                               uint8_t phaseIndex,
                                               uint16_t valueDs)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  phase.redClearDs = valueDs;

  return UpdatePhase(service, phaseIndex, &phase);
}

uint8_t ConfigurationServiceSetPhasePassageDs(ConfigurationService_t *service,
                                              uint8_t phaseIndex,
                                              uint16_t valueDs)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  phase.passageDs = valueDs;

  return UpdatePhase(service, phaseIndex, &phase);
}

uint8_t ConfigurationServiceSetPhaseMaxInitialDs(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint16_t valueDs)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  phase.maxInitialDs = valueDs;

  return UpdatePhase(service, phaseIndex, &phase);
}

uint8_t ConfigurationServiceSetPhasePedAdvanceWalkDs(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint16_t valueDs)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  phase.pedAdvanceWalkDs = valueDs;

  return UpdatePhase(service, phaseIndex, &phase);
}

uint8_t ConfigurationServiceSetPhasePedDelayDs(ConfigurationService_t *service,
                                               uint8_t phaseIndex,
                                               uint16_t valueDs)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  phase.pedDelayDs = valueDs;

  return UpdatePhase(service, phaseIndex, &phase);
}

uint8_t ConfigurationServiceSetPhaseOptions(ConfigurationService_t *service,
                                            uint8_t phaseIndex,
                                            uint16_t options)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;
  uint8_t wasEnabled;
  uint8_t nowEnabled;
  uint8_t ringIndex;
  uint8_t sequenceIndex;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  wasEnabled = IntersectionPhaseOptionsEnabled(phase.phaseOptions);
  nowEnabled = IntersectionPhaseOptionsEnabled(options);
  ringIndex = phase.ring;
  phase.phaseOptions = options;
  candidateConfig.phases[phaseIndex] = phase;

  if (wasEnabled != nowEnabled)
  {
    for (sequenceIndex = 0U;
         sequenceIndex < INTERSECTION_SEQUENCE_COUNT_MAX;
         sequenceIndex++)
    {
      if (nowEnabled == 0U)
      {
        if (RingPlanRemovePhase(&candidateConfig.sequencePlans[sequenceIndex][
                                                             ringIndex],
                                phaseIndex) == 0U)
        {
          return 0U;
        }
      }
      else if (RingPlanAppendPhase(
                 &candidateConfig.sequencePlans[sequenceIndex][ringIndex],
                 phaseIndex) == 0U)
      {
        return 0U;
      }
    }
  }

  return CommitPhaseMutation(service,
                             phaseIndex,
                             &candidateConfig,
                             ringIndex,
                             INTERSECTION_RING_COUNT_MAX);
}

uint16_t ConfigurationServiceGetPhaseOptions(
  const ConfigurationService_t *service,
  uint8_t phaseIndex)
{
  if ((service == NULL) || (phaseIndex >= service->activeConfig.phaseCount))
  {
    return 0U;
  }

  return service->activeConfig.phases[phaseIndex].phaseOptions;
}

#define DEFINE_PHASE_SETTER_U16(FnName, FieldName) \
        uint8_t FnName(ConfigurationService_t * service, \
                       uint8_t phaseIndex, \
                       uint16_t value) \
        { \
          IntersectionConfig_t candidateConfig; \
          IntersectionPhaseConfig_t phase; \
          if ((service == NULL) || (service->candidate.inUse == 0U)) \
          { \
            return 0U; \
          } \
          if ((MaterializeCandidate(service, &candidateConfig) == 0U) \
              || (phaseIndex >= candidateConfig.phaseCount)) \
          { \
            return 0U; \
          } \
          phase = candidateConfig.phases[phaseIndex]; \
          phase.FieldName = value; \
          return UpdatePhase(service, phaseIndex, &phase); \
        }

#define DEFINE_PHASE_SETTER_U8(FnName, FieldName) \
        uint8_t FnName(ConfigurationService_t * service, \
                       uint8_t phaseIndex, \
                       uint8_t value) \
        { \
          IntersectionConfig_t candidateConfig; \
          IntersectionPhaseConfig_t phase; \
          if ((service == NULL) || (service->candidate.inUse == 0U)) \
          { \
            return 0U; \
          } \
          if ((MaterializeCandidate(service, &candidateConfig) == 0U) \
              || (phaseIndex >= candidateConfig.phaseCount)) \
          { \
            return 0U; \
          } \
          phase = candidateConfig.phases[phaseIndex]; \
          phase.FieldName = value; \
          return UpdatePhase(service, phaseIndex, &phase); \
        }

DEFINE_PHASE_SETTER_U16(ConfigurationServiceSetPhaseMaximum2Ds,
                        phaseMaximum2Ds)
DEFINE_PHASE_SETTER_U16(ConfigurationServiceSetPhaseMaximum3Ds,
                        phaseMaximum3Ds)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseRedRevertDs, redRevertDs)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseAddedInitialDs,
                       addedInitialDs)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseTimeBeforeReductionSec,
                       timeBeforeReductionSec)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseCarsBeforeReduction,
                       carsBeforeReduction)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseTimeToReduceSec,
                       timeToReduceSec)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseReduceByDs, reduceByDs)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseMinimumGapDs, minimumGapDs)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseDynamicMaxLimitSeconds,
                       dynamicMaxLimitSeconds)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseDynamicMaxStepDs,
                       dynamicMaxStepDs)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseStartup, startup)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseYellowRedBeforeEndPedClearDs,
                       yellowRedBeforeEndPedClearDs)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhasePedWalkService,
                       pedWalkService)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseDontWalkRevertDs,
                       dontWalkRevertDs)
DEFINE_PHASE_SETTER_U16(ConfigurationServiceSetPhasePedAlternateClearSeconds,
                        pedAlternateClearSeconds)
DEFINE_PHASE_SETTER_U16(ConfigurationServiceSetPhasePedAlternateWalkSeconds,
                        pedAlternateWalkSeconds)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseAdvWarnGrnStartTimeDs,
                       advWarnGrnStartTimeDs)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseAdvWarnRedStartTimeDs,
                       advWarnRedStartTimeDs)
DEFINE_PHASE_SETTER_U8(ConfigurationServiceSetPhaseAltMinTimeTransitionSeconds,
                       altMinTimeTransitionSeconds)

uint8_t ConfigurationServiceSetPhaseConcurrency(ConfigurationService_t *service,
                                                uint8_t phaseIndex,
                                                const uint8_t *phaseNumbers,
                                                uint8_t count)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;

  if ((service == NULL) || (service->candidate.inUse == 0U)
      || (count > INTERSECTION_PHASE_COUNT_MAX))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  CopyPhaseReferenceList(&phase.concurrency, phaseNumbers, count);

  return UpdatePhase(service, phaseIndex, &phase);
}

uint8_t ConfigurationServiceSetPhaseRing(ConfigurationService_t *service,
                                         uint8_t phaseIndex,
                                         uint8_t ringIndex)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;
  uint8_t previousRingIndex;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  previousRingIndex = phase.ring;
  phase.ring = ringIndex;
  candidateConfig.phases[phaseIndex] = phase;

  if ((IntersectionPhaseOptionsEnabled(phase.phaseOptions) != 0U)
      && (previousRingIndex != ringIndex))
  {
    uint8_t sequenceIndex;

    for (sequenceIndex = 0U;
         sequenceIndex < INTERSECTION_SEQUENCE_COUNT_MAX;
         sequenceIndex++)
    {
      if ((RingPlanRemovePhase(
             &candidateConfig.sequencePlans[sequenceIndex][previousRingIndex],
             phaseIndex) == 0U)
          || (RingPlanAppendPhase(
                &candidateConfig.sequencePlans[sequenceIndex][ringIndex],
                phaseIndex) == 0U))
      {
        return 0U;
      }
    }
  }

  return CommitPhaseMutation(service,
                             phaseIndex,
                             &candidateConfig,
                             previousRingIndex,
                             ringIndex);
}

uint8_t ConfigurationServiceSetRingSequenceData(ConfigurationService_t *service,
                                                uint8_t sequenceNumber,
                                                uint8_t ringIndex,
                                                const uint8_t *phaseNumbers,
                                                uint8_t length)
{
  IntersectionRingPlan_t ringPlan;
  IntersectionRingPlan_t originalRingPlan;
  uint8_t seen[INTERSECTION_PHASE_COUNT_MAX] = { 0U };
  uint8_t position;

  if ((service == NULL) || (service->candidate.inUse == 0U)
      || (phaseNumbers == NULL) || (length == 0U)
      || (SequenceNumberToIndex(sequenceNumber, NULL) == 0U)
      || (ringIndex >= service->activeConfig.ringCount))
  {
    return 0U;
  }

  if (GetCandidateSequenceRingPlan(service,
                                   sequenceNumber,
                                   ringIndex,
                                   &ringPlan) == 0U)
  {
    return 0U;
  }

  if (length != ringPlan.phaseCount)
  {
    return 0U;
  }

  originalRingPlan = ringPlan;

  for (position = 0U; position < length; position++)
  {
    uint8_t phaseNumber = phaseNumbers[position];
    uint8_t phaseIndex;
    uint8_t existingPosition;

    if ((phaseNumber == 0U) || (phaseNumber > service->activeConfig.phaseCount))
    {
      return 0U;
    }

    phaseIndex = (uint8_t) (phaseNumber - 1U);

    if (seen[phaseIndex] != 0U)
    {
      return 0U;
    }

    if (RingPlanFindPhase(&originalRingPlan, phaseIndex, &existingPosition)
        == 0U)
    {
      return 0U;
    }

    seen[phaseIndex] = 1U;
    ringPlan.phaseOrder[position] = phaseIndex;
  }

  return UpdateRingPlan(service, sequenceNumber, ringIndex, &ringPlan);
}

uint8_t ConfigurationServiceSetPhaseEnabled(ConfigurationService_t *service,
                                            uint8_t phaseIndex,
                                            uint8_t enabled)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPhaseConfig_t phase;
  uint8_t ringIndex;
  uint8_t sequenceIndex;

  if ((service == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseIndex >= candidateConfig.phaseCount))
  {
    return 0U;
  }

  phase = candidateConfig.phases[phaseIndex];
  ringIndex = phase.ring;
  if (enabled != 0U)
  {
    phase.phaseOptions |= PHASE_OPTIONS_ENABLED;
  }
  else
  {
    phase.phaseOptions &= (uint16_t) ~PHASE_OPTIONS_ENABLED;
  }

  candidateConfig.phases[phaseIndex] = phase;

  for (sequenceIndex = 0U;
       sequenceIndex < INTERSECTION_SEQUENCE_COUNT_MAX;
       sequenceIndex++)
  {
    if (enabled == 0U)
    {
      if (RingPlanRemovePhase(&candidateConfig.sequencePlans[sequenceIndex][
                                                           ringIndex],
                              phaseIndex) == 0U)
      {
        return 0U;
      }
    }
    else if (RingPlanAppendPhase(&candidateConfig.sequencePlans[sequenceIndex][
                                                              ringIndex],
                                 phaseIndex) == 0U)
    {
      return 0U;
    }
  }

  return CommitPhaseMutation(service,
                             phaseIndex,
                             &candidateConfig,
                             ringIndex,
                             INTERSECTION_RING_COUNT_MAX);
} /* ConfigurationServiceSetPhaseEnabled */

uint8_t ConfigurationServiceSetPhaseDetectorInput(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t detectorNumber)
{
  IntersectionConfig_t candidateConfig;
  uint8_t currentDetectorNumber;

  if ((service == NULL) || (service->candidate.inUse == 0U)
      || (phaseIndex >= INTERSECTION_PHASE_COUNT_MAX)
      || (MaterializeCandidate(service, &candidateConfig) == 0U))
  {
    return 0U;
  }

  currentDetectorNumber = candidateConfig.inputMapping.phaseDetectors[phaseIndex];

  if ((currentDetectorNumber != 0U)
      && (currentDetectorNumber <= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
      && (ConfigurationServiceSetVehicleDetectorCallPhase(
            service,
            (uint8_t) (currentDetectorNumber - 1U),
            0U) == 0U))
  {
    return 0U;
  }

  if (detectorNumber == 0U)
  {
    return 1U;
  }

  return ConfigurationServiceSetVehicleDetectorCallPhase(
    service,
    (uint8_t) (detectorNumber - 1U),
    (uint8_t) (phaseIndex + 1U));
}

uint8_t ConfigurationServiceSetPhasePedInput(ConfigurationService_t *service,
                                             uint8_t phaseIndex,
                                             uint8_t pedInputNumber)
{
  IntersectionConfig_t candidateConfig;
  uint8_t currentDetectorNumber;

  if ((service == NULL) || (service->candidate.inUse == 0U)
      || (phaseIndex >= INTERSECTION_PHASE_COUNT_MAX)
      || (MaterializeCandidate(service, &candidateConfig) == 0U))
  {
    return 0U;
  }

  currentDetectorNumber = candidateConfig.inputMapping.phasePedInputs[phaseIndex];

  if ((currentDetectorNumber != 0U)
      && (currentDetectorNumber <= INTERSECTION_PED_INPUT_COUNT_MAX)
      && (ConfigurationServiceSetPedestrianDetectorCallPhase(
            service,
            (uint8_t) (currentDetectorNumber - 1U),
            0U) == 0U))
  {
    return 0U;
  }

  if (pedInputNumber == 0U)
  {
    return 1U;
  }

  return ConfigurationServiceSetPedestrianDetectorCallPhase(
    service,
    (uint8_t) (pedInputNumber - 1U),
    (uint8_t) (phaseIndex + 1U));
}

uint8_t ConfigurationServiceSetPreemptInputSource(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t sourceNumber)
{
  IntersectionInputMappingConfig_t inputMapping;

  if ((preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (GetCandidateInputMapping(service, &inputMapping) == 0U))
  {
    return 0U;
  }

  inputMapping.preemptInputs[preemptIndex] = sourceNumber;

  return UpdateInputMapping(service, &inputMapping);
}

uint8_t ConfigurationServiceSetPreemptControlSource(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t sourceNumber)
{
  IntersectionInputMappingConfig_t inputMapping;

  if ((preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (GetCandidateInputMapping(service, &inputMapping) == 0U))
  {
    return 0U;
  }

  inputMapping.preemptControls[preemptIndex] = sourceNumber;

  return UpdateInputMapping(service, &inputMapping);
}

uint8_t ConfigurationServiceSetVehicleDetectorCallPhase(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t phaseNumber)
{
  IntersectionConfig_t candidateConfig;
  IntersectionVehicleDetectorConfig_t detector;

  if ((service == NULL) || (service->candidate.inUse == 0U)
      || (detectorIndex >= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseNumber > candidateConfig.phaseCount))
  {
    return 0U;
  }

  detector = candidateConfig.vehicleDetectors[detectorIndex];
  detector.callPhase = phaseNumber;

  return UpdateVehicleDetector(service, detectorIndex, &detector);
}

uint8_t ConfigurationServiceSetPedestrianDetectorCallPhase(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t phaseNumber)
{
  IntersectionConfig_t candidateConfig;
  IntersectionPedestrianDetectorConfig_t detector;

  if ((service == NULL) || (service->candidate.inUse == 0U)
      || (detectorIndex >= INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseNumber > candidateConfig.phaseCount))
  {
    return 0U;
  }

  detector = candidateConfig.pedestrianDetectors[detectorIndex];
  detector.callPhase = phaseNumber;

  return UpdatePedestrianDetector(service, detectorIndex, &detector);
}

#define DEFINE_VEHICLE_DETECTOR_SETTER_U8(FnName, FieldName) \
        uint8_t FnName(ConfigurationService_t * service, \
                       uint8_t detectorIndex, \
                       uint8_t value) \
        { \
          IntersectionVehicleDetectorConfig_t detector; \
          if (GetCandidateVehicleDetector(service, detectorIndex, &detector) == 0U) \
          { \
            return 0U; \
          } \
          detector.FieldName = value; \
          return UpdateVehicleDetector(service, detectorIndex, &detector); \
        }

#define DEFINE_VEHICLE_DETECTOR_SETTER_U16(FnName, FieldName) \
        uint8_t FnName(ConfigurationService_t * service, \
                       uint8_t detectorIndex, \
                       uint16_t value) \
        { \
          IntersectionVehicleDetectorConfig_t detector; \
          if (GetCandidateVehicleDetector(service, detectorIndex, &detector) == 0U) \
          { \
            return 0U; \
          } \
          detector.FieldName = value; \
          return UpdateVehicleDetector(service, detectorIndex, &detector); \
        }

#define DEFINE_PED_DETECTOR_SETTER_U8(FnName, FieldName) \
        uint8_t FnName(ConfigurationService_t * service, \
                       uint8_t detectorIndex, \
                       uint8_t value) \
        { \
          IntersectionPedestrianDetectorConfig_t detector; \
          if (GetCandidatePedestrianDetector(service, detectorIndex, &detector) == 0U) \
          { \
            return 0U; \
          } \
          detector.FieldName = value; \
          return UpdatePedestrianDetector(service, detectorIndex, &detector); \
        }

uint8_t ConfigurationServiceSetVehicleDetectorOptions(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t options)
{
  IntersectionVehicleDetectorConfig_t detector;

  if (GetCandidateVehicleDetector(service, detectorIndex, &detector) == 0U)
  {
    return 0U;
  }

  detector.options = NormalizeVehicleDetectorOptions(options);

  return UpdateVehicleDetector(service, detectorIndex, &detector);
}

uint8_t ConfigurationServiceSetVehicleDetectorSwitchPhase(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t phaseNumber)
{
  IntersectionVehicleDetectorConfig_t detector;
  IntersectionConfig_t candidateConfig;

  if ((GetCandidateVehicleDetector(service, detectorIndex, &detector) == 0U)
      || (MaterializeCandidate(service, &candidateConfig) == 0U)
      || (phaseNumber > candidateConfig.phaseCount))
  {
    return 0U;
  }

  detector.switchPhase = phaseNumber;

  return UpdateVehicleDetector(service, detectorIndex, &detector);
}

DEFINE_VEHICLE_DETECTOR_SETTER_U16(ConfigurationServiceSetVehicleDetectorDelayDs,
                                   delayDs)
DEFINE_VEHICLE_DETECTOR_SETTER_U8(ConfigurationServiceSetVehicleDetectorExtendDs,
                                  extendDs)
DEFINE_VEHICLE_DETECTOR_SETTER_U8(
  ConfigurationServiceSetVehicleDetectorQueueLimitSeconds,
  queueLimitSeconds)
DEFINE_VEHICLE_DETECTOR_SETTER_U8(
  ConfigurationServiceSetVehicleDetectorNoActivityMinutes,
  noActivityMinutes)
DEFINE_VEHICLE_DETECTOR_SETTER_U8(
  ConfigurationServiceSetVehicleDetectorMaxPresenceMinutes,
  maxPresenceMinutes)
DEFINE_VEHICLE_DETECTOR_SETTER_U8(
  ConfigurationServiceSetVehicleDetectorErraticCountsPerMinute,
  erraticCountsPerMinute)
DEFINE_VEHICLE_DETECTOR_SETTER_U8(
  ConfigurationServiceSetVehicleDetectorFailTimeSeconds,
  failTimeSeconds)
DEFINE_VEHICLE_DETECTOR_SETTER_U8(ConfigurationServiceSetVehicleDetectorOptions2,
                                  options2)
DEFINE_VEHICLE_DETECTOR_SETTER_U16(
  ConfigurationServiceSetVehicleDetectorPairedDetectorSpacingCm,
  pairedDetectorSpacingCm)
DEFINE_VEHICLE_DETECTOR_SETTER_U16(
  ConfigurationServiceSetVehicleDetectorAvgVehicleLengthCm,
  avgVehicleLengthCm)
DEFINE_VEHICLE_DETECTOR_SETTER_U16(ConfigurationServiceSetVehicleDetectorLengthCm,
                                   detectorLengthCm)
DEFINE_VEHICLE_DETECTOR_SETTER_U8(ConfigurationServiceSetVehicleDetectorTravelMode,
                                  travelMode)

DEFINE_PED_DETECTOR_SETTER_U8(
  ConfigurationServiceSetPedestrianDetectorNoActivityMinutes,
  noActivityMinutes)
DEFINE_PED_DETECTOR_SETTER_U8(
  ConfigurationServiceSetPedestrianDetectorMaxPresenceMinutes,
  maxPresenceMinutes)
DEFINE_PED_DETECTOR_SETTER_U8(
  ConfigurationServiceSetPedestrianDetectorErraticCountsPerMinute,
  erraticCountsPerMinute)
DEFINE_PED_DETECTOR_SETTER_U8(
  ConfigurationServiceSetPedestrianDetectorApsMinimumActuationDs,
  apsMinimumActuationDs)
DEFINE_PED_DETECTOR_SETTER_U8(ConfigurationServiceSetPedestrianDetectorOptions,
                              options)

uint8_t ConfigurationServiceSetVolumeOccupancyPeriodSeconds(
  ConfigurationService_t *service,
  uint8_t seconds)
{
  IntersectionDetectorReportConfig_t detectorReports;

  if (GetCandidateDetectorReportConfig(service, &detectorReports) == 0U)
  {
    return 0U;
  }

  detectorReports.volumeOccupancyPeriodSeconds = seconds;
  service->candidate.detectorReportsValid = 1U;
  service->candidate.detectorReports = detectorReports;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

uint8_t ConfigurationServiceSetVolumeOccupancyPeriodV3Seconds(
  ConfigurationService_t *service,
  uint16_t seconds)
{
  IntersectionDetectorReportConfig_t detectorReports;

  if (GetCandidateDetectorReportConfig(service, &detectorReports) == 0U)
  {
    return 0U;
  }

  detectorReports.volumeOccupancyPeriodV3Seconds = seconds;
  service->candidate.detectorReportsValid = 1U;
  service->candidate.detectorReports = detectorReports;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

uint8_t ConfigurationServiceSetPedestrianDetectorPeriodSeconds(
  ConfigurationService_t *service,
  uint16_t seconds)
{
  IntersectionDetectorReportConfig_t detectorReports;

  if (GetCandidateDetectorReportConfig(service, &detectorReports) == 0U)
  {
    return 0U;
  }

  detectorReports.pedestrianDetectorPeriodSeconds = seconds;
  service->candidate.detectorReportsValid = 1U;
  service->candidate.detectorReports = detectorReports;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

uint8_t ConfigurationServiceSetVehicleDetectorPairedDetector(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t pairedDetectorNumber)
{
  IntersectionConfig_t candidateConfig;
  uint8_t detectorNumber;
  uint8_t oldPairNumber;
  uint8_t displacedPairNumber;
  uint8_t affectedIndex;

  if ((service == NULL) || (service->candidate.inUse == 0U)
      || (detectorIndex >= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
      || (pairedDetectorNumber > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
      || (pairedDetectorNumber == (uint8_t) (detectorIndex + 1U))
      || (MaterializeCandidate(service, &candidateConfig) == 0U))
  {
    return 0U;
  }

  detectorNumber = (uint8_t) (detectorIndex + 1U);
  oldPairNumber = candidateConfig.vehicleDetectors[detectorIndex].pairedDetector;
  displacedPairNumber = 0U;

  if ((oldPairNumber != 0U) && (oldPairNumber != pairedDetectorNumber)
      && (candidateConfig.vehicleDetectors[oldPairNumber - 1U].pairedDetector
          == detectorNumber))
  {
    candidateConfig.vehicleDetectors[oldPairNumber - 1U].pairedDetector = 0U;
  }

  candidateConfig.vehicleDetectors[detectorIndex].pairedDetector =
    pairedDetectorNumber;

  if (pairedDetectorNumber != 0U)
  {
    displacedPairNumber =
      candidateConfig.vehicleDetectors[pairedDetectorNumber - 1U].pairedDetector;

    if ((displacedPairNumber != 0U) && (displacedPairNumber != detectorNumber)
        && (displacedPairNumber
            <= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
        && (candidateConfig.vehicleDetectors[displacedPairNumber - 1U].
            pairedDetector == pairedDetectorNumber))
    {
      candidateConfig.vehicleDetectors[displacedPairNumber - 1U].pairedDetector
        = 0U;
    }

    candidateConfig.vehicleDetectors[pairedDetectorNumber - 1U].pairedDetector =
      detectorNumber;
  }

  RebuildLegacyInputMapping(&candidateConfig);

  service->candidate.vehicleDetectorValid[detectorIndex] = 1U;
  service->candidate.vehicleDetectors[detectorIndex] =
    candidateConfig.vehicleDetectors[detectorIndex];

  for (affectedIndex = 0U;
       affectedIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       ++affectedIndex)
  {
    uint8_t affectedNumber = (uint8_t) (affectedIndex + 1U);

    if ((affectedNumber == oldPairNumber) || (affectedNumber == pairedDetectorNumber)
        || (affectedNumber == displacedPairNumber))
    {
      service->candidate.vehicleDetectorValid[affectedIndex] = 1U;
      service->candidate.vehicleDetectors[affectedIndex] =
        candidateConfig.vehicleDetectors[affectedIndex];
    }
  }

  service->candidate.inputMappingValid = 1U;
  service->candidate.inputMapping = candidateConfig.inputMapping;
  service->candidate.dirty = 1U;
  service->verifyStatus = CONFIGURATION_VERIFY_STATUS_IDLE;
  SetError(service, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  return 1U;
}

static uint8_t GetCandidateVehicleDetector(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  IntersectionVehicleDetectorConfig_t *detector)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (detector == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (detectorIndex >= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  *detector = candidateConfig.vehicleDetectors[detectorIndex];

  return 1U;
}

static uint8_t GetCandidatePedestrianDetector(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  IntersectionPedestrianDetectorConfig_t *detector)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (detector == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (detectorIndex >= INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  *detector = candidateConfig.pedestrianDetectors[detectorIndex];

  return 1U;
}

static uint8_t GetCandidateDetectorReportConfig(
  ConfigurationService_t *service,
  IntersectionDetectorReportConfig_t *detectorReportConfig)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (detectorReportConfig == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    return 0U;
  }

  *detectorReportConfig = candidateConfig.detectorReports;

  return 1U;
}

static uint8_t GetCandidateChannel(ConfigurationService_t *service,
                                   uint8_t channelIndex,
                                   IntersectionChannelConfig_t *channel)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (channel == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (channelIndex >= INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  *channel = candidateConfig.channels[channelIndex];

  return 1U;
}

static uint8_t GetCandidateOverlap(ConfigurationService_t *service,
                                   uint8_t overlapIndex,
                                   IntersectionOverlapConfig_t *overlap)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (overlap == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if ((MaterializeCandidate(service, &candidateConfig) == 0U)
      || (overlapIndex >= INTERSECTION_OVERLAP_COUNT_MAX))
  {
    return 0U;
  }

  *overlap = candidateConfig.overlaps[overlapIndex];

  return 1U;
}

static uint8_t GetCandidateCoordination(ConfigurationService_t *service,
                                        IntersectionCoordinationConfig_t *
                                        coordination)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (coordination == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    return 0U;
  }

  *coordination = candidateConfig.coordination;

  return 1U;
}

static uint8_t GetCandidateTimebase(ConfigurationService_t *service,
                                    IntersectionTimebaseConfig_t *timebase)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (timebase == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    return 0U;
  }

  *timebase = candidateConfig.timebase;

  return 1U;
}

static uint8_t GetCandidateGlobalTimeManagement(
  ConfigurationService_t *service,
  IntersectionGlobalTimeManagementConfig_t *globalTimeManagement)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (globalTimeManagement == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    return 0U;
  }

  *globalTimeManagement = candidateConfig.globalTimeManagement;

  return 1U;
}

static uint8_t GetCandidateCabinetEnvironment(
  ConfigurationService_t *service,
  IntersectionCabinetEnvironmentConfig_t *cabinetEnvironment)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (cabinetEnvironment == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    return 0U;
  }

  *cabinetEnvironment = candidateConfig.cabinetEnvironment;

  return 1U;
}

static uint8_t GetCandidateUnit(ConfigurationService_t *service,
                                IntersectionUnitConfig_t *unit)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (unit == NULL) || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    return 0U;
  }

  *unit = candidateConfig.unit;

  return 1U;
}

static uint8_t GetCandidateRingPlan(ConfigurationService_t *service,
                                    uint8_t ringIndex,
                                    IntersectionRingPlan_t *ringPlan)
{
  return GetCandidateSequenceRingPlan(service, 1U, ringIndex, ringPlan);
}

static uint8_t GetCandidateSequenceRingPlan(ConfigurationService_t *service,
                                            uint8_t sequenceNumber,
                                            uint8_t ringIndex,
                                            IntersectionRingPlan_t *ringPlan)
{
  IntersectionConfig_t candidateConfig;
  uint8_t sequenceIndex = 0U;

  if ((service == NULL) || (ringPlan == NULL) || (service->candidate.inUse == 0U)
      || (ringIndex >= service->activeConfig.ringCount)
      || (SequenceNumberToIndex(sequenceNumber, &sequenceIndex) == 0U))
  {
    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    return 0U;
  }

  *ringPlan = candidateConfig.sequencePlans[sequenceIndex][ringIndex];

  return 1U;
}

static uint8_t GetCandidateUserDefinedBackupContent(
  ConfigurationService_t *service,
  uint8_t contentIndex,
  IntersectionUserDefinedBackupContentConfig_t *content)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (content == NULL) || (service->candidate.inUse == 0U)
      || (contentIndex >= INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX))
  {
    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    return 0U;
  }

  *content = candidateConfig.userDefinedBackupContents[contentIndex];

  return 1U;
}

static uint8_t GetCandidateInputMapping(ConfigurationService_t *service,
                                        IntersectionInputMappingConfig_t *
                                        inputMapping)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (inputMapping == NULL)
      || (service->candidate.inUse == 0U))
  {
    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    return 0U;
  }

  *inputMapping = candidateConfig.inputMapping;

  return 1U;
}

static uint8_t GetCandidatePreempt(ConfigurationService_t *service,
                                   uint8_t preemptIndex,
                                   IntersectionPreemptConfig_t *preempt)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (preempt == NULL)
      || (service->candidate.inUse == 0U)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    return 0U;
  }

  *preempt = candidateConfig.preempts[preemptIndex];

  return 1U;
}

static uint8_t GetCandidatePreemptGate(ConfigurationService_t *service,
                                       uint8_t gateIndex,
                                       IntersectionPreemptGateConfig_t *gate)
{
  IntersectionConfig_t candidateConfig;

  if ((service == NULL) || (gate == NULL)
      || (service->candidate.inUse == 0U)
      || (gateIndex >= INTERSECTION_PREEMPT_GATE_COUNT_MAX))
  {
    return 0U;
  }

  if (MaterializeCandidate(service, &candidateConfig) == 0U)
  {
    return 0U;
  }

  *gate = candidateConfig.preemptGates[gateIndex];

  return 1U;
}

uint8_t ConfigurationServiceSetChannelControlSource(
  ConfigurationService_t *service,
  uint8_t channelIndex,
  uint8_t controlSource)
{
  IntersectionChannelConfig_t channel;

  if (GetCandidateChannel(service, channelIndex, &channel) == 0U)
  {
    return 0U;
  }

  channel.controlSource = controlSource;

  return UpdateChannel(service, channelIndex, &channel);
}

uint8_t ConfigurationServiceSetChannelControlType(
  ConfigurationService_t *service,
  uint8_t channelIndex,
  uint8_t controlType)
{
  IntersectionChannelConfig_t channel;

  if (GetCandidateChannel(service, channelIndex, &channel) == 0U)
  {
    return 0U;
  }

  channel.controlType = controlType;

  return UpdateChannel(service, channelIndex, &channel);
}

uint8_t ConfigurationServiceSetChannelFlashMask(ConfigurationService_t *service,
                                                uint8_t channelIndex,
                                                uint8_t flashMask)
{
  IntersectionChannelConfig_t channel;

  if (GetCandidateChannel(service, channelIndex, &channel) == 0U)
  {
    return 0U;
  }

  if ((flashMask & 0x04U) != 0U)
  {
    flashMask = (uint8_t) (flashMask & (uint8_t) ~0x02U);
  }

  channel.flashMask = flashMask;

  return UpdateChannel(service, channelIndex, &channel);
}

uint8_t ConfigurationServiceSetChannelDimMask(ConfigurationService_t *service,
                                              uint8_t channelIndex,
                                              uint8_t dimMask)
{
  IntersectionChannelConfig_t channel;

  if (GetCandidateChannel(service, channelIndex, &channel) == 0U)
  {
    return 0U;
  }

  channel.dimMask = dimMask;

  return UpdateChannel(service, channelIndex, &channel);
}

uint8_t ConfigurationServiceSetChannelGreenType(ConfigurationService_t *service,
                                                uint8_t channelIndex,
                                                uint8_t greenType)
{
  IntersectionChannelConfig_t channel;

  if (GetCandidateChannel(service, channelIndex, &channel) == 0U)
  {
    return 0U;
  }

  channel.greenType = greenType;

  return UpdateChannel(service, channelIndex, &channel);
}

uint8_t ConfigurationServiceSetChannelGreenIncluded(
  ConfigurationService_t *service,
  uint8_t channelIndex,
  const uint8_t *
  channelNumbers,
  uint8_t channelCount)
{
  IntersectionChannelConfig_t channel;

  if ((channelCount != 0U) && (channelNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidateChannel(service, channelIndex, &channel) == 0U)
  {
    return 0U;
  }

  CopyChannelReferenceList(&channel.greenIncluded, channelNumbers,
                           channelCount);

  return UpdateChannel(service, channelIndex, &channel);
}

uint8_t ConfigurationServiceSetChannelIntersectionId(
  ConfigurationService_t *service,
  uint8_t channelIndex,
  uint16_t intersectionId)
{
  IntersectionChannelConfig_t channel;

  if (GetCandidateChannel(service, channelIndex, &channel) == 0U)
  {
    return 0U;
  }

  channel.intersectionId = intersectionId;

  return UpdateChannel(service, channelIndex, &channel);
}

uint8_t ConfigurationServiceSetOverlapType(ConfigurationService_t *service,
                                           uint8_t overlapIndex,
                                           uint8_t overlapType)
{
  IntersectionOverlapConfig_t overlap;

  if (GetCandidateOverlap(service, overlapIndex, &overlap) == 0U)
  {
    return 0U;
  }

  overlap.type = overlapType;

  return UpdateOverlap(service, overlapIndex, &overlap);
}

uint8_t ConfigurationServiceSetOverlapIncludedPhases(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount)
{
  IntersectionOverlapConfig_t overlap;

  if ((phaseCount != 0U) && (phaseNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidateOverlap(service, overlapIndex, &overlap) == 0U)
  {
    return 0U;
  }

  CopyPhaseReferenceList(&overlap.includedPhases, phaseNumbers, phaseCount);

  return UpdateOverlap(service, overlapIndex, &overlap);
}

uint8_t ConfigurationServiceSetOverlapModifierPhases(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount)
{
  IntersectionOverlapConfig_t overlap;

  if ((phaseCount != 0U) && (phaseNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidateOverlap(service, overlapIndex, &overlap) == 0U)
  {
    return 0U;
  }

  CopyPhaseReferenceList(&overlap.modifierPhases, phaseNumbers, phaseCount);

  return UpdateOverlap(service, overlapIndex, &overlap);
}

uint8_t ConfigurationServiceSetOverlapTrailGreenDs(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  uint16_t trailGreenDs)
{
  IntersectionOverlapConfig_t overlap;

  if (GetCandidateOverlap(service, overlapIndex, &overlap) == 0U)
  {
    return 0U;
  }

  overlap.trailGreenDs = trailGreenDs;

  return UpdateOverlap(service, overlapIndex, &overlap);
}

uint8_t ConfigurationServiceSetOverlapTrailYellowDs(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  uint16_t trailYellowDs)
{
  IntersectionOverlapConfig_t overlap;

  if (GetCandidateOverlap(service, overlapIndex, &overlap) == 0U)
  {
    return 0U;
  }

  overlap.trailYellowDs = trailYellowDs;

  return UpdateOverlap(service, overlapIndex, &overlap);
}

uint8_t ConfigurationServiceSetOverlapTrailRedDs(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  uint16_t trailRedDs)
{
  IntersectionOverlapConfig_t overlap;

  if (GetCandidateOverlap(service, overlapIndex, &overlap) == 0U)
  {
    return 0U;
  }

  overlap.trailRedDs = trailRedDs;

  return UpdateOverlap(service, overlapIndex, &overlap);
}

uint8_t ConfigurationServiceSetOverlapWalkSeconds(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  uint16_t walkSeconds)
{
  IntersectionOverlapConfig_t overlap;

  if (GetCandidateOverlap(service, overlapIndex, &overlap) == 0U)
  {
    return 0U;
  }

  overlap.walkSeconds = walkSeconds;

  return UpdateOverlap(service, overlapIndex, &overlap);
}

uint8_t ConfigurationServiceSetOverlapPedClearSeconds(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  uint16_t pedClearSeconds)
{
  IntersectionOverlapConfig_t overlap;

  if (GetCandidateOverlap(service, overlapIndex, &overlap) == 0U)
  {
    return 0U;
  }

  overlap.pedClearSeconds = pedClearSeconds;

  return UpdateOverlap(service, overlapIndex, &overlap);
}

uint8_t ConfigurationServiceSetOverlapConflictingPedPhases(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  const uint8_t *
  phaseNumbers,
  uint8_t phaseCount)
{
  IntersectionOverlapConfig_t overlap;

  if ((phaseCount != 0U) && (phaseNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidateOverlap(service, overlapIndex, &overlap) == 0U)
  {
    return 0U;
  }

  CopyPhaseReferenceList(&overlap.conflictingPedPhases,
                         phaseNumbers,
                         phaseCount);

  return UpdateOverlap(service, overlapIndex, &overlap);
}

uint8_t ConfigurationServiceSetCoordOperationalMode(
  ConfigurationService_t *service,
  uint8_t operationalMode)
{
  IntersectionCoordinationConfig_t coordination;

  if (GetCandidateCoordination(service, &coordination) == 0U)
  {
    return 0U;
  }

  coordination.operationalMode = operationalMode;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetCoordCorrectionMode(
  ConfigurationService_t *service,
  uint8_t correctionMode)
{
  IntersectionCoordinationConfig_t coordination;

  if (GetCandidateCoordination(service, &coordination) == 0U)
  {
    return 0U;
  }

  coordination.correctionMode = correctionMode;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetCoordMaximumMode(ConfigurationService_t *service,
                                                uint8_t maximumMode)
{
  IntersectionCoordinationConfig_t coordination;

  if (GetCandidateCoordination(service, &coordination) == 0U)
  {
    return 0U;
  }

  coordination.maximumMode = maximumMode;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetCoordForceMode(ConfigurationService_t *service,
                                              uint8_t forceMode)
{
  IntersectionCoordinationConfig_t coordination;

  if (GetCandidateCoordination(service, &coordination) == 0U)
  {
    return 0U;
  }

  coordination.forceMode = forceMode;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetCoordUnitSyncPoint(
  ConfigurationService_t *service,
  uint8_t unitCoordSyncPoint)
{
  IntersectionCoordinationConfig_t coordination;

  if (GetCandidateCoordination(service, &coordination) == 0U)
  {
    return 0U;
  }

  coordination.unitCoordSyncPoint = unitCoordSyncPoint;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetPatternCycleTimeSeconds(
  ConfigurationService_t *service,
  uint8_t patternIndex,
  uint8_t cycleTimeSeconds)
{
  IntersectionCoordinationConfig_t coordination;

  if ((patternIndex >= INTERSECTION_PATTERN_COUNT_MAX)
      || (GetCandidateCoordination(service, &coordination) == 0U))
  {
    return 0U;
  }

  coordination.patterns[patternIndex].cycleTimeSeconds = cycleTimeSeconds;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetPatternOffsetTimeSeconds(
  ConfigurationService_t *service,
  uint8_t patternIndex,
  uint8_t
  offsetTimeSeconds)
{
  IntersectionCoordinationConfig_t coordination;

  if ((patternIndex >= INTERSECTION_PATTERN_COUNT_MAX)
      || (GetCandidateCoordination(service, &coordination) == 0U))
  {
    return 0U;
  }

  coordination.patterns[patternIndex].offsetTimeSeconds = offsetTimeSeconds;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetPatternSplitNumber(
  ConfigurationService_t *service,
  uint8_t patternIndex,
  uint8_t splitNumber)
{
  IntersectionCoordinationConfig_t coordination;

  if ((patternIndex >= INTERSECTION_PATTERN_COUNT_MAX)
      || (GetCandidateCoordination(service, &coordination) == 0U))
  {
    return 0U;
  }

  coordination.patterns[patternIndex].splitNumber = splitNumber;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetPatternSequenceNumber(
  ConfigurationService_t *service,
  uint8_t patternIndex,
  uint8_t sequenceNumber)
{
  IntersectionCoordinationConfig_t coordination;
  uint8_t maxSequences = ConfigurationServiceGetSequenceCount(service);

  if ((patternIndex >= INTERSECTION_PATTERN_COUNT_MAX)
      || (sequenceNumber == 0U)
      || (sequenceNumber > maxSequences)
      || (GetCandidateCoordination(service, &coordination) == 0U))
  {
    return 0U;
  }

  coordination.patterns[patternIndex].sequenceNumber = sequenceNumber;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetPatternCoordSyncPoint(
  ConfigurationService_t *service,
  uint8_t patternIndex,
  uint8_t coordSyncPoint)
{
  IntersectionCoordinationConfig_t coordination;

  if ((patternIndex >= INTERSECTION_PATTERN_COUNT_MAX)
      || (GetCandidateCoordination(service, &coordination) == 0U))
  {
    return 0U;
  }

  coordination.patterns[patternIndex].coordSyncPoint = coordSyncPoint;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetPatternOptions(ConfigurationService_t *service,
                                              uint8_t patternIndex,
                                              uint8_t options)
{
  IntersectionCoordinationConfig_t coordination;

  if ((patternIndex >= INTERSECTION_PATTERN_COUNT_MAX)
      || (GetCandidateCoordination(service, &coordination) == 0U))
  {
    return 0U;
  }

  coordination.patterns[patternIndex].options = options;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetSplitTimeSeconds(ConfigurationService_t *service,
                                                uint8_t splitIndex,
                                                uint8_t phaseIndex,
                                                uint8_t timeSeconds)
{
  IntersectionCoordinationConfig_t coordination;

  if ((splitIndex >= INTERSECTION_SPLIT_COUNT_MAX)
      || (phaseIndex >= INTERSECTION_PHASE_COUNT_MAX)
      || (GetCandidateCoordination(service, &coordination) == 0U))
  {
    return 0U;
  }

  coordination.splits[splitIndex][phaseIndex].timeSeconds = timeSeconds;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetSplitMode(ConfigurationService_t *service,
                                         uint8_t splitIndex,
                                         uint8_t phaseIndex,
                                         uint8_t mode)
{
  IntersectionCoordinationConfig_t coordination;

  if ((splitIndex >= INTERSECTION_SPLIT_COUNT_MAX)
      || (phaseIndex >= INTERSECTION_PHASE_COUNT_MAX)
      || (GetCandidateCoordination(service, &coordination) == 0U))
  {
    return 0U;
  }

  coordination.splits[splitIndex][phaseIndex].mode = mode;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetSplitCoordPhase(ConfigurationService_t *service,
                                               uint8_t splitIndex,
                                               uint8_t phaseIndex,
                                               uint8_t coordPhase)
{
  IntersectionCoordinationConfig_t coordination;

  if ((splitIndex >= INTERSECTION_SPLIT_COUNT_MAX)
      || (phaseIndex >= INTERSECTION_PHASE_COUNT_MAX)
      || (GetCandidateCoordination(service, &coordination) == 0U))
  {
    return 0U;
  }

  coordination.splits[splitIndex][phaseIndex].coordPhase = coordPhase;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetSplitOptions(ConfigurationService_t *service,
                                            uint8_t splitIndex,
                                            uint8_t phaseIndex,
                                            uint8_t options)
{
  IntersectionCoordinationConfig_t coordination;

  if ((splitIndex >= INTERSECTION_SPLIT_COUNT_MAX)
      || (phaseIndex >= INTERSECTION_PHASE_COUNT_MAX)
      || (GetCandidateCoordination(service, &coordination) == 0U))
  {
    return 0U;
  }

  coordination.splits[splitIndex][phaseIndex].options = options;

  return UpdateCoordination(service, &coordination);
}

uint8_t ConfigurationServiceSetTimebasePatternSyncMinutes(
  ConfigurationService_t *service,
  uint16_t patternSyncMinutes)
{
  IntersectionTimebaseConfig_t timebase;

  if (GetCandidateTimebase(service, &timebase) == 0U)
  {
    return 0U;
  }

  timebase.patternSyncMinutes = patternSyncMinutes;

  return UpdateTimebase(service, &timebase);
}

uint8_t ConfigurationServiceSetTimebaseActionPattern(
  ConfigurationService_t *service,
  uint8_t actionIndex,
  uint8_t pattern)
{
  IntersectionTimebaseConfig_t timebase;

  if ((actionIndex >= INTERSECTION_TIMEBASE_ACTION_COUNT_MAX)
      || (GetCandidateTimebase(service, &timebase) == 0U))
  {
    return 0U;
  }

  timebase.actions[actionIndex].pattern = pattern;

  return UpdateTimebase(service, &timebase);
}

uint8_t ConfigurationServiceSetTimebaseActionAuxiliaryFunction(
  ConfigurationService_t *service,
  uint8_t actionIndex,
  uint8_t auxiliaryFunction)
{
  IntersectionTimebaseConfig_t timebase;

  if ((actionIndex >= INTERSECTION_TIMEBASE_ACTION_COUNT_MAX)
      || (GetCandidateTimebase(service, &timebase) == 0U))
  {
    return 0U;
  }

  timebase.actions[actionIndex].auxiliaryFunction = auxiliaryFunction;

  return UpdateTimebase(service, &timebase);
}

uint8_t ConfigurationServiceSetTimebaseActionSpecialFunction(
  ConfigurationService_t *service,
  uint8_t actionIndex,
  uint8_t specialFunction)
{
  IntersectionTimebaseConfig_t timebase;

  if ((actionIndex >= INTERSECTION_TIMEBASE_ACTION_COUNT_MAX)
      || (GetCandidateTimebase(service, &timebase) == 0U))
  {
    return 0U;
  }

  timebase.actions[actionIndex].specialFunction = specialFunction;

  return UpdateTimebase(service, &timebase);
}

uint8_t ConfigurationServiceSetTimebaseActionEnabledLane(
  ConfigurationService_t *service,
  uint8_t actionIndex,
  uint8_t enabledLane)
{
  IntersectionTimebaseConfig_t timebase;

  if ((actionIndex >= INTERSECTION_TIMEBASE_ACTION_COUNT_MAX)
      || (GetCandidateTimebase(service, &timebase) == 0U))
  {
    return 0U;
  }

  timebase.actions[actionIndex].enabledLane = enabledLane;

  return UpdateTimebase(service, &timebase);
}

uint8_t ConfigurationServiceSetGlobalTimeManagementConfig(
  ConfigurationService_t *service,
  const IntersectionGlobalTimeManagementConfig_t *globalTimeManagementConfig)
{
  if (globalTimeManagementConfig == NULL)
  {
    return 0U;
  }

  return UpdateGlobalTimeManagement(service, globalTimeManagementConfig);
}

uint8_t ConfigurationServiceSetCabinetEnvironmentConfig(
  ConfigurationService_t *service,
  const IntersectionCabinetEnvironmentConfig_t *cabinetEnvironmentConfig)
{
  if (cabinetEnvironmentConfig == NULL)
  {
    return 0U;
  }

  return UpdateCabinetEnvironment(service, cabinetEnvironmentConfig);
}

uint8_t ConfigurationServiceSetUnitTimeSourceCommanded(
  ConfigurationService_t *service,
  uint8_t timeSourceCommanded)
{
  IntersectionUnitConfig_t unit;

  if (GetCandidateUnit(service, &unit) == 0U)
  {
    return 0U;
  }

  unit.timeSourceCommanded = timeSourceCommanded;

  return UpdateUnit(service, &unit);
}

uint8_t ConfigurationServiceSetUnitElevationOffsetMeters(
  ConfigurationService_t *service,
  uint8_t elevationOffsetMeters)
{
  IntersectionUnitConfig_t unit;

  if (GetCandidateUnit(service, &unit) == 0U)
  {
    return 0U;
  }

  unit.elevationOffsetMeters = elevationOffsetMeters;

  return UpdateUnit(service, &unit);
}

uint8_t ConfigurationServiceSetUnitStartUpFlashSeconds(
  ConfigurationService_t *service,
  uint8_t startUpFlashSeconds)
{
  IntersectionUnitConfig_t unit;

  if (GetCandidateUnit(service, &unit) == 0U)
  {
    return 0U;
  }

  unit.startUpFlashSeconds = startUpFlashSeconds;

  return UpdateUnit(service, &unit);
}

uint8_t ConfigurationServiceSetUnitAutoPedestrianClear(
  ConfigurationService_t *service,
  uint8_t autoPedestrianClear)
{
  IntersectionUnitConfig_t unit;

  if (GetCandidateUnit(service, &unit) == 0U)
  {
    return 0U;
  }

  unit.autoPedestrianClear = autoPedestrianClear;

  return UpdateUnit(service, &unit);
}

uint8_t ConfigurationServiceSetUnitBackupTimeSeconds(
  ConfigurationService_t *service,
  uint16_t backupTimeSeconds)
{
  IntersectionUnitConfig_t unit;

  if (GetCandidateUnit(service, &unit) == 0U)
  {
    return 0U;
  }

  if (unit.userDefinedBackupTimeSeconds != 0U)
  {
    return 1U;
  }

  unit.backupTimeSeconds = backupTimeSeconds;

  return UpdateUnit(service, &unit);
}

uint8_t ConfigurationServiceSetUnitUserDefinedBackupTimeSeconds(
  ConfigurationService_t *service,
  uint32_t backupTimeSeconds)
{
  IntersectionUnitConfig_t unit;

  if (GetCandidateUnit(service, &unit) == 0U)
  {
    return 0U;
  }

  unit.userDefinedBackupTimeSeconds = backupTimeSeconds;

  return UpdateUnit(service, &unit);
}

uint8_t ConfigurationServiceSetUnitRedRevertDs(ConfigurationService_t *service,
                                               uint8_t redRevertDs)
{
  IntersectionUnitConfig_t unit;

  if (GetCandidateUnit(service, &unit) == 0U)
  {
    return 0U;
  }

  unit.redRevertDs = redRevertDs;

  return UpdateUnit(service, &unit);
}

uint8_t ConfigurationServiceSetUnitStartUpFlashMode(
  ConfigurationService_t *service,
  uint8_t startUpFlashMode)
{
  IntersectionUnitConfig_t unit;

  if (GetCandidateUnit(service, &unit) == 0U)
  {
    return 0U;
  }

  unit.startUpFlashMode = startUpFlashMode;

  return UpdateUnit(service, &unit);
}

uint8_t ConfigurationServiceSetUserDefinedBackupContentOid(
  ConfigurationService_t *service,
  uint8_t contentIndex,
  const uint32_t *oid,
  uint8_t oidLength)
{
  IntersectionUserDefinedBackupContentConfig_t content;
  uint8_t index;

  if ((oidLength != 0U) && (oid == NULL))
  {
    return 0U;
  }

  if (GetCandidateUserDefinedBackupContent(service, contentIndex, &content) == 0U)
  {
    return 0U;
  }

  content.oidLength = oidLength;

  for (index = 0U; index < INTERSECTION_USER_DEFINED_BACKUP_OID_COMPONENT_COUNT_MAX;
       index++)
  {
    content.oid[index] = (index < oidLength) ? oid[index] : 0U;
  }

  return UpdateUserDefinedBackupContent(service, contentIndex, &content);
}

uint8_t ConfigurationServiceSetUserDefinedBackupContentDescription(
  ConfigurationService_t *service,
  uint8_t contentIndex,
  const uint8_t *description,
  uint8_t descriptionLength)
{
  IntersectionUserDefinedBackupContentConfig_t content;
  uint8_t index;

  if ((descriptionLength != 0U) && (description == NULL))
  {
    return 0U;
  }

  if (GetCandidateUserDefinedBackupContent(service, contentIndex, &content) == 0U)
  {
    return 0U;
  }

  content.descriptionLength = descriptionLength;

  for (index = 0U; index < INTERSECTION_USER_DEFINED_BACKUP_DESCRIPTION_MAX;
       index++)
  {
    content.description[index] =
      (index < descriptionLength) ? description[index] : 0U;
  }

  return UpdateUserDefinedBackupContent(service, contentIndex, &content);
}

uint8_t ConfigurationServiceSetPreemptControl(ConfigurationService_t *service,
                                              uint8_t preemptIndex,
                                              uint8_t control)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.control = control;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptLink(ConfigurationService_t *service,
                                           uint8_t preemptIndex,
                                           uint8_t link)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.link = link;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptDelaySeconds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint16_t delaySeconds)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.delaySeconds = delaySeconds;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptMinimumDurationSeconds(
  ConfigurationService_t *service,
  uint8_t
  preemptIndex,
  uint16_t
  minimumDurationSeconds)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.minimumDurationSeconds = minimumDurationSeconds;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptMinimumGreenSeconds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  minimumGreenSeconds)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.minimumGreenSeconds = minimumGreenSeconds;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptMinimumWalkSeconds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  minimumWalkSeconds)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.minimumWalkSeconds = minimumWalkSeconds;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptEnterPedClearSeconds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  enterPedClearSeconds)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.enterPedClearSeconds = enterPedClearSeconds;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptTrackGreenSeconds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  trackGreenSeconds)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.trackGreenSeconds = trackGreenSeconds;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptDwellGreenSeconds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  dwellGreenSeconds)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.dwellGreenSeconds = dwellGreenSeconds;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptMaximumPresenceSeconds(
  ConfigurationService_t *service,
  uint8_t
  preemptIndex,
  uint16_t
  maximumPresenceSeconds)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.maximumPresenceSeconds = maximumPresenceSeconds;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptTrackPhases(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount)
{
  IntersectionPreemptConfig_t preempt;

  if ((phaseCount != 0U) && (phaseNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  CopyPhaseReferenceList(&preempt.trackPhases, phaseNumbers, phaseCount);

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptDwellPhases(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount)
{
  IntersectionPreemptConfig_t preempt;

  if ((phaseCount != 0U) && (phaseNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  CopyPhaseReferenceList(&preempt.dwellPhases, phaseNumbers, phaseCount);

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptDwellPeds(ConfigurationService_t *service,
                                                uint8_t preemptIndex,
                                                const uint8_t *phaseNumbers,
                                                uint8_t phaseCount)
{
  IntersectionPreemptConfig_t preempt;

  if ((phaseCount != 0U) && (phaseNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  CopyPhaseReferenceList(&preempt.dwellPeds, phaseNumbers, phaseCount);

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptExitPhases(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount)
{
  IntersectionPreemptConfig_t preempt;

  if ((phaseCount != 0U) && (phaseNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  CopyPhaseReferenceList(&preempt.exitPhases, phaseNumbers, phaseCount);

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptTrackOverlaps(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *
  overlapNumbers,
  uint8_t overlapCount)
{
  IntersectionPreemptConfig_t preempt;

  if ((overlapCount != 0U) && (overlapNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  CopyOverlapReferenceList(&preempt.trackOverlaps, overlapNumbers,
                           overlapCount);

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptDwellOverlaps(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *
  overlapNumbers,
  uint8_t overlapCount)
{
  IntersectionPreemptConfig_t preempt;

  if ((overlapCount != 0U) && (overlapNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  CopyOverlapReferenceList(&preempt.dwellOverlaps, overlapNumbers,
                           overlapCount);

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptCyclingPhases(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount)
{
  IntersectionPreemptConfig_t preempt;

  if ((phaseCount != 0U) && (phaseNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  CopyPhaseReferenceList(&preempt.cyclingPhases, phaseNumbers, phaseCount);

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptCyclingPeds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount)
{
  IntersectionPreemptConfig_t preempt;

  if ((phaseCount != 0U) && (phaseNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  CopyPhaseReferenceList(&preempt.cyclingPeds, phaseNumbers, phaseCount);

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptCyclingOverlaps(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *
  overlapNumbers,
  uint8_t overlapCount)
{
  IntersectionPreemptConfig_t preempt;

  if ((overlapCount != 0U) && (overlapNumbers == NULL))
  {
    return 0U;
  }

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  CopyOverlapReferenceList(&preempt.cyclingOverlaps,
                           overlapNumbers,
                           overlapCount);

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptEnterYellowChangeDs(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  enterYellowChangeDs)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.enterYellowChangeDs = enterYellowChangeDs;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptEnterRedClearDs(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t enterRedClearDs)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.enterRedClearDs = enterRedClearDs;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptTrackYellowChangeDs(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  trackYellowChangeDs)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.trackYellowChangeDs = trackYellowChangeDs;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptTrackRedClearDs(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t trackRedClearDs)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.trackRedClearDs = trackRedClearDs;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptSequenceNumber(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t sequenceNumber)
{
  IntersectionPreemptConfig_t preempt;
  uint8_t maxSequences = ConfigurationServiceGetSequenceCount(service);

  if ((sequenceNumber == 0U) || (sequenceNumber > maxSequences)
      || (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U))
  {
    return 0U;
  }

  preempt.sequenceNumber = sequenceNumber;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptExitType(ConfigurationService_t *service,
                                               uint8_t preemptIndex,
                                               uint8_t exitType)
{
  IntersectionPreemptConfig_t preempt;

  if (GetCandidatePreempt(service, preemptIndex, &preempt) == 0U)
  {
    return 0U;
  }

  preempt.exitType = exitType;

  return UpdatePreempt(service, preemptIndex, &preempt);
}

uint8_t ConfigurationServiceSetPreemptQueueDelayWeight(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t detectorIndex,
  uint16_t detectorWeight)
{
  if ((service == NULL)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (detectorIndex >= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  return UpdatePreemptQueueDelayWeight(service,
                                       preemptIndex,
                                       detectorIndex,
                                       detectorWeight);
}

uint8_t ConfigurationServiceSetPreemptGateDescription(
  ConfigurationService_t *service,
  uint8_t gateIndex,
  const uint8_t *description,
  uint8_t descriptionLength)
{
  IntersectionPreemptGateConfig_t gate;
  uint8_t index;

  if ((descriptionLength != 0U) && (description == NULL))
  {
    return 0U;
  }

  if (GetCandidatePreemptGate(service, gateIndex, &gate) == 0U)
  {
    return 0U;
  }

  gate.descriptionLength = descriptionLength;

  for (index = 0U; index < INTERSECTION_PREEMPT_GATE_DESCRIPTION_MAX; index++)
  {
    gate.description[index] =
      (index < descriptionLength) ? description[index] : 0U;
  }

  return UpdatePreemptGate(service, gateIndex, &gate);
}

uint8_t ConfigurationServiceReadMigrationJournal(
  ConfigurationService_t *service,
  ConfigurationMigrationJournal_t
  *journal)
{
  uint8_t bytes[CONFIGURATION_MIGRATION_JOURNAL_SIZE];

  if ((service == NULL) || (service->repositoryPort == NULL)
      || (journal == NULL))
  {
    return 0U;
  }

  if (ConfigRepositoryRead(service->repositoryPort,
                           CONFIG_REPOSITORY_OBJECT_MIGRATION_JOURNAL,
                           0U,
                           bytes,
                           sizeof(bytes)) == 0U)
  {
    return 0U;
  }

  JournalDecode(bytes, journal);

  if (journal->magic != CONFIGURATION_MIGRATION_JOURNAL_MAGIC)
  {
    return 0U;
  }

  return 1U;
}

uint8_t ConfigurationServiceWriteMigrationJournal(
  ConfigurationService_t *service,
  const
  ConfigurationMigrationJournal_t
  *journal)
{
  uint8_t bytes[CONFIGURATION_MIGRATION_JOURNAL_SIZE];
  uint32_t capacity;

  if ((service == NULL) || (service->repositoryPort == NULL)
      || (journal == NULL))
  {
    return 0U;
  }

  capacity = ConfigRepositoryGetCapacity(service->repositoryPort,
                                         CONFIG_REPOSITORY_OBJECT_MIGRATION_JOURNAL);

  if (capacity < sizeof(bytes))
  {
    return 0U;
  }

  JournalEncode(journal, bytes);

  if ((ConfigRepositoryErase(service->repositoryPort,
                             CONFIG_REPOSITORY_OBJECT_MIGRATION_JOURNAL,
                             0U,
                             capacity) == 0U)
      || (ConfigRepositoryWrite(service->repositoryPort,
                                CONFIG_REPOSITORY_OBJECT_MIGRATION_JOURNAL,
                                0U,
                                bytes,
                                sizeof(bytes)) == 0U))
  {
    return 0U;
  }

  return 1U;
}
