/* App/Domain/NTCIP/Mib1202v0335/RingObjects.c
 *
 * Small 1202 ring subtree projection used by controller-core objects.
 */
#include "RingObjects.h"

#include <stddef.h>

enum
{
  RING_OBJECT_TAG_MAX_RINGS = 1
};

static const uint32_t kMaxRingsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 1U
};

static NtcipError_t GetRingObject(void *groupContext,
                                  const NtcipObjectDescriptor_t *descriptor,
                                  const uint32_t *indexes,
                                  uint8_t indexCount,
                                  const NtcipRequestContext_t *requestContext,
                                  NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;

  (void) indexes;
  (void) indexCount;
  (void) requestContext;

  if ((context == NULL) || (context->configurationService == NULL)
      || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case RING_OBJECT_TAG_MAX_RINGS:
      {
        NtcipValueSetUnsigned32(value,
                                ConfigurationServiceGetRingCount(
                                  context->configurationService));

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static const NtcipObjectDescriptor_t kRingObjects[] =
{
  { kMaxRingsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, RING_OBJECT_TAG_MAX_RINGS,
    GetRingObject, NULL, NULL }
};

void RingObjectsRegister(NtcipObjectDirectory_t *directory,
                         NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.ring",
    kRingObjects,
    (uint16_t) (sizeof(kRingObjects) / sizeof(kRingObjects[0])),
    context);
}
