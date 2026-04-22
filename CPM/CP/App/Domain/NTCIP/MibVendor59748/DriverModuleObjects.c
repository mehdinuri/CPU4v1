/* App/Domain/NTCIP/MibVendor59748/DriverModuleObjects.c
 *
 * Teknotel vendor 'driverModule' group. The single object below is the
 * sole varbind of the teknotelDriverModuleMissingTrap. The trap is
 * emitted asynchronously by LWIP/App/snmp_client.c; a GET on this object
 * returns the last published SSM status word (or 0 when none is known).
 *
 *   driverModuleStatus  .4.2.1.21.1
 */
#include "DriverModuleObjects.h"

#include <stddef.h>

enum
{
  DRIVER_MODULE_TAG_STATUS = 1
};

static const uint32_t kDriverModuleStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 21U, 1U
};

static NtcipError_t GetDriverModuleObject(void *groupContext,
                                          const NtcipObjectDescriptor_t *
                                          descriptor,
                                          const uint32_t *indexes,
                                          uint8_t indexCount,
                                          const NtcipRequestContext_t *
                                          requestContext,
                                          NtcipValue_t *value)
{
  (void) groupContext;
  (void) indexes;
  (void) indexCount;
  (void) requestContext;

  if ((descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case DRIVER_MODULE_TAG_STATUS:
      {
        NtcipValueSetUnsigned32(value, 0U);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static const NtcipObjectDescriptor_t kDriverModuleObjects[] = {
  { kDriverModuleStatusOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DRIVER_MODULE_TAG_STATUS, GetDriverModuleObject, NULL, NULL }
};

void TeknotelDriverModuleObjectsRegister(NtcipObjectDirectory_t *directory,
                                         NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "59748.driverModule",
    kDriverModuleObjects,
    (uint16_t) (sizeof(kDriverModuleObjects)
                / sizeof(kDriverModuleObjects[0])),
    context);
}
