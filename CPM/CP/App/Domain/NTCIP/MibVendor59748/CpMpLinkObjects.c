/* App/Domain/NTCIP/MibVendor59748/CpMpLinkObjects.c
 *
 * Teknotel vendor 'cpMpLink' group. All scalars are read-only projections of
 * CpMpLinkService state and the latest CpMpFaultStatusImage from the MP.
 *
 *   cpMpLinkProtocolVersion     .4.2.1.20.1
 *   cpMpLinkPeerHealthy         .4.2.1.20.2
 *   cpMpLinkAuthorityReady      .4.2.1.20.3
 *   cpMpLinkConfigState         .4.2.1.20.4
 *   cpMpLinkSafetyAction        .4.2.1.20.5
 *   cpMpLinkSafetyReasonCode    .4.2.1.20.6
 *   cpMpLinkStatusSequence      .4.2.1.20.7
 *   cpMpLinkGlobalFlags         .4.2.1.20.8
 */
#include "CpMpLinkObjects.h"

#include <stddef.h>

enum
{
  CP_MP_LINK_TAG_PROTOCOL_VERSION = 1,
  CP_MP_LINK_TAG_PEER_HEALTHY,
  CP_MP_LINK_TAG_AUTHORITY_READY,
  CP_MP_LINK_TAG_CONFIG_STATE,
  CP_MP_LINK_TAG_SAFETY_ACTION,
  CP_MP_LINK_TAG_SAFETY_REASON_CODE,
  CP_MP_LINK_TAG_STATUS_SEQUENCE,
  CP_MP_LINK_TAG_GLOBAL_FLAGS
};

static const uint32_t kCpMpLinkProtocolVersionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 20U, 1U
};
static const uint32_t kCpMpLinkPeerHealthyOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 20U, 2U
};
static const uint32_t kCpMpLinkAuthorityReadyOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 20U, 3U
};
static const uint32_t kCpMpLinkConfigStateOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 20U, 4U
};
static const uint32_t kCpMpLinkSafetyActionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 20U, 5U
};
static const uint32_t kCpMpLinkSafetyReasonCodeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 20U, 6U
};
static const uint32_t kCpMpLinkStatusSequenceOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 20U, 7U
};
static const uint32_t kCpMpLinkGlobalFlagsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 20U, 8U
};

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

static NtcipError_t GetCpMpLinkObject(void *groupContext,
                                      const NtcipObjectDescriptor_t *descriptor,
                                      const uint32_t *indexes,
                                      uint8_t indexCount,
                                      const NtcipRequestContext_t *
                                      requestContext,
                                      NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  CpMpFaultStatusImageV1_t faultStatus;

  (void) indexes;
  (void) indexCount;
  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case CP_MP_LINK_TAG_PROTOCOL_VERSION:
      {
        NtcipValueSetUnsigned32(value, CPMP_PROTOCOL_VERSION);

        return NTCIP_ERROR_OK;
      }

      case CP_MP_LINK_TAG_PEER_HEALTHY:
      {
        NtcipValueSetUnsigned32(
          value,
          (context->cpMpLinkService != NULL)
          ? CpMpLinkServicePeerHealthy(context->cpMpLinkService)
          : 0U);

        return NTCIP_ERROR_OK;
      }

      case CP_MP_LINK_TAG_AUTHORITY_READY:
      {
        NtcipValueSetUnsigned32(
          value,
          (context->cpMpLinkService != NULL)
          ? CpMpLinkServiceAuthorityReady(context->cpMpLinkService)
          : 0U);

        return NTCIP_ERROR_OK;
      }

      case CP_MP_LINK_TAG_CONFIG_STATE:
      {
        NtcipValueSetUnsigned32(
          value,
          (context->cpMpLinkService != NULL)
          ? (uint32_t) context->cpMpLinkService->lastMpConfigState
          : (uint32_t) CPMP_CONFIG_STATE_EMPTY);

        return NTCIP_ERROR_OK;
      }

      case CP_MP_LINK_TAG_SAFETY_ACTION:
      {
        NtcipValueSetUnsigned32(
          value,
          (context->cpMpLinkService != NULL)
          ? (uint32_t) CpMpLinkServiceGetEffectiveSafetyAction(
            context->cpMpLinkService)
          : (uint32_t) CPMP_SAFETY_ACTION_FLASH);

        return NTCIP_ERROR_OK;
      }

      case CP_MP_LINK_TAG_SAFETY_REASON_CODE:
      {
        NtcipValueSetUnsigned32(
          value,
          (context->cpMpLinkService != NULL)
          ? CpMpLinkServiceGetLastSafetyReasonCode(context->cpMpLinkService)
          : 0U);

        return NTCIP_ERROR_OK;
      }

      case CP_MP_LINK_TAG_STATUS_SEQUENCE:
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

      case CP_MP_LINK_TAG_GLOBAL_FLAGS:
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

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  } /* switch */
} /* GetCpMpLinkObject */

static const NtcipObjectDescriptor_t kCpMpLinkObjects[] = {
  { kCpMpLinkProtocolVersionOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CP_MP_LINK_TAG_PROTOCOL_VERSION, GetCpMpLinkObject, NULL, NULL },
  { kCpMpLinkPeerHealthyOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CP_MP_LINK_TAG_PEER_HEALTHY, GetCpMpLinkObject, NULL, NULL },
  { kCpMpLinkAuthorityReadyOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CP_MP_LINK_TAG_AUTHORITY_READY, GetCpMpLinkObject, NULL, NULL },
  { kCpMpLinkConfigStateOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CP_MP_LINK_TAG_CONFIG_STATE, GetCpMpLinkObject, NULL, NULL },
  { kCpMpLinkSafetyActionOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CP_MP_LINK_TAG_SAFETY_ACTION, GetCpMpLinkObject, NULL, NULL },
  { kCpMpLinkSafetyReasonCodeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CP_MP_LINK_TAG_SAFETY_REASON_CODE, GetCpMpLinkObject, NULL, NULL },
  { kCpMpLinkStatusSequenceOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CP_MP_LINK_TAG_STATUS_SEQUENCE, GetCpMpLinkObject, NULL, NULL },
  { kCpMpLinkGlobalFlagsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CP_MP_LINK_TAG_GLOBAL_FLAGS, GetCpMpLinkObject, NULL, NULL }
};

void TeknotelCpMpLinkObjectsRegister(NtcipObjectDirectory_t *directory,
                                     NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "59748.cpMpLink",
    kCpMpLinkObjects,
    (uint16_t) (sizeof(kCpMpLinkObjects) / sizeof(kCpMpLinkObjects[0])),
    context);
}
