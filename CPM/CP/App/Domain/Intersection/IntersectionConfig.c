/* App/Domain/Intersection/IntersectionConfig.c
 *
 * Validation and default initialization for the canonical persisted
 * controller configuration.
 */
#include "IntersectionConfig.h"

#include <stddef.h>

#define INTERSECTION_CHANNEL_FLASH_ALLOWED_MASK 0x0EU
#define INTERSECTION_CHANNEL_DIM_ALLOWED_MASK 0x0FU
#define INTERSECTION_TIMEBASE_AUX_FUNCTION_ALLOWED_MASK 0x0FU
#define INTERSECTION_SPLIT_OPTIONS_ALLOWED_MASK 0x01U
#define INTERSECTION_PREEMPT_CONTROL_ALLOWED_MASK 0x3FU
#define INTERSECTION_OVERLAP_TRAIL_GREEN_DS_MAX 2550U
#define INTERSECTION_OVERLAP_TRAIL_YELLOW_DS_MAX 255U
#define INTERSECTION_OVERLAP_TRAIL_RED_DS_MAX 255U
#define INTERSECTION_OVERLAP_WALK_SECONDS_MAX 255U
#define INTERSECTION_OVERLAP_PED_CLEAR_SECONDS_MAX 255U
#define VEHICLE_DETECTOR_OPTIONS_LOCK_MASK \
        (VEHICLE_DETECTOR_OPTIONS_YELLOW_LOCK \
         | VEHICLE_DETECTOR_OPTIONS_RED_LOCK)
#define VEHICLE_DETECTOR_OPTIONS2_ALLOWED_MASK 0x07U
#define PED_DETECTOR_OPTIONS_ALLOWED_MASK 0x07U

static void SetError(IntersectionConfigErrorInfo_t *errorInfo,
                     IntersectionConfigError_t type,
                     uint16_t objectIndex)
{
  if (errorInfo != NULL)
  {
    errorInfo->type = type;
    errorInfo->objectIndex = objectIndex;
  }
}

static uint8_t ValidatePhaseReferenceList(
  const IntersectionPhaseReferenceList_t *list,
  uint8_t phaseCount)
{
  uint8_t seen[INTERSECTION_PHASE_COUNT_MAX] = { 0U };
  uint8_t index;

  if (list == NULL)
  {
    return 0U;
  }

  if (list->length > INTERSECTION_PHASE_COUNT_MAX)
  {
    return 0U;
  }

  for (index = 0U; index < list->length; index++)
  {
    uint8_t phaseNumber = list->values[index];
    uint8_t phaseIndex;

    if ((phaseNumber == 0U) || (phaseNumber > phaseCount))
    {
      return 0U;
    }

    phaseIndex = (uint8_t) (phaseNumber - 1U);

    if (seen[phaseIndex] != 0U)
    {
      return 0U;
    }

    seen[phaseIndex] = 1U;
  }

  return 1U;
}

static uint8_t ValidatePhaseConcurrencyList(
  const IntersectionConfig_t *config,
  uint8_t phaseIndex,
  const IntersectionPhaseReferenceList_t *list)
{
  uint8_t index;

  if ((config == NULL) || (list == NULL) || (phaseIndex >= config->phaseCount))
  {
    return 0U;
  }

  if (ValidatePhaseReferenceList(list, config->phaseCount) == 0U)
  {
    return 0U;
  }

  for (index = 0U; index < list->length; index++)
  {
    uint8_t otherPhaseIndex = (uint8_t) (list->values[index] - 1U);

    if (otherPhaseIndex == phaseIndex)
    {
      return 0U;
    }

    if ((otherPhaseIndex >= config->phaseCount)
        || (IntersectionPhaseOptionsEnabled(
              config->phases[otherPhaseIndex].phaseOptions) == 0U)
        || (config->phases[otherPhaseIndex].ring
            == config->phases[phaseIndex].ring))
    {
      return 0U;
    }
  }

  return 1U;
}

static uint8_t ValidateChannelReferenceList(
  const IntersectionChannelReferenceList_t *list)
{
  uint8_t seen[INTERSECTION_CHANNEL_COUNT_MAX] = { 0U };
  uint8_t index;

  if (list == NULL)
  {
    return 0U;
  }

  if (list->length > INTERSECTION_CHANNEL_COUNT_MAX)
  {
    return 0U;
  }

  for (index = 0U; index < list->length; index++)
  {
    uint8_t channelNumber = list->values[index];
    uint8_t channelIndex;

    if ((channelNumber == 0U)
        || (channelNumber > INTERSECTION_CHANNEL_COUNT_MAX))
    {
      return 0U;
    }

    channelIndex = (uint8_t) (channelNumber - 1U);

    if (seen[channelIndex] != 0U)
    {
      return 0U;
    }

    seen[channelIndex] = 1U;
  }

  return 1U;
}

static uint8_t ValidateOverlapReferenceList(
  const IntersectionOverlapReferenceList_t *list)
{
  uint8_t seen[INTERSECTION_OVERLAP_COUNT_MAX] = { 0U };
  uint8_t index;

  if (list == NULL)
  {
    return 0U;
  }

  if (list->length > INTERSECTION_OVERLAP_COUNT_MAX)
  {
    return 0U;
  }

  for (index = 0U; index < list->length; index++)
  {
    uint8_t overlapNumber = list->values[index];
    uint8_t overlapIndex;

    if ((overlapNumber == 0U)
        || (overlapNumber > INTERSECTION_OVERLAP_COUNT_MAX))
    {
      return 0U;
    }

    overlapIndex = (uint8_t) (overlapNumber - 1U);

    if (seen[overlapIndex] != 0U)
    {
      return 0U;
    }

    seen[overlapIndex] = 1U;
  }

  return 1U;
}

