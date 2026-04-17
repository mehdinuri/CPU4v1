/* App/Domain/NTCIP/Mib1202v0335/PhaseObjects.c
 *
 * Controller-core slice of the 1202 phase subtree. Values are exposed in
 * MIB units even though most timing values are stored internally in tenths.
 */
#include "PhaseObjects.h"

#include <stddef.h>

enum
{
  PHASE_OBJECT_TAG_MAX_PHASES = 1,
  PHASE_OBJECT_TAG_PHASE_NUMBER,
  PHASE_OBJECT_TAG_PHASE_WALK,
  PHASE_OBJECT_TAG_PHASE_PEDESTRIAN_CLEAR,
  PHASE_OBJECT_TAG_PHASE_MINIMUM_GREEN,
  PHASE_OBJECT_TAG_PHASE_PASSAGE,
  PHASE_OBJECT_TAG_PHASE_MAXIMUM1,
  PHASE_OBJECT_TAG_PHASE_MAXIMUM2,
  PHASE_OBJECT_TAG_PHASE_YELLOW_CHANGE,
  PHASE_OBJECT_TAG_PHASE_RED_CLEAR,
  PHASE_OBJECT_TAG_PHASE_RED_REVERT,
  PHASE_OBJECT_TAG_PHASE_ADDED_INITIAL,
  PHASE_OBJECT_TAG_PHASE_MAXIMUM_INITIAL,
  PHASE_OBJECT_TAG_PHASE_TIME_BEFORE_REDUCTION,
  PHASE_OBJECT_TAG_PHASE_CARS_BEFORE_REDUCTION,
  PHASE_OBJECT_TAG_PHASE_TIME_TO_REDUCE,
  PHASE_OBJECT_TAG_PHASE_REDUCE_BY,
  PHASE_OBJECT_TAG_PHASE_MINIMUM_GAP,
  PHASE_OBJECT_TAG_PHASE_DYNAMIC_MAX_LIMIT,
  PHASE_OBJECT_TAG_PHASE_DYNAMIC_MAX_STEP,
  PHASE_OBJECT_TAG_PHASE_STARTUP,
  PHASE_OBJECT_TAG_PHASE_OPTIONS,
  PHASE_OBJECT_TAG_PHASE_RING,
  PHASE_OBJECT_TAG_PHASE_CONCURRENCY,
  PHASE_OBJECT_TAG_PHASE_MAXIMUM3,
  PHASE_OBJECT_TAG_PHASE_YELLOW_RED_BEFORE_END_PED_CLEAR,
  PHASE_OBJECT_TAG_PHASE_PED_WALK_SERVICE,
  PHASE_OBJECT_TAG_PHASE_DONT_WALK_REVERT,
  PHASE_OBJECT_TAG_PHASE_PED_ALTERNATE_CLEARANCE,
  PHASE_OBJECT_TAG_PHASE_PED_ALTERNATE_WALK,
  PHASE_OBJECT_TAG_PHASE_PED_ADVANCE_WALK,
  PHASE_OBJECT_TAG_PHASE_PED_DELAY,
  PHASE_OBJECT_TAG_PHASE_ADV_WARN_GRN_START_TIME,
  PHASE_OBJECT_TAG_PHASE_ADV_WARN_RED_START_TIME,
  PHASE_OBJECT_TAG_PHASE_ALT_MIN_TIME_TRANSITION
};

