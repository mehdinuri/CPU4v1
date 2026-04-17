/* App/Domain/NTCIP/Mib1202v0335/PreemptObjects.c
 *
 * 1202 preempt subtree projection.
 */
#include "PreemptObjects.h"

#include <stddef.h>

enum
{
  PREEMPT_OBJECT_TAG_MAX_PREEMPTS = 1,
  PREEMPT_OBJECT_TAG_NUMBER,
  PREEMPT_OBJECT_TAG_CONTROL,
  PREEMPT_OBJECT_TAG_LINK,
  PREEMPT_OBJECT_TAG_DELAY,
  PREEMPT_OBJECT_TAG_MINIMUM_DURATION,
  PREEMPT_OBJECT_TAG_MINIMUM_GREEN,
  PREEMPT_OBJECT_TAG_MINIMUM_WALK,
  PREEMPT_OBJECT_TAG_ENTER_PED_CLEAR,
  PREEMPT_OBJECT_TAG_TRACK_GREEN,
  PREEMPT_OBJECT_TAG_DWELL_GREEN,
  PREEMPT_OBJECT_TAG_MAXIMUM_PRESENCE,
  PREEMPT_OBJECT_TAG_TRACK_PHASE,
  PREEMPT_OBJECT_TAG_DWELL_PHASE,
  PREEMPT_OBJECT_TAG_DWELL_PED,
  PREEMPT_OBJECT_TAG_EXIT_PHASE,
  PREEMPT_OBJECT_TAG_STATE,
  PREEMPT_OBJECT_TAG_TRACK_OVERLAP,
  PREEMPT_OBJECT_TAG_DWELL_OVERLAP,
  PREEMPT_OBJECT_TAG_CYCLING_PHASE,
  PREEMPT_OBJECT_TAG_CYCLING_PED,
  PREEMPT_OBJECT_TAG_CYCLING_OVERLAP,
  PREEMPT_OBJECT_TAG_ENTER_YELLOW_CHANGE,
  PREEMPT_OBJECT_TAG_ENTER_RED_CLEAR,
  PREEMPT_OBJECT_TAG_TRACK_YELLOW_CHANGE,
  PREEMPT_OBJECT_TAG_TRACK_RED_CLEAR,
  PREEMPT_OBJECT_TAG_SEQUENCE_NUMBER,
  PREEMPT_OBJECT_TAG_EXIT_TYPE,
  PREEMPT_OBJECT_TAG_CONTROL_NUMBER,
  PREEMPT_OBJECT_TAG_CONTROL_STATE,
  PREEMPT_OBJECT_TAG_STATUS,
  PREEMPT_OBJECT_TAG_MAX_GROUPS,
  PREEMPT_OBJECT_TAG_GROUP_NUMBER,
  PREEMPT_OBJECT_TAG_GROUP_STATUS,
  PREEMPT_OBJECT_TAG_QUEUE_DELAY_WEIGHT,
  PREEMPT_OBJECT_TAG_MAX_GATES,
  PREEMPT_OBJECT_TAG_GATE_NUMBER,
  PREEMPT_OBJECT_TAG_GATE_STATUS,
  PREEMPT_OBJECT_TAG_GATE_DESCRIPTION
};

static const uint32_t kMaxPreemptsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                            2U, 1U, 6U, 1U };
static const uint32_t kPreemptNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                              2U, 1U, 6U, 2U, 1U, 1U };
static const uint32_t kPreemptControlOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                               4U, 2U, 1U, 6U, 2U, 1U, 2U };
static const uint32_t kPreemptLinkOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                            2U, 1U, 6U, 2U, 1U, 3U };
static const uint32_t kPreemptDelayOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                             2U, 1U, 6U, 2U, 1U, 4U };
static const uint32_t kPreemptMinimumDurationOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                       1206U, 4U, 2U, 1U, 6U,
                                                       2U, 1U, 5U };
static const uint32_t kPreemptMinimumGreenOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                    1206U, 4U, 2U, 1U, 6U, 2U,
                                                    1U, 6U };
static const uint32_t kPreemptMinimumWalkOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                   1206U, 4U, 2U, 1U, 6U, 2U,
                                                   1U, 7U };
static const uint32_t kPreemptEnterPedClearOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                     1206U, 4U, 2U, 1U, 6U, 2U,
                                                     1U, 8U };
