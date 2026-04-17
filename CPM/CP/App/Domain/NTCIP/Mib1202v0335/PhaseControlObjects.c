/* App/Domain/NTCIP/Mib1202v0335/PhaseControlObjects.c
 *
 * Runtime-backed phase control group projection for NTCIP 1202 v03.35e.
 */
#include "PhaseControlObjects.h"

#include <stddef.h>

enum
{
  PHASE_CONTROL_OBJECT_TAG_GROUP_NUMBER = 1,
  PHASE_CONTROL_OBJECT_TAG_PHASE_OMIT,
  PHASE_CONTROL_OBJECT_TAG_PED_OMIT,
  PHASE_CONTROL_OBJECT_TAG_HOLD,
  PHASE_CONTROL_OBJECT_TAG_FORCE_OFF,
  PHASE_CONTROL_OBJECT_TAG_VEH_CALL,
  PHASE_CONTROL_OBJECT_TAG_PED_CALL
};

static const uint32_t kPhaseControlGroupNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 5U, 1U, 1U
};
static const uint32_t kPhaseControlGroupPhaseOmitOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 5U, 1U, 2U
};
static const uint32_t kPhaseControlGroupPedOmitOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 5U, 1U, 3U
};
static const uint32_t kPhaseControlGroupHoldOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 5U, 1U, 4U
};
static const uint32_t kPhaseControlGroupForceOffOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 5U, 1U, 5U
};
static const uint32_t kPhaseControlGroupVehCallOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 5U, 1U, 6U
};
static const uint32_t kPhaseControlGroupPedCallOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 5U, 1U, 7U
};

static uint8_t GetMaxPhaseGroups(const NtcipContext_t *context)
{
  if ((context == NULL) || (context->configurationService == NULL))
  {
    return 0U;
  }

  return (uint8_t) ((ConfigurationServiceGetPhaseCount(
                       context->configurationService) + 7U) / 8U);
}

static uint8_t GetGroupBitMask(uint8_t phaseNumber)
{
  return (uint8_t) (1U << ((phaseNumber - 1U) % 8U));
}