static uint8_t NormalizeVehicleDetectorOptions(uint8_t options)
{
  if ((options & VEHICLE_DETECTOR_OPTIONS_LOCK_MASK)
      == VEHICLE_DETECTOR_OPTIONS_LOCK_MASK)
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

void IntersectionConfigInitDefaults(IntersectionConfig_t *config)
{
  uint8_t i;
  uint8_t ringIndex;
  uint8_t detectorIndex;

  if (config == NULL)
  {
    return;
  }

  config->phaseCount = INTERSECTION_PHASE_COUNT_MAX;
  config->ringCount = INTERSECTION_RING_COUNT_MAX;
  config->barrierCount = INTERSECTION_BARRIER_COUNT_MAX;
  config->reserved = 0U;

  for (i = 0U; i < INTERSECTION_PHASE_COUNT_MAX; i++)
  {
    config->phases[i].phaseOptions = PHASE_OPTIONS_ENABLED;
    config->phases[i].ring = (i < 4U) ? 0U : 1U;
    config->phases[i].startup =
      (uint8_t) INTERSECTION_PHASE_STARTUP_GREEN_NO_WALK;
    config->phases[i].walkSeconds = 7U;
    config->phases[i].pedClearSeconds = 12U;
    config->phases[i].minGreenDs = 50U;
    config->phases[i].phaseMaximum2Ds = 300U;
    config->phases[i].maxGreenDs = 300U;
    config->phases[i].phaseMaximum3Ds = 300U;
    config->phases[i].passageDs = 30U;
    config->phases[i].maxInitialDs = 70U;
    config->phases[i].yellowChangeDs = 40U;
    config->phases[i].redClearDs = 20U;
    config->phases[i].redRevertDs = 0U;
    config->phases[i].addedInitialDs = 0U;
    config->phases[i].timeBeforeReductionSec = 0U;
    config->phases[i].carsBeforeReduction = 0U;
    config->phases[i].timeToReduceSec = 0U;
    config->phases[i].reduceByDs = 0U;
    config->phases[i].minimumGapDs = config->phases[i].passageDs;
    config->phases[i].dynamicMaxLimitSeconds = 0U;
    config->phases[i].dynamicMaxStepDs = 0U;
    config->phases[i].concurrency.length = 0U;
    config->phases[i].yellowRedBeforeEndPedClearDs = 0U;
    config->phases[i].pedWalkService = 1U;
    config->phases[i].dontWalkRevertDs = 0U;
    config->phases[i].pedAlternateClearSeconds = 0U;
    config->phases[i].pedAlternateWalkSeconds = 0U;
    config->phases[i].pedAdvanceWalkDs = 0U;
    config->phases[i].pedDelayDs = 0U;
    config->phases[i].advWarnGrnStartTimeDs = 0U;
    config->phases[i].advWarnRedStartTimeDs = 0U;
    config->phases[i].altMinTimeTransitionSeconds = 0U;
    config->phases[i].reserved = 0U;
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
  {
    config->rings[ringIndex].phaseCount = INTERSECTION_RING_PHASE_COUNT_MAX;
    config->rings[ringIndex].barrierPhaseCount = 2U;
    config->rings[ringIndex].reserved0 = 0U;
    config->rings[ringIndex].reserved1 = 0U;

    for (i = 0U; i < INTERSECTION_RING_PHASE_COUNT_MAX; i++)
    {
      config->rings[ringIndex].phaseOrder[i] =
        (uint8_t) (ringIndex * INTERSECTION_RING_PHASE_COUNT_MAX + i);
    }
  }

  config->coordination.operationalMode = 0U;
  config->coordination.correctionMode =
    (uint8_t) INTERSECTION_COORD_CORRECTION_MODE_DWELL;
  config->coordination.maximumMode =
    (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM1;
  config->coordination.forceMode =
    (uint8_t) INTERSECTION_COORD_FORCE_MODE_FLOATING;
  config->coordination.unitCoordSyncPoint =
    (uint8_t) INTERSECTION_UNIT_COORD_SYNC_POINT_FIRST_PHASE_GREEN_BEGIN;
  config->coordination.reserved0 = 0U;
  config->coordination.reserved1 = 0U;
  config->coordination.reserved2 = 0U;

  for (i = 0U; i < INTERSECTION_PATTERN_COUNT_MAX; i++)
  {
    IntersectionPatternConfig_t *pattern = &config->coordination.patterns[i];

    pattern->cycleTimeSeconds = 120U;
    pattern->offsetTimeSeconds = 0U;
    pattern->splitNumber = 1U;
    pattern->sequenceNumber = 1U;
    pattern->coordSyncPoint =
      (uint8_t) INTERSECTION_COORD_SYNC_POINT_FIRST_COORD_GREEN_BEGIN;
    pattern->options =
      (uint8_t) INTERSECTION_PATTERN_OPTIONS_COORD_MAXIMUM_MODE;
    pattern->reserved0 = 0U;
    pattern->reserved1 = 0U;
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_SPLIT_COUNT_MAX; ringIndex++)
  {
    for (i = 0U; i < INTERSECTION_PHASE_COUNT_MAX; i++)
    {
      IntersectionSplitPhaseConfig_t *split =
        &config->coordination.splits[ringIndex][i];

      split->timeSeconds = 0U;
      split->mode = (uint8_t) INTERSECTION_SPLIT_MODE_NONE;
      split->coordPhase = 0U;
      split->options = 0U;
    }
  }

  config->timebase.patternSyncMinutes = 65535U;

  for (i = 0U; i < INTERSECTION_TIMEBASE_ACTION_COUNT_MAX; i++)
  {
    IntersectionTimebaseActionConfig_t *action = &config->timebase.actions[i];

    action->pattern = 0U;
    action->auxiliaryFunction = 0U;
    action->specialFunction = 0U;
    action->enabledLane = 0U;
  }

  config->unit.startUpFlashSeconds = 0U;
  config->unit.autoPedestrianClear =
    (uint8_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_DISABLE;
  config->unit.backupTimeSeconds = 0U;
  config->unit.userDefinedBackupTimeSeconds = 0U;
  config->unit.redRevertDs = 0U;
  config->unit.startUpFlashMode =
    (uint8_t) INTERSECTION_UNIT_STARTUP_FLASH_MODE_AUTO_FLASH;
  config->unit.timeSourceCommanded = (uint8_t) UNIT_CLOCK_SOURCE_GNSS;
  config->unit.elevationOffsetMeters =
    INTERSECTION_UNIT_ELEVATION_OFFSET_UNKNOWN;

  for (i = 0U; i < INTERSECTION_PREEMPT_COUNT_MAX; i++)
  {
    IntersectionPreemptConfig_t *preempt = &config->preempts[i];

    preempt->control = 0U;
    preempt->link = 0U;
    preempt->delaySeconds = 0U;
    preempt->minimumDurationSeconds = 0U;
    preempt->minimumGreenSeconds = 255U;
    preempt->minimumWalkSeconds = 255U;
    preempt->enterPedClearSeconds = 255U;
    preempt->trackGreenSeconds = 0U;
    preempt->dwellGreenSeconds = 10U;
    preempt->maximumPresenceSeconds = 0U;
    preempt->trackPhases.length = 0U;
    preempt->dwellPhases.length = 0U;
    preempt->dwellPeds.length = 0U;
    preempt->exitPhases.length = 0U;
    preempt->trackOverlaps.length = 0U;
    preempt->dwellOverlaps.length = 0U;
    preempt->cyclingPhases.length = 0U;
    preempt->cyclingPeds.length = 0U;
    preempt->cyclingOverlaps.length = 0U;
    preempt->enterYellowChangeDs = 255U;
    preempt->enterRedClearDs = 255U;
    preempt->trackYellowChangeDs = 255U;
    preempt->trackRedClearDs = 255U;
    preempt->sequenceNumber = 1U;
    preempt->exitType = (uint8_t) INTERSECTION_PREEMPT_EXIT_TYPE_EXIT_PHASES;

    for (detectorIndex = 0U;
         detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
         detectorIndex++)
    {
      config->preemptQueueDelayWeights[i][detectorIndex] = 0U;
    }
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    IntersectionVehicleDetectorConfig_t *detector =
      &config->vehicleDetectors[detectorIndex];

    detector->options = (uint8_t) (VEHICLE_DETECTOR_OPTIONS_CALL
                                   | VEHICLE_DETECTOR_OPTIONS_PASSAGE
                                   | VEHICLE_DETECTOR_OPTIONS_YELLOW_LOCK);
    detector->callPhase =
      (detectorIndex < INTERSECTION_PHASE_COUNT_MAX)
      ? (uint8_t) (detectorIndex + 1U)
      : 0U;
    detector->switchPhase = 0U;
    detector->delayDs = 0U;
    detector->extendDs = 0U;
    detector->queueLimitSeconds = 0U;
    detector->noActivityMinutes = 0U;
    detector->maxPresenceMinutes = 0U;
    detector->erraticCountsPerMinute = 0U;
    detector->failTimeSeconds = 255U;
    detector->options2 = 0U;
    detector->pairedDetector = 0U;
    detector->pairedDetectorSpacingCm = 0U;
    detector->avgVehicleLengthCm = 500U;
    detector->detectorLengthCm = 180U;
    detector->travelMode = 1U;
  }

  for (i = 0U; i < INTERSECTION_PED_INPUT_COUNT_MAX; i++)
  {
    IntersectionPedestrianDetectorConfig_t *detector =
      &config->pedestrianDetectors[i];

    detector->callPhase =
      (i < INTERSECTION_PHASE_COUNT_MAX) ? (uint8_t) (i + 1U) : 0U;
    detector->noActivityMinutes = 0U;
    detector->maxPresenceMinutes = 0U;
    detector->erraticCountsPerMinute = 0U;
    detector->apsMinimumActuationDs = 0U;
    detector->options = 0U;
  }

  config->detectorReports.volumeOccupancyPeriodSeconds = 0U;
  config->detectorReports.volumeOccupancyPeriodV3Seconds = 0U;
  config->detectorReports.pedestrianDetectorPeriodSeconds = 0U;
  config->detectorReports.reserved0 = 0U;
  config->detectorReports.reserved1 = 0U;
  config->detectorReports.reserved2 = 0U;

  for (i = 0U; i < INTERSECTION_PREEMPT_GATE_COUNT_MAX; i++)
  {
    config->preemptGates[i].descriptionLength = 0U;
  }

  for (i = 0U; i < INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX; i++)
  {
    config->userDefinedBackupContents[i].oidLength = 0U;
    config->userDefinedBackupContents[i].descriptionLength = 0U;
    config->userDefinedBackupContents[i].reserved0 = 0U;
    config->userDefinedBackupContents[i].reserved1 = 0U;
  }

  for (i = 0U; i < INTERSECTION_PREEMPT_COUNT_MAX; i++)
  {
    config->inputMapping.preemptInputs[i] = (uint8_t) (i + 1U);
    config->inputMapping.preemptControls[i] = (uint8_t) (i + 1U);
  }

  for (i = 0U; i < INTERSECTION_CHANNEL_COUNT_MAX; i++)
  {
    IntersectionChannelConfig_t *channel = &config->channels[i];

    channel->controlSource = 0U;
    channel->controlType = (uint8_t) INTERSECTION_CHANNEL_CONTROL_TYPE_OTHER;
    channel->flashMask = 0U;
    channel->dimMask = 0U;
    channel->greenType = (uint8_t) INTERSECTION_CHANNEL_GREEN_TYPE_OTHER;
    channel->greenIncluded.length = 0U;
    channel->intersectionId = 0U;
  }

  for (i = 0U; i < INTERSECTION_OVERLAP_COUNT_MAX; i++)
  {
    IntersectionOverlapConfig_t *overlap = &config->overlaps[i];

    overlap->type = (uint8_t) INTERSECTION_OVERLAP_TYPE_OTHER;
    overlap->includedPhases.length = 0U;
    overlap->modifierPhases.length = 0U;
    overlap->trailGreenDs = 0U;
    overlap->trailYellowDs = 0U;
    overlap->trailRedDs = 0U;
    overlap->walkSeconds = 0U;
    overlap->pedClearSeconds = 0U;
    overlap->conflictingPedPhases.length = 0U;
  }

  RebuildLegacyInputMapping(config);
} /* IntersectionConfigInitDefaults */

uint8_t IntersectionConfigValidate(const IntersectionConfig_t *config,
                                   IntersectionConfigErrorInfo_t *errorInfo)
{
  uint8_t phaseIndex;
  uint8_t ringIndex;
  uint8_t detectorIndex;
  uint8_t seenDetectors[INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX] = { 0U };
  uint8_t seenPedInputs[INTERSECTION_PED_INPUT_COUNT_MAX] = { 0U };
  uint8_t channelIndex;
  uint8_t overlapIndex;
  uint8_t phaseSeen[INTERSECTION_PHASE_COUNT_MAX] = { 0U };

  if (config == NULL)
  {
    SetError(errorInfo, INTERSECTION_CONFIG_ERROR_STORAGE, 0U);

    return 0U;
  }

  SetError(errorInfo, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  if ((config->phaseCount == 0U)
      || (config->phaseCount > INTERSECTION_PHASE_COUNT_MAX))
  {
    SetError(errorInfo, INTERSECTION_CONFIG_ERROR_PHASE_COUNT, 0U);

    return 0U;
  }

  if ((config->ringCount == 0U)
      || (config->ringCount > INTERSECTION_RING_COUNT_MAX))
  {
    SetError(errorInfo, INTERSECTION_CONFIG_ERROR_RING_COUNT, 0U);

    return 0U;
  }

  if ((config->barrierCount == 0U)
      || (config->barrierCount > INTERSECTION_BARRIER_COUNT_MAX))
  {
    SetError(errorInfo, INTERSECTION_CONFIG_ERROR_BARRIER_COUNT, 0U);

    return 0U;
  }

  if ((config->unit.autoPedestrianClear
       != (uint8_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_DISABLE)
      && (config->unit.autoPedestrianClear
          != (uint8_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_ENABLE))
  {
    SetError(errorInfo,
             INTERSECTION_CONFIG_ERROR_UNIT_AUTO_PEDESTRIAN_CLEAR,
             0U);

    return 0U;
  }

  if ((config->unit.startUpFlashMode
       < (uint8_t) INTERSECTION_UNIT_STARTUP_FLASH_MODE_AUTO_FLASH)
      || (config->unit.startUpFlashMode
          > (uint8_t) INTERSECTION_UNIT_STARTUP_FLASH_MODE_ALL_RED_CONTROLLER_FLASH))
  {
    SetError(errorInfo,
             INTERSECTION_CONFIG_ERROR_UNIT_STARTUP_FLASH_MODE,
             0U);

    return 0U;
  }

  if ((config->unit.timeSourceCommanded
       != (uint8_t) UNIT_CLOCK_SOURCE_RTC_SQWR)
      && (config->unit.timeSourceCommanded
          != (uint8_t) UNIT_CLOCK_SOURCE_GNSS)
      && (config->unit.timeSourceCommanded
          != (uint8_t) UNIT_CLOCK_SOURCE_LINE_SYNC))
  {
    SetError(errorInfo, INTERSECTION_CONFIG_ERROR_UNIT_TIME_SOURCE, 0U);

    return 0U;
  }

  if (config->unit.elevationOffsetMeters
      > INTERSECTION_UNIT_ELEVATION_OFFSET_UNKNOWN)
  {
    SetError(errorInfo, INTERSECTION_CONFIG_ERROR_UNIT_ELEVATION_OFFSET, 0U);

    return 0U;
  }

  if (config->unit.userDefinedBackupTimeSeconds > 16777216UL)
  {
    SetError(errorInfo,
             INTERSECTION_CONFIG_ERROR_UNIT_USER_DEFINED_BACKUP_TIME,
             0U);

    return 0U;
  }

  for (phaseIndex = 0U;
       phaseIndex < INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX;
       phaseIndex++)
  {
    const IntersectionUserDefinedBackupContentConfig_t *content =
      &config->userDefinedBackupContents[phaseIndex];
    uint16_t objectIndex = (uint16_t) phaseIndex + 1U;

    if (content->oidLength > INTERSECTION_USER_DEFINED_BACKUP_OID_COMPONENT_COUNT_MAX)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_UNIT_USER_DEFINED_BACKUP_OID,
               objectIndex);

      return 0U;
    }

    if (content->descriptionLength
        > INTERSECTION_USER_DEFINED_BACKUP_DESCRIPTION_MAX)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_UNIT_USER_DEFINED_BACKUP_DESCRIPTION,
               objectIndex);

      return 0U;
    }
  }

  for (phaseIndex = 0U; phaseIndex < config->phaseCount; phaseIndex++)
  {
    const IntersectionPhaseConfig_t *phase = &config->phases[phaseIndex];
    uint16_t objectIndex = (uint16_t) phaseIndex + 1U;

    if (phase->ring >= config->ringCount)
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_PHASE_RING, objectIndex);

      return 0U;
    }

    if ((phase->startup < (uint8_t) INTERSECTION_PHASE_STARTUP_OTHER)
        || (phase->startup
            > (uint8_t) INTERSECTION_PHASE_STARTUP_RED_CLEAR))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PHASE_STARTUP,
               objectIndex);

      return 0U;
    }

    if (IntersectionPhaseOptionsEnabled(phase->phaseOptions) == 0U)
    {
      continue;
    }

    if (phase->minGreenDs == 0U)
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_MIN_GREEN, objectIndex);

      return 0U;
    }

    if (phase->walkSeconds > 255U)
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_WALK, objectIndex);

      return 0U;
    }

    if (phase->pedClearSeconds > 255U)
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_PED_CLEAR, objectIndex);

      return 0U;
    }

    if (phase->maxGreenDs < phase->minGreenDs)
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_MAX_GREEN, objectIndex);

      return 0U;
    }

    if (phase->yellowChangeDs == 0U)
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_YELLOW_CHANGE, objectIndex);

      return 0U;
    }

    if (phase->redClearDs == 0U)
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_RED_CLEAR, objectIndex);

      return 0U;
    }

    if ((phase->passageDs == 0U) || (phase->passageDs > phase->maxGreenDs))
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_PASSAGE, objectIndex);

      return 0U;
    }

    if ((phase->maxInitialDs == 0U)
        || (phase->maxInitialDs > phase->maxGreenDs)
        || (phase->maxInitialDs < phase->minGreenDs))
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_MAX_INITIAL, objectIndex);

      return 0U;
    }

    if (phase->pedAdvanceWalkDs > 255U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PED_ADVANCE_WALK,
               objectIndex);

      return 0U;
    }

    if (phase->pedDelayDs > 255U)
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_PED_DELAY, objectIndex);

      return 0U;
    }

    if ((phase->phaseMaximum2Ds != 0U)
        && (phase->phaseMaximum2Ds < phase->minGreenDs))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PHASE_MAXIMUM2,
               objectIndex);

      return 0U;
    }

    if ((phase->phaseMaximum3Ds > 60000U)
        || ((phase->phaseMaximum3Ds != 0U)
            && (phase->phaseMaximum3Ds < phase->minGreenDs)))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PHASE_MAXIMUM3,
               objectIndex);

      return 0U;
    }

    if ((phase->minimumGapDs != 0U) && (phase->minimumGapDs > phase->passageDs))
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_MINIMUM_GAP, objectIndex);

      return 0U;
    }

    if ((phase->pedWalkService == 0U)
        || (phase->pedAlternateClearSeconds > 255U)
        || (phase->pedAlternateWalkSeconds > 255U)
        || (phase->advWarnGrnStartTimeDs > 128U)
        || ((phase->altMinTimeTransitionSeconds != 0U)
            && (((uint16_t) phase->altMinTimeTransitionSeconds * 10U)
                < phase->minGreenDs)))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PHASE_ALT_MIN_TIME_TRANSITION,
               objectIndex);

      return 0U;
    }

    if (ValidatePhaseConcurrencyList(config,
                                     phaseIndex,
                                     &phase->concurrency) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PHASE_CONCURRENCY,
               objectIndex);

      return 0U;
    }
  }

  for (ringIndex = 0U; ringIndex < config->ringCount; ringIndex++)
  {
    const IntersectionRingPlan_t *ringPlan = &config->rings[ringIndex];
    uint8_t serviceIndex;

    if ((ringPlan->phaseCount == 0U)
        || (ringPlan->phaseCount > INTERSECTION_RING_PHASE_COUNT_MAX))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_RING_PHASE_COUNT,
               (uint16_t) ringIndex + 1U);

      return 0U;
    }

    if ((ringPlan->barrierPhaseCount == 0U)
        || (ringPlan->barrierPhaseCount > ringPlan->phaseCount))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_RING_BARRIER_POSITION,
               (uint16_t) ringIndex + 1U);

      return 0U;
    }

    for (serviceIndex = 0U; serviceIndex < ringPlan->phaseCount; serviceIndex++)
    {
      uint8_t plannedPhaseIndex = ringPlan->phaseOrder[serviceIndex];

      if (plannedPhaseIndex >= config->phaseCount)
      {
        SetError(errorInfo,
                 INTERSECTION_CONFIG_ERROR_RING_PHASE_ORDER,
                 (uint16_t) ringIndex + 1U);

        return 0U;
      }

      if (phaseSeen[plannedPhaseIndex] != 0U)
      {
        SetError(errorInfo,
                 INTERSECTION_CONFIG_ERROR_RING_PHASE_ORDER,
                 (uint16_t) ringIndex + 1U);

        return 0U;
      }

      if ((IntersectionPhaseOptionsEnabled(
             config->phases[plannedPhaseIndex].phaseOptions) == 0U)
          || (config->phases[plannedPhaseIndex].ring != ringIndex))
      {
        SetError(errorInfo,
                 INTERSECTION_CONFIG_ERROR_PHASE_ASSIGNMENT,
                 (uint16_t) plannedPhaseIndex + 1U);

        return 0U;
      }

      phaseSeen[plannedPhaseIndex] = 1U;
    }
  }

  for (phaseIndex = 0U; phaseIndex < config->phaseCount; phaseIndex++)
  {
    if ((IntersectionPhaseOptionsEnabled(config->phases[phaseIndex].phaseOptions)
         != 0U)
        && (phaseSeen[phaseIndex] == 0U))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PHASE_ASSIGNMENT,
               (uint16_t) phaseIndex + 1U);

      return 0U;
    }
  }

  if ((config->coordination.correctionMode
       < (uint8_t) INTERSECTION_COORD_CORRECTION_MODE_OTHER)
      || (config->coordination.correctionMode
          > (uint8_t) INTERSECTION_COORD_CORRECTION_MODE_SUBTRACT_ONLY))
  {
    SetError(errorInfo, INTERSECTION_CONFIG_ERROR_COORD_CORRECTION_MODE, 0U);

    return 0U;
  }

  if ((config->coordination.maximumMode
       < (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_OTHER)
      || (config->coordination.maximumMode
          > (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM3))
  {
    SetError(errorInfo, INTERSECTION_CONFIG_ERROR_COORD_MAXIMUM_MODE, 0U);

    return 0U;
  }

  if ((config->coordination.forceMode
       < (uint8_t) INTERSECTION_COORD_FORCE_MODE_OTHER)
      || (config->coordination.forceMode
          > (uint8_t) INTERSECTION_COORD_FORCE_MODE_FIXED))
  {
    SetError(errorInfo, INTERSECTION_CONFIG_ERROR_COORD_FORCE_MODE, 0U);

    return 0U;
  }

  if ((config->coordination.unitCoordSyncPoint
       < (uint8_t) INTERSECTION_UNIT_COORD_SYNC_POINT_OTHER)
      || (config->coordination.unitCoordSyncPoint
          > (uint8_t) INTERSECTION_UNIT_COORD_SYNC_POINT_LAST_PHASE_YELLOW_END))
  {
    SetError(errorInfo,
             INTERSECTION_CONFIG_ERROR_COORD_UNIT_SYNC_POINT,
             0U);

    return 0U;
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_PATTERN_COUNT_MAX; ringIndex++)
  {
    const IntersectionPatternConfig_t *pattern =
      &config->coordination.patterns[ringIndex];
    uint16_t objectIndex = (uint16_t) ringIndex + 1U;

    if ((pattern->splitNumber == 0U)
        || (pattern->splitNumber > INTERSECTION_SPLIT_COUNT_MAX))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PATTERN_SPLIT_NUMBER,
               objectIndex);

      return 0U;
    }

    if ((pattern->sequenceNumber == 0U)
        || (pattern->sequenceNumber > 1U))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PATTERN_SEQUENCE_NUMBER,
               objectIndex);

      return 0U;
    }

    if ((pattern->coordSyncPoint
         < (uint8_t) INTERSECTION_COORD_SYNC_POINT_OTHER)
        || (pattern->coordSyncPoint
            > (uint8_t) INTERSECTION_COORD_SYNC_POINT_LAST_COORD_YELLOW_END))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PATTERN_SYNC_POINT,
               objectIndex);

      return 0U;
    }

    if ((pattern->options < (uint8_t) INTERSECTION_PATTERN_OPTIONS_OTHER)
        || (pattern->options > (uint8_t) INTERSECTION_PATTERN_OPTIONS_MAXIMUM3))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PATTERN_OPTIONS,
               objectIndex);

      return 0U;
    }
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_SPLIT_COUNT_MAX; ringIndex++)
  {
    uint8_t splitIndex;

    for (splitIndex = 0U; splitIndex < config->phaseCount; splitIndex++)
    {
      const IntersectionSplitPhaseConfig_t *split =
        &config->coordination.splits[ringIndex][splitIndex];
      uint16_t objectIndex = (uint16_t) (ringIndex
                                         * INTERSECTION_PHASE_COUNT_MAX)
                             + (uint16_t) splitIndex + 1U;

      if ((split->mode < (uint8_t) INTERSECTION_SPLIT_MODE_OTHER)
          || (split->mode > (uint8_t) INTERSECTION_SPLIT_MODE_NON_ACTUATED))
      {
        SetError(errorInfo, INTERSECTION_CONFIG_ERROR_SPLIT_MODE, objectIndex);

        return 0U;
      }

      if (split->coordPhase > 1U)
      {
        SetError(errorInfo,
                 INTERSECTION_CONFIG_ERROR_SPLIT_COORD_PHASE,
                 objectIndex);

        return 0U;
      }

      if ((split->options
           & (uint8_t) (~INTERSECTION_SPLIT_OPTIONS_ALLOWED_MASK)) != 0U)
      {
        SetError(errorInfo, INTERSECTION_CONFIG_ERROR_SPLIT_OPTIONS,
                 objectIndex);

        return 0U;
      }
    }
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_TIMEBASE_ACTION_COUNT_MAX;
       ringIndex++)
  {
    const IntersectionTimebaseActionConfig_t *action =
      &config->timebase.actions[ringIndex];
    uint16_t objectIndex = (uint16_t) ringIndex + 1U;

    if ((action->pattern != 0U)
        && ((action->pattern > INTERSECTION_PATTERN_COUNT_MAX)
            && (action->pattern != 254U) && (action->pattern != 255U)))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_TIMEBASE_ACTION_PATTERN,
               objectIndex);

      return 0U;
    }

    if ((action->auxiliaryFunction
         & (uint8_t) (~INTERSECTION_TIMEBASE_AUX_FUNCTION_ALLOWED_MASK)) != 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_TIMEBASE_ACTION_AUXILIARY_FUNCTION,
               objectIndex);

      return 0U;
    }
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    const IntersectionVehicleDetectorConfig_t *detector =
      &config->vehicleDetectors[detectorIndex];
    uint16_t objectIndex = (uint16_t) detectorIndex + 1U;

    if (NormalizeVehicleDetectorOptions(detector->options) != detector->options)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_VEHICLE_DETECTOR_OPTIONS,
               objectIndex);

      return 0U;
    }

    if (detector->callPhase > config->phaseCount)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_VEHICLE_DETECTOR_CALL_PHASE,
               objectIndex);

      return 0U;
    }

    if (detector->switchPhase > config->phaseCount)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_VEHICLE_DETECTOR_SWITCH_PHASE,
               objectIndex);

      return 0U;
    }

    if (detector->delayDs > 2550U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_VEHICLE_DETECTOR_DELAY,
               objectIndex);

      return 0U;
    }

    if ((detector->options2 & (uint8_t) (~VEHICLE_DETECTOR_OPTIONS2_ALLOWED_MASK))
        != 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_VEHICLE_DETECTOR_OPTIONS2,
               objectIndex);

      return 0U;
    }

    if ((detector->pairedDetector > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
        || (detector->pairedDetector == (uint8_t) (detectorIndex + 1U)))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_VEHICLE_DETECTOR_PAIRED,
               objectIndex);

      return 0U;
    }

    if (((detector->avgVehicleLengthCm == 0U)
         || (detector->avgVehicleLengthCm > 4000U))
        || ((detector->detectorLengthCm != 65535U)
            && ((detector->detectorLengthCm == 0U)
                || (detector->detectorLengthCm > 4000U))))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_VEHICLE_DETECTOR_OPTIONS2,
               objectIndex);

      return 0U;
    }

    if ((detector->travelMode < 1U) || (detector->travelMode > 4U))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_VEHICLE_DETECTOR_TRAVEL_MODE,
               objectIndex);

      return 0U;
    }
  }

  for (detectorIndex = 0U; detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       detectorIndex++)
  {
    const IntersectionPedestrianDetectorConfig_t *detector =
      &config->pedestrianDetectors[detectorIndex];
    uint16_t objectIndex = (uint16_t) detectorIndex + 1U;

    if (detector->callPhase > config->phaseCount)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PED_DETECTOR_CALL_PHASE,
               objectIndex);

      return 0U;
    }

    if ((detector->options & (uint8_t) (~PED_DETECTOR_OPTIONS_ALLOWED_MASK))
        != 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PED_DETECTOR_OPTIONS,
               objectIndex);

      return 0U;
    }
  }

  if ((config->detectorReports.volumeOccupancyPeriodV3Seconds != 0U)
      && (config->detectorReports.volumeOccupancyPeriodV3Seconds > 3600U)
      && (config->detectorReports.volumeOccupancyPeriodV3Seconds != 65535U))
  {
    SetError(errorInfo,
             INTERSECTION_CONFIG_ERROR_VEHICLE_REPORT_PERIOD_V3,
             1U);

    return 0U;
  }

  if ((config->detectorReports.pedestrianDetectorPeriodSeconds != 0U)
      && (config->detectorReports.pedestrianDetectorPeriodSeconds > 3600U)
      && (config->detectorReports.pedestrianDetectorPeriodSeconds != 65534U)
      && (config->detectorReports.pedestrianDetectorPeriodSeconds != 65535U))
  {
    SetError(errorInfo,
             INTERSECTION_CONFIG_ERROR_PED_REPORT_PERIOD,
             1U);

    return 0U;
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_PREEMPT_COUNT_MAX; ringIndex++)
  {
    const IntersectionPreemptConfig_t *preempt = &config->preempts[ringIndex];
    uint16_t objectIndex = (uint16_t) ringIndex + 1U;

    if ((preempt->control
         & (uint8_t) (~INTERSECTION_PREEMPT_CONTROL_ALLOWED_MASK)) != 0U)
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_PREEMPT_CONTROL,
               objectIndex);

      return 0U;
    }

    if (preempt->link > INTERSECTION_PREEMPT_COUNT_MAX)
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_PREEMPT_LINK, objectIndex);

      return 0U;
    }

    if (ValidatePhaseReferenceList(&preempt->trackPhases,
                                   config->phaseCount) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_TRACK_PHASES,
               objectIndex);

      return 0U;
    }

    if (ValidatePhaseReferenceList(&preempt->dwellPhases,
                                   config->phaseCount) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_DWELL_PHASES,
               objectIndex);

      return 0U;
    }

    if (ValidatePhaseReferenceList(&preempt->dwellPeds,
                                   config->phaseCount) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_DWELL_PEDS,
               objectIndex);

      return 0U;
    }

    if (ValidatePhaseReferenceList(&preempt->exitPhases,
                                   config->phaseCount) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_EXIT_PHASES,
               objectIndex);

      return 0U;
    }

    if (ValidateOverlapReferenceList(&preempt->trackOverlaps) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_TRACK_OVERLAPS,
               objectIndex);

      return 0U;
    }

    if (ValidateOverlapReferenceList(&preempt->dwellOverlaps) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_DWELL_OVERLAPS,
               objectIndex);

      return 0U;
    }

    if (ValidatePhaseReferenceList(&preempt->cyclingPhases,
                                   config->phaseCount) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_CYCLING_PHASES,
               objectIndex);

      return 0U;
    }

    if (ValidatePhaseReferenceList(&preempt->cyclingPeds,
                                   config->phaseCount) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_CYCLING_PEDS,
               objectIndex);

      return 0U;
    }

    if (ValidateOverlapReferenceList(&preempt->cyclingOverlaps) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_CYCLING_OVERLAPS,
               objectIndex);

      return 0U;
    }

    if ((preempt->sequenceNumber == 0U)
        || (preempt->sequenceNumber > 1U))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_SEQUENCE_NUMBER,
               objectIndex);

      return 0U;
    }

    if ((preempt->exitType
         < (uint8_t) INTERSECTION_PREEMPT_EXIT_TYPE_EXIT_PHASES)
        || (preempt->exitType
            > (uint8_t) INTERSECTION_PREEMPT_EXIT_TYPE_EXIT_COORD))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_EXIT_TYPE,
               objectIndex);

      return 0U;
    }

    for (detectorIndex = 0U;
         detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
         detectorIndex++)
    {
      uint16_t queueDelayObjectIndex =
        (uint16_t) (ringIndex * INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
        + (uint16_t) detectorIndex + 1U;

      if (config->preemptQueueDelayWeights[ringIndex][detectorIndex] > 1000U)
      {
        SetError(errorInfo,
                 INTERSECTION_CONFIG_ERROR_PREEMPT_QUEUE_DELAY_WEIGHT,
                 queueDelayObjectIndex);

        return 0U;
      }
    }
  }

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    uint16_t objectIndex = (uint16_t) phaseIndex + 1U;

    if (config->inputMapping.phaseDetectors[phaseIndex]
        > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PHASE_DETECTOR_INPUT,
               objectIndex);

      return 0U;
    }

    if (config->inputMapping.phaseDetectors[phaseIndex] != 0U)
    {
      uint8_t detectorMapIndex =
        (uint8_t) (config->inputMapping.phaseDetectors[phaseIndex] - 1U);

      if (seenDetectors[detectorMapIndex] != 0U)
      {
        SetError(errorInfo,
                 INTERSECTION_CONFIG_ERROR_PHASE_DETECTOR_INPUT,
                 objectIndex);

        return 0U;
      }

      seenDetectors[detectorMapIndex] = 1U;
    }

    if (config->inputMapping.phasePedInputs[phaseIndex]
        > INTERSECTION_PED_INPUT_COUNT_MAX)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PHASE_PED_INPUT,
               objectIndex);

      return 0U;
    }

    if (config->inputMapping.phasePedInputs[phaseIndex] != 0U)
    {
      uint8_t pedMapIndex =
        (uint8_t) (config->inputMapping.phasePedInputs[phaseIndex] - 1U);

      if (seenPedInputs[pedMapIndex] != 0U)
      {
        SetError(errorInfo,
                 INTERSECTION_CONFIG_ERROR_PHASE_PED_INPUT,
                 objectIndex);

        return 0U;
      }

      seenPedInputs[pedMapIndex] = 1U;
    }
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_PREEMPT_COUNT_MAX; ringIndex++)
  {
    uint16_t objectIndex = (uint16_t) ringIndex + 1U;

    if (config->inputMapping.preemptInputs[ringIndex]
        > INTERSECTION_PREEMPT_COUNT_MAX)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_INPUT_SOURCE,
               objectIndex);

      return 0U;
    }

    if (config->inputMapping.preemptControls[ringIndex]
        > INTERSECTION_PREEMPT_COUNT_MAX)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_CONTROL_SOURCE,
               objectIndex);

      return 0U;
    }
  }

  for (channelIndex = 0U;
       channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    const IntersectionChannelConfig_t *channel =
      &config->channels[channelIndex];
    uint16_t objectIndex = (uint16_t) channelIndex + 1U;

    if ((channel->controlType
         < (uint8_t) INTERSECTION_CHANNEL_CONTROL_TYPE_OTHER)
        || (channel->controlType
            > (uint8_t) INTERSECTION_CHANNEL_CONTROL_TYPE_QUEUE_JUMP))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_CHANNEL_CONTROL_TYPE,
               objectIndex);

      return 0U;
    }

    if ((channel->flashMask
         & (uint8_t) (~INTERSECTION_CHANNEL_FLASH_ALLOWED_MASK)) != 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_CHANNEL_FLASH_MASK,
               objectIndex);

      return 0U;
    }

    if ((channel->dimMask
         & (uint8_t) (~INTERSECTION_CHANNEL_DIM_ALLOWED_MASK)) != 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_CHANNEL_DIM_MASK,
               objectIndex);

      return 0U;
    }

    if ((channel->greenType < (uint8_t) INTERSECTION_CHANNEL_GREEN_TYPE_OTHER)
        || (channel->greenType
            > (uint8_t) INTERSECTION_CHANNEL_GREEN_TYPE_FLASH_RED))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_CHANNEL_GREEN_TYPE,
               objectIndex);

      return 0U;
    }

    if (ValidateChannelReferenceList(&channel->greenIncluded) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_CHANNEL_GREEN_INCLUDED,
               objectIndex);

      return 0U;
    }

    if (channel->controlSource != 0U)
    {
      switch ((IntersectionChannelControlType_t) channel->controlType)
      {
          case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE:
          case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN:
          {
            if (channel->controlSource > config->phaseCount)
            {
              SetError(errorInfo,
                       INTERSECTION_CONFIG_ERROR_CHANNEL_CONTROL_SOURCE,
                       objectIndex);

              return 0U;
            }

            break;
          }

          case INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP:
          case INTERSECTION_CHANNEL_CONTROL_TYPE_PED_OVERLAP:
          case INTERSECTION_CHANNEL_CONTROL_TYPE_QUEUE_JUMP:
          {
            if (channel->controlSource > INTERSECTION_OVERLAP_COUNT_MAX)
            {
              SetError(errorInfo,
                       INTERSECTION_CONFIG_ERROR_CHANNEL_CONTROL_SOURCE,
                       objectIndex);

              return 0U;
            }

            break;
          }

          case INTERSECTION_CHANNEL_CONTROL_TYPE_OTHER:
          default:
          {
            SetError(errorInfo,
                     INTERSECTION_CONFIG_ERROR_CHANNEL_CONTROL_SOURCE,
                     objectIndex);

            return 0U;
          }
      } /* switch */
    }
  }

  for (overlapIndex = 0U;
       overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX;
       overlapIndex++)
  {
    const IntersectionOverlapConfig_t *overlap =
      &config->overlaps[overlapIndex];
    uint16_t objectIndex = (uint16_t) overlapIndex + 1U;

    if ((overlap->type < (uint8_t) INTERSECTION_OVERLAP_TYPE_OTHER)
        || (overlap->type
            > (uint8_t) INTERSECTION_OVERLAP_TYPE_MINUS_GREEN_YELLOW_ALTERNATE))
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_OVERLAP_TYPE, objectIndex);

      return 0U;
    }

    if (ValidatePhaseReferenceList(&overlap->includedPhases,
                                   config->phaseCount) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_OVERLAP_INCLUDED_PHASES,
               objectIndex);

      return 0U;
    }

    if (ValidatePhaseReferenceList(&overlap->modifierPhases,
                                   config->phaseCount) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_OVERLAP_MODIFIER_PHASES,
               objectIndex);

      return 0U;
    }

    if (ValidatePhaseReferenceList(&overlap->conflictingPedPhases,
                                   config->phaseCount) == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_OVERLAP_CONFLICTING_PED_PHASES,
               objectIndex);

      return 0U;
    }

    if (overlap->trailGreenDs > INTERSECTION_OVERLAP_TRAIL_GREEN_DS_MAX)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_OVERLAP_TRAIL_GREEN,
               objectIndex);

      return 0U;
    }

    if (overlap->trailYellowDs > INTERSECTION_OVERLAP_TRAIL_YELLOW_DS_MAX)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_OVERLAP_TRAIL_YELLOW,
               objectIndex);

      return 0U;
    }

    if (overlap->trailRedDs > INTERSECTION_OVERLAP_TRAIL_RED_DS_MAX)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_OVERLAP_TRAIL_RED,
               objectIndex);

      return 0U;
    }

    if (overlap->walkSeconds > INTERSECTION_OVERLAP_WALK_SECONDS_MAX)
    {
      SetError(errorInfo, INTERSECTION_CONFIG_ERROR_OVERLAP_WALK, objectIndex);

      return 0U;
    }

    if (overlap->pedClearSeconds > INTERSECTION_OVERLAP_PED_CLEAR_SECONDS_MAX)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_OVERLAP_PED_CLEAR,
               objectIndex);

      return 0U;
    }

    if (overlap->type == (uint8_t) INTERSECTION_OVERLAP_TYPE_OTHER)
    {
      continue;
    }

    if (overlap->includedPhases.length == 0U)
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_OVERLAP_INCLUDED_PHASES,
               objectIndex);

      return 0U;
    }

    switch ((IntersectionOverlapType_t) overlap->type)
    {
        case INTERSECTION_OVERLAP_TYPE_NORMAL:
        case INTERSECTION_OVERLAP_TYPE_PEDESTRIAN_NORMAL:
        case INTERSECTION_OVERLAP_TYPE_TRANSIT_2:
        {
          break;
        }

        case INTERSECTION_OVERLAP_TYPE_MINUS_GREEN_YELLOW:
        case INTERSECTION_OVERLAP_TYPE_FYA_THREE_SECTION:
        case INTERSECTION_OVERLAP_TYPE_FYA_FOUR_SECTION:
        case INTERSECTION_OVERLAP_TYPE_FRA_THREE_SECTION:
        case INTERSECTION_OVERLAP_TYPE_FRA_FOUR_SECTION:
        case INTERSECTION_OVERLAP_TYPE_MINUS_GREEN_YELLOW_ALTERNATE:
        {
          if (overlap->modifierPhases.length == 0U)
          {
            SetError(errorInfo,
                     INTERSECTION_CONFIG_ERROR_OVERLAP_MODIFIER_PHASES,
                     objectIndex);

            return 0U;
          }

          break;
        }

        case INTERSECTION_OVERLAP_TYPE_OTHER:
        default:
        {
          SetError(errorInfo, INTERSECTION_CONFIG_ERROR_OVERLAP_TYPE, objectIndex);

          return 0U;
        }
    }
  }

  return 1U;
} /* IntersectionConfigValidate */

