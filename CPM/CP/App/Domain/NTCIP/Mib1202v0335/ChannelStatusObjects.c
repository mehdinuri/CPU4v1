/* App/Domain/NTCIP/Mib1202v0335/ChannelStatusObjects.c
 *
 * Runtime-backed channel status group projection.
 */
#include "ChannelStatusObjects.h"

#include <stddef.h>

enum
{
  CHANNEL_STATUS_OBJECT_TAG_MAX_GROUPS = 1,
  CHANNEL_STATUS_OBJECT_TAG_GROUP_NUMBER,
  CHANNEL_STATUS_OBJECT_TAG_REDS,
  CHANNEL_STATUS_OBJECT_TAG_YELLOWS,
  CHANNEL_STATUS_OBJECT_TAG_GREENS
};

static const uint32_t kMaxChannelStatusGroupsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                       1206U, 4U, 2U, 1U, 8U,
                                                       3U };
static const uint32_t kChannelStatusGroupNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                         1206U, 4U, 2U, 1U, 8U,
                                                         4U, 1U, 1U };
static const uint32_t kChannelStatusGroupRedsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                       1206U, 4U, 2U, 1U, 8U,
                                                       4U, 1U, 2U };
static const uint32_t kChannelStatusGroupYellowsOid[] = { 1U, 3U, 6U, 1U, 4U,
                                                          1U, 1206U, 4U, 2U, 1U,
                                                          8U, 4U, 1U, 3U };
static const uint32_t kChannelStatusGroupGreensOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                         1206U, 4U, 2U, 1U, 8U,
                                                         4U, 1U, 4U };

static NtcipError_t ReadChannelStatusGroup(NtcipContext_t *context,
                                           const uint32_t *indexes,
                                           uint8_t indexCount,
                                           IntersectionChannelStatusGroup_t *
                                           statusGroup)
{
  uint8_t maxGroups = (uint8_t) ((INTERSECTION_CHANNEL_COUNT_MAX + 7U) / 8U);

  if ((context == NULL) || (context->intersectionEngine == NULL)
      || (indexes == NULL) || (indexCount != 1U) || (statusGroup == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if ((indexes[0] == 0U) || (indexes[0] > maxGroups))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  if (IntersectionEngineGetChannelStatusGroup(context->intersectionEngine,
                                              (uint8_t) indexes[0],
                                              statusGroup) == 0U)
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetChannelStatusObject(void *groupContext,
                                           const NtcipObjectDescriptor_t *
                                           descriptor,
                                           const uint32_t *indexes,
                                           uint8_t indexCount,
                                           const NtcipRequestContext_t *
                                           requestContext,
                                           NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionChannelStatusGroup_t statusGroup;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag == CHANNEL_STATUS_OBJECT_TAG_MAX_GROUPS)
  {
    NtcipValueSetUnsigned32(value, (INTERSECTION_CHANNEL_COUNT_MAX + 7U) / 8U);

    return NTCIP_ERROR_OK;
  }

  error = ReadChannelStatusGroup(context, indexes, indexCount, &statusGroup);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case CHANNEL_STATUS_OBJECT_TAG_GROUP_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case CHANNEL_STATUS_OBJECT_TAG_REDS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.reds);

        return NTCIP_ERROR_OK;
      }

      case CHANNEL_STATUS_OBJECT_TAG_YELLOWS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.yellows);

        return NTCIP_ERROR_OK;
      }

      case CHANNEL_STATUS_OBJECT_TAG_GREENS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.greens);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
} /* GetChannelStatusObject */

static const NtcipObjectDescriptor_t kChannelStatusObjects[] =
{
  { kMaxChannelStatusGroupsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, CHANNEL_STATUS_OBJECT_TAG_MAX_GROUPS,
    GetChannelStatusObject, NULL, NULL },
  { kChannelStatusGroupNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CHANNEL_STATUS_OBJECT_TAG_GROUP_NUMBER, GetChannelStatusObject, NULL,
    NULL },
  { kChannelStatusGroupRedsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CHANNEL_STATUS_OBJECT_TAG_REDS, GetChannelStatusObject, NULL, NULL },
  { kChannelStatusGroupYellowsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CHANNEL_STATUS_OBJECT_TAG_YELLOWS, GetChannelStatusObject, NULL, NULL },
  { kChannelStatusGroupGreensOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CHANNEL_STATUS_OBJECT_TAG_GREENS, GetChannelStatusObject, NULL, NULL }
};

void ChannelStatusObjectsRegister(NtcipObjectDirectory_t *directory,
                                  NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.channelStatus",
    kChannelStatusObjects,
    (uint16_t) (sizeof(kChannelStatusObjects)
                / sizeof(kChannelStatusObjects[0])),
    context);
}
