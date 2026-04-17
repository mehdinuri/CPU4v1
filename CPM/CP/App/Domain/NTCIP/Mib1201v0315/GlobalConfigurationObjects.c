/* App/Domain/NTCIP/Mib1201v0315/GlobalConfigurationObjects.c
 *
 * Global 1201 configuration scalars and a minimal module table projection.
 */
#include "GlobalConfigurationObjects.h"

#include <stddef.h>

enum
{
  GLOBAL_CONFIGURATION_TAG_SET_ID = 1,
  GLOBAL_CONFIGURATION_TAG_MAX_MODULES,
  GLOBAL_CONFIGURATION_TAG_MODULE_NUMBER,
  GLOBAL_CONFIGURATION_TAG_MODULE_DEVICE_NODE,
  GLOBAL_CONFIGURATION_TAG_MODULE_MAKE,
  GLOBAL_CONFIGURATION_TAG_MODULE_MODEL,
  GLOBAL_CONFIGURATION_TAG_MODULE_VERSION,
  GLOBAL_CONFIGURATION_TAG_MODULE_TYPE,
  GLOBAL_CONFIGURATION_TAG_CONTROLLER_BASE_STANDARDS
};

static const uint32_t kGlobalSetIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 1U, 1U
};
static const uint32_t kGlobalMaxModulesOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 1U, 2U
};
static const uint32_t kModuleNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 1U, 3U, 1U, 1U
};
static const uint32_t kModuleDeviceNodeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 1U, 3U, 1U, 2U
};
static const uint32_t kModuleMakeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 1U, 3U, 1U, 3U
};
static const uint32_t kModuleModelOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 1U, 3U, 1U, 4U
};
static const uint32_t kModuleVersionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 1U, 3U, 1U, 5U
};
static const uint32_t kModuleTypeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 1U, 3U, 1U, 6U
};
static const uint32_t kControllerBaseStandardsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 1U, 4U
};
static const uint32_t kAscDeviceNodeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U
};

static NtcipError_t GetGlobalConfigurationObject(void *groupContext,
                                                 const NtcipObjectDescriptor_t *
                                                 descriptor,
                                                 const uint32_t *indexes,
                                                 uint8_t indexCount,
                                                 const NtcipRequestContext_t *
                                                 requestContext,
                                                 NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;

  (void) indexes;
  (void) indexCount;
  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case GLOBAL_CONFIGURATION_TAG_SET_ID:
      {
        NtcipValueSetUnsigned32(value,
                                ConfigurationServiceGetActiveSetId(
                                  context->configurationService));

        return NTCIP_ERROR_OK;
      }

      case GLOBAL_CONFIGURATION_TAG_MAX_MODULES:
      {
        NtcipValueSetUnsigned32(value, 1U);

        return NTCIP_ERROR_OK;
      }

      case GLOBAL_CONFIGURATION_TAG_MODULE_NUMBER:
      {
        if ((indexCount != 1U) || (indexes[0] != 1U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        NtcipValueSetUnsigned32(value, 1U);

        return NTCIP_ERROR_OK;
      }

      case GLOBAL_CONFIGURATION_TAG_MODULE_DEVICE_NODE:
      {
        if ((indexCount != 1U) || (indexes[0] != 1U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return NtcipValueSetObjectId(value,
                                     kAscDeviceNodeOid,
                                     (uint8_t) (sizeof(kAscDeviceNodeOid)
                                                / sizeof(uint32_t)));
      }

      case GLOBAL_CONFIGURATION_TAG_MODULE_MAKE:
      {
        if ((indexCount != 1U) || (indexes[0] != 1U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return NtcipValueSetCString(value, "Teknotel");
      }

      case GLOBAL_CONFIGURATION_TAG_MODULE_MODEL:
      {
        if ((indexCount != 1U) || (indexes[0] != 1U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return NtcipValueSetCString(value, "C0502 CPU4");
      }

      case GLOBAL_CONFIGURATION_TAG_MODULE_VERSION:
      {
        if ((indexCount != 1U) || (indexes[0] != 1U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return NtcipValueSetCString(value, "20260416 - v0.1");
      }

      case GLOBAL_CONFIGURATION_TAG_MODULE_TYPE:
      {
        if ((indexCount != 1U) || (indexes[0] != 1U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        NtcipValueSetUnsigned32(value, 3U);

        return NTCIP_ERROR_OK;
      }

      case GLOBAL_CONFIGURATION_TAG_CONTROLLER_BASE_STANDARDS:
      {
        return NtcipValueSetCString(value,
                                    "NTCIP 1201:v03.15r\r\n"
                                    "NTCIP 1201 NewAuxIO:v03.15r\r\n"
                                    "NTCIP 1202:v03.35e");
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  } /* switch */
} /* GetGlobalConfigurationObject */

static const NtcipObjectDescriptor_t kGlobalConfigurationObjects[] =
{
  { kGlobalSetIdOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, GLOBAL_CONFIGURATION_TAG_SET_ID,
    GetGlobalConfigurationObject, NULL, NULL },
  { kGlobalMaxModulesOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, GLOBAL_CONFIGURATION_TAG_MAX_MODULES,
    GetGlobalConfigurationObject, NULL, NULL },
  { kModuleNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, GLOBAL_CONFIGURATION_TAG_MODULE_NUMBER,
    GetGlobalConfigurationObject, NULL, NULL },
  { kModuleDeviceNodeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_OBJECT_ID, GLOBAL_CONFIGURATION_TAG_MODULE_DEVICE_NODE,
    GetGlobalConfigurationObject, NULL, NULL },
  { kModuleMakeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_OCTET_STRING, GLOBAL_CONFIGURATION_TAG_MODULE_MAKE,
    GetGlobalConfigurationObject, NULL, NULL },
  { kModuleModelOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_OCTET_STRING, GLOBAL_CONFIGURATION_TAG_MODULE_MODEL,
    GetGlobalConfigurationObject, NULL, NULL },
  { kModuleVersionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_OCTET_STRING, GLOBAL_CONFIGURATION_TAG_MODULE_VERSION,
    GetGlobalConfigurationObject, NULL, NULL },
  { kModuleTypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, GLOBAL_CONFIGURATION_TAG_MODULE_TYPE,
    GetGlobalConfigurationObject, NULL, NULL },
  { kControllerBaseStandardsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OCTET_STRING,
    GLOBAL_CONFIGURATION_TAG_CONTROLLER_BASE_STANDARDS,
    GetGlobalConfigurationObject, NULL, NULL }
};

void GlobalConfigurationObjectsRegister(NtcipObjectDirectory_t *directory,
                                        NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1201.globalConfiguration",
    kGlobalConfigurationObjects,
    (uint16_t) (sizeof(kGlobalConfigurationObjects)
                / sizeof(kGlobalConfigurationObjects[0])),
    context);
}
