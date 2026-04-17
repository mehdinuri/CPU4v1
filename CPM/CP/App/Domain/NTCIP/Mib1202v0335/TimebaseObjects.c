/* App/Domain/NTCIP/Mib1202v0335/TimebaseObjects.c
 *
 * 1202 timebaseAsc subtree projection backed by canonical configuration and
 * engine runtime command state.
 */
#include "TimebaseObjects.h"

#include <stddef.h>

enum
{
  TIMEBASE_OBJECT_TAG_PATTERN_SYNC = 1,
  TIMEBASE_OBJECT_TAG_MAX_ACTIONS,
  TIMEBASE_OBJECT_TAG_ACTION_NUMBER,
  TIMEBASE_OBJECT_TAG_ACTION_PATTERN,
  TIMEBASE_OBJECT_TAG_ACTION_AUX_FUNCTION,
  TIMEBASE_OBJECT_TAG_ACTION_SPECIAL_FUNCTION,
  TIMEBASE_OBJECT_TAG_ACTION_ENABLED_LANE,
  TIMEBASE_OBJECT_TAG_ACTION_STATUS,
  TIMEBASE_OBJECT_TAG_ACTION_PLAN_CONTROL
};

static const uint32_t kTimebaseAscPatternSyncOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 1U
};
static const uint32_t kMaxTimebaseAscActionsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 2U
};
static const uint32_t kTimebaseAscActionNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 3U, 1U, 1U
};
static const uint32_t kTimebaseAscPatternOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 3U, 1U, 2U
};
static const uint32_t kTimebaseAscAuxiliaryFunctionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 3U, 1U, 3U
};
static const uint32_t kTimebaseAscSpecialFunctionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 3U, 1U, 4U
};
static const uint32_t kTimebaseAscEnabledLaneOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 3U, 1U, 5U
};
static const uint32_t kTimebaseAscActionStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 4U
};
static const uint32_t kActionPlanControlOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 5U, 5U
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

