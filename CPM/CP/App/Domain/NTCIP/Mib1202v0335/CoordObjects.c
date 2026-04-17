/* App/Domain/NTCIP/Mib1202v0335/CoordObjects.c
 *
 * 1202 coordination subtree projection backed by the canonical controller
 * configuration and runtime engine state.
 */
#include "CoordObjects.h"

#include <stddef.h>

enum
{
  COORD_OBJECT_TAG_OPERATIONAL_MODE = 1,
  COORD_OBJECT_TAG_CORRECTION_MODE,
  COORD_OBJECT_TAG_MAXIMUM_MODE,
  COORD_OBJECT_TAG_FORCE_MODE,
  COORD_OBJECT_TAG_MAX_PATTERNS,
  COORD_OBJECT_TAG_PATTERN_TABLE_TYPE,
  COORD_OBJECT_TAG_PATTERN_NUMBER,
  COORD_OBJECT_TAG_PATTERN_CYCLE_TIME,
  COORD_OBJECT_TAG_PATTERN_OFFSET_TIME,
  COORD_OBJECT_TAG_PATTERN_SPLIT_NUMBER,
  COORD_OBJECT_TAG_PATTERN_SEQUENCE_NUMBER,
  COORD_OBJECT_TAG_PATTERN_SYNC_POINT,
  COORD_OBJECT_TAG_PATTERN_OPTIONS,
  COORD_OBJECT_TAG_MAX_SPLITS,
  COORD_OBJECT_TAG_SPLIT_NUMBER,
  COORD_OBJECT_TAG_SPLIT_PHASE,
  COORD_OBJECT_TAG_SPLIT_TIME,
  COORD_OBJECT_TAG_SPLIT_MODE,
  COORD_OBJECT_TAG_SPLIT_COORD_PHASE,
  COORD_OBJECT_TAG_SPLIT_OPTIONS,
  COORD_OBJECT_TAG_PATTERN_STATUS,
  COORD_OBJECT_TAG_LOCAL_FREE_STATUS,
  COORD_OBJECT_TAG_CYCLE_STATUS,
  COORD_OBJECT_TAG_SYNC_STATUS,
  COORD_OBJECT_TAG_SYSTEM_PATTERN_CONTROL,
  COORD_OBJECT_TAG_SYSTEM_SYNC_CONTROL,
  COORD_OBJECT_TAG_UNIT_SYNC_POINT
};

enum
{
  COORD_PATTERN_TABLE_TYPE_PATTERNS = 2
};

static const uint32_t kCoordOperationalModeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 1U
};
static const uint32_t kCoordCorrectionModeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 2U
};
static const uint32_t kCoordMaximumModeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 3U
};
static const uint32_t kCoordForceModeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 4U
};
static const uint32_t kMaxPatternsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 5U
};
static const uint32_t kPatternTableTypeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 6U
};
static const uint32_t kPatternNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 7U, 1U, 1U
};
static const uint32_t kPatternCycleTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 7U, 1U, 2U
};
static const uint32_t kPatternOffsetTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 7U, 1U, 3U
};
static const uint32_t kPatternSplitNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 7U, 1U, 4U
};
static const uint32_t kPatternSequenceNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 7U, 1U, 5U
};
static const uint32_t kPatternCoordSyncPointOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 7U, 1U, 6U
};
static const uint32_t kPatternOptionsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 7U, 1U, 7U
};
static const uint32_t kMaxSplitsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 8U
};
static const uint32_t kSplitNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 9U, 1U, 1U
};
static const uint32_t kSplitPhaseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 9U, 1U, 2U
};
static const uint32_t kSplitTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 9U, 1U, 3U
};
static const uint32_t kSplitModeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 9U, 1U, 4U
};
static const uint32_t kSplitCoordPhaseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 9U, 1U, 5U
};
static const uint32_t kSplitOptionsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 9U, 1U, 6U
};
static const uint32_t kCoordPatternStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 10U
};
static const uint32_t kLocalFreeStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 11U
};
static const uint32_t kCoordCycleStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 12U
};
static const uint32_t kCoordSyncStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 13U
};
static const uint32_t kSystemPatternControlOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 14U
};
static const uint32_t kSystemSyncControlOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 15U
};
static const uint32_t kUnitCoordSyncPointOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 16U
};