uint8_t IntersectionConfigValidateRuntimeSupport(
  const IntersectionConfig_t *config,
  IntersectionConfigErrorInfo_t *errorInfo)
{
  uint8_t channelIndex;
  uint8_t overlapIndex;
  uint8_t preemptIndex;

  if (config == NULL)
  {
    SetError(errorInfo, INTERSECTION_CONFIG_ERROR_STORAGE, 0U);

    return 0U;
  }

  SetError(errorInfo, INTERSECTION_CONFIG_ERROR_NONE, 0U);

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       ++channelIndex)
  {
    const IntersectionChannelConfig_t *channel = &config->channels[channelIndex];
    uint16_t objectIndex = (uint16_t) channelIndex + 1U;

    if (channel->controlSource == 0U)
    {
      continue;
    }

    if (((IntersectionChannelControlType_t) channel->controlType
         == INTERSECTION_CHANNEL_CONTROL_TYPE_PED_OVERLAP)
        || ((IntersectionChannelControlType_t) channel->controlType
            == INTERSECTION_CHANNEL_CONTROL_TYPE_QUEUE_JUMP))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_CHANNEL_CONTROL_TYPE,
               objectIndex);

      return 0U;
    }
  }

  for (overlapIndex = 0U; overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX;
       ++overlapIndex)
  {
    const IntersectionOverlapConfig_t *overlap = &config->overlaps[overlapIndex];
    uint16_t objectIndex = (uint16_t) overlapIndex + 1U;

    if ((overlap->type == (uint8_t) INTERSECTION_OVERLAP_TYPE_OTHER)
        || (overlap->includedPhases.length == 0U))
    {
      continue;
    }

    switch ((IntersectionOverlapType_t) overlap->type)
    {
        case INTERSECTION_OVERLAP_TYPE_NORMAL:
        case INTERSECTION_OVERLAP_TYPE_MINUS_GREEN_YELLOW:
        case INTERSECTION_OVERLAP_TYPE_MINUS_GREEN_YELLOW_ALTERNATE:
        case INTERSECTION_OVERLAP_TYPE_FYA_THREE_SECTION:
        case INTERSECTION_OVERLAP_TYPE_FYA_FOUR_SECTION:
        case INTERSECTION_OVERLAP_TYPE_FRA_THREE_SECTION:
        case INTERSECTION_OVERLAP_TYPE_FRA_FOUR_SECTION:
        {
          break;
        }

        case INTERSECTION_OVERLAP_TYPE_PEDESTRIAN_NORMAL:
        case INTERSECTION_OVERLAP_TYPE_TRANSIT_2:
        default:
        {
          SetError(errorInfo, INTERSECTION_CONFIG_ERROR_OVERLAP_TYPE, objectIndex);

          return 0U;
        }
    }
  }

  for (preemptIndex = 0U; preemptIndex < INTERSECTION_PREEMPT_COUNT_MAX;
       ++preemptIndex)
  {
    const IntersectionPreemptConfig_t *preempt = &config->preempts[preemptIndex];
    uint16_t objectIndex = (uint16_t) preemptIndex + 1U;

    if (preempt->control == 0U)
    {
      continue;
    }

    if ((preempt->sequenceNumber != 0U) && (preempt->sequenceNumber != 1U))
    {
      SetError(errorInfo,
               INTERSECTION_CONFIG_ERROR_PREEMPT_SEQUENCE_NUMBER,
               objectIndex);

      return 0U;
    }

  }

  return 1U;
} /* IntersectionConfigValidateRuntimeSupport */