static const uint32_t kPreemptTrackGreenOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                  4U, 2U, 1U, 6U, 2U, 1U, 9U };
static const uint32_t kPreemptDwellGreenOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                  4U, 2U, 1U, 6U, 2U, 1U, 10U };
static const uint32_t kPreemptMaximumPresenceOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                       1206U, 4U, 2U, 1U, 6U,
                                                       2U, 1U, 11U };
static const uint32_t kPreemptTrackPhaseOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                  4U, 2U, 1U, 6U, 2U, 1U, 12U };
static const uint32_t kPreemptDwellPhaseOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                  4U, 2U, 1U, 6U, 2U, 1U, 13U };
static const uint32_t kPreemptDwellPedOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                4U, 2U, 1U, 6U, 2U, 1U, 14U };
static const uint32_t kPreemptExitPhaseOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                 4U, 2U, 1U, 6U, 2U, 1U, 15U };
static const uint32_t kPreemptStateOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                             2U, 1U, 6U, 2U, 1U, 16U };
static const uint32_t kPreemptTrackOverlapOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                    1206U, 4U, 2U, 1U, 6U, 2U,
                                                    1U, 17U };
static const uint32_t kPreemptDwellOverlapOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                    1206U, 4U, 2U, 1U, 6U, 2U,
                                                    1U, 18U };
static const uint32_t kPreemptCyclingPhaseOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                    1206U, 4U, 2U, 1U, 6U, 2U,
                                                    1U, 19U };
static const uint32_t kPreemptCyclingPedOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                  4U, 2U, 1U, 6U, 2U, 1U, 20U };
static const uint32_t kPreemptCyclingOverlapOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                      1206U, 4U, 2U, 1U, 6U, 2U,
                                                      1U, 21U };
static const uint32_t kPreemptEnterYellowChangeOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                         1206U, 4U, 2U, 1U, 6U,
                                                         2U, 1U, 22U };
static const uint32_t kPreemptEnterRedClearOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                     1206U, 4U, 2U, 1U, 6U, 2U,
                                                     1U, 23U };
static const uint32_t kPreemptTrackYellowChangeOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                         1206U, 4U, 2U, 1U, 6U,
                                                         2U, 1U, 24U };
static const uint32_t kPreemptTrackRedClearOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                     1206U, 4U, 2U, 1U, 6U, 2U,
                                                     1U, 25U };
static const uint32_t kPreemptSequenceNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                      1206U, 4U, 2U, 1U, 6U, 2U,
                                                      1U, 26U };
static const uint32_t kPreemptExitTypeOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                4U, 2U, 1U, 6U, 2U, 1U, 28U };
static const uint32_t kPreemptControlNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                     1206U, 4U, 2U, 1U, 6U, 3U,
                                                     1U, 1U };
static const uint32_t kPreemptControlStateOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                    1206U, 4U, 2U, 1U, 6U, 3U,
                                                    1U, 2U };
static const uint32_t kPreemptStatusOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                              2U, 1U, 6U, 4U };
static const uint32_t kMaxPreemptGroupsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                 4U, 2U, 1U, 6U, 5U };
static const uint32_t kPreemptStatusGroupNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                         1206U, 4U, 2U, 1U, 6U,
                                                         6U, 1U, 1U };
static const uint32_t kPreemptStatusGroupOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                   1206U, 4U, 2U, 1U, 6U, 6U,
                                                   1U, 2U };
static const uint32_t kPreemptDetectorWeightOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                      1206U, 4U, 2U, 1U, 6U, 7U,
                                                      1U, 1U };
static const uint32_t kMaxPreemptGatesOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                4U, 2U, 1U, 6U, 8U };
static const uint32_t kPreemptGateNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                  4U, 2U, 1U, 6U, 9U, 1U, 1U };
static const uint32_t kPreemptGateStatusOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                  4U, 2U, 1U, 6U, 9U, 1U, 2U };
static const uint32_t kPreemptGateDescriptionOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                       1206U, 4U, 2U, 1U, 6U,
                                                       9U, 1U, 3U };

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

