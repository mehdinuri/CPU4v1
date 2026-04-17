/* App/Domain/NTCIP/Mib1201v0315NewAuxIO/AuxIoV2Objects.c
 *
 * Reserved auxiliary I/O subtree. The architecture is present now even
 * though no auxiliary ports are exposed yet.
 */
#include "AuxIoV2Objects.h"

#include <stddef.h>

enum
{
  AUX_IO_V2_TAG_MAX_DIGITAL_PORTS = 1,
  AUX_IO_V2_TAG_MAX_ANALOG_PORTS,
  AUX_IO_V2_TAG_PORT_TYPE,
  AUX_IO_V2_TAG_PORT_NUMBER,
  AUX_IO_V2_TAG_PORT_DESCRIPTION,
  AUX_IO_V2_TAG_PORT_RESOLUTION,
  AUX_IO_V2_TAG_PORT_VALUE,
  AUX_IO_V2_TAG_PORT_DIRECTION,
  AUX_IO_V2_TAG_PORT_LAST_COMMANDED_STATE
};

static const uint32_t kMaxDigitalPortsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 7U, 1U
};
static const uint32_t kMaxAnalogPortsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 7U, 2U
};
static const uint32_t kPortTypeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 7U, 3U, 1U, 1U
};
static const uint32_t kPortNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 7U, 3U, 1U, 2U
};
static const uint32_t kPortDescriptionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 7U, 3U, 1U, 3U
};
static const uint32_t kPortResolutionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 7U, 3U, 1U, 4U
};
static const uint32_t kPortValueOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 7U, 3U, 1U, 5U
};
static const uint32_t kPortDirectionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 7U, 3U, 1U, 6U
};
static const uint32_t kPortLastCommandedStateOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 7U, 3U, 1U, 7U
};

static NtcipError_t GetAuxIoV2Object(void *groupContext,
                                     const NtcipObjectDescriptor_t *descriptor,
                                     const uint32_t *indexes,
                                     uint8_t indexCount,
                                     const NtcipRequestContext_t *requestContext,
                                     NtcipValue_t *value)
{
  (void) groupContext;
  (void) requestContext;

  if ((descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case AUX_IO_V2_TAG_MAX_DIGITAL_PORTS:
      case AUX_IO_V2_TAG_MAX_ANALOG_PORTS:
      {
        NtcipValueSetUnsigned32(value, 0U);

        return NTCIP_ERROR_OK;
      }

      case AUX_IO_V2_TAG_PORT_TYPE:
      case AUX_IO_V2_TAG_PORT_NUMBER:
      case AUX_IO_V2_TAG_PORT_DESCRIPTION:
      case AUX_IO_V2_TAG_PORT_RESOLUTION:
      case AUX_IO_V2_TAG_PORT_VALUE:
      case AUX_IO_V2_TAG_PORT_DIRECTION:
      case AUX_IO_V2_TAG_PORT_LAST_COMMANDED_STATE:
      {
        if ((indexCount != 2U) || (indexes[0] == 0U) || (indexes[1] == 0U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return NTCIP_ERROR_NOT_FOUND;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
} /* GetAuxIoV2Object */

static const NtcipObjectDescriptor_t kAuxIoV2Objects[] =
{
  { kMaxDigitalPortsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, AUX_IO_V2_TAG_MAX_DIGITAL_PORTS,
    GetAuxIoV2Object, NULL, NULL },
  { kMaxAnalogPortsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, AUX_IO_V2_TAG_MAX_ANALOG_PORTS,
    GetAuxIoV2Object, NULL, NULL },
  { kPortTypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, AUX_IO_V2_TAG_PORT_TYPE,
    GetAuxIoV2Object, NULL, NULL },
  { kPortNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, AUX_IO_V2_TAG_PORT_NUMBER,
    GetAuxIoV2Object, NULL, NULL },
  { kPortDescriptionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, AUX_IO_V2_TAG_PORT_DESCRIPTION,
    GetAuxIoV2Object, NULL, NULL },
  { kPortResolutionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, AUX_IO_V2_TAG_PORT_RESOLUTION,
    GetAuxIoV2Object, NULL, NULL },
  { kPortValueOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, AUX_IO_V2_TAG_PORT_VALUE,
    GetAuxIoV2Object, NULL, NULL },
  { kPortDirectionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, AUX_IO_V2_TAG_PORT_DIRECTION,
    GetAuxIoV2Object, NULL, NULL },
  { kPortLastCommandedStateOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    AUX_IO_V2_TAG_PORT_LAST_COMMANDED_STATE,
    GetAuxIoV2Object, NULL, NULL }
};

void AuxIoV2ObjectsRegister(NtcipObjectDirectory_t *directory,
                            NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1201.auxIoV2",
    kAuxIoV2Objects,
    (uint16_t) (sizeof(kAuxIoV2Objects) / sizeof(kAuxIoV2Objects[0])),
    context);
}
