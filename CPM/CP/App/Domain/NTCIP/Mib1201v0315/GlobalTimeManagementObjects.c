/* App/Domain/NTCIP/Mib1201v0315/GlobalTimeManagementObjects.c */
#include "GlobalTimeManagementObjects.h"

#include <stddef.h>

enum
{
  GLOBAL_TIME_TAG_GLOBAL_TIME = 1,
  GLOBAL_TIME_TAG_DAYLIGHT_SAVING,
  GLOBAL_TIME_TAG_MAX_SCHEDULES,
  GLOBAL_TIME_TAG_SCHEDULE_NUMBER,
  GLOBAL_TIME_TAG_SCHEDULE_MONTH,
  GLOBAL_TIME_TAG_SCHEDULE_DAY,
  GLOBAL_TIME_TAG_SCHEDULE_DATE,
  GLOBAL_TIME_TAG_SCHEDULE_DAY_PLAN,
  GLOBAL_TIME_TAG_MAX_DAY_PLANS,
  GLOBAL_TIME_TAG_MAX_DAY_PLAN_EVENTS,
  GLOBAL_TIME_TAG_DAY_PLAN_NUMBER,
  GLOBAL_TIME_TAG_DAY_PLAN_EVENT_NUMBER,
  GLOBAL_TIME_TAG_DAY_PLAN_HOUR,
  GLOBAL_TIME_TAG_DAY_PLAN_MINUTE,
  GLOBAL_TIME_TAG_DAY_PLAN_ACTION_OID,
  GLOBAL_TIME_TAG_DAY_PLAN_STATUS,
  GLOBAL_TIME_TAG_SCHEDULE_STATUS,
  GLOBAL_TIME_TAG_LOCAL_TIME_DIFFERENTIAL,
  GLOBAL_TIME_TAG_STANDARD_TIME_ZONE,
  GLOBAL_TIME_TAG_CONTROLLER_LOCAL_TIME,
  GLOBAL_TIME_TAG_MAX_DST_ENTRIES,
  GLOBAL_TIME_TAG_DST_ENTRY_NUMBER,
  GLOBAL_TIME_TAG_DST_BEGIN_MONTH,
  GLOBAL_TIME_TAG_DST_BEGIN_OCCURRENCES,
  GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_WEEK,
  GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_MONTH,
  GLOBAL_TIME_TAG_DST_BEGIN_SECONDS,
  GLOBAL_TIME_TAG_DST_END_MONTH,
  GLOBAL_TIME_TAG_DST_END_OCCURRENCES,
  GLOBAL_TIME_TAG_DST_END_DAY_OF_WEEK,
  GLOBAL_TIME_TAG_DST_END_DAY_OF_MONTH,
  GLOBAL_TIME_TAG_DST_END_SECONDS,
  GLOBAL_TIME_TAG_DST_SECONDS_TO_ADJUST
};

static const uint32_t kGlobalTimeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 1U
};
static const uint32_t kGlobalDaylightSavingOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 2U
};
static const uint32_t kMaxTimeBaseScheduleEntriesOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 1U
};
static const uint32_t kTimeBaseScheduleNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 2U, 1U, 1U
};
static const uint32_t kTimeBaseScheduleMonthOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 2U, 1U, 2U
};
static const uint32_t kTimeBaseScheduleDayOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 2U, 1U, 3U
};
static const uint32_t kTimeBaseScheduleDateOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 2U, 1U, 4U
};
static const uint32_t kTimeBaseScheduleDayPlanOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 2U, 1U, 5U
};
static const uint32_t kMaxDayPlansOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 3U
};
static const uint32_t kMaxDayPlanEventsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 4U
};
static const uint32_t kDayPlanNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 5U, 1U, 1U
};
static const uint32_t kDayPlanEventNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 5U, 1U, 2U
};
static const uint32_t kDayPlanHourOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 5U, 1U, 3U
};
static const uint32_t kDayPlanMinuteOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 5U, 1U, 4U
};
static const uint32_t kDayPlanActionNumberOidOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 5U, 1U, 5U
};
static const uint32_t kDayPlanStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 6U
};
static const uint32_t kTimeBaseScheduleStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 7U
};
static const uint32_t kGlobalLocalTimeDifferentialOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 4U
};
static const uint32_t kControllerStandardTimeZoneOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 5U
};
static const uint32_t kControllerLocalTimeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 6U
};
static const uint32_t kMaxDaylightSavingEntriesOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 1U
};
static const uint32_t kDstEntryNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 1U
};
static const uint32_t kDstBeginMonthOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 2U
};
static const uint32_t kDstBeginOccurrencesOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 3U
};
static const uint32_t kDstBeginDayOfWeekOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 4U
};
static const uint32_t kDstBeginDayOfMonthOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 5U
};
static const uint32_t kDstBeginSecondsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 6U
};
static const uint32_t kDstEndMonthOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 7U
};
static const uint32_t kDstEndOccurrencesOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 8U
};
static const uint32_t kDstEndDayOfWeekOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 9U
};
static const uint32_t kDstEndDayOfMonthOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 10U
};
static const uint32_t kDstEndSecondsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 11U
};
static const uint32_t kDstSecondsToAdjustOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 12U
};
static const uint32_t kTimebaseAscActionNumberInstanceOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 3U, 1U, 1U
};

