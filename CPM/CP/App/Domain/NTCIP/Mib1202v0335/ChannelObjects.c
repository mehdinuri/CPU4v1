/* App/Domain/NTCIP/Mib1202v0335/ChannelObjects.c
 *
 * Controller-core channel subtree projection.
 */
#include "ChannelObjects.h"

#include <stddef.h>

enum
{
  CHANNEL_OBJECT_TAG_MAX_CHANNELS = 1,
  CHANNEL_OBJECT_TAG_NUMBER,
  CHANNEL_OBJECT_TAG_CONTROL_SOURCE,
  CHANNEL_OBJECT_TAG_CONTROL_TYPE,
  CHANNEL_OBJECT_TAG_FLASH,
  CHANNEL_OBJECT_TAG_DIM,
  CHANNEL_OBJECT_TAG_GREEN_TYPE,
  CHANNEL_OBJECT_TAG_GREEN_INCLUDED,
  CHANNEL_OBJECT_TAG_INTERSECTION_ID
};

static const uint32_t kMaxChannelsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                            2U, 1U, 8U, 1U };
static const uint32_t kChannelNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                              2U, 1U, 8U, 2U, 1U, 1U };
static const uint32_t kChannelControlSourceOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                     1206U, 4U, 2U, 1U, 8U, 2U,
                                                     1U, 2U };
static const uint32_t kChannelControlTypeOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                   1206U, 4U, 2U, 1U, 8U, 2U,
                                                   1U, 3U };
static const uint32_t kChannelFlashOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                             2U, 1U, 8U, 2U, 1U, 4U };
static const uint32_t kChannelDimOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                           2U, 1U, 8U, 2U, 1U, 5U };
static const uint32_t kChannelGreenTypeOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                 4U, 2U, 1U, 8U, 2U, 1U, 6U };
static const uint32_t kChannelGreenIncludedOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                     1206U, 4U, 2U, 1U, 8U, 2U,
                                                     1U, 7U };
static const uint32_t kChannelIntersectionIdOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                      1206U, 4U, 2U, 1U, 8U, 2U,
                                                      1U, 8U };

