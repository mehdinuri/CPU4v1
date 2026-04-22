/* App/Domain/NTCIP/MibVendor59748/ChannelFaultObjects.c
 *
 * Teknotel vendor 'channel' group. Publishes the per-channel fault-flag
 * table the MP mirrors through CpMpFaultStatusImage.channelFlags[].
 *
 *   channelFaultCount       .4.2.1.8.1         -- row count (scalar)
 *   channelFaultTable       .4.2.1.8.2
 *     channelFaultEntry     .4.2.1.8.2.1
 *       channelFaultNumber  .4.2.1.8.2.1.1     -- index, 1..32
 *       channelFaultFlags   .4.2.1.8.2.1.2     -- CpMpFaultChannelFlags_t
 */
#include "ChannelFaultObjects.h"

#include <stddef.h>

enum
{
  CHANNEL_FAULT_TAG_COUNT = 1,
  CHANNEL_FAULT_TAG_NUMBER,
  CHANNEL_FAULT_TAG_FLAGS
};

static const uint32_t kChannelFaultCountOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 8U, 1U
};
static const uint32_t kChannelFaultNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 8U, 2U, 1U, 1U
};
static const uint32_t kChannelFaultFlagsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 8U, 2U, 1U, 2U
};

static uint8_t GetChannelCount(const NtcipContext_t *context)
{
  if ((context == NULL) || (context->configurationService == NULL))
  {
    return 0U;
  }

  return ConfigurationServiceGetChannelCount(context->configurationService);
}

static uint8_t GetFaultStatus(const NtcipContext_t *context,
                              CpMpFaultStatusImageV1_t *faultStatus)
{
  if ((context == NULL) || (faultStatus == NULL)
      || (context->cpMpLinkService == NULL))
  {
    return 0U;
  }

  return CpMpLinkServiceGetFaultStatus(context->cpMpLinkService, faultStatus);
}

static NtcipError_t GetChannelIndex(const NtcipContext_t *context,
                                    const uint32_t *indexes,
                                    uint8_t indexCount,
                                    uint8_t *channelIndex)
{
  uint8_t channelCount;

  if ((indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (channelIndex == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  channelCount = GetChannelCount(context);
  if ((channelCount == 0U) || (indexes[0] > channelCount))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *channelIndex = (uint8_t) (indexes[0] - 1U);

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetChannelFaultObject(void *groupContext,
                                          const NtcipObjectDescriptor_t *
                                          descriptor,
                                          const uint32_t *indexes,
                                          uint8_t indexCount,
                                          const NtcipRequestContext_t *
                                          requestContext,
                                          NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  CpMpFaultStatusImageV1_t faultStatus;
  uint8_t channelIndex = 0U;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case CHANNEL_FAULT_TAG_COUNT:
      {
        NtcipValueSetUnsigned32(value, GetChannelCount(context));

        return NTCIP_ERROR_OK;
      }

      case CHANNEL_FAULT_TAG_NUMBER:
      {
        NtcipError_t error =
          GetChannelIndex(context, indexes, indexCount, &channelIndex);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        NtcipValueSetUnsigned32(value, (uint32_t) (channelIndex + 1U));

        return NTCIP_ERROR_OK;
      }

      case CHANNEL_FAULT_TAG_FLAGS:
      {
        NtcipError_t error =
          GetChannelIndex(context, indexes, indexCount, &channelIndex);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        if (GetFaultStatus(context, &faultStatus) == 0U)
        {
          NtcipValueSetUnsigned32(value, 0U);
        }
        else
        {
          NtcipValueSetUnsigned32(value,
                                  (uint32_t) faultStatus.channelFlags[
                                    channelIndex]);
        }

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  } /* switch */
} /* GetChannelFaultObject */

static const NtcipObjectDescriptor_t kChannelFaultObjects[] = {
  { kChannelFaultCountOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CHANNEL_FAULT_TAG_COUNT, GetChannelFaultObject, NULL, NULL },
  { kChannelFaultNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CHANNEL_FAULT_TAG_NUMBER, GetChannelFaultObject, NULL, NULL },
  { kChannelFaultFlagsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CHANNEL_FAULT_TAG_FLAGS, GetChannelFaultObject, NULL, NULL }
};

void TeknotelChannelFaultObjectsRegister(NtcipObjectDirectory_t *directory,
                                         NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "59748.channel",
    kChannelFaultObjects,
    (uint16_t) (sizeof(kChannelFaultObjects)
                / sizeof(kChannelFaultObjects[0])),
    context);
}
