/* App/Domain/NTCIP/Mib1202v0335/RingControlObjects.c
 *
 * Runtime-backed ring control group projection for NTCIP 1202 v03.35e.
 */
#include "RingControlObjects.h"

#include <stddef.h>

enum
{
  RING_CONTROL_OBJECT_TAG_MAX_GROUPS = 1,
  RING_CONTROL_OBJECT_TAG_GROUP_NUMBER,
  RING_CONTROL_OBJECT_TAG_STOP_TIME,
  RING_CONTROL_OBJECT_TAG_FORCE_OFF,
  RING_CONTROL_OBJECT_TAG_MAX2,
  RING_CONTROL_OBJECT_TAG_MAX_INHIBIT,
  RING_CONTROL_OBJECT_TAG_PED_RECYCLE,
  RING_CONTROL_OBJECT_TAG_RED_REST,
  RING_CONTROL_OBJECT_TAG_OMIT_RED_CLEAR,
  RING_CONTROL_OBJECT_TAG_MAX3
};

static const uint32_t kMaxRingControlGroupsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 4U
};
static const uint32_t kRingControlGroupNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 1U
};
static const uint32_t kRingControlGroupStopTimeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 2U
};
static const uint32_t kRingControlGroupForceOffOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 3U
};
static const uint32_t kRingControlGroupMax2Oid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 4U
};
static const uint32_t kRingControlGroupMaxInhibitOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 5U
};
static const uint32_t kRingControlGroupPedRecycleOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 6U
};
static const uint32_t kRingControlGroupRedRestOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 7U
};
static const uint32_t kRingControlGroupOmitRedClearOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 8U
};
static const uint32_t kRingControlGroupMax3Oid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 5U, 1U, 9U
};

static uint8_t GetMaxRingControlGroups(const NtcipContext_t *context)
{
  if ((context == NULL) || (context->configurationService == NULL))
  {
    return 0U;
  }

  return (uint8_t) ((ConfigurationServiceGetRingCount(
                       context->configurationService) + 7U) / 8U);
}

static uint8_t GetGroupBitMask(uint8_t ringNumber)
{
  return (uint8_t) (1U << ((ringNumber - 1U) % 8U));
}

static uint8_t GetGroupRingLimit(const NtcipContext_t *context,
                                 uint8_t groupNumber)
{
  uint8_t ringCount;
  uint8_t startRing;

  if ((context == NULL) || (context->configurationService == NULL)
      || (groupNumber == 0U))
  {
    return 0U;
  }

  ringCount = ConfigurationServiceGetRingCount(context->configurationService);
  startRing = (uint8_t) (((groupNumber - 1U) * 8U) + 1U);

  if (startRing > ringCount)
  {
    return 0U;
  }

  if ((uint8_t) (ringCount - startRing + 1U) >= 8U)
  {
    return 8U;
  }

  return (uint8_t) (ringCount - startRing + 1U);
}

static NtcipError_t GetGroupNumber(const NtcipContext_t *context,
                                   const uint32_t *indexes,
                                   uint8_t indexCount,
                                   uint8_t *groupNumber)
{
  uint8_t maxGroups;

  if ((context == NULL) || (indexes == NULL) || (indexCount != 1U)
      || (groupNumber == NULL) || (indexes[0] == 0U))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  maxGroups = GetMaxRingControlGroups(context);

  if ((maxGroups == 0U) || (indexes[0] > maxGroups))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *groupNumber = (uint8_t) indexes[0];

  return NTCIP_ERROR_OK;
}