static NtcipError_t GetTimebaseActionFromIndex(
  const NtcipContext_t *context,
  const uint32_t *indexes,
  uint8_t indexCount,
  uint8_t *actionIndex,
  IntersectionTimebaseActionConfig_t *actionConfig)
{
  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (actionIndex == NULL) || (actionConfig == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (indexes[0]
      > ConfigurationServiceGetTimebaseActionCount(context->configurationService))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *actionIndex = (uint8_t) (indexes[0] - 1U);

  if (ConfigurationServiceGetActiveTimebaseActionConfig(
        context->configurationService,
        *actionIndex,
        actionConfig) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetTimebaseObject(void *groupContext,
                                      const NtcipObjectDescriptor_t *descriptor,
                                      const uint32_t *indexes,
                                      uint8_t indexCount,
                                      const NtcipRequestContext_t *
                                      requestContext,
                                      NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  const IntersectionRuntime_t *runtime;
  IntersectionTimebaseConfig_t timebase;
  IntersectionTimebaseActionConfig_t actionConfig;
  uint8_t actionIndex;
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
      case TIMEBASE_OBJECT_TAG_MAX_ACTIONS:
      {
        NtcipValueSetUnsigned32(
          value,
          ConfigurationServiceGetTimebaseActionCount(context->configurationService));

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_ACTION_STATUS:
      {
        NtcipValueSetUnsigned32(
          value,
          (runtime != NULL) ? runtime->timebaseActionStatus : 0U);

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_ACTION_PLAN_CONTROL:
      {
        NtcipValueSetUnsigned32(
          value,
          (runtime != NULL) ? runtime->actionPlanControl : 0U);

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_PATTERN_SYNC:
      {
        if (ConfigurationServiceGetActiveTimebaseConfig(
              context->configurationService,
              &timebase) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, timebase.patternSyncMinutes);

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_ACTION_NUMBER:
      case TIMEBASE_OBJECT_TAG_ACTION_PATTERN:
      case TIMEBASE_OBJECT_TAG_ACTION_AUX_FUNCTION:
      case TIMEBASE_OBJECT_TAG_ACTION_SPECIAL_FUNCTION:
      case TIMEBASE_OBJECT_TAG_ACTION_ENABLED_LANE:
      {
        error = GetTimebaseActionFromIndex(context,
                                           indexes,
                                           indexCount,
                                           &actionIndex,
                                           &actionConfig);

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
  }

  switch (descriptor->tag)
  {
      case TIMEBASE_OBJECT_TAG_ACTION_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_ACTION_PATTERN:
      {
        NtcipValueSetUnsigned32(value, actionConfig.pattern);

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_ACTION_AUX_FUNCTION:
      {
        NtcipValueSetUnsigned32(value, actionConfig.auxiliaryFunction);

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_ACTION_SPECIAL_FUNCTION:
      {
        NtcipValueSetUnsigned32(value, actionConfig.specialFunction);

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_ACTION_ENABLED_LANE:
      {
        NtcipValueSetUnsigned32(value, actionConfig.enabledLane);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetTestTimebaseObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (value->type != NTCIP_VALUE_TYPE_UNSIGNED32)
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case TIMEBASE_OBJECT_TAG_ACTION_PLAN_CONTROL:
      {
        if ((indexCount != 0U) || (value->data.unsigned32 > 255U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_PATTERN_SYNC:
      {
        if ((indexCount != 0U) || (value->data.unsigned32 > 65535U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return ValidateDatabaseWrite(context, requestContext);
      }

      case TIMEBASE_OBJECT_TAG_ACTION_PATTERN:
      case TIMEBASE_OBJECT_TAG_ACTION_AUX_FUNCTION:
      case TIMEBASE_OBJECT_TAG_ACTION_SPECIAL_FUNCTION:
      case TIMEBASE_OBJECT_TAG_ACTION_ENABLED_LANE:
      {
        if ((indexCount != 1U) || (indexes == NULL) || (indexes[0] == 0U)
            || (indexes[0]
                > ConfigurationServiceGetTimebaseActionCount(
                  context->configurationService))
            || (value->data.unsigned32 > 255U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        if ((descriptor->tag == TIMEBASE_OBJECT_TAG_ACTION_AUX_FUNCTION)
            && ((value->data.unsigned32 & 0xF0U) != 0U))
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return ValidateDatabaseWrite(context, requestContext);
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }
}

static NtcipError_t SetValueTimebaseObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  uint8_t actionIndex;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case TIMEBASE_OBJECT_TAG_ACTION_PLAN_CONTROL:
      {
        if ((context->intersectionEngine == NULL)
            || (IntersectionEngineSetActionPlanControl(
                  context->intersectionEngine,
                  (uint8_t) value->data.unsigned32) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_PATTERN_SYNC:
      {
        if (ConfigurationServiceSetTimebasePatternSyncMinutes(
              context->configurationService,
              (uint16_t) value->data.unsigned32) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_ACTION_PATTERN:
      case TIMEBASE_OBJECT_TAG_ACTION_AUX_FUNCTION:
      case TIMEBASE_OBJECT_TAG_ACTION_SPECIAL_FUNCTION:
      case TIMEBASE_OBJECT_TAG_ACTION_ENABLED_LANE:
      {
        if ((indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
            || (indexes[0]
                > ConfigurationServiceGetTimebaseActionCount(
                  context->configurationService)))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        actionIndex = (uint8_t) (indexes[0] - 1U);
        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }

  switch (descriptor->tag)
  {
      case TIMEBASE_OBJECT_TAG_ACTION_PATTERN:
      {
        if (ConfigurationServiceSetTimebaseActionPattern(
              context->configurationService,
              actionIndex,
              (uint8_t) value->data.unsigned32) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_ACTION_AUX_FUNCTION:
      {
        if (ConfigurationServiceSetTimebaseActionAuxiliaryFunction(
              context->configurationService,
              actionIndex,
              (uint8_t) value->data.unsigned32) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_ACTION_SPECIAL_FUNCTION:
      {
        if (ConfigurationServiceSetTimebaseActionSpecialFunction(
              context->configurationService,
              actionIndex,
              (uint8_t) value->data.unsigned32) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      case TIMEBASE_OBJECT_TAG_ACTION_ENABLED_LANE:
      {
        if (ConfigurationServiceSetTimebaseActionEnabledLane(
              context->configurationService,
              actionIndex,
              (uint8_t) value->data.unsigned32) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }
}

static const NtcipObjectDescriptor_t kTimebaseObjects[] = {
  { kTimebaseAscPatternSyncOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TIMEBASE_OBJECT_TAG_PATTERN_SYNC, GetTimebaseObject, SetTestTimebaseObject,
    SetValueTimebaseObject },
  { kMaxTimebaseAscActionsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    TIMEBASE_OBJECT_TAG_MAX_ACTIONS, GetTimebaseObject, NULL, NULL },
  { kTimebaseAscActionNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    TIMEBASE_OBJECT_TAG_ACTION_NUMBER, GetTimebaseObject, NULL, NULL },
  { kTimebaseAscPatternOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TIMEBASE_OBJECT_TAG_ACTION_PATTERN, GetTimebaseObject,
    SetTestTimebaseObject, SetValueTimebaseObject },
  { kTimebaseAscAuxiliaryFunctionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TIMEBASE_OBJECT_TAG_ACTION_AUX_FUNCTION, GetTimebaseObject,
    SetTestTimebaseObject, SetValueTimebaseObject },
  { kTimebaseAscSpecialFunctionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TIMEBASE_OBJECT_TAG_ACTION_SPECIAL_FUNCTION, GetTimebaseObject,
    SetTestTimebaseObject, SetValueTimebaseObject },
  { kTimebaseAscEnabledLaneOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TIMEBASE_OBJECT_TAG_ACTION_ENABLED_LANE, GetTimebaseObject,
    SetTestTimebaseObject, SetValueTimebaseObject },
  { kTimebaseAscActionStatusOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    TIMEBASE_OBJECT_TAG_ACTION_STATUS, GetTimebaseObject, NULL, NULL },
  { kActionPlanControlOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TIMEBASE_OBJECT_TAG_ACTION_PLAN_CONTROL, GetTimebaseObject,
    SetTestTimebaseObject, SetValueTimebaseObject }
};

void TimebaseObjectsRegister(NtcipObjectDirectory_t *directory,
                             NtcipContext_t *context)
{
  NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.timebaseAsc",
    kTimebaseObjects,
    (uint16_t) (sizeof(kTimebaseObjects) / sizeof(kTimebaseObjects[0])),
    context);
}
