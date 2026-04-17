/* App/Domain/NTCIP/Mib1202v0335/ClockObjects.c
 *
 * 1202 ascClock subtree projection backed by canonical configuration and a
 * platform clock-source port.
 */
#include "ClockObjects.h"

#include <stddef.h>

enum
{
  CLOCK_OBJECT_TAG_MAX_TIME_SOURCES = 1,
  CLOCK_OBJECT_TAG_TIME_INDEX,
  CLOCK_OBJECT_TAG_TIME_SOURCE_AVAILABLE,
  CLOCK_OBJECT_TAG_TIME_SOURCE_COMMANDED,
  CLOCK_OBJECT_TAG_TIME_SOURCE_CURRENT,
  CLOCK_OBJECT_TAG_TIME_SOURCE_STATUS,
  CLOCK_OBJECT_TAG_TIME_NON_SEQUENTIAL_SOURCE,
  CLOCK_OBJECT_TAG_TIME_NON_SEQUENTIAL_CHANGE,
  CLOCK_OBJECT_TAG_TIME_NON_SEQUENTIAL_DELTA
};

static const uint32_t kMaxTimeSourcesOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 1U
};
static const uint32_t kUnitTimeIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 2U, 1U, 1U
};
static const uint32_t kUnitTimeSourceAvailableOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 2U, 1U, 2U
};
static const uint32_t kUnitTimeSourceCommandedOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 3U
};
static const uint32_t kUnitTimeSourceCurrentOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 4U
};
static const uint32_t kUnitTimeSourceStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 5U
};
static const uint32_t kUnitTimeNonSequentialSourceOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 6U
};
static const uint32_t kUnitTimeNonSequentialChangeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 7U
};
static const uint32_t kUnitTimeNonSequentialDeltaOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 8U
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