static NtcipError_t ValidateDatabaseWrite(const NtcipContext_t *context,
                                          const NtcipRequestContext_t *
                                          requestContext)
{
  if ((context == NULL) || (context->dbTransactionService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return NtcipDbTransactionServiceValidateDatabaseWrite(
    context->dbTransactionService,
    requestContext);
}

static NtcipError_t GetPatternFromIndex(const NtcipContext_t *context,
                                        const uint32_t *indexes,
                                        uint8_t indexCount,
                                        uint8_t *patternIndex,
                                        IntersectionPatternConfig_t *pattern)
{
  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 1U)
      || (indexes[0] == 0U) || (patternIndex == NULL) || (pattern == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (indexes[0]
      > ConfigurationServiceGetPatternCount(context->configurationService))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *patternIndex = (uint8_t) (indexes[0] - 1U);

  if (ConfigurationServiceGetActivePatternConfig(context->configurationService,
                                                 *patternIndex,
                                                 pattern) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetSplitFromIndex(const NtcipContext_t *context,
                                      const uint32_t *indexes,
                                      uint8_t indexCount,
                                      uint8_t *splitIndex,
                                      uint8_t *phaseIndex,
                                      IntersectionSplitPhaseConfig_t *split)
{
  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 2U)
      || (indexes[0] == 0U) || (indexes[1] == 0U)
      || (splitIndex == NULL) || (phaseIndex == NULL) || (split == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if ((indexes[0]
       > ConfigurationServiceGetSplitCount(context->configurationService))
      || (indexes[1]
          > ConfigurationServiceGetPhaseCount(context->configurationService)))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *splitIndex = (uint8_t) (indexes[0] - 1U);
  *phaseIndex = (uint8_t) (indexes[1] - 1U);

  if (ConfigurationServiceGetActiveSplitPhaseConfig(
        context->configurationService,
        *splitIndex,
        *phaseIndex,
        split) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetCoordObject(void *groupContext,
                                   const NtcipObjectDescriptor_t *descriptor,
                                   const uint32_t *indexes,
                                   uint8_t indexCount,
                                   const NtcipRequestContext_t *requestContext,
                                   NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  const IntersectionRuntime_t *runtime;
  IntersectionCoordinationConfig_t coordination;
  IntersectionPatternConfig_t pattern;
  IntersectionSplitPhaseConfig_t split;
  uint8_t patternIndex;
  uint8_t splitIndex;
  uint8_t phaseIndex;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL)
      || (context->configurationService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  runtime = IntersectionEngineGetRuntime(context->intersectionEngine);

  switch (descriptor->tag)
  {
      case COORD_OBJECT_TAG_MAX_PATTERNS:
      {
        NtcipValueSetUnsigned32(value,
                                ConfigurationServiceGetPatternCount(
                                  context->configurationService));

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_PATTERN_TABLE_TYPE:
      {
        NtcipValueSetUnsigned32(value, COORD_PATTERN_TABLE_TYPE_PATTERNS);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_MAX_SPLITS:
      {
        NtcipValueSetUnsigned32(value,
                                ConfigurationServiceGetSplitCount(
                                  context->configurationService));

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_PATTERN_STATUS:
      {
        NtcipValueSetUnsigned32(value,
                                (runtime
                                 != NULL) ? runtime->coordPatternStatus : 254U);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_LOCAL_FREE_STATUS:
      {
        NtcipValueSetUnsigned32(
          value,
          (runtime != NULL)
          ? runtime->localFreeStatus
          : (uint32_t) INTERSECTION_LOCAL_FREE_STATUS_OTHER);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_CYCLE_STATUS:
      {
        NtcipValueSetUnsigned32(value,
                                (runtime
                                 != NULL) ? runtime->coordCycleStatusSeconds
                                : 0U);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_SYNC_STATUS:
      {
        NtcipValueSetUnsigned32(value,
                                (runtime
                                 != NULL) ? runtime->coordSyncStatusSeconds
                                : 0U);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_SYSTEM_PATTERN_CONTROL:
      {
        NtcipValueSetUnsigned32(value,
                                (runtime
                                 != NULL) ? runtime->systemPatternControl : 0U);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_SYSTEM_SYNC_CONTROL:
      {
        NtcipValueSetUnsigned32(value,
                                (runtime != NULL)
                                ? runtime->systemSyncControlSeconds
                                : 0U);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_OPERATIONAL_MODE:
      case COORD_OBJECT_TAG_CORRECTION_MODE:
      case COORD_OBJECT_TAG_MAXIMUM_MODE:
      case COORD_OBJECT_TAG_FORCE_MODE:
      case COORD_OBJECT_TAG_UNIT_SYNC_POINT:
      {
        if (ConfigurationServiceGetActiveCoordinationConfig(
              context->configurationService,
              &coordination)
            == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        break;
      }

      case COORD_OBJECT_TAG_PATTERN_NUMBER:
      case COORD_OBJECT_TAG_PATTERN_CYCLE_TIME:
      case COORD_OBJECT_TAG_PATTERN_OFFSET_TIME:
      case COORD_OBJECT_TAG_PATTERN_SPLIT_NUMBER:
      case COORD_OBJECT_TAG_PATTERN_SEQUENCE_NUMBER:
      case COORD_OBJECT_TAG_PATTERN_SYNC_POINT:
      case COORD_OBJECT_TAG_PATTERN_OPTIONS:
      {
        error = GetPatternFromIndex(context,
                                    indexes,
                                    indexCount,
                                    &patternIndex,
                                    &pattern);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        break;
      }

      case COORD_OBJECT_TAG_SPLIT_NUMBER:
      case COORD_OBJECT_TAG_SPLIT_PHASE:
      case COORD_OBJECT_TAG_SPLIT_TIME:
      case COORD_OBJECT_TAG_SPLIT_MODE:
      case COORD_OBJECT_TAG_SPLIT_COORD_PHASE:
      case COORD_OBJECT_TAG_SPLIT_OPTIONS:
      {
        error = GetSplitFromIndex(context,
                                  indexes,
                                  indexCount,
                                  &splitIndex,
                                  &phaseIndex,
                                  &split);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        break;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  } /* switch */

  switch (descriptor->tag)
  {
      case COORD_OBJECT_TAG_OPERATIONAL_MODE:
      {
        NtcipValueSetUnsigned32(value, coordination.operationalMode);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_CORRECTION_MODE:
      {
        NtcipValueSetUnsigned32(value, coordination.correctionMode);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_MAXIMUM_MODE:
      {
        NtcipValueSetUnsigned32(value, coordination.maximumMode);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_FORCE_MODE:
      {
        NtcipValueSetUnsigned32(value, coordination.forceMode);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_UNIT_SYNC_POINT:
      {
        NtcipValueSetUnsigned32(value, coordination.unitCoordSyncPoint);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_PATTERN_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_PATTERN_CYCLE_TIME:
      {
        NtcipValueSetUnsigned32(value, pattern.cycleTimeSeconds);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_PATTERN_OFFSET_TIME:
      {
        NtcipValueSetUnsigned32(value, pattern.offsetTimeSeconds);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_PATTERN_SPLIT_NUMBER:
      {
        NtcipValueSetUnsigned32(value, pattern.splitNumber);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_PATTERN_SEQUENCE_NUMBER:
      {
        NtcipValueSetUnsigned32(value, pattern.sequenceNumber);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_PATTERN_SYNC_POINT:
      {
        NtcipValueSetUnsigned32(value, pattern.coordSyncPoint);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_PATTERN_OPTIONS:
      {
        NtcipValueSetUnsigned32(value, pattern.options);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_SPLIT_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_SPLIT_PHASE:
      {
        NtcipValueSetUnsigned32(value, indexes[1]);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_SPLIT_TIME:
      {
        NtcipValueSetUnsigned32(value, split.timeSeconds);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_SPLIT_MODE:
      {
        NtcipValueSetUnsigned32(value, split.mode);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_SPLIT_COORD_PHASE:
      {
        NtcipValueSetUnsigned32(value, split.coordPhase);

        return NTCIP_ERROR_OK;
      }

      case COORD_OBJECT_TAG_SPLIT_OPTIONS:
      {
        NtcipValueSetUnsigned32(value, split.options);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  } /* switch */
} /* GetCoordObject */

static NtcipError_t SetTestCoordObject(void *groupContext,
                                       const NtcipObjectDescriptor_t *descriptor,
                                       const uint32_t *indexes,
                                       uint8_t indexCount,
                                       const NtcipRequestContext_t *
                                       requestContext,
                                       const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  NtcipError_t error = NTCIP_ERROR_OK;
  IntersectionPatternConfig_t pattern;
  IntersectionSplitPhaseConfig_t split;
  uint8_t patternIndex;
  uint8_t splitIndex;
  uint8_t phaseIndex;

  switch (descriptor->tag)
  {
      case COORD_OBJECT_TAG_SYSTEM_PATTERN_CONTROL:
      case COORD_OBJECT_TAG_SYSTEM_SYNC_CONTROL:
      {
        break;
      }

      case COORD_OBJECT_TAG_OPERATIONAL_MODE:
      case COORD_OBJECT_TAG_CORRECTION_MODE:
      case COORD_OBJECT_TAG_MAXIMUM_MODE:
      case COORD_OBJECT_TAG_FORCE_MODE:
      case COORD_OBJECT_TAG_UNIT_SYNC_POINT:
      {
        error = ValidateDatabaseWrite(context, requestContext);
        break;
      }

      case COORD_OBJECT_TAG_PATTERN_CYCLE_TIME:
      case COORD_OBJECT_TAG_PATTERN_OFFSET_TIME:
      case COORD_OBJECT_TAG_PATTERN_SPLIT_NUMBER:
      case COORD_OBJECT_TAG_PATTERN_SEQUENCE_NUMBER:
      case COORD_OBJECT_TAG_PATTERN_SYNC_POINT:
      case COORD_OBJECT_TAG_PATTERN_OPTIONS:
      {
        error = ValidateDatabaseWrite(context, requestContext);

        if (error == NTCIP_ERROR_OK)
        {
          error = GetPatternFromIndex(context,
                                      indexes,
                                      indexCount,
                                      &patternIndex,
                                      &pattern);
        }

        break;
      }

      case COORD_OBJECT_TAG_SPLIT_TIME:
      case COORD_OBJECT_TAG_SPLIT_MODE:
      case COORD_OBJECT_TAG_SPLIT_COORD_PHASE:
      case COORD_OBJECT_TAG_SPLIT_OPTIONS:
      {
        error = ValidateDatabaseWrite(context, requestContext);

        if (error == NTCIP_ERROR_OK)
        {
          error = GetSplitFromIndex(context,
                                    indexes,
                                    indexCount,
                                    &splitIndex,
                                    &phaseIndex,
                                    &split);
        }

        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  } /* switch */

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case COORD_OBJECT_TAG_OPERATIONAL_MODE:
      case COORD_OBJECT_TAG_SYSTEM_PATTERN_CONTROL:
      case COORD_OBJECT_TAG_SYSTEM_SYNC_CONTROL:
      {
        return (value->data.unsigned32 <= 255UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case COORD_OBJECT_TAG_CORRECTION_MODE:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= 5UL))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case COORD_OBJECT_TAG_MAXIMUM_MODE:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= 5UL))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case COORD_OBJECT_TAG_FORCE_MODE:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= 3UL))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case COORD_OBJECT_TAG_UNIT_SYNC_POINT:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= 7UL))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case COORD_OBJECT_TAG_PATTERN_CYCLE_TIME:
      case COORD_OBJECT_TAG_PATTERN_OFFSET_TIME:
      case COORD_OBJECT_TAG_SPLIT_TIME:
      {
        return (value->data.unsigned32 <= 255UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case COORD_OBJECT_TAG_PATTERN_SPLIT_NUMBER:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= INTERSECTION_SPLIT_COUNT_MAX))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case COORD_OBJECT_TAG_PATTERN_SEQUENCE_NUMBER:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32
                    <= ConfigurationServiceGetSequenceCount(
                      context->configurationService)))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case COORD_OBJECT_TAG_PATTERN_SYNC_POINT:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= 8UL))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case COORD_OBJECT_TAG_PATTERN_OPTIONS:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= 6UL))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case COORD_OBJECT_TAG_SPLIT_MODE:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= 8UL))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case COORD_OBJECT_TAG_SPLIT_COORD_PHASE:
      {
        return (value->data.unsigned32 <= 1UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case COORD_OBJECT_TAG_SPLIT_OPTIONS:
      {
        return ((value->data.unsigned32 & ~0x01UL) == 0UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  } /* switch */
} /* SetTestCoordObject */

static NtcipError_t SetValueCoordObject(void *groupContext,
                                        const NtcipObjectDescriptor_t *
                                        descriptor,
                                        const uint32_t *indexes,
                                        uint8_t indexCount,
                                        const NtcipRequestContext_t *
                                        requestContext,
                                        const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  NtcipError_t error = NTCIP_ERROR_OK;
  IntersectionPatternConfig_t pattern;
  IntersectionSplitPhaseConfig_t split;
  uint8_t patternIndex;
  uint8_t splitIndex;
  uint8_t phaseIndex;
  uint8_t ok = 0U;

  switch (descriptor->tag)
  {
      case COORD_OBJECT_TAG_SYSTEM_PATTERN_CONTROL:
      case COORD_OBJECT_TAG_SYSTEM_SYNC_CONTROL:
      {
        break;
      }

      case COORD_OBJECT_TAG_OPERATIONAL_MODE:
      case COORD_OBJECT_TAG_CORRECTION_MODE:
      case COORD_OBJECT_TAG_MAXIMUM_MODE:
      case COORD_OBJECT_TAG_FORCE_MODE:
      case COORD_OBJECT_TAG_UNIT_SYNC_POINT:
      {
        error = ValidateDatabaseWrite(context, requestContext);
        break;
      }

      case COORD_OBJECT_TAG_PATTERN_CYCLE_TIME:
      case COORD_OBJECT_TAG_PATTERN_OFFSET_TIME:
      case COORD_OBJECT_TAG_PATTERN_SPLIT_NUMBER:
      case COORD_OBJECT_TAG_PATTERN_SEQUENCE_NUMBER:
      case COORD_OBJECT_TAG_PATTERN_SYNC_POINT:
      case COORD_OBJECT_TAG_PATTERN_OPTIONS:
      {
        error = ValidateDatabaseWrite(context, requestContext);

        if (error == NTCIP_ERROR_OK)
        {
          error = GetPatternFromIndex(context,
                                      indexes,
                                      indexCount,
                                      &patternIndex,
                                      &pattern);
        }

        break;
      }

      case COORD_OBJECT_TAG_SPLIT_TIME:
      case COORD_OBJECT_TAG_SPLIT_MODE:
      case COORD_OBJECT_TAG_SPLIT_COORD_PHASE:
      case COORD_OBJECT_TAG_SPLIT_OPTIONS:
      {
        error = ValidateDatabaseWrite(context, requestContext);

        if (error == NTCIP_ERROR_OK)
        {
          error = GetSplitFromIndex(context,
                                    indexes,
                                    indexCount,
                                    &splitIndex,
                                    &phaseIndex,
                                    &split);
        }

        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  } /* switch */

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case COORD_OBJECT_TAG_OPERATIONAL_MODE:
      {
        ok = ConfigurationServiceSetCoordOperationalMode(
          context->configurationService,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_CORRECTION_MODE:
      {
        ok = ConfigurationServiceSetCoordCorrectionMode(
          context->configurationService,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_MAXIMUM_MODE:
      {
        ok = ConfigurationServiceSetCoordMaximumMode(
          context->configurationService,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_FORCE_MODE:
      {
        ok = ConfigurationServiceSetCoordForceMode(
          context->configurationService,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_UNIT_SYNC_POINT:
      {
        ok = ConfigurationServiceSetCoordUnitSyncPoint(
          context->configurationService,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_PATTERN_CYCLE_TIME:
      {
        ok = ConfigurationServiceSetPatternCycleTimeSeconds(
          context->configurationService,
          patternIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_PATTERN_OFFSET_TIME:
      {
        ok = ConfigurationServiceSetPatternOffsetTimeSeconds(
          context->configurationService,
          patternIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_PATTERN_SPLIT_NUMBER:
      {
        ok = ConfigurationServiceSetPatternSplitNumber(
          context->configurationService,
          patternIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_PATTERN_SEQUENCE_NUMBER:
      {
        ok = ConfigurationServiceSetPatternSequenceNumber(
          context->configurationService,
          patternIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_PATTERN_SYNC_POINT:
      {
        ok = ConfigurationServiceSetPatternCoordSyncPoint(
          context->configurationService,
          patternIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_PATTERN_OPTIONS:
      {
        ok = ConfigurationServiceSetPatternOptions(
          context->configurationService,
          patternIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_SPLIT_TIME:
      {
        ok = ConfigurationServiceSetSplitTimeSeconds(
          context->configurationService,
          splitIndex,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_SPLIT_MODE:
      {
        ok = ConfigurationServiceSetSplitMode(
          context->configurationService,
          splitIndex,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_SPLIT_COORD_PHASE:
      {
        ok = ConfigurationServiceSetSplitCoordPhase(
          context->configurationService,
          splitIndex,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_SPLIT_OPTIONS:
      {
        ok = ConfigurationServiceSetSplitOptions(
          context->configurationService,
          splitIndex,
          phaseIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_SYSTEM_PATTERN_CONTROL:
      {
        ok = IntersectionEngineSetSystemPatternControl(
          context->intersectionEngine,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case COORD_OBJECT_TAG_SYSTEM_SYNC_CONTROL:
      {
        ok = IntersectionEngineSetSystemSyncControl(
          context->intersectionEngine,
          (uint8_t) value->data.unsigned32);
        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  } /* switch */

  return (ok != 0U) ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;
} /* SetValueCoordObject */

static const NtcipObjectDescriptor_t kCoordObjects[] =
{
  { kCoordOperationalModeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_OPERATIONAL_MODE,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kCoordCorrectionModeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_CORRECTION_MODE,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kCoordMaximumModeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_MAXIMUM_MODE,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kCoordForceModeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_FORCE_MODE,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kMaxPatternsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_MAX_PATTERNS,
    GetCoordObject, NULL, NULL },
  { kPatternTableTypeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_PATTERN_TABLE_TYPE,
    GetCoordObject, NULL, NULL },
  { kPatternNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_PATTERN_NUMBER,
    GetCoordObject, NULL, NULL },
  { kPatternCycleTimeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_PATTERN_CYCLE_TIME,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kPatternOffsetTimeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_PATTERN_OFFSET_TIME,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kPatternSplitNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_PATTERN_SPLIT_NUMBER,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kPatternSequenceNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_PATTERN_SEQUENCE_NUMBER,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kPatternCoordSyncPointOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_PATTERN_SYNC_POINT,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kPatternOptionsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_PATTERN_OPTIONS,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kMaxSplitsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_MAX_SPLITS,
    GetCoordObject, NULL, NULL },
  { kSplitNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_SPLIT_NUMBER,
    GetCoordObject, NULL, NULL },
  { kSplitPhaseOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_SPLIT_PHASE,
    GetCoordObject, NULL, NULL },
  { kSplitTimeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_SPLIT_TIME,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kSplitModeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_SPLIT_MODE,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kSplitCoordPhaseOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_SPLIT_COORD_PHASE,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kSplitOptionsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_SPLIT_OPTIONS,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kCoordPatternStatusOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_PATTERN_STATUS,
    GetCoordObject, NULL, NULL },
  { kLocalFreeStatusOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_LOCAL_FREE_STATUS,
    GetCoordObject, NULL, NULL },
  { kCoordCycleStatusOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_CYCLE_STATUS,
    GetCoordObject, NULL, NULL },
  { kCoordSyncStatusOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_SYNC_STATUS,
    GetCoordObject, NULL, NULL },
  { kSystemPatternControlOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_SYSTEM_PATTERN_CONTROL,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kSystemSyncControlOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_SYSTEM_SYNC_CONTROL,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject },
  { kUnitCoordSyncPointOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, COORD_OBJECT_TAG_UNIT_SYNC_POINT,
    GetCoordObject, SetTestCoordObject, SetValueCoordObject }
};

void CoordObjectsRegister(NtcipObjectDirectory_t *directory,
                          NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.coord",
    kCoordObjects,
    (uint16_t) (sizeof(kCoordObjects) / sizeof(kCoordObjects[0])),
    context);
}