static NtcipError_t ValidateDatabaseWrite(const NtcipContext_t *context,
                                          const NtcipRequestContext_t *request)
{
  if ((context == NULL) || (context->dbTransactionService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return NtcipDbTransactionServiceValidateDatabaseWrite(
    context->dbTransactionService,
    request);
}

static NtcipError_t GetGlobalTimeManagementConfig(
  const NtcipContext_t *context,
  IntersectionGlobalTimeManagementConfig_t *globalTimeManagement)
{
  if ((context == NULL) || (globalTimeManagement == NULL)
      || (context->configurationService == NULL)
      || (ConfigurationServiceGetActiveGlobalTimeManagementConfig(
            context->configurationService,
            globalTimeManagement) == 0U))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetCandidateGlobalTimeManagementConfig(
  NtcipContext_t *context,
  IntersectionGlobalTimeManagementConfig_t *globalTimeManagement)
{
  if ((context == NULL) || (globalTimeManagement == NULL)
      || (context->configurationService == NULL)
      || (ConfigurationServiceGetCandidateGlobalTimeManagementConfig(
            context->configurationService,
            globalTimeManagement) == 0U))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t SaveCandidateGlobalTimeManagementConfig(
  NtcipContext_t *context,
  const IntersectionGlobalTimeManagementConfig_t *globalTimeManagement)
{
  if ((context == NULL) || (globalTimeManagement == NULL)
      || (context->configurationService == NULL)
      || (ConfigurationServiceSetGlobalTimeManagementConfig(
            context->configurationService,
            globalTimeManagement) == 0U))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetScheduleEntry(const uint32_t *indexes,
                                     uint8_t indexCount,
                                     uint8_t *scheduleIndex)
{
  if ((indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (indexes[0] > INTERSECTION_TIMEBASE_SCHEDULE_COUNT_MAX)
      || (scheduleIndex == NULL))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *scheduleIndex = (uint8_t) (indexes[0] - 1U);

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetDayPlanEntry(const uint32_t *indexes,
                                    uint8_t indexCount,
                                    uint8_t *dayPlanIndex,
                                    uint8_t *eventIndex)
{
  if ((indexes == NULL) || (indexCount != 2U) || (indexes[0] == 0U)
      || (indexes[0] > INTERSECTION_DAY_PLAN_COUNT_MAX)
      || (indexes[1] == 0U)
      || (indexes[1] > INTERSECTION_DAY_PLAN_EVENT_COUNT_MAX)
      || (dayPlanIndex == NULL) || (eventIndex == NULL))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *dayPlanIndex = (uint8_t) (indexes[0] - 1U);
  *eventIndex = (uint8_t) (indexes[1] - 1U);

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetDstEntry(const uint32_t *indexes,
                                uint8_t indexCount,
                                uint8_t *dstIndex)
{
  if ((indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (indexes[0] > INTERSECTION_DAYLIGHT_SAVING_ENTRY_COUNT_MAX)
      || (dstIndex == NULL))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *dstIndex = (uint8_t) (indexes[0] - 1U);

  return NTCIP_ERROR_OK;
}

static uint8_t DayPlanActionOidMatches(const NtcipValue_t *value,
                                       uint8_t *actionNumber)
{
  if ((value == NULL) || (value->type != NTCIP_VALUE_TYPE_OBJECT_ID)
      || (actionNumber == NULL))
  {
    return 0U;
  }

  if (value->data.objectId.length == 0U)
  {
    *actionNumber = 0U;
    return 1U;
  }

  if ((value->data.objectId.length != 15U)
      || (value->data.objectId.elements[14] == 0U)
      || (value->data.objectId.elements[14]
          > INTERSECTION_TIMEBASE_ACTION_COUNT_MAX))
  {
    return 0U;
  }

  for (uint8_t index = 0U; index < 14U; index++)
  {
    if (value->data.objectId.elements[index]
        != kTimebaseAscActionNumberInstanceOid[index])
    {
      return 0U;
    }
  }

  *actionNumber = (uint8_t) value->data.objectId.elements[14];

  return 1U;
}

static NtcipError_t SetDayPlanActionOidValue(NtcipValue_t *value,
                                             uint8_t actionNumber)
{
  uint32_t oid[15];
  uint8_t index;

  if (actionNumber == 0U)
  {
    return NtcipValueSetObjectId(value, NULL, 0U);
  }

  for (index = 0U; index < 14U; index++)
  {
    oid[index] = kTimebaseAscActionNumberInstanceOid[index];
  }

  oid[14] = actionNumber;

  return NtcipValueSetObjectId(value, oid, 15U);
}

static NtcipError_t GetGlobalTimeManagementObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionGlobalTimeManagementConfig_t globalTimeManagement;
  uint8_t scheduleIndex = 0U;
  uint8_t dayPlanIndex = 0U;
  uint8_t eventIndex = 0U;
  uint8_t dstIndex = 0U;
  uint32_t unsignedValue = 0U;
  int32_t signedValue = 0;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case GLOBAL_TIME_TAG_GLOBAL_TIME:
      {
        if ((context->globalTimeManagementService == NULL)
            || (GlobalTimeManagementServiceGetGlobalTime(
                  context->globalTimeManagementService,
                  &unsignedValue) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, unsignedValue);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DAY_PLAN_STATUS:
      {
        if ((context->globalTimeManagementService == NULL)
            || (GlobalTimeManagementServiceGetDayPlanStatus(
                  context->globalTimeManagementService,
                  &scheduleIndex) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, scheduleIndex);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_STATUS:
      {
        if ((context->globalTimeManagementService == NULL)
            || (GlobalTimeManagementServiceGetScheduleStatus(
                  context->globalTimeManagementService,
                  &scheduleIndex) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, scheduleIndex);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_LOCAL_TIME_DIFFERENTIAL:
      {
        if ((context->globalTimeManagementService == NULL)
            || (GlobalTimeManagementServiceGetGlobalLocalTimeDifferential(
                  context->globalTimeManagementService,
                  &signedValue) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetSigned32(value, signedValue);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_CONTROLLER_LOCAL_TIME:
      {
        if ((context->globalTimeManagementService == NULL)
            || (GlobalTimeManagementServiceGetControllerLocalTime(
                  context->globalTimeManagementService,
                  &unsignedValue) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, unsignedValue);
        return NTCIP_ERROR_OK;
      }

      default:
      {
        break;
      }
  }

  if (GetGlobalTimeManagementConfig(context, &globalTimeManagement) != NTCIP_ERROR_OK)
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  switch (descriptor->tag)
  {
      case GLOBAL_TIME_TAG_DAYLIGHT_SAVING:
      {
        NtcipValueSetUnsigned32(value, globalTimeManagement.globalDaylightSaving);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_MAX_SCHEDULES:
      {
        NtcipValueSetUnsigned32(value, INTERSECTION_TIMEBASE_SCHEDULE_COUNT_MAX);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_NUMBER:
      case GLOBAL_TIME_TAG_SCHEDULE_MONTH:
      case GLOBAL_TIME_TAG_SCHEDULE_DAY:
      case GLOBAL_TIME_TAG_SCHEDULE_DATE:
      case GLOBAL_TIME_TAG_SCHEDULE_DAY_PLAN:
      {
        if (GetScheduleEntry(indexes, indexCount, &scheduleIndex) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        break;
      }

      case GLOBAL_TIME_TAG_MAX_DAY_PLANS:
      {
        NtcipValueSetUnsigned32(value, INTERSECTION_DAY_PLAN_COUNT_MAX);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_MAX_DAY_PLAN_EVENTS:
      {
        NtcipValueSetUnsigned32(value, INTERSECTION_DAY_PLAN_EVENT_COUNT_MAX);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DAY_PLAN_NUMBER:
      case GLOBAL_TIME_TAG_DAY_PLAN_EVENT_NUMBER:
      case GLOBAL_TIME_TAG_DAY_PLAN_HOUR:
      case GLOBAL_TIME_TAG_DAY_PLAN_MINUTE:
      case GLOBAL_TIME_TAG_DAY_PLAN_ACTION_OID:
      {
        if (GetDayPlanEntry(indexes,
                            indexCount,
                            &dayPlanIndex,
                            &eventIndex) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        break;
      }

      case GLOBAL_TIME_TAG_STANDARD_TIME_ZONE:
      {
        NtcipValueSetSigned32(
          value,
          globalTimeManagement.controllerStandardTimeZoneSeconds);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_MAX_DST_ENTRIES:
      {
        NtcipValueSetUnsigned32(value, INTERSECTION_DAYLIGHT_SAVING_ENTRY_COUNT_MAX);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_ENTRY_NUMBER:
      case GLOBAL_TIME_TAG_DST_BEGIN_MONTH:
      case GLOBAL_TIME_TAG_DST_BEGIN_OCCURRENCES:
      case GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_WEEK:
      case GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_MONTH:
      case GLOBAL_TIME_TAG_DST_BEGIN_SECONDS:
      case GLOBAL_TIME_TAG_DST_END_MONTH:
      case GLOBAL_TIME_TAG_DST_END_OCCURRENCES:
      case GLOBAL_TIME_TAG_DST_END_DAY_OF_WEEK:
      case GLOBAL_TIME_TAG_DST_END_DAY_OF_MONTH:
      case GLOBAL_TIME_TAG_DST_END_SECONDS:
      case GLOBAL_TIME_TAG_DST_SECONDS_TO_ADJUST:
      {
        if (GetDstEntry(indexes, indexCount, &dstIndex) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        break;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }

  switch (descriptor->tag)
  {
      case GLOBAL_TIME_TAG_SCHEDULE_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_MONTH:
      {
        NtcipValueSetUnsigned32(value,
                                globalTimeManagement.schedules[scheduleIndex].monthMask);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_DAY:
      {
        NtcipValueSetUnsigned32(value,
                                globalTimeManagement.schedules[scheduleIndex].dayMask);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_DATE:
      {
        NtcipValueSetUnsigned32(value,
                                globalTimeManagement.schedules[scheduleIndex].dateMask);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_DAY_PLAN:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.schedules[scheduleIndex].dayPlanNumber);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DAY_PLAN_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DAY_PLAN_EVENT_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[1]);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DAY_PLAN_HOUR:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.dayPlans[dayPlanIndex][eventIndex].hour);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DAY_PLAN_MINUTE:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.dayPlans[dayPlanIndex][eventIndex].minute);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DAY_PLAN_ACTION_OID:
      {
        return SetDayPlanActionOidValue(
          value,
          globalTimeManagement.dayPlans[dayPlanIndex][eventIndex].actionNumber);
      }

      case GLOBAL_TIME_TAG_DST_ENTRY_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_BEGIN_MONTH:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.daylightSavingEntries[dstIndex].beginMonth);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_BEGIN_OCCURRENCES:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.daylightSavingEntries[dstIndex].beginOccurrences);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_WEEK:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.daylightSavingEntries[dstIndex].beginDayOfWeek);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_MONTH:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.daylightSavingEntries[dstIndex].beginDayOfMonth);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_BEGIN_SECONDS:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.daylightSavingEntries[dstIndex].beginSecondsToTransition);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_END_MONTH:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.daylightSavingEntries[dstIndex].endMonth);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_END_OCCURRENCES:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.daylightSavingEntries[dstIndex].endOccurrences);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_END_DAY_OF_WEEK:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.daylightSavingEntries[dstIndex].endDayOfWeek);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_END_DAY_OF_MONTH:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.daylightSavingEntries[dstIndex].endDayOfMonth);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_END_SECONDS:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.daylightSavingEntries[dstIndex].endSecondsToTransition);
        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_SECONDS_TO_ADJUST:
      {
        NtcipValueSetUnsigned32(
          value,
          globalTimeManagement.daylightSavingEntries[dstIndex].secondsToAdjust);
        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetTestGlobalTimeManagementObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  uint8_t scheduleIndex = 0U;
  uint8_t actionNumber = 0U;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag != GLOBAL_TIME_TAG_GLOBAL_TIME)
  {
    NtcipError_t accessError = ValidateDatabaseWrite(context, requestContext);

    if (accessError != NTCIP_ERROR_OK)
    {
      return accessError;
    }
  }

  switch (descriptor->tag)
  {
      case GLOBAL_TIME_TAG_GLOBAL_TIME:
      case GLOBAL_TIME_TAG_DAYLIGHT_SAVING:
      case GLOBAL_TIME_TAG_SCHEDULE_MONTH:
      case GLOBAL_TIME_TAG_SCHEDULE_DAY:
      case GLOBAL_TIME_TAG_SCHEDULE_DATE:
      case GLOBAL_TIME_TAG_SCHEDULE_DAY_PLAN:
      case GLOBAL_TIME_TAG_DAY_PLAN_HOUR:
      case GLOBAL_TIME_TAG_DAY_PLAN_MINUTE:
      case GLOBAL_TIME_TAG_DST_BEGIN_MONTH:
      case GLOBAL_TIME_TAG_DST_BEGIN_OCCURRENCES:
      case GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_WEEK:
      case GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_MONTH:
      case GLOBAL_TIME_TAG_DST_BEGIN_SECONDS:
      case GLOBAL_TIME_TAG_DST_END_MONTH:
      case GLOBAL_TIME_TAG_DST_END_OCCURRENCES:
      case GLOBAL_TIME_TAG_DST_END_DAY_OF_WEEK:
      case GLOBAL_TIME_TAG_DST_END_DAY_OF_MONTH:
      case GLOBAL_TIME_TAG_DST_END_SECONDS:
      case GLOBAL_TIME_TAG_DST_SECONDS_TO_ADJUST:
      {
        if (value->type != NTCIP_VALUE_TYPE_UNSIGNED32)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        break;
      }

      case GLOBAL_TIME_TAG_LOCAL_TIME_DIFFERENTIAL:
      case GLOBAL_TIME_TAG_STANDARD_TIME_ZONE:
      {
        if (value->type != NTCIP_VALUE_TYPE_SIGNED32)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        break;
      }

      case GLOBAL_TIME_TAG_DAY_PLAN_ACTION_OID:
      {
        if (DayPlanActionOidMatches(value, &actionNumber) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }

  switch (descriptor->tag)
  {
      case GLOBAL_TIME_TAG_DAYLIGHT_SAVING:
      {
        return ((value->data.unsigned32 >= 1U) && (value->data.unsigned32 <= 20U))
               ? NTCIP_ERROR_OK : NTCIP_ERROR_RANGE_ERROR;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_MONTH:
      {
        return (GetScheduleEntry(indexes, indexCount, &actionNumber) == NTCIP_ERROR_OK)
               ? NTCIP_ERROR_OK : NTCIP_ERROR_RANGE_ERROR;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_DAY:
      {
        if (GetScheduleEntry(indexes, indexCount, &actionNumber) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return (value->data.unsigned32 <= 255U)
               ? NTCIP_ERROR_OK : NTCIP_ERROR_RANGE_ERROR;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_DATE:
      case GLOBAL_TIME_TAG_SCHEDULE_DAY_PLAN:
      {
        if (GetScheduleEntry(indexes, indexCount, &actionNumber) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        if ((descriptor->tag == GLOBAL_TIME_TAG_SCHEDULE_DAY_PLAN)
            && (value->data.unsigned32 > INTERSECTION_DAY_PLAN_COUNT_MAX))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DAY_PLAN_HOUR:
      case GLOBAL_TIME_TAG_DAY_PLAN_MINUTE:
      case GLOBAL_TIME_TAG_DAY_PLAN_ACTION_OID:
      {
        if (GetDayPlanEntry(indexes,
                            indexCount,
                            &scheduleIndex,
                            &actionNumber) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        if ((descriptor->tag == GLOBAL_TIME_TAG_DAY_PLAN_HOUR)
            && (value->data.unsigned32 > 23U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        if ((descriptor->tag == GLOBAL_TIME_TAG_DAY_PLAN_MINUTE)
            && (value->data.unsigned32 > 59U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_LOCAL_TIME_DIFFERENTIAL:
      case GLOBAL_TIME_TAG_STANDARD_TIME_ZONE:
      {
        return ((value->data.signed32 >= -43200)
                && (value->data.signed32 <= 43200))
               ? NTCIP_ERROR_OK : NTCIP_ERROR_RANGE_ERROR;
      }

      case GLOBAL_TIME_TAG_DST_BEGIN_MONTH:
      {
        if (GetDstEntry(indexes, indexCount, &actionNumber) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return ((value->data.unsigned32 >= 1U) && (value->data.unsigned32 <= 14U))
               ? NTCIP_ERROR_OK : NTCIP_ERROR_RANGE_ERROR;
      }

      case GLOBAL_TIME_TAG_DST_BEGIN_OCCURRENCES:
      case GLOBAL_TIME_TAG_DST_END_OCCURRENCES:
      {
        if (GetDstEntry(indexes, indexCount, &actionNumber) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return ((value->data.unsigned32 >= 1U) && (value->data.unsigned32 <= 9U))
               ? NTCIP_ERROR_OK : NTCIP_ERROR_RANGE_ERROR;
      }

      case GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_WEEK:
      case GLOBAL_TIME_TAG_DST_END_DAY_OF_WEEK:
      {
        if (GetDstEntry(indexes, indexCount, &actionNumber) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return ((value->data.unsigned32 >= 1U) && (value->data.unsigned32 <= 7U))
               ? NTCIP_ERROR_OK : NTCIP_ERROR_RANGE_ERROR;
      }

      case GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_MONTH:
      case GLOBAL_TIME_TAG_DST_END_DAY_OF_MONTH:
      {
        if (GetDstEntry(indexes, indexCount, &actionNumber) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return ((value->data.unsigned32 >= 1U) && (value->data.unsigned32 <= 31U))
               ? NTCIP_ERROR_OK : NTCIP_ERROR_RANGE_ERROR;
      }

      case GLOBAL_TIME_TAG_DST_BEGIN_SECONDS:
      case GLOBAL_TIME_TAG_DST_END_SECONDS:
      case GLOBAL_TIME_TAG_GLOBAL_TIME:
      case GLOBAL_TIME_TAG_DST_END_MONTH:
      {
        if ((descriptor->tag != GLOBAL_TIME_TAG_GLOBAL_TIME)
            && (GetDstEntry(indexes, indexCount, &actionNumber) != NTCIP_ERROR_OK))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        if ((descriptor->tag == GLOBAL_TIME_TAG_DST_END_MONTH)
            && ((value->data.unsigned32 < 1U) || (value->data.unsigned32 > 12U)))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      case GLOBAL_TIME_TAG_DST_SECONDS_TO_ADJUST:
      {
        if (GetDstEntry(indexes, indexCount, &actionNumber) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return (value->data.unsigned32 <= 21600U)
               ? NTCIP_ERROR_OK : NTCIP_ERROR_RANGE_ERROR;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }
}

static NtcipError_t SetValueGlobalTimeManagementObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionGlobalTimeManagementConfig_t globalTimeManagement;
  uint8_t scheduleIndex = 0U;
  uint8_t dayPlanIndex = 0U;
  uint8_t eventIndex = 0U;
  uint8_t dstIndex = 0U;
  uint8_t actionNumber = 0U;
  uint32_t globalTimeSeconds = 0U;
  int32_t standardTimeZoneSeconds = 0;
  NtcipError_t error;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag == GLOBAL_TIME_TAG_GLOBAL_TIME)
  {
    if ((context->globalTimeManagementService == NULL)
        || (GlobalTimeManagementServiceSetGlobalTime(
              context->globalTimeManagementService,
              value->data.unsigned32) == 0U))
    {
      return NTCIP_ERROR_GEN_ERROR;
    }

    return NTCIP_ERROR_OK;
  }

  error = ValidateDatabaseWrite(context, requestContext);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  error = GetCandidateGlobalTimeManagementConfig(context, &globalTimeManagement);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case GLOBAL_TIME_TAG_DAYLIGHT_SAVING:
      {
        globalTimeManagement.globalDaylightSaving = (uint8_t) value->data.unsigned32;
        break;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_MONTH:
      {
        error = GetScheduleEntry(indexes, indexCount, &scheduleIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        globalTimeManagement.schedules[scheduleIndex].monthMask =
          (uint16_t) value->data.unsigned32;
        break;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_DAY:
      {
        error = GetScheduleEntry(indexes, indexCount, &scheduleIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        globalTimeManagement.schedules[scheduleIndex].dayMask =
          (uint8_t) value->data.unsigned32;
        break;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_DATE:
      {
        error = GetScheduleEntry(indexes, indexCount, &scheduleIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        globalTimeManagement.schedules[scheduleIndex].dateMask =
          value->data.unsigned32;
        break;
      }

      case GLOBAL_TIME_TAG_SCHEDULE_DAY_PLAN:
      {
        error = GetScheduleEntry(indexes, indexCount, &scheduleIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        globalTimeManagement.schedules[scheduleIndex].dayPlanNumber =
          (uint8_t) value->data.unsigned32;
        break;
      }

      case GLOBAL_TIME_TAG_DAY_PLAN_HOUR:
      {
        error = GetDayPlanEntry(indexes,
                                indexCount,
                                &dayPlanIndex,
                                &eventIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        globalTimeManagement.dayPlans[dayPlanIndex][eventIndex].hour =
          (uint8_t) value->data.unsigned32;
        break;
      }

      case GLOBAL_TIME_TAG_DAY_PLAN_MINUTE:
      {
        error = GetDayPlanEntry(indexes,
                                indexCount,
                                &dayPlanIndex,
                                &eventIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        globalTimeManagement.dayPlans[dayPlanIndex][eventIndex].minute =
          (uint8_t) value->data.unsigned32;
        break;
      }

      case GLOBAL_TIME_TAG_DAY_PLAN_ACTION_OID:
      {
        error = GetDayPlanEntry(indexes,
                                indexCount,
                                &dayPlanIndex,
                                &eventIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        if (DayPlanActionOidMatches(value, &actionNumber) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        globalTimeManagement.dayPlans[dayPlanIndex][eventIndex].actionNumber =
          actionNumber;
        break;
      }

      case GLOBAL_TIME_TAG_LOCAL_TIME_DIFFERENTIAL:
      {
        if ((context->globalTimeManagementService == NULL)
            || (GlobalTimeManagementServiceGetGlobalTime(
                  context->globalTimeManagementService,
                  &globalTimeSeconds) == 0U)
            || (GlobalTimeManagementServiceComputeStandardTimeZone(
                  &globalTimeManagement,
                  globalTimeSeconds,
                  value->data.signed32,
                  &standardTimeZoneSeconds) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        globalTimeManagement.controllerStandardTimeZoneSeconds =
          standardTimeZoneSeconds;
        break;
      }

      case GLOBAL_TIME_TAG_STANDARD_TIME_ZONE:
      {
        globalTimeManagement.controllerStandardTimeZoneSeconds =
          value->data.signed32;
        break;
      }

      case GLOBAL_TIME_TAG_DST_BEGIN_MONTH:
      case GLOBAL_TIME_TAG_DST_BEGIN_OCCURRENCES:
      case GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_WEEK:
      case GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_MONTH:
      case GLOBAL_TIME_TAG_DST_BEGIN_SECONDS:
      case GLOBAL_TIME_TAG_DST_END_MONTH:
      case GLOBAL_TIME_TAG_DST_END_OCCURRENCES:
      case GLOBAL_TIME_TAG_DST_END_DAY_OF_WEEK:
      case GLOBAL_TIME_TAG_DST_END_DAY_OF_MONTH:
      case GLOBAL_TIME_TAG_DST_END_SECONDS:
      case GLOBAL_TIME_TAG_DST_SECONDS_TO_ADJUST:
      {
        error = GetDstEntry(indexes, indexCount, &dstIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        switch (descriptor->tag)
        {
            case GLOBAL_TIME_TAG_DST_BEGIN_MONTH:
              globalTimeManagement.daylightSavingEntries[dstIndex].beginMonth =
                (uint8_t) value->data.unsigned32;
              break;
            case GLOBAL_TIME_TAG_DST_BEGIN_OCCURRENCES:
              globalTimeManagement.daylightSavingEntries[dstIndex].beginOccurrences =
                (uint8_t) value->data.unsigned32;
              break;
            case GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_WEEK:
              globalTimeManagement.daylightSavingEntries[dstIndex].beginDayOfWeek =
                (uint8_t) value->data.unsigned32;
              break;
            case GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_MONTH:
              globalTimeManagement.daylightSavingEntries[dstIndex].beginDayOfMonth =
                (uint8_t) value->data.unsigned32;
              break;
            case GLOBAL_TIME_TAG_DST_BEGIN_SECONDS:
              globalTimeManagement.daylightSavingEntries[dstIndex].beginSecondsToTransition =
                value->data.unsigned32;
              break;
            case GLOBAL_TIME_TAG_DST_END_MONTH:
              globalTimeManagement.daylightSavingEntries[dstIndex].endMonth =
                (uint8_t) value->data.unsigned32;
              break;
            case GLOBAL_TIME_TAG_DST_END_OCCURRENCES:
              globalTimeManagement.daylightSavingEntries[dstIndex].endOccurrences =
                (uint8_t) value->data.unsigned32;
              break;
            case GLOBAL_TIME_TAG_DST_END_DAY_OF_WEEK:
              globalTimeManagement.daylightSavingEntries[dstIndex].endDayOfWeek =
                (uint8_t) value->data.unsigned32;
              break;
            case GLOBAL_TIME_TAG_DST_END_DAY_OF_MONTH:
              globalTimeManagement.daylightSavingEntries[dstIndex].endDayOfMonth =
                (uint8_t) value->data.unsigned32;
              break;
            case GLOBAL_TIME_TAG_DST_END_SECONDS:
              globalTimeManagement.daylightSavingEntries[dstIndex].endSecondsToTransition =
                value->data.unsigned32;
              break;
            default:
              globalTimeManagement.daylightSavingEntries[dstIndex].secondsToAdjust =
                value->data.unsigned32;
              break;
        }

        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }

  return SaveCandidateGlobalTimeManagementConfig(context, &globalTimeManagement);
}

static const NtcipObjectDescriptor_t kGlobalTimeManagementObjects[] = {
  { kGlobalTimeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U, NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, GLOBAL_TIME_TAG_GLOBAL_TIME,
    GetGlobalTimeManagementObject, SetTestGlobalTimeManagementObject,
    SetValueGlobalTimeManagementObject },
  { kGlobalDaylightSavingOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DAYLIGHT_SAVING, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kMaxTimeBaseScheduleEntriesOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_MAX_SCHEDULES, GetGlobalTimeManagementObject, NULL, NULL },
  { kTimeBaseScheduleNumberOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_SCHEDULE_NUMBER, GetGlobalTimeManagementObject, NULL, NULL },
  { kTimeBaseScheduleMonthOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_SCHEDULE_MONTH, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kTimeBaseScheduleDayOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_SCHEDULE_DAY, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kTimeBaseScheduleDateOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_SCHEDULE_DATE, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kTimeBaseScheduleDayPlanOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_SCHEDULE_DAY_PLAN, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kMaxDayPlansOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, GLOBAL_TIME_TAG_MAX_DAY_PLANS,
    GetGlobalTimeManagementObject, NULL, NULL },
  { kMaxDayPlanEventsOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_MAX_DAY_PLAN_EVENTS, GetGlobalTimeManagementObject, NULL,
    NULL },
  { kDayPlanNumberOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DAY_PLAN_NUMBER, GetGlobalTimeManagementObject, NULL, NULL },
  { kDayPlanEventNumberOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DAY_PLAN_EVENT_NUMBER, GetGlobalTimeManagementObject, NULL,
    NULL },
  { kDayPlanHourOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DAY_PLAN_HOUR, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDayPlanMinuteOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DAY_PLAN_MINUTE, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDayPlanActionNumberOidOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OBJECT_ID,
    GLOBAL_TIME_TAG_DAY_PLAN_ACTION_OID, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDayPlanStatusOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, GLOBAL_TIME_TAG_DAY_PLAN_STATUS,
    GetGlobalTimeManagementObject, NULL, NULL },
  { kTimeBaseScheduleStatusOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_SCHEDULE_STATUS, GetGlobalTimeManagementObject, NULL, NULL },
  { kGlobalLocalTimeDifferentialOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_SIGNED32,
    GLOBAL_TIME_TAG_LOCAL_TIME_DIFFERENTIAL, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kControllerStandardTimeZoneOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_SIGNED32,
    GLOBAL_TIME_TAG_STANDARD_TIME_ZONE, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kControllerLocalTimeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_CONTROLLER_LOCAL_TIME, GetGlobalTimeManagementObject, NULL,
    NULL },
  { kMaxDaylightSavingEntriesOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_MAX_DST_ENTRIES, GetGlobalTimeManagementObject, NULL, NULL },
  { kDstEntryNumberOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DST_ENTRY_NUMBER, GetGlobalTimeManagementObject, NULL, NULL },
  { kDstBeginMonthOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DST_BEGIN_MONTH, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDstBeginOccurrencesOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DST_BEGIN_OCCURRENCES, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDstBeginDayOfWeekOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_WEEK, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDstBeginDayOfMonthOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DST_BEGIN_DAY_OF_MONTH, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDstBeginSecondsOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DST_BEGIN_SECONDS, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDstEndMonthOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DST_END_MONTH, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDstEndOccurrencesOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DST_END_OCCURRENCES, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDstEndDayOfWeekOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DST_END_DAY_OF_WEEK, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDstEndDayOfMonthOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DST_END_DAY_OF_MONTH, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDstEndSecondsOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DST_END_SECONDS, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject },
  { kDstSecondsToAdjustOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_TIME_TAG_DST_SECONDS_TO_ADJUST, GetGlobalTimeManagementObject,
    SetTestGlobalTimeManagementObject, SetValueGlobalTimeManagementObject }
};

void GlobalTimeManagementObjectsRegister(NtcipObjectDirectory_t *directory,
                                         NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1201.globalTimeManagement",
    kGlobalTimeManagementObjects,
    (uint16_t) (sizeof(kGlobalTimeManagementObjects)
                / sizeof(kGlobalTimeManagementObjects[0])),
    context);
}