static NtcipError_t GetTimeSourceFromIndex(const NtcipContext_t *context,
                                           const uint32_t *indexes,
                                           uint8_t indexCount,
                                           uint8_t *sourceIndex,
                                           uint8_t *sourceAvailable)
{
  uint8_t sourceCount = 0U;

  if ((context == NULL) || (context->unitClockPort == NULL)
      || (indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (sourceIndex == NULL) || (sourceAvailable == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  sourceCount = UnitClockPortGetSourceCount(context->unitClockPort);
  if ((sourceCount == 0U) || (indexes[0] > sourceCount))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *sourceIndex = (uint8_t) (indexes[0] - 1U);

  if (UnitClockPortGetSourceAvailable(context->unitClockPort,
                                      *sourceIndex,
                                      sourceAvailable) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static uint8_t IsSupportedCommandedSource(const NtcipContext_t *context,
                                          uint8_t source)
{
  uint8_t sourceCount;
  uint8_t sourceIndex;
  uint8_t sourceAvailable = 0U;

  if ((context == NULL) || (context->unitClockPort == NULL))
  {
    return 0U;
  }

  sourceCount = UnitClockPortGetSourceCount(context->unitClockPort);
  for (sourceIndex = 0U; sourceIndex < sourceCount; sourceIndex++)
  {
    if ((UnitClockPortGetSourceAvailable(context->unitClockPort,
                                         sourceIndex,
                                         &sourceAvailable) != 0U)
        && (sourceAvailable == source))
    {
      return 1U;
    }
  }

  return 0U;
}

static NtcipError_t GetClockObject(void *groupContext,
                                   const NtcipObjectDescriptor_t *descriptor,
                                   const uint32_t *indexes,
                                   uint8_t indexCount,
                                   const NtcipRequestContext_t *requestContext,
                                   NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionUnitConfig_t unitConfig;
  UnitClockNonSequentialDelta_t delta = { { 0U }, 0U };
  uint8_t sourceIndex = 0U;
  uint8_t sourceValue = 0U;
  uint32_t nonSequentialChange = 0U;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case CLOCK_OBJECT_TAG_MAX_TIME_SOURCES:
      {
        if (context->unitClockPort == NULL)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value,
                                UnitClockPortGetSourceCount(
                                  context->unitClockPort));

        return NTCIP_ERROR_OK;
      }

      case CLOCK_OBJECT_TAG_TIME_SOURCE_COMMANDED:
      {
        if ((context->configurationService == NULL)
            || (ConfigurationServiceGetActiveUnitConfig(
                  context->configurationService,
                  &unitConfig) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, unitConfig.timeSourceCommanded);

        return NTCIP_ERROR_OK;
      }

      case CLOCK_OBJECT_TAG_TIME_SOURCE_CURRENT:
      {
        if ((context->unitClockPort == NULL)
            || (UnitClockPortGetCurrentSource(context->unitClockPort,
                                              &sourceValue) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, sourceValue);

        return NTCIP_ERROR_OK;
      }

      case CLOCK_OBJECT_TAG_TIME_SOURCE_STATUS:
      {
        if ((context->unitClockPort == NULL)
            || (UnitClockPortGetCurrentStatus(context->unitClockPort,
                                              &sourceValue) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, sourceValue);

        if (sourceValue == (uint8_t) UNIT_CLOCK_SOURCE_STATUS_NON_SEQUENTIAL)
        {
          UnitClockPortAcknowledgeCurrentStatusRead(context->unitClockPort);
        }

        return NTCIP_ERROR_OK;
      }

      case CLOCK_OBJECT_TAG_TIME_NON_SEQUENTIAL_SOURCE:
      {
        if ((context->unitClockPort == NULL)
            || (UnitClockPortGetNonSequentialSource(context->unitClockPort,
                                                    &sourceValue) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, sourceValue);

        return NTCIP_ERROR_OK;
      }

      case CLOCK_OBJECT_TAG_TIME_NON_SEQUENTIAL_CHANGE:
      {
        if ((context->unitClockPort == NULL)
            || (UnitClockPortGetNonSequentialChange(context->unitClockPort,
                                                    &nonSequentialChange) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, nonSequentialChange);

        return NTCIP_ERROR_OK;
      }

      case CLOCK_OBJECT_TAG_TIME_NON_SEQUENTIAL_DELTA:
      {
        if ((context->unitClockPort == NULL)
            || (UnitClockPortGetNonSequentialDelta(context->unitClockPort,
                                                   &delta) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        return NtcipValueSetOctetString(value, delta.bytes, delta.length);
      }

      case CLOCK_OBJECT_TAG_TIME_INDEX:
      case CLOCK_OBJECT_TAG_TIME_SOURCE_AVAILABLE:
      {
        error = GetTimeSourceFromIndex(context,
                                       indexes,
                                       indexCount,
                                       &sourceIndex,
                                       &sourceValue);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        if (descriptor->tag == CLOCK_OBJECT_TAG_TIME_INDEX)
        {
          NtcipValueSetUnsigned32(value, indexes[0]);
        }
        else
        {
          NtcipValueSetUnsigned32(value, sourceValue);
        }

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetTestClockObject(void *groupContext,
                                       const NtcipObjectDescriptor_t *descriptor,
                                       const uint32_t *indexes,
                                       uint8_t indexCount,
                                       const NtcipRequestContext_t *requestContext,
                                       const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  NtcipError_t error;

  (void) indexes;
  (void) indexCount;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag != CLOCK_OBJECT_TAG_TIME_SOURCE_COMMANDED)
  {
    return NTCIP_ERROR_READ_ONLY;
  }

  error = ValidateDatabaseWrite(context, requestContext);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if (value->type != NTCIP_VALUE_TYPE_UNSIGNED32)
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (value->data.unsigned32 > 255U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  if (IsSupportedCommandedSource(context, (uint8_t) value->data.unsigned32) == 0U)
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t SetValueClockObject(void *groupContext,
                                        const NtcipObjectDescriptor_t *descriptor,
                                        const uint32_t *indexes,
                                        uint8_t indexCount,
                                        const NtcipRequestContext_t *requestContext,
                                        const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  NtcipError_t error;

  error = SetTestClockObject(groupContext,
                             descriptor,
                             indexes,
                             indexCount,
                             requestContext,
                             value);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if ((context == NULL) || (context->configurationService == NULL))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return (ConfigurationServiceSetUnitTimeSourceCommanded(
            context->configurationService,
            (uint8_t) value->data.unsigned32) != 0U)
           ? NTCIP_ERROR_OK
           : NTCIP_ERROR_GEN_ERROR;
}

static const NtcipObjectDescriptor_t kClockObjects[] = {
  { kMaxTimeSourcesOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CLOCK_OBJECT_TAG_MAX_TIME_SOURCES, GetClockObject, NULL, NULL },
  { kUnitTimeIndexOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CLOCK_OBJECT_TAG_TIME_INDEX, GetClockObject, NULL, NULL },
  { kUnitTimeSourceAvailableOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CLOCK_OBJECT_TAG_TIME_SOURCE_AVAILABLE, GetClockObject, NULL, NULL },
  { kUnitTimeSourceCommandedOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    CLOCK_OBJECT_TAG_TIME_SOURCE_COMMANDED, GetClockObject, SetTestClockObject,
    SetValueClockObject },
  { kUnitTimeSourceCurrentOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CLOCK_OBJECT_TAG_TIME_SOURCE_CURRENT, GetClockObject, NULL, NULL },
  { kUnitTimeSourceStatusOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CLOCK_OBJECT_TAG_TIME_SOURCE_STATUS, GetClockObject, NULL, NULL },
  { kUnitTimeNonSequentialSourceOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CLOCK_OBJECT_TAG_TIME_NON_SEQUENTIAL_SOURCE, GetClockObject, NULL, NULL },
  { kUnitTimeNonSequentialChangeOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CLOCK_OBJECT_TAG_TIME_NON_SEQUENTIAL_CHANGE, GetClockObject, NULL, NULL },
  { kUnitTimeNonSequentialDeltaOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OCTET_STRING,
    CLOCK_OBJECT_TAG_TIME_NON_SEQUENTIAL_DELTA, GetClockObject, NULL, NULL }
};

void ClockObjectsRegister(NtcipObjectDirectory_t *directory,
                          NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.ascClock",
    kClockObjects,
    (uint16_t) (sizeof(kClockObjects) / sizeof(kClockObjects[0])),
    context);
}