static NtcipError_t GetPreemptFromIndex(const NtcipContext_t *context,
                                        const uint32_t *indexes,
                                        uint8_t indexCount,
                                        uint8_t *preemptIndex,
                                        IntersectionPreemptConfig_t *preempt)
{
  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (preemptIndex == NULL) || (preempt == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (indexes[0]
      > ConfigurationServiceGetPreemptCount(context->configurationService))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *preemptIndex = (uint8_t) (indexes[0] - 1U);

  if (ConfigurationServiceGetActivePreemptConfig(context->configurationService,
                                                 *preemptIndex,
                                                 preempt) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t ValidatePreemptWriteRequest(const NtcipContext_t *context,
                                                const uint32_t *indexes,
                                                uint8_t indexCount,
                                                const NtcipRequestContext_t *
                                                requestContext,
                                                uint8_t *preemptIndex,
                                                IntersectionPreemptConfig_t *
                                                preempt)
{
  NtcipError_t error;

  error = ValidateDatabaseWrite(context, requestContext);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  return GetPreemptFromIndex(context, indexes, indexCount, preemptIndex,
                             preempt);
}

static NtcipError_t GetQueueDelayFromIndex(const NtcipContext_t *context,
                                           const uint32_t *indexes,
                                           uint8_t indexCount,
                                           uint8_t *preemptIndex,
                                           uint8_t *detectorIndex,
                                           uint16_t *detectorWeight)
{
  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 2U)
      || (indexes[0] == 0U) || (indexes[1] == 0U)
      || (preemptIndex == NULL) || (detectorIndex == NULL)
      || (detectorWeight == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if ((indexes[0]
       > ConfigurationServiceGetPreemptCount(context->configurationService))
      || (indexes[1]
          > ConfigurationServiceGetVehicleDetectorCount(
            context->configurationService)))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *preemptIndex = (uint8_t) (indexes[0] - 1U);
  *detectorIndex = (uint8_t) (indexes[1] - 1U);

  if (ConfigurationServiceGetPreemptQueueDelayWeight(
        context->configurationService,
        *preemptIndex,
        *detectorIndex,
        detectorWeight) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetGateFromIndex(const NtcipContext_t *context,
                                     const uint32_t *indexes,
                                     uint8_t indexCount,
                                     uint8_t *gateIndex,
                                     IntersectionPreemptGateConfig_t *gate)
{
  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (gateIndex == NULL) || (gate == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (indexes[0]
      > ConfigurationServiceGetPreemptGateCount(context->configurationService))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *gateIndex = (uint8_t) (indexes[0] - 1U);

  if (ConfigurationServiceGetActivePreemptGateConfig(
        context->configurationService,
        *gateIndex,
        gate) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetPreemptObject(void *groupContext,
                                     const NtcipObjectDescriptor_t *descriptor,
                                     const uint32_t *indexes,
                                     uint8_t indexCount,
                                     const NtcipRequestContext_t *requestContext,
                                     NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  const IntersectionRuntime_t *runtime;
  IntersectionPreemptConfig_t preempt;
  IntersectionPreemptGateConfig_t gate;
  uint16_t detectorWeight;
  uint8_t detectorIndex;
  uint8_t gateIndex;
  uint8_t preemptIndex;
  NtcipError_t error;
  uint8_t statusGroup;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  runtime = IntersectionEngineGetRuntime(context->intersectionEngine);

  switch (descriptor->tag)
  {
      case PREEMPT_OBJECT_TAG_MAX_PREEMPTS:
      {
        NtcipValueSetUnsigned32(value,
                                ConfigurationServiceGetPreemptCount(
                                  context->configurationService));

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_STATUS:
      {
        NtcipValueSetUnsigned32(value,
                                (runtime
                                 != NULL) ? runtime->preemptStatus : 0U);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_MAX_GROUPS:
      {
        NtcipValueSetUnsigned32(value,
                                (uint32_t) ((ConfigurationServiceGetPreemptCount
                                             (
                                               context->configurationService)
                                             + 7U)
                                            / 8U));

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_MAX_GATES:
      {
        NtcipValueSetUnsigned32(value,
                                ConfigurationServiceGetPreemptGateCount(
                                  context->configurationService));

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_GROUP_NUMBER:
      {
        if ((indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
            || (indexes[0]
                > ((ConfigurationServiceGetPreemptCount(
                      context->configurationService) + 7U) / 8U)))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_GROUP_STATUS:
      {
        if ((indexes == NULL) || (indexCount != 1U))
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        if (IntersectionEngineGetPreemptStatusGroup(context->intersectionEngine,
                                                    (uint8_t) indexes[0],
                                                    &statusGroup) == 0U)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        NtcipValueSetUnsigned32(value, statusGroup);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        break;
      }
  } /* switch */

  if (descriptor->tag == PREEMPT_OBJECT_TAG_QUEUE_DELAY_WEIGHT)
  {
    error = GetQueueDelayFromIndex(context,
                                   indexes,
                                   indexCount,
                                   &preemptIndex,
                                   &detectorIndex,
                                   &detectorWeight);

    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }

    NtcipValueSetUnsigned32(value, detectorWeight);

    return NTCIP_ERROR_OK;
  }

  if ((descriptor->tag == PREEMPT_OBJECT_TAG_GATE_NUMBER)
      || (descriptor->tag == PREEMPT_OBJECT_TAG_GATE_STATUS)
      || (descriptor->tag == PREEMPT_OBJECT_TAG_GATE_DESCRIPTION))
  {
    error = GetGateFromIndex(context, indexes, indexCount, &gateIndex, &gate);

    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }

    switch (descriptor->tag)
    {
        case PREEMPT_OBJECT_TAG_GATE_NUMBER:
        {
          NtcipValueSetUnsigned32(value, indexes[0]);

          return NTCIP_ERROR_OK;
        }

        case PREEMPT_OBJECT_TAG_GATE_STATUS:
        {
          NtcipValueSetUnsigned32(value, 1U);

          return NTCIP_ERROR_OK;
        }

        case PREEMPT_OBJECT_TAG_GATE_DESCRIPTION:
        {
          return NtcipValueSetOctetString(value,
                                          gate.description,
                                          gate.descriptionLength);
        }

        default:
        {
          return NTCIP_ERROR_NOT_FOUND;
        }
    }
  }

  error = GetPreemptFromIndex(context,
                              indexes,
                              indexCount,
                              &preemptIndex,
                              &preempt);

  if (error != NTCIP_ERROR_OK)
  {
    if ((descriptor->tag == PREEMPT_OBJECT_TAG_CONTROL_NUMBER)
        || (descriptor->tag == PREEMPT_OBJECT_TAG_CONTROL_STATE))
    {
      if ((indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
          || (indexes[0]
              > ConfigurationServiceGetPreemptCount(
                context->configurationService)))
      {
        return NTCIP_ERROR_RANGE_ERROR;
      }

      preemptIndex = (uint8_t) (indexes[0] - 1U);
    }
    else
    {
      return error;
    }
  }

  switch (descriptor->tag)
  {
      case PREEMPT_OBJECT_TAG_NUMBER:
      case PREEMPT_OBJECT_TAG_CONTROL_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_CONTROL:
      {
        NtcipValueSetUnsigned32(value, preempt.control);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_LINK:
      {
        NtcipValueSetUnsigned32(value, preempt.link);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_DELAY:
      {
        NtcipValueSetUnsigned32(value, preempt.delaySeconds);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_MINIMUM_DURATION:
      {
        NtcipValueSetUnsigned32(value, preempt.minimumDurationSeconds);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_MINIMUM_GREEN:
      {
        NtcipValueSetUnsigned32(value, preempt.minimumGreenSeconds);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_MINIMUM_WALK:
      {
        NtcipValueSetUnsigned32(value, preempt.minimumWalkSeconds);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_ENTER_PED_CLEAR:
      {
        NtcipValueSetUnsigned32(value, preempt.enterPedClearSeconds);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_TRACK_GREEN:
      {
        NtcipValueSetUnsigned32(value, preempt.trackGreenSeconds);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_DWELL_GREEN:
      {
        NtcipValueSetUnsigned32(value, preempt.dwellGreenSeconds);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_MAXIMUM_PRESENCE:
      {
        NtcipValueSetUnsigned32(value, preempt.maximumPresenceSeconds);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_TRACK_PHASE:
      {
        return NtcipValueSetOctetString(value,
                                        preempt.trackPhases.values,
                                        preempt.trackPhases.length);
      }

      case PREEMPT_OBJECT_TAG_DWELL_PHASE:
      {
        return NtcipValueSetOctetString(value,
                                        preempt.dwellPhases.values,
                                        preempt.dwellPhases.length);
      }

      case PREEMPT_OBJECT_TAG_DWELL_PED:
      {
        return NtcipValueSetOctetString(value,
                                        preempt.dwellPeds.values,
                                        preempt.dwellPeds.length);
      }

      case PREEMPT_OBJECT_TAG_EXIT_PHASE:
      {
        return NtcipValueSetOctetString(value,
                                        preempt.exitPhases.values,
                                        preempt.exitPhases.length);
      }

      case PREEMPT_OBJECT_TAG_STATE:
      {
        NtcipValueSetUnsigned32(
          value,
          (runtime != NULL) ? runtime->preemptStates[preemptIndex]
          : INTERSECTION_PREEMPT_STATE_NOT_ACTIVE);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_TRACK_OVERLAP:
      {
        return NtcipValueSetOctetString(value,
                                        preempt.trackOverlaps.values,
                                        preempt.trackOverlaps.length);
      }

      case PREEMPT_OBJECT_TAG_DWELL_OVERLAP:
      {
        return NtcipValueSetOctetString(value,
                                        preempt.dwellOverlaps.values,
                                        preempt.dwellOverlaps.length);
      }

      case PREEMPT_OBJECT_TAG_CYCLING_PHASE:
      {
        return NtcipValueSetOctetString(value,
                                        preempt.cyclingPhases.values,
                                        preempt.cyclingPhases.length);
      }

      case PREEMPT_OBJECT_TAG_CYCLING_PED:
      {
        return NtcipValueSetOctetString(value,
                                        preempt.cyclingPeds.values,
                                        preempt.cyclingPeds.length);
      }

      case PREEMPT_OBJECT_TAG_CYCLING_OVERLAP:
      {
        return NtcipValueSetOctetString(value,
                                        preempt.cyclingOverlaps.values,
                                        preempt.cyclingOverlaps.length);
      }

      case PREEMPT_OBJECT_TAG_ENTER_YELLOW_CHANGE:
      {
        NtcipValueSetUnsigned32(value, preempt.enterYellowChangeDs);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_ENTER_RED_CLEAR:
      {
        NtcipValueSetUnsigned32(value, preempt.enterRedClearDs);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_TRACK_YELLOW_CHANGE:
      {
        NtcipValueSetUnsigned32(value, preempt.trackYellowChangeDs);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_TRACK_RED_CLEAR:
      {
        NtcipValueSetUnsigned32(value, preempt.trackRedClearDs);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_SEQUENCE_NUMBER:
      {
        NtcipValueSetUnsigned32(value, preempt.sequenceNumber);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_EXIT_TYPE:
      {
        NtcipValueSetUnsigned32(value, preempt.exitType);

        return NTCIP_ERROR_OK;
      }

      case PREEMPT_OBJECT_TAG_CONTROL_STATE:
      {
        NtcipValueSetUnsigned32(value,
                                (runtime != NULL)
                                ? runtime->preemptControlState[preemptIndex]
                                : 0U);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  } /* switch */
} /* GetPreemptObject */

static NtcipError_t SetTestPreemptObject(void *groupContext,
                                         const NtcipObjectDescriptor_t *
                                         descriptor,
                                         const uint32_t *indexes,
                                         uint8_t indexCount,
                                         const NtcipRequestContext_t *
                                         requestContext,
                                         const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionPreemptConfig_t preempt;
  IntersectionPreemptGateConfig_t gate;
  uint16_t detectorWeight;
  uint8_t detectorIndex;
  uint8_t gateIndex;
  uint8_t preemptIndex;
  NtcipError_t error = NTCIP_ERROR_OK;

  if (descriptor->tag == PREEMPT_OBJECT_TAG_CONTROL_STATE)
  {
    if ((indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
        || (indexes[0]
            > ConfigurationServiceGetPreemptCount(
              context->configurationService)))
    {
      return NTCIP_ERROR_RANGE_ERROR;
    }
  }
  else if (descriptor->tag == PREEMPT_OBJECT_TAG_QUEUE_DELAY_WEIGHT)
  {
    error = ValidateDatabaseWrite(context, requestContext);

    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }

    error = GetQueueDelayFromIndex(context,
                                   indexes,
                                   indexCount,
                                   &preemptIndex,
                                   &detectorIndex,
                                   &detectorWeight);

    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }
  }
  else if (descriptor->tag == PREEMPT_OBJECT_TAG_GATE_DESCRIPTION)
  {
    error = ValidateDatabaseWrite(context, requestContext);

    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }

    error = GetGateFromIndex(context, indexes, indexCount, &gateIndex, &gate);

    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }
  }
  else
  {
    error = ValidatePreemptWriteRequest(context,
                                        indexes,
                                        indexCount,
                                        requestContext,
                                        &preemptIndex,
                                        &preempt);

    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }
  }

  switch (descriptor->tag)
  {
      case PREEMPT_OBJECT_TAG_CONTROL:
      {
        return ((value->data.unsigned32 & ~0x3FUL) == 0UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PREEMPT_OBJECT_TAG_LINK:
      {
        return (value->data.unsigned32 <= INTERSECTION_PREEMPT_COUNT_MAX)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PREEMPT_OBJECT_TAG_DELAY:
      case PREEMPT_OBJECT_TAG_MINIMUM_DURATION:
      case PREEMPT_OBJECT_TAG_MAXIMUM_PRESENCE:
      {
        return (value->data.unsigned32 <= 65535UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PREEMPT_OBJECT_TAG_MINIMUM_GREEN:
      case PREEMPT_OBJECT_TAG_MINIMUM_WALK:
      case PREEMPT_OBJECT_TAG_ENTER_PED_CLEAR:
      case PREEMPT_OBJECT_TAG_TRACK_GREEN:
      case PREEMPT_OBJECT_TAG_DWELL_GREEN:
      case PREEMPT_OBJECT_TAG_ENTER_YELLOW_CHANGE:
      case PREEMPT_OBJECT_TAG_ENTER_RED_CLEAR:
      case PREEMPT_OBJECT_TAG_TRACK_YELLOW_CHANGE:
      case PREEMPT_OBJECT_TAG_TRACK_RED_CLEAR:
      {
        return (value->data.unsigned32 <= 255UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PREEMPT_OBJECT_TAG_TRACK_PHASE:
      case PREEMPT_OBJECT_TAG_DWELL_PHASE:
      case PREEMPT_OBJECT_TAG_DWELL_PED:
      case PREEMPT_OBJECT_TAG_EXIT_PHASE:
      case PREEMPT_OBJECT_TAG_CYCLING_PHASE:
      case PREEMPT_OBJECT_TAG_CYCLING_PED:
      {
        return (value->data.octetString.length <= INTERSECTION_PHASE_COUNT_MAX)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PREEMPT_OBJECT_TAG_TRACK_OVERLAP:
      case PREEMPT_OBJECT_TAG_DWELL_OVERLAP:
      case PREEMPT_OBJECT_TAG_CYCLING_OVERLAP:
      {
        return (value->data.octetString.length
                <= INTERSECTION_OVERLAP_COUNT_MAX)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PREEMPT_OBJECT_TAG_SEQUENCE_NUMBER:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32
                    <= ConfigurationServiceGetSequenceCount(
                      context->configurationService)))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PREEMPT_OBJECT_TAG_EXIT_TYPE:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= 4UL))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PREEMPT_OBJECT_TAG_CONTROL_STATE:
      {
        return (value->data.unsigned32 <= 1UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PREEMPT_OBJECT_TAG_QUEUE_DELAY_WEIGHT:
      {
        return (value->data.unsigned32 <= 1000UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case PREEMPT_OBJECT_TAG_GATE_DESCRIPTION:
      {
        return (value->data.octetString.length
                <= INTERSECTION_PREEMPT_GATE_DESCRIPTION_MAX)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  } /* switch */
} /* SetTestPreemptObject */

static NtcipError_t SetValuePreemptObject(void *groupContext,
                                          const NtcipObjectDescriptor_t *
                                          descriptor,
                                          const uint32_t *indexes,
                                          uint8_t indexCount,
                                          const NtcipRequestContext_t *
                                          requestContext,
                                          const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionPreemptConfig_t preempt;
  IntersectionPreemptGateConfig_t gate;
  uint16_t detectorWeight;
  uint8_t detectorIndex;
  uint8_t gateIndex;
  uint8_t preemptIndex;
  NtcipError_t error = NTCIP_ERROR_OK;
  uint8_t ok = 0U;

  if (descriptor->tag == PREEMPT_OBJECT_TAG_CONTROL_STATE)
  {
    if ((indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
        || (indexes[0]
            > ConfigurationServiceGetPreemptCount(
              context->configurationService)))
    {
      return NTCIP_ERROR_RANGE_ERROR;
    }

    return (IntersectionEngineSetPreemptControlState(
              context->intersectionEngine,
              (uint8_t) indexes[0],
              (uint8_t) value->data.
              unsigned32)
            != 0U)
           ? NTCIP_ERROR_OK
           : NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag == PREEMPT_OBJECT_TAG_QUEUE_DELAY_WEIGHT)
  {
    error = ValidateDatabaseWrite(context, requestContext);

    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }

    error = GetQueueDelayFromIndex(context,
                                   indexes,
                                   indexCount,
                                   &preemptIndex,
                                   &detectorIndex,
                                   &detectorWeight);

    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }

    return (ConfigurationServiceSetPreemptQueueDelayWeight(
              context->configurationService,
              preemptIndex,
              detectorIndex,
              (uint16_t) value->data.unsigned32)
            != 0U)
           ? NTCIP_ERROR_OK
           : NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag == PREEMPT_OBJECT_TAG_GATE_DESCRIPTION)
  {
    error = ValidateDatabaseWrite(context, requestContext);

    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }

    error = GetGateFromIndex(context,
                             indexes,
                             indexCount,
                             &gateIndex,
                             &gate);

    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }

    return (ConfigurationServiceSetPreemptGateDescription(
              context->configurationService,
              gateIndex,
              value->data.octetString.bytes,
              (uint8_t) value->data.octetString.length)
            != 0U)
           ? NTCIP_ERROR_OK
           : NTCIP_ERROR_BAD_VALUE;
  }

  error = ValidatePreemptWriteRequest(context,
                                      indexes,
                                      indexCount,
                                      requestContext,
                                      &preemptIndex,
                                      &preempt);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case PREEMPT_OBJECT_TAG_CONTROL:
      {
        ok =
          ConfigurationServiceSetPreemptControl(context->configurationService,
                                                preemptIndex,
                                                (uint8_t) value->data.
                                                unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_LINK:
      {
        ok = ConfigurationServiceSetPreemptLink(context->configurationService,
                                                preemptIndex,
                                                (uint8_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_DELAY:
      {
        ok =
          ConfigurationServiceSetPreemptDelaySeconds(
            context->configurationService,
            preemptIndex,
            (uint16_t) value->data.
            unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_MINIMUM_DURATION:
      {
        ok = ConfigurationServiceSetPreemptMinimumDurationSeconds(
          context->configurationService,
          preemptIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_MINIMUM_GREEN:
      {
        ok = ConfigurationServiceSetPreemptMinimumGreenSeconds(
          context->configurationService,
          preemptIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_MINIMUM_WALK:
      {
        ok = ConfigurationServiceSetPreemptMinimumWalkSeconds(
          context->configurationService,
          preemptIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_ENTER_PED_CLEAR:
      {
        ok = ConfigurationServiceSetPreemptEnterPedClearSeconds(
          context->configurationService,
          preemptIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_TRACK_GREEN:
      {
        ok = ConfigurationServiceSetPreemptTrackGreenSeconds(
          context->configurationService,
          preemptIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_DWELL_GREEN:
      {
        ok = ConfigurationServiceSetPreemptDwellGreenSeconds(
          context->configurationService,
          preemptIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_MAXIMUM_PRESENCE:
      {
        ok = ConfigurationServiceSetPreemptMaximumPresenceSeconds(
          context->configurationService,
          preemptIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_TRACK_PHASE:
      {
        ok = ConfigurationServiceSetPreemptTrackPhases(
          context->configurationService,
          preemptIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case PREEMPT_OBJECT_TAG_DWELL_PHASE:
      {
        ok = ConfigurationServiceSetPreemptDwellPhases(
          context->configurationService,
          preemptIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case PREEMPT_OBJECT_TAG_DWELL_PED:
      {
        ok = ConfigurationServiceSetPreemptDwellPeds(
          context->configurationService,
          preemptIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case PREEMPT_OBJECT_TAG_EXIT_PHASE:
      {
        ok = ConfigurationServiceSetPreemptExitPhases(
          context->configurationService,
          preemptIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case PREEMPT_OBJECT_TAG_TRACK_OVERLAP:
      {
        ok = ConfigurationServiceSetPreemptTrackOverlaps(
          context->configurationService,
          preemptIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case PREEMPT_OBJECT_TAG_DWELL_OVERLAP:
      {
        ok = ConfigurationServiceSetPreemptDwellOverlaps(
          context->configurationService,
          preemptIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case PREEMPT_OBJECT_TAG_CYCLING_PHASE:
      {
        ok = ConfigurationServiceSetPreemptCyclingPhases(
          context->configurationService,
          preemptIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case PREEMPT_OBJECT_TAG_CYCLING_PED:
      {
        ok = ConfigurationServiceSetPreemptCyclingPeds(
          context->configurationService,
          preemptIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case PREEMPT_OBJECT_TAG_CYCLING_OVERLAP:
      {
        ok = ConfigurationServiceSetPreemptCyclingOverlaps(
          context->configurationService,
          preemptIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case PREEMPT_OBJECT_TAG_ENTER_YELLOW_CHANGE:
      {
        ok = ConfigurationServiceSetPreemptEnterYellowChangeDs(
          context->configurationService,
          preemptIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_ENTER_RED_CLEAR:
      {
        ok = ConfigurationServiceSetPreemptEnterRedClearDs(
          context->configurationService,
          preemptIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_TRACK_YELLOW_CHANGE:
      {
        ok = ConfigurationServiceSetPreemptTrackYellowChangeDs(
          context->configurationService,
          preemptIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_TRACK_RED_CLEAR:
      {
        ok = ConfigurationServiceSetPreemptTrackRedClearDs(
          context->configurationService,
          preemptIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_SEQUENCE_NUMBER:
      {
        ok = ConfigurationServiceSetPreemptSequenceNumber(
          context->configurationService,
          preemptIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case PREEMPT_OBJECT_TAG_EXIT_TYPE:
      {
        ok =
          ConfigurationServiceSetPreemptExitType(context->configurationService,
                                                 preemptIndex,
                                                 (uint8_t) value->data.
                                                 unsigned32);
        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  } /* switch */

  return (ok != 0U) ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;
} /* SetValuePreemptObject */

static const NtcipObjectDescriptor_t kPreemptObjects[] =
{
  { kMaxPreemptsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_MAX_PREEMPTS,
    GetPreemptObject, NULL, NULL },
  { kPreemptNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_NUMBER,
    GetPreemptObject, NULL, NULL },
  { kPreemptControlOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_CONTROL,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptLinkOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_LINK,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptDelayOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_DELAY,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptMinimumDurationOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_MINIMUM_DURATION,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptMinimumGreenOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_MINIMUM_GREEN,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptMinimumWalkOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_MINIMUM_WALK,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptEnterPedClearOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_ENTER_PED_CLEAR,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptTrackGreenOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_TRACK_GREEN,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptDwellGreenOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_DWELL_GREEN,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptMaximumPresenceOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_MAXIMUM_PRESENCE,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptTrackPhaseOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, PREEMPT_OBJECT_TAG_TRACK_PHASE,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptDwellPhaseOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, PREEMPT_OBJECT_TAG_DWELL_PHASE,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptDwellPedOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, PREEMPT_OBJECT_TAG_DWELL_PED,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptExitPhaseOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, PREEMPT_OBJECT_TAG_EXIT_PHASE,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptStateOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_STATE,
    GetPreemptObject, NULL, NULL },
  { kPreemptTrackOverlapOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, PREEMPT_OBJECT_TAG_TRACK_OVERLAP,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptDwellOverlapOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, PREEMPT_OBJECT_TAG_DWELL_OVERLAP,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptCyclingPhaseOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, PREEMPT_OBJECT_TAG_CYCLING_PHASE,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptCyclingPedOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, PREEMPT_OBJECT_TAG_CYCLING_PED,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptCyclingOverlapOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, PREEMPT_OBJECT_TAG_CYCLING_OVERLAP,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptEnterYellowChangeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_ENTER_YELLOW_CHANGE,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptEnterRedClearOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_ENTER_RED_CLEAR,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptTrackYellowChangeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_TRACK_YELLOW_CHANGE,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptTrackRedClearOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_TRACK_RED_CLEAR,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptSequenceNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_SEQUENCE_NUMBER,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptExitTypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_EXIT_TYPE,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptControlNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_CONTROL_NUMBER,
    GetPreemptObject, NULL, NULL },
  { kPreemptControlStateOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_CONTROL_STATE,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kPreemptStatusOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_STATUS,
    GetPreemptObject, NULL, NULL },
  { kMaxPreemptGroupsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_MAX_GROUPS,
    GetPreemptObject, NULL, NULL },
  { kPreemptStatusGroupNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_GROUP_NUMBER,
    GetPreemptObject, NULL, NULL },
  { kPreemptStatusGroupOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_GROUP_STATUS,
    GetPreemptObject, NULL, NULL },
  { kPreemptDetectorWeightOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_QUEUE_DELAY_WEIGHT,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject },
  { kMaxPreemptGatesOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_MAX_GATES,
    GetPreemptObject, NULL, NULL },
  { kPreemptGateNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_GATE_NUMBER,
    GetPreemptObject, NULL, NULL },
  { kPreemptGateStatusOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PREEMPT_OBJECT_TAG_GATE_STATUS,
    GetPreemptObject, NULL, NULL },
  { kPreemptGateDescriptionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, PREEMPT_OBJECT_TAG_GATE_DESCRIPTION,
    GetPreemptObject, SetTestPreemptObject, SetValuePreemptObject }
};

void PreemptObjectsRegister(NtcipObjectDirectory_t *directory,
                            NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.preempt",
    kPreemptObjects,
    (uint16_t) (sizeof(kPreemptObjects) / sizeof(kPreemptObjects[0])),
    context);
}