static const uint32_t kMaxPhasesOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U
};
static const uint32_t kPhaseNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 1U
};
static const uint32_t kPhaseWalkOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 2U
};
static const uint32_t kPhasePedestrianClearOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 3U
};
static const uint32_t kPhaseMinimumGreenOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 4U
};
static const uint32_t kPhasePassageOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 5U
};
static const uint32_t kPhaseMaximum1Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 6U
};
static const uint32_t kPhaseMaximum2Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 7U
};
static const uint32_t kPhaseYellowChangeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 8U
};
static const uint32_t kPhaseRedClearOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 9U
};
static const uint32_t kPhaseRedRevertOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 10U
};
static const uint32_t kPhaseAddedInitialOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 11U
};
static const uint32_t kPhaseMaximumInitialOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 12U
};
static const uint32_t kPhaseTimeBeforeReductionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 13U
};
static const uint32_t kPhaseCarsBeforeReductionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 14U
};
static const uint32_t kPhaseTimeToReduceOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 15U
};
static const uint32_t kPhaseReduceByOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 16U
};
static const uint32_t kPhaseMinimumGapOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 17U
};
static const uint32_t kPhaseDynamicMaxLimitOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 18U
};
static const uint32_t kPhaseDynamicMaxStepOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 19U
};
static const uint32_t kPhaseStartupOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 20U
};
static const uint32_t kPhaseOptionsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 21U
};
static const uint32_t kPhaseRingOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 22U
};
static const uint32_t kPhaseConcurrencyOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 23U
};
static const uint32_t kPhaseMaximum3Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 24U
};
static const uint32_t kPhaseYellowRedBeforeEndPedClearOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 25U
};
static const uint32_t kPhasePedWalkServiceOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 26U
};
static const uint32_t kPhaseDontWalkRevertOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 27U
};
static const uint32_t kPhasePedAlternateClearanceOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 28U
};
static const uint32_t kPhasePedAlternateWalkOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 29U
};
static const uint32_t kPhasePedAdvanceWalkOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 30U
};
static const uint32_t kPhasePedDelayOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 31U
};
static const uint32_t kPhaseAdvWarnGrnStartTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 32U
};
static const uint32_t kPhaseAdvWarnRedStartTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 33U
};
static const uint32_t kPhaseAltMinTimeTransitionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 34U
};

