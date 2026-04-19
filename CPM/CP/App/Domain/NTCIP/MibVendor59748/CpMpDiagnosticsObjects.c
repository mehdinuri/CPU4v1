/* App/Domain/NTCIP/MibVendor59748/CpMpDiagnosticsObjects.c
 *
 * Vendor-private diagnostics for the clean CP<->MP safety link.
 */
#include "CpMpDiagnosticsObjects.h"

#include <stddef.h>

enum
{
  CPMP_DIAG_TAG_PROTOCOL_VERSION = 1,
  CPMP_DIAG_TAG_PEER_HEALTHY,
  CPMP_DIAG_TAG_AUTHORITY_READY,
  CPMP_DIAG_TAG_CONFIG_STATE,
  CPMP_DIAG_TAG_SAFETY_ACTION,
  CPMP_DIAG_TAG_SAFETY_REASON_CODE,
  CPMP_DIAG_TAG_STATUS_SEQUENCE,
  CPMP_DIAG_TAG_GLOBAL_FLAGS,
  CPMP_DIAG_TAG_CHANNEL_COUNT,
  CPMP_DIAG_TAG_CHANNEL_NUMBER,
  CPMP_DIAG_TAG_CHANNEL_FLAGS
};

static const uint32_t kCpMpDiagProtocolVersionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 1U
};
static const uint32_t kCpMpDiagPeerHealthyOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 2U
};
static const uint32_t kCpMpDiagAuthorityReadyOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 3U
};
static const uint32_t kCpMpDiagConfigStateOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 4U
};
static const uint32_t kCpMpDiagSafetyActionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 5U
};
static const uint32_t kCpMpDiagSafetyReasonCodeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 6U
};
static const uint32_t kCpMpDiagStatusSequenceOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 7U
};
static const uint32_t kCpMpDiagGlobalFlagsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 8U
};
static const uint32_t kCpMpDiagChannelCountOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 9U
};
static const uint32_t kCpMpDiagChannelNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 10U, 1U, 1U
};
static const uint32_t kCpMpDiagChannelFlagsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 1U, 10U, 1U, 2U
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

static NtcipError_t GetCpMpDiagnosticsObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
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
      case CPMP_DIAG_TAG_PROTOCOL_VERSION:
      {
        NtcipValueSetUnsigned32(value, CPMP_PROTOCOL_VERSION);
        return NTCIP_ERROR_OK;
      }

      case CPMP_DIAG_TAG_PEER_HEALTHY:
      {
        NtcipValueSetUnsigned32(
          value,
          (context->cpMpLinkService != NULL)
          ? CpMpLinkServicePeerHealthy(context->cpMpLinkService)
          : 0U);
        return NTCIP_ERROR_OK;
      }

      case CPMP_DIAG_TAG_AUTHORITY_READY:
      {
        NtcipValueSetUnsigned32(
          value,
          (context->cpMpLinkService != NULL)
          ? CpMpLinkServiceAuthorityReady(context->cpMpLinkService)
          : 0U);
        return NTCIP_ERROR_OK;
      }

      case CPMP_DIAG_TAG_CONFIG_STATE:
      {
        NtcipValueSetUnsigned32(
          value,
          (context->cpMpLinkService != NULL)
          ? (uint32_t) context->cpMpLinkService->lastMpConfigState
          : (uint32_t) CPMP_CONFIG_STATE_EMPTY);
        return NTCIP_ERROR_OK;
      }

      case CPMP_DIAG_TAG_SAFETY_ACTION:
      {
        NtcipValueSetUnsigned32(
          value,
          (context->cpMpLinkService != NULL)
          ? (uint32_t) CpMpLinkServiceGetEffectiveSafetyAction(
              context->cpMpLinkService)
          : (uint32_t) CPMP_SAFETY_ACTION_FLASH);
        return NTCIP_ERROR_OK;
      }

      case CPMP_DIAG_TAG_SAFETY_REASON_CODE:
      {
        NtcipValueSetUnsigned32(
          value,
          (context->cpMpLinkService != NULL)
          ? CpMpLinkServiceGetLastSafetyReasonCode(context->cpMpLinkService)
          : 0U);
        return NTCIP_ERROR_OK;
      }

      case CPMP_DIAG_TAG_STATUS_SEQUENCE:
      {
        if (GetFaultStatus(context, &faultStatus) == 0U)
        {
          NtcipValueSetUnsigned32(value, 0U);
        }
        else
        {
          NtcipValueSetUnsigned32(value, faultStatus.sequence);
        }
        return NTCIP_ERROR_OK;
      }

      case CPMP_DIAG_TAG_GLOBAL_FLAGS:
      {
        if (GetFaultStatus(context, &faultStatus) == 0U)
        {
          NtcipValueSetUnsigned32(value, 0U);
        }
        else
        {
          NtcipValueSetUnsigned32(value, faultStatus.globalFlags);
        }
        return NTCIP_ERROR_OK;
      }

      case CPMP_DIAG_TAG_CHANNEL_COUNT:
      {
        NtcipValueSetUnsigned32(value, GetChannelCount(context));
        return NTCIP_ERROR_OK;
      }

      case CPMP_DIAG_TAG_CHANNEL_NUMBER:
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

      case CPMP_DIAG_TAG_CHANNEL_FLAGS:
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
                                  (uint32_t) faultStatus.channelFlags[channelIndex]);
        }
        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static const NtcipObjectDescriptor_t kCpMpDiagnosticsObjects[] = {
  { kCpMpDiagProtocolVersionOid, 9U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CPMP_DIAG_TAG_PROTOCOL_VERSION, GetCpMpDiagnosticsObject, NULL, NULL },
  { kCpMpDiagPeerHealthyOid, 9U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CPMP_DIAG_TAG_PEER_HEALTHY, GetCpMpDiagnosticsObject, NULL, NULL },
  { kCpMpDiagAuthorityReadyOid, 9U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CPMP_DIAG_TAG_AUTHORITY_READY, GetCpMpDiagnosticsObject, NULL, NULL },
  { kCpMpDiagConfigStateOid, 9U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CPMP_DIAG_TAG_CONFIG_STATE, GetCpMpDiagnosticsObject, NULL, NULL },
  { kCpMpDiagSafetyActionOid, 9U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CPMP_DIAG_TAG_SAFETY_ACTION, GetCpMpDiagnosticsObject, NULL, NULL },
  { kCpMpDiagSafetyReasonCodeOid, 9U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CPMP_DIAG_TAG_SAFETY_REASON_CODE, GetCpMpDiagnosticsObject, NULL, NULL },
  { kCpMpDiagStatusSequenceOid, 9U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CPMP_DIAG_TAG_STATUS_SEQUENCE, GetCpMpDiagnosticsObject, NULL, NULL },
  { kCpMpDiagGlobalFlagsOid, 9U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CPMP_DIAG_TAG_GLOBAL_FLAGS, GetCpMpDiagnosticsObject, NULL, NULL },
  { kCpMpDiagChannelCountOid, 9U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CPMP_DIAG_TAG_CHANNEL_COUNT, GetCpMpDiagnosticsObject, NULL, NULL },
  { kCpMpDiagChannelNumberOid, 11U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CPMP_DIAG_TAG_CHANNEL_NUMBER, GetCpMpDiagnosticsObject, NULL, NULL },
  { kCpMpDiagChannelFlagsOid, 11U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CPMP_DIAG_TAG_CHANNEL_FLAGS, GetCpMpDiagnosticsObject, NULL, NULL }
};

void CpMpDiagnosticsObjectsRegister(NtcipObjectDirectory_t *directory,
                                    NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "59748.cpMpDiagnostics",
    kCpMpDiagnosticsObjects,
    (uint16_t) (sizeof(kCpMpDiagnosticsObjects)
                / sizeof(kCpMpDiagnosticsObjects[0])),
    context);
}
