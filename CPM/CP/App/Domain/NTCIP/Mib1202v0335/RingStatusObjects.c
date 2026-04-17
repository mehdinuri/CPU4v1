/* App/Domain/NTCIP/Mib1202v0335/RingStatusObjects.c
 *
 * Runtime-backed ring status table projection.
 */
#include "RingStatusObjects.h"

#include <stddef.h>

enum
{
  RING_STATUS_OBJECT_TAG_RING_STATUS = 1
};

static const uint32_t kRingStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 6U, 1U, 1U
};

static NtcipError_t GetRingStatusObject(void *groupContext,
                                        const NtcipObjectDescriptor_t *
                                        descriptor,
                                        const uint32_t *indexes,
                                        uint8_t indexCount,
                                        const NtcipRequestContext_t *
                                        requestContext,
                                        NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  uint8_t ringStatus;

  (void) requestContext;

  if ((context == NULL) || (context->intersectionEngine == NULL)
      || (descriptor == NULL) || (value == NULL)
      || (indexes == NULL) || (indexCount != 1U))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag != RING_STATUS_OBJECT_TAG_RING_STATUS)
  {
    return NTCIP_ERROR_NOT_FOUND;
  }

  if ((indexes[0] == 0U)
      || (indexes[0]
          > ConfigurationServiceGetRingCount(context->configurationService)))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  if (IntersectionEngineGetRingStatus(context->intersectionEngine,
                                      (uint8_t) indexes[0],
                                      &ringStatus) == 0U)
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  NtcipValueSetUnsigned32(value, ringStatus);

  return NTCIP_ERROR_OK;
}

static const NtcipObjectDescriptor_t kRingStatusObjects[] =
{
  { kRingStatusOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, RING_STATUS_OBJECT_TAG_RING_STATUS,
    GetRingStatusObject, NULL, NULL }
};

void RingStatusObjectsRegister(NtcipObjectDirectory_t *directory,
                               NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.ringStatus",
    kRingStatusObjects,
    (uint16_t) (sizeof(kRingStatusObjects) / sizeof(kRingStatusObjects[0])),
    context);
}