static NtcipError_t GetChannelFromIndex(const NtcipContext_t *context,
                                        const uint32_t *indexes,
                                        uint8_t indexCount,
                                        uint8_t *channelIndex,
                                        IntersectionChannelConfig_t *channel)
{
  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (channelIndex == NULL) || (channel == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (indexes[0]
      > ConfigurationServiceGetChannelCount(context->configurationService))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *channelIndex = (uint8_t) (indexes[0] - 1U);

  if (ConfigurationServiceGetActiveChannelConfig(context->configurationService,
                                                 *channelIndex,
                                                 channel) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t ValidateChannelWriteRequest(const NtcipContext_t *context,
                                                const uint32_t *indexes,
                                                uint8_t indexCount,
                                                const NtcipRequestContext_t *
                                                requestContext,
                                                IntersectionChannelConfig_t *
                                                channel,
                                                uint8_t *channelIndex)
{
  NtcipError_t error;

  if ((context == NULL) || (context->dbTransactionService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  error = NtcipDbTransactionServiceValidateDatabaseWrite(
    context->dbTransactionService,
    requestContext);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  return GetChannelFromIndex(context, indexes, indexCount, channelIndex,
                             channel);
}

static NtcipError_t GetChannelObject(void *groupContext,
                                     const NtcipObjectDescriptor_t *descriptor,
                                     const uint32_t *indexes,
                                     uint8_t indexCount,
                                     const NtcipRequestContext_t *requestContext,
                                     NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionChannelConfig_t channel;
  uint8_t channelIndex;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag == CHANNEL_OBJECT_TAG_MAX_CHANNELS)
  {
    NtcipValueSetUnsigned32(value,
                            ConfigurationServiceGetChannelCount(
                              context->configurationService));

    return NTCIP_ERROR_OK;
  }

  error = GetChannelFromIndex(context,
                              indexes,
                              indexCount,
                              &channelIndex,
                              &channel);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case CHANNEL_OBJECT_TAG_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case CHANNEL_OBJECT_TAG_CONTROL_SOURCE:
      {
        NtcipValueSetUnsigned32(value, channel.controlSource);

        return NTCIP_ERROR_OK;
      }

      case CHANNEL_OBJECT_TAG_CONTROL_TYPE:
      {
        NtcipValueSetUnsigned32(value, channel.controlType);

        return NTCIP_ERROR_OK;
      }

      case CHANNEL_OBJECT_TAG_FLASH:
      {
        NtcipValueSetUnsigned32(value, channel.flashMask);

        return NTCIP_ERROR_OK;
      }

      case CHANNEL_OBJECT_TAG_DIM:
      {
        NtcipValueSetUnsigned32(value, channel.dimMask);

        return NTCIP_ERROR_OK;
      }

      case CHANNEL_OBJECT_TAG_GREEN_TYPE:
      {
        NtcipValueSetUnsigned32(value, channel.greenType);

        return NTCIP_ERROR_OK;
      }

      case CHANNEL_OBJECT_TAG_GREEN_INCLUDED:
      {
        return NtcipValueSetOctetString(value,
                                        channel.greenIncluded.values,
                                        channel.greenIncluded.length);
      }

      case CHANNEL_OBJECT_TAG_INTERSECTION_ID:
      {
        NtcipValueSetUnsigned32(value, channel.intersectionId);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  } /* switch */
} /* GetChannelObject */

static NtcipError_t SetTestChannelObject(void *groupContext,
                                         const NtcipObjectDescriptor_t *
                                         descriptor,
                                         const uint32_t *indexes,
                                         uint8_t indexCount,
                                         const NtcipRequestContext_t *
                                         requestContext,
                                         const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionChannelConfig_t channel;
  uint8_t channelIndex;
  NtcipError_t error;

  error = ValidateChannelWriteRequest(context,
                                      indexes,
                                      indexCount,
                                      requestContext,
                                      &channel,
                                      &channelIndex);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  (void) channel;
  (void) channelIndex;

  switch (descriptor->tag)
  {
      case CHANNEL_OBJECT_TAG_CONTROL_SOURCE:
      case CHANNEL_OBJECT_TAG_FLASH:
      case CHANNEL_OBJECT_TAG_DIM:
      {
        return (value->data.unsigned32 <= 255UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case CHANNEL_OBJECT_TAG_CONTROL_TYPE:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= 6UL))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case CHANNEL_OBJECT_TAG_GREEN_TYPE:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= 5UL))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case CHANNEL_OBJECT_TAG_GREEN_INCLUDED:
      {
        return (value->data.octetString.length
                <= INTERSECTION_CHANNEL_COUNT_MAX)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case CHANNEL_OBJECT_TAG_INTERSECTION_ID:
      {
        return (value->data.unsigned32 <= 65535UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  } /* switch */
} /* SetTestChannelObject */

static NtcipError_t SetValueChannelObject(void *groupContext,
                                          const NtcipObjectDescriptor_t *
                                          descriptor,
                                          const uint32_t *indexes,
                                          uint8_t indexCount,
                                          const NtcipRequestContext_t *
                                          requestContext,
                                          const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionChannelConfig_t channel;
  uint8_t channelIndex;
  NtcipError_t error;
  uint8_t ok;

  error = ValidateChannelWriteRequest(context,
                                      indexes,
                                      indexCount,
                                      requestContext,
                                      &channel,
                                      &channelIndex);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case CHANNEL_OBJECT_TAG_CONTROL_SOURCE:
      {
        ok = ConfigurationServiceSetChannelControlSource(
          context->configurationService,
          channelIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case CHANNEL_OBJECT_TAG_CONTROL_TYPE:
      {
        ok = ConfigurationServiceSetChannelControlType(
          context->configurationService,
          channelIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case CHANNEL_OBJECT_TAG_FLASH:
      {
        ok = ConfigurationServiceSetChannelFlashMask(
          context->configurationService,
          channelIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case CHANNEL_OBJECT_TAG_DIM:
      {
        ok = ConfigurationServiceSetChannelDimMask(
          context->configurationService,
          channelIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case CHANNEL_OBJECT_TAG_GREEN_TYPE:
      {
        ok = ConfigurationServiceSetChannelGreenType(
          context->configurationService,
          channelIndex,
          (uint8_t) value->data.unsigned32);
        break;
      }

      case CHANNEL_OBJECT_TAG_GREEN_INCLUDED:
      {
        ok = ConfigurationServiceSetChannelGreenIncluded(
          context->configurationService,
          channelIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case CHANNEL_OBJECT_TAG_INTERSECTION_ID:
      {
        ok = ConfigurationServiceSetChannelIntersectionId(
          context->configurationService,
          channelIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  } /* switch */

  return (ok != 0U) ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;
} /* SetValueChannelObject */

static const NtcipObjectDescriptor_t kChannelObjects[] =
{
  { kMaxChannelsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, CHANNEL_OBJECT_TAG_MAX_CHANNELS,
    GetChannelObject, NULL, NULL },
  { kChannelNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, CHANNEL_OBJECT_TAG_NUMBER,
    GetChannelObject, NULL, NULL },
  { kChannelControlSourceOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, CHANNEL_OBJECT_TAG_CONTROL_SOURCE,
    GetChannelObject, SetTestChannelObject, SetValueChannelObject },
  { kChannelControlTypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, CHANNEL_OBJECT_TAG_CONTROL_TYPE,
    GetChannelObject, SetTestChannelObject, SetValueChannelObject },
  { kChannelFlashOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, CHANNEL_OBJECT_TAG_FLASH,
    GetChannelObject, SetTestChannelObject, SetValueChannelObject },
  { kChannelDimOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, CHANNEL_OBJECT_TAG_DIM,
    GetChannelObject, SetTestChannelObject, SetValueChannelObject },
  { kChannelGreenTypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, CHANNEL_OBJECT_TAG_GREEN_TYPE,
    GetChannelObject, SetTestChannelObject, SetValueChannelObject },
  { kChannelGreenIncludedOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, CHANNEL_OBJECT_TAG_GREEN_INCLUDED,
    GetChannelObject, SetTestChannelObject, SetValueChannelObject },
  { kChannelIntersectionIdOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, CHANNEL_OBJECT_TAG_INTERSECTION_ID,
    GetChannelObject, SetTestChannelObject, SetValueChannelObject }
};

void ChannelObjectsRegister(NtcipObjectDirectory_t *directory,
                            NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.channel",
    kChannelObjects,
    (uint16_t) (sizeof(kChannelObjects) / sizeof(kChannelObjects[0])),
    context);
}