static uint8_t GetGroupPhaseLimit(const NtcipContext_t *context,
                                  uint8_t groupNumber)
{
  uint8_t phaseCount;
  uint8_t startPhase;

  if ((context == NULL) || (context->configurationService == NULL)
      || (groupNumber == 0U))
  {
    return 0U;
  }

  phaseCount = ConfigurationServiceGetPhaseCount(context->configurationService);
  startPhase = (uint8_t) (((groupNumber - 1U) * 8U) + 1U);

  if (startPhase > phaseCount)
  {
    return 0U;
  }

  if ((uint8_t) (phaseCount - startPhase + 1U) >= 8U)
  {
    return 8U;
  }

  return (uint8_t) (phaseCount - startPhase + 1U);
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

  maxGroups = GetMaxPhaseGroups(context);

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
  uint8_t phaseLimit;
  uint8_t validMask;

  if (rawMask > 255U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  phaseLimit = GetGroupPhaseLimit(context, groupNumber);

  if (phaseLimit == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  validMask = (phaseLimit >= 8U) ? 0xFFU : (uint8_t) ((1U << phaseLimit) - 1U);

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
  uint8_t phaseCount;
  uint8_t phaseNumber;

  if ((context == NULL) || (context->configurationService == NULL)
      || (context->intersectionEngine == NULL) || (mask == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  phaseCount = ConfigurationServiceGetPhaseCount(context->configurationService);
  *mask = 0U;

  for (phaseNumber = (uint8_t) (((groupNumber - 1U) * 8U) + 1U);
       (phaseNumber <= phaseCount)
       && (phaseNumber <= (uint8_t) (groupNumber * 8U));
       ++phaseNumber)
  {
    uint8_t active = 0U;
    uint8_t ok = 0U;

    switch (tag)
    {
        case PHASE_CONTROL_OBJECT_TAG_PHASE_OMIT:
        {
          ok = IntersectionEngineGetPhaseOmitControl(context->intersectionEngine,
                                                     phaseNumber,
                                                     &active);
          break;
        }

        case PHASE_CONTROL_OBJECT_TAG_PED_OMIT:
        {
          ok = IntersectionEngineGetPedOmitControl(context->intersectionEngine,
                                                   phaseNumber,
                                                   &active);
          break;
        }

        case PHASE_CONTROL_OBJECT_TAG_HOLD:
        {
          ok = IntersectionEngineGetPhaseHoldControl(context->intersectionEngine,
                                                     phaseNumber,
                                                     &active);
          break;
        }

        case PHASE_CONTROL_OBJECT_TAG_FORCE_OFF:
        {
          ok = IntersectionEngineGetPhaseForceOffControl(
            context->intersectionEngine,
            phaseNumber,
            &active);
          break;
        }

        case PHASE_CONTROL_OBJECT_TAG_VEH_CALL:
        {
          ok = IntersectionEngineGetVehCallControl(context->intersectionEngine,
                                                   phaseNumber,
                                                   &active);
          break;
        }

        case PHASE_CONTROL_OBJECT_TAG_PED_CALL:
        {
          ok = IntersectionEngineGetPedCallControl(context->intersectionEngine,
                                                   phaseNumber,
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
      *mask = (uint8_t) (*mask | GetGroupBitMask(phaseNumber));
    }
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t ApplyControlMask(const NtcipContext_t *context,
                                     uint8_t groupNumber,
                                     uint16_t tag,
                                     uint8_t mask)
{
  uint8_t phaseCount;
  uint8_t phaseNumber;

  if ((context == NULL) || (context->configurationService == NULL)
      || (context->intersectionEngine == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  phaseCount = ConfigurationServiceGetPhaseCount(context->configurationService);

  for (phaseNumber = (uint8_t) (((groupNumber - 1U) * 8U) + 1U);
       (phaseNumber <= phaseCount)
       && (phaseNumber <= (uint8_t) (groupNumber * 8U));
       ++phaseNumber)
  {
    uint8_t active = (uint8_t) ((mask & GetGroupBitMask(phaseNumber)) != 0U);
    uint8_t ok = 0U;

    switch (tag)
    {
        case PHASE_CONTROL_OBJECT_TAG_PHASE_OMIT:
        {
          ok = IntersectionEngineSetPhaseOmitControl(context->intersectionEngine,
                                                     phaseNumber,
                                                     active);
          break;
        }

        case PHASE_CONTROL_OBJECT_TAG_PED_OMIT:
        {
          ok = IntersectionEngineSetPedOmitControl(context->intersectionEngine,
                                                   phaseNumber,
                                                   active);
          break;
        }

        case PHASE_CONTROL_OBJECT_TAG_HOLD:
        {
          ok = IntersectionEngineSetPhaseHoldControl(context->intersectionEngine,
                                                     phaseNumber,
                                                     active);
          break;
        }

        case PHASE_CONTROL_OBJECT_TAG_FORCE_OFF:
        {
          ok = IntersectionEngineSetPhaseForceOffControl(
            context->intersectionEngine,
            phaseNumber,
            active);
          break;
        }

        case PHASE_CONTROL_OBJECT_TAG_VEH_CALL:
        {
          ok = IntersectionEngineSetVehCallControl(context->intersectionEngine,
                                                   phaseNumber,
                                                   active);
          break;
        }

        case PHASE_CONTROL_OBJECT_TAG_PED_CALL:
        {
          ok = IntersectionEngineSetPedCallControl(context->intersectionEngine,
                                                   phaseNumber,
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

static NtcipError_t GetPhaseControlObject(void *groupContext,
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

  error = GetGroupNumber(context, indexes, indexCount, &groupNumber);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if (descriptor->tag == PHASE_CONTROL_OBJECT_TAG_GROUP_NUMBER)
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

static NtcipError_t SetTestPhaseControlObject(
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

  error = GetGroupNumber(context, indexes, indexCount, &groupNumber);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if (descriptor->tag == PHASE_CONTROL_OBJECT_TAG_GROUP_NUMBER)
  {
    return NTCIP_ERROR_READ_ONLY;
  }

  return ValidateControlMask(context, groupNumber, value->data.unsigned32);
}

static NtcipError_t SetValuePhaseControlObject(
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

  error = SetTestPhaseControlObject(groupContext,
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

static const NtcipObjectDescriptor_t kPhaseControlObjects[] = {
  { kPhaseControlGroupNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_CONTROL_OBJECT_TAG_GROUP_NUMBER, GetPhaseControlObject, NULL, NULL },
  { kPhaseControlGroupPhaseOmitOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_CONTROL_OBJECT_TAG_PHASE_OMIT, GetPhaseControlObject,
    SetTestPhaseControlObject, SetValuePhaseControlObject },
  { kPhaseControlGroupPedOmitOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_CONTROL_OBJECT_TAG_PED_OMIT, GetPhaseControlObject,
    SetTestPhaseControlObject, SetValuePhaseControlObject },
  { kPhaseControlGroupHoldOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_CONTROL_OBJECT_TAG_HOLD, GetPhaseControlObject,
    SetTestPhaseControlObject, SetValuePhaseControlObject },
  { kPhaseControlGroupForceOffOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_CONTROL_OBJECT_TAG_FORCE_OFF, GetPhaseControlObject,
    SetTestPhaseControlObject, SetValuePhaseControlObject },
  { kPhaseControlGroupVehCallOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_CONTROL_OBJECT_TAG_VEH_CALL, GetPhaseControlObject,
    SetTestPhaseControlObject, SetValuePhaseControlObject },
  { kPhaseControlGroupPedCallOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_CONTROL_OBJECT_TAG_PED_CALL, GetPhaseControlObject,
    SetTestPhaseControlObject, SetValuePhaseControlObject }
};

void PhaseControlObjectsRegister(NtcipObjectDirectory_t *directory,
                                 NtcipContext_t *context)
{
  if ((directory == NULL) || (context == NULL))
  {
    return;
  }

  (void) NtcipObjectDirectoryRegisterGroup(directory,
                                           "phaseControl",
                                           kPhaseControlObjects,
                                           (uint16_t) (sizeof(kPhaseControlObjects)
                                                       / sizeof(kPhaseControlObjects[0])),
                                           context);
}
