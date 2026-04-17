/* App/Domain/NTCIP/Mib1202v0335/OverlapStatusObjects.c
 *
 * Runtime-backed overlap status group projection.
 */
#include "OverlapStatusObjects.h"

#include <stddef.h>

enum
{
  OVERLAP_STATUS_OBJECT_TAG_MAX_GROUPS = 1,
  OVERLAP_STATUS_OBJECT_TAG_GROUP_NUMBER,
  OVERLAP_STATUS_OBJECT_TAG_REDS,
  OVERLAP_STATUS_OBJECT_TAG_YELLOWS,
  OVERLAP_STATUS_OBJECT_TAG_GREENS
};

static const uint32_t kMaxOverlapStatusGroupsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                       1206U, 4U, 2U, 1U, 9U,
                                                       3U };
static const uint32_t kOverlapStatusGroupNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                         1206U, 4U, 2U, 1U, 9U,
                                                         4U, 1U, 1U };
static const uint32_t kOverlapStatusGroupRedsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                       1206U, 4U, 2U, 1U, 9U,
                                                       4U, 1U, 2U };
static const uint32_t kOverlapStatusGroupYellowsOid[] = { 1U, 3U, 6U, 1U, 4U,
                                                          1U, 1206U, 4U, 2U, 1U,
                                                          9U, 4U, 1U, 3U };
static const uint32_t kOverlapStatusGroupGreensOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                         1206U, 4U, 2U, 1U, 9U,
                                                         4U, 1U, 4U };

static NtcipError_t ReadOverlapStatusGroup(NtcipContext_t *context,
                                           const uint32_t *indexes,
                                           uint8_t indexCount,
                                           IntersectionOverlapStatusGroup_t *
                                           statusGroup)
{
  uint8_t maxGroups = (uint8_t) ((INTERSECTION_OVERLAP_COUNT_MAX + 7U) / 8U);

  if ((context == NULL) || (context->intersectionEngine == NULL)
      || (indexes == NULL) || (indexCount != 1U) || (statusGroup == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if ((indexes[0] == 0U) || (indexes[0] > maxGroups))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  if (IntersectionEngineGetOverlapStatusGroup(context->intersectionEngine,
                                              (uint8_t) indexes[0],
                                              statusGroup) == 0U)
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetOverlapStatusObject(void *groupContext,
                                           const NtcipObjectDescriptor_t *
                                           descriptor,
                                           const uint32_t *indexes,
                                           uint8_t indexCount,
                                           const NtcipRequestContext_t *
                                           requestContext,
                                           NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionOverlapStatusGroup_t statusGroup;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag == OVERLAP_STATUS_OBJECT_TAG_MAX_GROUPS)
  {
    NtcipValueSetUnsigned32(value, (INTERSECTION_OVERLAP_COUNT_MAX + 7U) / 8U);

    return NTCIP_ERROR_OK;
  }

  error = ReadOverlapStatusGroup(context, indexes, indexCount, &statusGroup);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case OVERLAP_STATUS_OBJECT_TAG_GROUP_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case OVERLAP_STATUS_OBJECT_TAG_REDS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.reds);

        return NTCIP_ERROR_OK;
      }

      case OVERLAP_STATUS_OBJECT_TAG_YELLOWS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.yellows);

        return NTCIP_ERROR_OK;
      }

      case OVERLAP_STATUS_OBJECT_TAG_GREENS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.greens);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
} /* GetOverlapStatusObject */

static const NtcipObjectDescriptor_t kOverlapStatusObjects[] =
{
  { kMaxOverlapStatusGroupsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, OVERLAP_STATUS_OBJECT_TAG_MAX_GROUPS,
    GetOverlapStatusObject, NULL, NULL },
  { kOverlapStatusGroupNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    OVERLAP_STATUS_OBJECT_TAG_GROUP_NUMBER, GetOverlapStatusObject, NULL,
    NULL },
  { kOverlapStatusGroupRedsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    OVERLAP_STATUS_OBJECT_TAG_REDS, GetOverlapStatusObject, NULL, NULL },
  { kOverlapStatusGroupYellowsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    OVERLAP_STATUS_OBJECT_TAG_YELLOWS, GetOverlapStatusObject, NULL, NULL },
  { kOverlapStatusGroupGreensOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    OVERLAP_STATUS_OBJECT_TAG_GREENS, GetOverlapStatusObject, NULL, NULL }
};

void OverlapStatusObjectsRegister(NtcipObjectDirectory_t *directory,
                                  NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.overlapStatus",
    kOverlapStatusObjects,
    (uint16_t) (sizeof(kOverlapStatusObjects)
                / sizeof(kOverlapStatusObjects[0])),
    context);
}