static NtcipError_t GetPhaseFromIndex(const NtcipContext_t *context,
                                      const uint32_t *indexes,
                                      uint8_t indexCount,
                                      uint8_t *phaseIndex,
                                      IntersectionPhaseConfig_t *phase)
{
  uint8_t phaseCount;

  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 1U)
      || (phaseIndex == NULL) || (phase == NULL)
      || (indexes[0] == 0U))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  phaseCount = ConfigurationServiceGetPhaseCount(context->configurationService);

  if (indexes[0] > phaseCount)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *phaseIndex = (uint8_t) (indexes[0] - 1U);

  if (ConfigurationServiceGetActivePhaseConfig(context->configurationService,
                                               *phaseIndex,
                                               phase) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t ValidatePhaseWriteRequest(const NtcipContext_t *context,
                                              const uint32_t *indexes,
                                              uint8_t indexCount,
                                              const NtcipRequestContext_t *
                                              requestContext,
                                              IntersectionPhaseConfig_t *phase,
                                              uint8_t *phaseIndex)
{
  NtcipError_t error;

  if ((context == NULL) || (context->dbTransactionService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  error = NtcipDbTransactionServiceValidateDatabaseWrite(
    context->dbTransactionService,
    requestContext);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  return GetPhaseFromIndex(context, indexes, indexCount, phaseIndex, phase);
}

static NtcipError_t GetPhaseObject(void *groupContext,
                                   const NtcipObjectDescriptor_t *descriptor,
                                   const uint32_t *indexes,
                                   uint8_t indexCount,
                                   const NtcipRequestContext_t *requestContext,
                                   NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionPhaseConfig_t phase;
  uint8_t phaseIndex;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case PHASE_OBJECT_TAG_MAX_PHASES:
      {
        NtcipValueSetUnsigned32(value,
                                ConfigurationServiceGetPhaseCount(
                                  context->configurationService));

        return NTCIP_ERROR_OK;
      }

      default:
      {
        error = GetPhaseFromIndex(context,
                                  indexes,
                                  indexCount,
                                  &phaseIndex,
                                  &phase);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        break;
      }
  }

  switch (descriptor->tag)
  {
      case PHASE_OBJECT_TAG_PHASE_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_WALK:
      {
        NtcipValueSetUnsigned32(value, phase.walkSeconds);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_PEDESTRIAN_CLEAR:
      {
        NtcipValueSetUnsigned32(value, phase.pedClearSeconds);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_MINIMUM_GREEN:
      {
        NtcipValueSetUnsigned32(value, (uint32_t) (phase.minGreenDs / 10U));

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_PASSAGE:
      {
        NtcipValueSetUnsigned32(value, phase.passageDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_MAXIMUM1:
      {
        NtcipValueSetUnsigned32(value, (uint32_t) (phase.maxGreenDs / 10U));

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_MAXIMUM2:
      {
        NtcipValueSetUnsigned32(value,
                                (uint32_t) (phase.phaseMaximum2Ds / 10U));

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_YELLOW_CHANGE:
      {
        NtcipValueSetUnsigned32(value, phase.yellowChangeDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_RED_CLEAR:
      {
        NtcipValueSetUnsigned32(value, phase.redClearDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_RED_REVERT:
      {
        NtcipValueSetUnsigned32(value, phase.redRevertDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_ADDED_INITIAL:
      {
        NtcipValueSetUnsigned32(value, phase.addedInitialDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_MAXIMUM_INITIAL:
      {
        NtcipValueSetUnsigned32(value, (uint32_t) (phase.maxInitialDs / 10U));

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_TIME_BEFORE_REDUCTION:
      {
        NtcipValueSetUnsigned32(value, phase.timeBeforeReductionSec);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_CARS_BEFORE_REDUCTION:
      {
        NtcipValueSetUnsigned32(value, phase.carsBeforeReduction);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_TIME_TO_REDUCE:
      {
        NtcipValueSetUnsigned32(value, phase.timeToReduceSec);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_REDUCE_BY:
      {
        NtcipValueSetUnsigned32(value, phase.reduceByDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_MINIMUM_GAP:
      {
        NtcipValueSetUnsigned32(value, phase.minimumGapDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_DYNAMIC_MAX_LIMIT:
      {
        NtcipValueSetUnsigned32(value, phase.dynamicMaxLimitSeconds);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_DYNAMIC_MAX_STEP:
      {
        NtcipValueSetUnsigned32(value, phase.dynamicMaxStepDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_STARTUP:
      {
        uint32_t startupValue = (uint32_t) phase.startup;

        if (IntersectionPhaseOptionsEnabled(phase.phaseOptions) == 0U)
        {
          startupValue = (uint32_t) INTERSECTION_PHASE_STARTUP_OTHER;
        }

        NtcipValueSetUnsigned32(value, startupValue);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_OPTIONS:
      {
        NtcipValueSetUnsigned32(value, phase.phaseOptions);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_RING:
      {
        NtcipValueSetUnsigned32(
          value,
          (IntersectionPhaseOptionsEnabled(phase.phaseOptions) != 0U)
          ? ((uint32_t) phase.ring + 1U) : 0U);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_CONCURRENCY:
      {
        return NtcipValueSetOctetString(value,
                                        phase.concurrency.values,
                                        phase.concurrency.length);
      }

      case PHASE_OBJECT_TAG_PHASE_MAXIMUM3:
      {
        NtcipValueSetUnsigned32(value,
                                (uint32_t) (phase.phaseMaximum3Ds / 10U));

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_YELLOW_RED_BEFORE_END_PED_CLEAR:
      {
        NtcipValueSetUnsigned32(value, phase.yellowRedBeforeEndPedClearDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_PED_WALK_SERVICE:
      {
        NtcipValueSetUnsigned32(value, phase.pedWalkService);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_DONT_WALK_REVERT:
      {
        NtcipValueSetUnsigned32(value, phase.dontWalkRevertDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_PED_ALTERNATE_CLEARANCE:
      {
        NtcipValueSetUnsigned32(value, phase.pedAlternateClearSeconds);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_PED_ALTERNATE_WALK:
      {
        NtcipValueSetUnsigned32(value, phase.pedAlternateWalkSeconds);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_PED_ADVANCE_WALK:
      {
        NtcipValueSetUnsigned32(value, phase.pedAdvanceWalkDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_PED_DELAY:
      {
        NtcipValueSetUnsigned32(value, phase.pedDelayDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_ADV_WARN_GRN_START_TIME:
      {
        NtcipValueSetUnsigned32(value, phase.advWarnGrnStartTimeDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_ADV_WARN_RED_START_TIME:
      {
        NtcipValueSetUnsigned32(value, phase.advWarnRedStartTimeDs);

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_ALT_MIN_TIME_TRANSITION:
      {
        NtcipValueSetUnsigned32(value, phase.altMinTimeTransitionSeconds);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetTestPhaseObject(void *groupContext,
                                       const NtcipObjectDescriptor_t *descriptor,
                                       const uint32_t *indexes,
                                       uint8_t indexCount,
                                       const NtcipRequestContext_t *
                                       requestContext,
                                       const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionPhaseConfig_t phase;
  uint8_t phaseIndex;
  NtcipError_t error;

  if ((descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  error = ValidatePhaseWriteRequest(context,
                                    indexes,
                                    indexCount,
                                    requestContext,
                                    &phase,
                                    &phaseIndex);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  (void) phase;
  (void) phaseIndex;

  switch (descriptor->tag)
  {
      case PHASE_OBJECT_TAG_PHASE_WALK:
      case PHASE_OBJECT_TAG_PHASE_PEDESTRIAN_CLEAR:
      case PHASE_OBJECT_TAG_PHASE_MINIMUM_GREEN:
      case PHASE_OBJECT_TAG_PHASE_MAXIMUM1:
      case PHASE_OBJECT_TAG_PHASE_MAXIMUM2:
      case PHASE_OBJECT_TAG_PHASE_MAXIMUM_INITIAL:
      case PHASE_OBJECT_TAG_PHASE_TIME_BEFORE_REDUCTION:
      case PHASE_OBJECT_TAG_PHASE_CARS_BEFORE_REDUCTION:
      case PHASE_OBJECT_TAG_PHASE_TIME_TO_REDUCE:
      case PHASE_OBJECT_TAG_PHASE_DYNAMIC_MAX_LIMIT:
      case PHASE_OBJECT_TAG_PHASE_PED_ALTERNATE_CLEARANCE:
      case PHASE_OBJECT_TAG_PHASE_PED_ALTERNATE_WALK:
      case PHASE_OBJECT_TAG_PHASE_ALT_MIN_TIME_TRANSITION:
      {
        return (value->data.unsigned32 <= 255UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PHASE_OBJECT_TAG_PHASE_PASSAGE:
      case PHASE_OBJECT_TAG_PHASE_YELLOW_CHANGE:
      case PHASE_OBJECT_TAG_PHASE_RED_CLEAR:
      case PHASE_OBJECT_TAG_PHASE_RED_REVERT:
      case PHASE_OBJECT_TAG_PHASE_ADDED_INITIAL:
      case PHASE_OBJECT_TAG_PHASE_REDUCE_BY:
      case PHASE_OBJECT_TAG_PHASE_MINIMUM_GAP:
      case PHASE_OBJECT_TAG_PHASE_DYNAMIC_MAX_STEP:
      case PHASE_OBJECT_TAG_PHASE_YELLOW_RED_BEFORE_END_PED_CLEAR:
      case PHASE_OBJECT_TAG_PHASE_DONT_WALK_REVERT:
      case PHASE_OBJECT_TAG_PHASE_PED_ADVANCE_WALK:
      case PHASE_OBJECT_TAG_PHASE_PED_DELAY:
      case PHASE_OBJECT_TAG_PHASE_ADV_WARN_RED_START_TIME:
      {
        return (value->data.unsigned32 <= 255UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PHASE_OBJECT_TAG_PHASE_ADV_WARN_GRN_START_TIME:
      {
        return (value->data.unsigned32 <= 128UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PHASE_OBJECT_TAG_PHASE_STARTUP:
      {
        return ((value->data.unsigned32
                 >= (uint32_t) INTERSECTION_PHASE_STARTUP_OTHER)
                && (value->data.unsigned32
                    <= (uint32_t) INTERSECTION_PHASE_STARTUP_RED_CLEAR))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PHASE_OBJECT_TAG_PHASE_OPTIONS:
      {
        return (value->data.unsigned32 <= 0xFFFFUL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PHASE_OBJECT_TAG_PHASE_RING:
      {
        if (value->data.unsigned32
            > (uint32_t) ConfigurationServiceGetRingCount(
              context->configurationService))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      case PHASE_OBJECT_TAG_PHASE_CONCURRENCY:
      {
        if (value->type != NTCIP_VALUE_TYPE_OCTET_STRING)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (value->data.octetString.length <= INTERSECTION_PHASE_COUNT_MAX)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PHASE_OBJECT_TAG_PHASE_MAXIMUM3:
      {
        return (value->data.unsigned32 <= 6000UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PHASE_OBJECT_TAG_PHASE_PED_WALK_SERVICE:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= 255UL))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }
}

static NtcipError_t SetValuePhaseObject(void *groupContext,
                                        const NtcipObjectDescriptor_t *
                                        descriptor,
                                        const uint32_t *indexes,
                                        uint8_t indexCount,
                                        const NtcipRequestContext_t *
                                        requestContext,
                                        const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionPhaseConfig_t phase;
  uint8_t phaseIndex;
  NtcipError_t error;
  uint8_t ok;

  if ((context == NULL) || (context->configurationService == NULL)
      || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  error = ValidatePhaseWriteRequest(context,
                                    indexes,
                                    indexCount,
                                    requestContext,
                                    &phase,
                                    &phaseIndex);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case PHASE_OBJECT_TAG_PHASE_WALK:
      {
        ok = ConfigurationServiceSetPhaseWalkSeconds(
          context->configurationService,
          phaseIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_PEDESTRIAN_CLEAR:
      {
        ok = ConfigurationServiceSetPhasePedClearSeconds(
          context->configurationService,
          phaseIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_MINIMUM_GREEN:
      {
        ok = ConfigurationServiceSetPhaseMinGreenDs(
          context->configurationService,
          phaseIndex,
          (uint16_t) (value->data.unsigned32 * 10UL));
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_PASSAGE:
      {
        ok = ConfigurationServiceSetPhasePassageDs(
          context->configurationService,
          phaseIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_MAXIMUM1:
      {
        ok = ConfigurationServiceSetPhaseMaxGreenDs(
          context->configurationService,
          phaseIndex,
          (uint16_t) (value->data.unsigned32 * 10UL));
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_MAXIMUM2:
      {
        ok = ConfigurationServiceSetPhaseMaximum2Ds(
          context->configurationService,
          phaseIndex,
          (uint16_t) (value->data.unsigned32 * 10UL));
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_YELLOW_CHANGE:
      {
        ok = ConfigurationServiceSetPhaseYellowChangeDs(
          context->configurationService,
          phaseIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_RED_CLEAR:
      {
        ok = ConfigurationServiceSetPhaseRedClearDs(
          context->configurationService,
          phaseIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_RED_REVERT:
      {
        ok = ConfigurationServiceSetPhaseRedRevertDs(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_ADDED_INITIAL:
      {
        ok = ConfigurationServiceSetPhaseAddedInitialDs(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_MAXIMUM_INITIAL:
      {
        ok = ConfigurationServiceSetPhaseMaxInitialDs(
          context->configurationService,
          phaseIndex,
          (uint16_t) (value->data.unsigned32 * 10UL));
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_TIME_BEFORE_REDUCTION:
      {
        ok = ConfigurationServiceSetPhaseTimeBeforeReductionSec(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_CARS_BEFORE_REDUCTION:
      {
        ok = ConfigurationServiceSetPhaseCarsBeforeReduction(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_TIME_TO_REDUCE:
      {
        ok = ConfigurationServiceSetPhaseTimeToReduceSec(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_REDUCE_BY:
      {
        ok = ConfigurationServiceSetPhaseReduceByDs(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_MINIMUM_GAP:
      {
        ok = ConfigurationServiceSetPhaseMinimumGapDs(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_DYNAMIC_MAX_LIMIT:
      {
        ok = ConfigurationServiceSetPhaseDynamicMaxLimitSeconds(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_DYNAMIC_MAX_STEP:
      {
        ok = ConfigurationServiceSetPhaseDynamicMaxStepDs(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_STARTUP:
      {
        ok = ConfigurationServiceSetPhaseStartup(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_OPTIONS:
      {
        ok = ConfigurationServiceSetPhaseOptions(
          context->configurationService,
          phaseIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_RING:
      {
        if (value->data.unsigned32 == 0UL)
        {
          ok = ConfigurationServiceSetPhaseEnabled(
            context->configurationService,
            phaseIndex,
            0U);
        }
        else
        {
          ok = ConfigurationServiceSetPhaseRing(
            context->configurationService,
            phaseIndex,
            (uint8_t) (value->data.unsigned32 - 1UL));

          if (ok != 0U)
          {
            ok = ConfigurationServiceSetPhaseEnabled(
              context->configurationService,
              phaseIndex,
              1U);
          }
        }

        break;
      }

      case PHASE_OBJECT_TAG_PHASE_CONCURRENCY:
      {
        ok = ConfigurationServiceSetPhaseConcurrency(
          context->configurationService,
          phaseIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_MAXIMUM3:
      {
        ok = ConfigurationServiceSetPhaseMaximum3Ds(
          context->configurationService,
          phaseIndex,
          (uint16_t) (value->data.unsigned32 * 10UL));
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_YELLOW_RED_BEFORE_END_PED_CLEAR:
      {
        ok = ConfigurationServiceSetPhaseYellowRedBeforeEndPedClearDs(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_PED_WALK_SERVICE:
      {
        ok = ConfigurationServiceSetPhasePedWalkService(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_DONT_WALK_REVERT:
      {
        ok = ConfigurationServiceSetPhaseDontWalkRevertDs(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_PED_ALTERNATE_CLEARANCE:
      {
        ok = ConfigurationServiceSetPhasePedAlternateClearSeconds(
          context->configurationService,
          phaseIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_PED_ALTERNATE_WALK:
      {
        ok = ConfigurationServiceSetPhasePedAlternateWalkSeconds(
          context->configurationService,
          phaseIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_PED_ADVANCE_WALK:
      {
        ok = ConfigurationServiceSetPhasePedAdvanceWalkDs(
          context->configurationService,
          phaseIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_PED_DELAY:
      {
        ok = ConfigurationServiceSetPhasePedDelayDs(
          context->configurationService,
          phaseIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_ADV_WARN_GRN_START_TIME:
      {
        ok = ConfigurationServiceSetPhaseAdvWarnGrnStartTimeDs(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_ADV_WARN_RED_START_TIME:
      {
        ok = ConfigurationServiceSetPhaseAdvWarnRedStartTimeDs(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PHASE_OBJECT_TAG_PHASE_ALT_MIN_TIME_TRANSITION:
      {
        ok = ConfigurationServiceSetPhaseAltMinTimeTransitionSeconds(
          context->configurationService,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }

  return (ok != 0U) ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;
}

static const NtcipObjectDescriptor_t kPhaseObjects[] =
{
  { kMaxPhasesOid, 11U, NTCIP_OBJECT_KIND_SCALAR, 0U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PHASE_OBJECT_TAG_MAX_PHASES,
    GetPhaseObject, NULL, NULL },
  { kPhaseNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_NUMBER, GetPhaseObject, NULL, NULL },
  { kPhaseWalkOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_WALK, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhasePedestrianClearOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_PEDESTRIAN_CLEAR, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseMinimumGreenOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_MINIMUM_GREEN, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhasePassageOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_PASSAGE, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhaseMaximum1Oid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_MAXIMUM1, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhaseMaximum2Oid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_MAXIMUM2, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhaseYellowChangeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_YELLOW_CHANGE, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseRedClearOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_RED_CLEAR, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhaseRedRevertOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_RED_REVERT, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhaseAddedInitialOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_ADDED_INITIAL, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseMaximumInitialOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_MAXIMUM_INITIAL, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseTimeBeforeReductionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_TIME_BEFORE_REDUCTION, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseCarsBeforeReductionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_CARS_BEFORE_REDUCTION, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseTimeToReduceOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_TIME_TO_REDUCE, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseReduceByOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_REDUCE_BY, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhaseMinimumGapOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_MINIMUM_GAP, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseDynamicMaxLimitOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_DYNAMIC_MAX_LIMIT, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseDynamicMaxStepOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_DYNAMIC_MAX_STEP, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseStartupOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_STARTUP, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhaseOptionsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_OPTIONS, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhaseRingOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_RING, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhaseConcurrencyOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    PHASE_OBJECT_TAG_PHASE_CONCURRENCY, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhaseMaximum3Oid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_MAXIMUM3, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhaseYellowRedBeforeEndPedClearOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_YELLOW_RED_BEFORE_END_PED_CLEAR, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhasePedWalkServiceOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_PED_WALK_SERVICE, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseDontWalkRevertOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_DONT_WALK_REVERT, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhasePedAlternateClearanceOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_PED_ALTERNATE_CLEARANCE, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhasePedAlternateWalkOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_PED_ALTERNATE_WALK, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhasePedAdvanceWalkOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_PED_ADVANCE_WALK, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhasePedDelayOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_PED_DELAY, GetPhaseObject, SetTestPhaseObject,
    SetValuePhaseObject },
  { kPhaseAdvWarnGrnStartTimeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_ADV_WARN_GRN_START_TIME, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseAdvWarnRedStartTimeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_ADV_WARN_RED_START_TIME, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject },
  { kPhaseAltMinTimeTransitionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_OBJECT_TAG_PHASE_ALT_MIN_TIME_TRANSITION, GetPhaseObject,
    SetTestPhaseObject, SetValuePhaseObject }
};

void PhaseObjectsRegister(NtcipObjectDirectory_t *directory,
                          NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.phase",
    kPhaseObjects,
    (uint16_t) (sizeof(kPhaseObjects) / sizeof(kPhaseObjects[0])),
    context);
}