static NtcipError_t ValidateControlMask(const NtcipContext_t *context,
                                        uint8_t groupNumber,
                                        uint32_t rawMask)
{
  uint8_t ringLimit;
  uint8_t validMask;

  if (rawMask > 255U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  ringLimit = GetGroupRingLimit(context, groupNumber);

  if (ringLimit == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  validMask = (ringLimit >= 8U) ? 0xFFU : (uint8_t) ((1U << ringLimit) - 1U);

  if ((((uint8_t) rawMask) & (uint8_t) (~validMask)) != 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t ReadControlMask(const NtcipContext_t *context,
                                    uint8_t groupNumber,
                                    uint16_t tag,
                                    uint8_t *mask)
{
  uint8_t ringCount;
  uint8_t ringNumber;

  if ((context == NULL) || (context->configurationService == NULL)
      || (context->intersectionEngine == NULL) || (mask == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  ringCount = ConfigurationServiceGetRingCount(context->configurationService);
  *mask = 0U;

  for (ringNumber = (uint8_t) (((groupNumber - 1U) * 8U) + 1U);
       (ringNumber <= ringCount)
       && (ringNumber <= (uint8_t) (groupNumber * 8U));
       ++ringNumber)
  {
    uint8_t active = 0U;
    uint8_t ok = 0U;

    switch (tag)
    {
        case RING_CONTROL_OBJECT_TAG_STOP_TIME:
        {
          ok = IntersectionEngineGetRingStopTimeControl(
            context->intersectionEngine,
            ringNumber,
            &active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_FORCE_OFF:
        {
          ok = IntersectionEngineGetRingForceOffControl(
            context->intersectionEngine,
            ringNumber,
            &active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_MAX2:
        {
          ok = IntersectionEngineGetRingMaximum2Control(
            context->intersectionEngine,
            ringNumber,
            &active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_MAX_INHIBIT:
        {
          ok = IntersectionEngineGetRingMaximumInhibitControl(
            context->intersectionEngine,
            ringNumber,
            &active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_PED_RECYCLE:
        {
          ok = IntersectionEngineGetRingPedRecycleControl(
            context->intersectionEngine,
            ringNumber,
            &active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_RED_REST:
        {
          ok = IntersectionEngineGetRingRedRestControl(
            context->intersectionEngine,
            ringNumber,
            &active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_OMIT_RED_CLEAR:
        {
          ok = IntersectionEngineGetRingOmitRedClearControl(
            context->intersectionEngine,
            ringNumber,
            &active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_MAX3:
        {
          ok = IntersectionEngineGetRingMaximum3Control(
            context->intersectionEngine,
            ringNumber,
            &active);
          break;
        }

        default:
        {
          return NTCIP_ERROR_NOT_FOUND;
        }
    }

    if (ok == 0U)
    {
      return NTCIP_ERROR_GEN_ERROR;
    }

    if (active != 0U)
    {
      *mask = (uint8_t) (*mask | GetGroupBitMask(ringNumber));
    }
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t ApplyControlMask(const NtcipContext_t *context,
                                     uint8_t groupNumber,
                                     uint16_t tag,
                                     uint8_t mask)
{
  uint8_t ringCount;
  uint8_t ringNumber;

  if ((context == NULL) || (context->configurationService == NULL)
      || (context->intersectionEngine == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  ringCount = ConfigurationServiceGetRingCount(context->configurationService);

  for (ringNumber = (uint8_t) (((groupNumber - 1U) * 8U) + 1U);
       (ringNumber <= ringCount)
       && (ringNumber <= (uint8_t) (groupNumber * 8U));
       ++ringNumber)
  {
    uint8_t active = (uint8_t) ((mask & GetGroupBitMask(ringNumber)) != 0U);
    uint8_t ok = 0U;

    switch (tag)
    {
        case RING_CONTROL_OBJECT_TAG_STOP_TIME:
        {
          ok = IntersectionEngineSetRingStopTimeControl(
            context->intersectionEngine,
            ringNumber,
            active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_FORCE_OFF:
        {
          ok = IntersectionEngineSetRingForceOffControl(
            context->intersectionEngine,
            ringNumber,
            active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_MAX2:
        {
          ok = IntersectionEngineSetRingMaximum2Control(
            context->intersectionEngine,
            ringNumber,
            active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_MAX_INHIBIT:
        {
          ok = IntersectionEngineSetRingMaximumInhibitControl(
            context->intersectionEngine,
            ringNumber,
            active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_PED_RECYCLE:
        {
          ok = IntersectionEngineSetRingPedRecycleControl(
            context->intersectionEngine,
            ringNumber,
            active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_RED_REST:
        {
          ok = IntersectionEngineSetRingRedRestControl(
            context->intersectionEngine,
            ringNumber,
            active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_OMIT_RED_CLEAR:
        {
          ok = IntersectionEngineSetRingOmitRedClearControl(
            context->intersectionEngine,
            ringNumber,
            active);
          break;
        }

        case RING_CONTROL_OBJECT_TAG_MAX3:
        {
          ok = IntersectionEngineSetRingMaximum3Control(
            context->intersectionEngine,
            ringNumber,
            active);
          break;
        }

        default:
        {
          return NTCIP_ERROR_NOT_FOUND;
        }
    }

    if (ok == 0U)
    {
      return NTCIP_ERROR_GEN_ERROR;
    }
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetRingControlObject(void *groupContext,
                                         const NtcipObjectDescriptor_t *descriptor,
                                         const uint32_t *indexes,
                                         uint8_t indexCount,
                                         const NtcipRequestContext_t *requestContext,
                                         NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  uint8_t groupNumber;
  uint8_t mask;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag == RING_CONTROL_OBJECT_TAG_MAX_GROUPS)
  {
    NtcipValueSetUnsigned32(value, GetMaxRingControlGroups(context));

    return NTCIP_ERROR_OK;
  }

  error = GetGroupNumber(context, indexes, indexCount, &groupNumber);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if (descriptor->tag == RING_CONTROL_OBJECT_TAG_GROUP_NUMBER)
  {
    NtcipValueSetUnsigned32(value, groupNumber);

    return NTCIP_ERROR_OK;
  }

  error = ReadControlMask(context, groupNumber, descriptor->tag, &mask);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  NtcipValueSetUnsigned32(value, mask);

  return NTCIP_ERROR_OK;
}

static NtcipError_t SetTestRingControlObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  uint8_t groupNumber;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag == RING_CONTROL_OBJECT_TAG_MAX_GROUPS)
  {
    return NTCIP_ERROR_READ_ONLY;
  }

  error = GetGroupNumber(context, indexes, indexCount, &groupNumber);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if (descriptor->tag == RING_CONTROL_OBJECT_TAG_GROUP_NUMBER)
  {
    return NTCIP_ERROR_READ_ONLY;
  }

  return ValidateControlMask(context, groupNumber, value->data.unsigned32);
}

static NtcipError_t SetValueRingControlObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  uint8_t groupNumber;
  NtcipError_t error;

  error = SetTestRingControlObject(groupContext,
                                   descriptor,
                                   indexes,
                                   indexCount,
                                   requestContext,
                                   value);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  error = GetGroupNumber(context, indexes, indexCount, &groupNumber);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  return ApplyControlMask(context,
                          groupNumber,
                          descriptor->tag,
                          (uint8_t) value->data.unsigned32);
}

static const NtcipObjectDescriptor_t kRingControlObjects[] = {
  { kMaxRingControlGroupsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    RING_CONTROL_OBJECT_TAG_MAX_GROUPS, GetRingControlObject, NULL, NULL },
  { kRingControlGroupNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    RING_CONTROL_OBJECT_TAG_GROUP_NUMBER, GetRingControlObject, NULL, NULL },
  { kRingControlGroupStopTimeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    RING_CONTROL_OBJECT_TAG_STOP_TIME, GetRingControlObject,
    SetTestRingControlObject, SetValueRingControlObject },
  { kRingControlGroupForceOffOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    RING_CONTROL_OBJECT_TAG_FORCE_OFF, GetRingControlObject,
    SetTestRingControlObject, SetValueRingControlObject },
  { kRingControlGroupMax2Oid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    RING_CONTROL_OBJECT_TAG_MAX2, GetRingControlObject,
    SetTestRingControlObject, SetValueRingControlObject },
  { kRingControlGroupMaxInhibitOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    RING_CONTROL_OBJECT_TAG_MAX_INHIBIT, GetRingControlObject,
    SetTestRingControlObject, SetValueRingControlObject },
  { kRingControlGroupPedRecycleOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    RING_CONTROL_OBJECT_TAG_PED_RECYCLE, GetRingControlObject,
    SetTestRingControlObject, SetValueRingControlObject },
  { kRingControlGroupRedRestOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    RING_CONTROL_OBJECT_TAG_RED_REST, GetRingControlObject,
    SetTestRingControlObject, SetValueRingControlObject },
  { kRingControlGroupOmitRedClearOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    RING_CONTROL_OBJECT_TAG_OMIT_RED_CLEAR, GetRingControlObject,
    SetTestRingControlObject, SetValueRingControlObject },
  { kRingControlGroupMax3Oid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    RING_CONTROL_OBJECT_TAG_MAX3, GetRingControlObject,
    SetTestRingControlObject, SetValueRingControlObject }
};

void RingControlObjectsRegister(NtcipObjectDirectory_t *directory,
                                NtcipContext_t *context)
{
  if ((directory == NULL) || (context == NULL))
  {
    return;
  }

  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "ringControl",
    kRingControlObjects,
    (uint16_t) (sizeof(kRingControlObjects) / sizeof(kRingControlObjects[0])),
    context);
}
