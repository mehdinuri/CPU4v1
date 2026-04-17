/* App/Domain/NTCIP/Mib1202v0335/SpecialFunctionObjects.c
 *
 * 1202 unit special function output table backed by runtime controls and
 * status in the new controller engine.
 */
#include "SpecialFunctionObjects.h"

#include <stddef.h>

enum
{
  SPECIAL_FUNCTION_OBJECT_TAG_MAX_OUTPUTS = 1,
  SPECIAL_FUNCTION_OBJECT_TAG_NUMBER,
  SPECIAL_FUNCTION_OBJECT_TAG_STATE,
  SPECIAL_FUNCTION_OBJECT_TAG_CONTROL,
  SPECIAL_FUNCTION_OBJECT_TAG_STATUS
};

static const uint32_t kMaxSpecialFunctionOutputsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 13U
};
static const uint32_t kSpecialFunctionOutputNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 14U, 1U, 1U
};
static const uint32_t kSpecialFunctionOutputStateOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 14U, 1U, 2U
};
static const uint32_t kSpecialFunctionOutputControlOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 14U, 1U, 3U
};
static const uint32_t kSpecialFunctionOutputStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 14U, 1U, 4U
};

static NtcipError_t GetOutputIndex(const uint32_t *indexes,
                                   uint8_t indexCount,
                                   uint8_t *outputIndex)
{
  if ((indexes == NULL) || (indexCount != 1U) || (outputIndex == NULL)
      || (indexes[0] == 0U)
      || (indexes[0] > INTERSECTION_SPECIAL_FUNCTION_OUTPUT_COUNT))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *outputIndex = (uint8_t) (indexes[0] - 1U);

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetSpecialFunctionObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  uint8_t outputIndex = 0U;
  uint8_t active = 0U;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL)
      || (context->intersectionEngine == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case SPECIAL_FUNCTION_OBJECT_TAG_MAX_OUTPUTS:
      {
        NtcipValueSetUnsigned32(value,
                                INTERSECTION_SPECIAL_FUNCTION_OUTPUT_COUNT);

        return NTCIP_ERROR_OK;
      }

      case SPECIAL_FUNCTION_OBJECT_TAG_NUMBER:
      case SPECIAL_FUNCTION_OBJECT_TAG_STATE:
      case SPECIAL_FUNCTION_OBJECT_TAG_CONTROL:
      case SPECIAL_FUNCTION_OBJECT_TAG_STATUS:
      {
        error = GetOutputIndex(indexes, indexCount, &outputIndex);

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
      case SPECIAL_FUNCTION_OBJECT_TAG_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case SPECIAL_FUNCTION_OBJECT_TAG_CONTROL:
      {
        if (IntersectionEngineGetSpecialFunctionOutputControl(
              context->intersectionEngine,
              (uint8_t) (outputIndex + 1U),
              &active) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, active);

        return NTCIP_ERROR_OK;
      }

      case SPECIAL_FUNCTION_OBJECT_TAG_STATE:
      case SPECIAL_FUNCTION_OBJECT_TAG_STATUS:
      {
        if (IntersectionEngineGetSpecialFunctionOutputStatus(
              context->intersectionEngine,
              (uint8_t) (outputIndex + 1U),
              &active) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, active);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }

  return NTCIP_ERROR_NOT_FOUND;
}

static NtcipError_t SetTestSpecialFunctionObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  uint8_t outputIndex = 0U;

  (void) groupContext;
  (void) requestContext;

  if ((descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if ((descriptor->tag != SPECIAL_FUNCTION_OBJECT_TAG_STATE)
      && (descriptor->tag != SPECIAL_FUNCTION_OBJECT_TAG_CONTROL))
  {
    return NTCIP_ERROR_READ_ONLY;
  }

  if (GetOutputIndex(indexes, indexCount, &outputIndex) != NTCIP_ERROR_OK)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  if ((value->type != NTCIP_VALUE_TYPE_UNSIGNED32)
      || (value->data.unsigned32 > 1U))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  (void) outputIndex;

  return NTCIP_ERROR_OK;
}

static NtcipError_t SetValueSpecialFunctionObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  uint8_t outputIndex = 0U;
  NtcipError_t error;

  error = SetTestSpecialFunctionObject(groupContext,
                                       descriptor,
                                       indexes,
                                       indexCount,
                                       requestContext,
                                       value);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if ((context == NULL) || (context->intersectionEngine == NULL)
      || (GetOutputIndex(indexes, indexCount, &outputIndex) != NTCIP_ERROR_OK))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return (IntersectionEngineSetSpecialFunctionOutputControl(
            context->intersectionEngine,
            (uint8_t) (outputIndex + 1U),
            (uint8_t) value->data.unsigned32) != 0U)
           ? NTCIP_ERROR_OK
           : NTCIP_ERROR_GEN_ERROR;
}

static const NtcipObjectDescriptor_t kSpecialFunctionObjects[] = {
  { kMaxSpecialFunctionOutputsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    SPECIAL_FUNCTION_OBJECT_TAG_MAX_OUTPUTS, GetSpecialFunctionObject, NULL,
    NULL },
  { kSpecialFunctionOutputNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    SPECIAL_FUNCTION_OBJECT_TAG_NUMBER, GetSpecialFunctionObject, NULL, NULL },
  { kSpecialFunctionOutputStateOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    SPECIAL_FUNCTION_OBJECT_TAG_STATE, GetSpecialFunctionObject,
    SetTestSpecialFunctionObject, SetValueSpecialFunctionObject },
  { kSpecialFunctionOutputControlOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    SPECIAL_FUNCTION_OBJECT_TAG_CONTROL, GetSpecialFunctionObject,
    SetTestSpecialFunctionObject, SetValueSpecialFunctionObject },
  { kSpecialFunctionOutputStatusOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    SPECIAL_FUNCTION_OBJECT_TAG_STATUS, GetSpecialFunctionObject, NULL, NULL }
};

void SpecialFunctionObjectsRegister(NtcipObjectDirectory_t *directory,
                                    NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.unit.specialFunction",
    kSpecialFunctionObjects,
    (uint16_t) (sizeof(kSpecialFunctionObjects)
                / sizeof(kSpecialFunctionObjects[0])),
    context);
}
