/* App/Adapters/STM32/LWIPSNMPBridge.c
 *
 * Exact-OID bridge from lwIP's generated MIB tree into the canonical
 * domain-side NTCIP object directory.
 */
#include "LWIPSNMPBridge.h"

#include "lwip/def.h"
#include "lwip/apps/snmp_core.h"
#include "snmp_msg.h"

#include <limits.h>
#include <string.h>

#define FNV1A_OFFSET_BASIS 2166136261UL
#define FNV1A_PRIME 16777619UL
#define LWIP_SNMP_INDEX_VALUE_LIMIT 255U

static LWIPSNMPAdapterCtx_t *spAdapterCtx;

void snmp_external_bind_node_instance(const u32_t *oid,
                                      u8_t oid_len,
                                      struct snmp_node_instance *node_instance);

typedef struct
{
  const uint32_t *oid;
  uint8_t oidLength;
  uint8_t asn1Type;
} LWIPSNMPTypeOverride_t;

static const uint32_t kGlobalTimeCounterOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 1U, 0U
};
static const uint32_t kTimeBaseScheduleDateCounterOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 3U, 2U, 1U, 4U, 1U
};
static const uint32_t kControllerLocalTimeCounterOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 6U, 0U
};
static const uint32_t kDstBeginSecondsCounterOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 6U, 1U
};
static const uint32_t kDstEndSecondsCounterOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 3U, 7U, 2U, 1U, 11U, 1U
};
static const uint32_t kDetectorSampleTimeCounterOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 6U, 0U
};
static const uint32_t kPedestrianDetectorSampleTimeCounterOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 5U, 0U
};
static const uint32_t kUnitTimeNonSequentialChangeCounterOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 22U, 7U, 0U
};

static const LWIPSNMPTypeOverride_t kTypeOverrides[] =
{
  { kGlobalTimeCounterOid, (uint8_t) LWIP_ARRAYSIZE(kGlobalTimeCounterOid),
    SNMP_ASN1_TYPE_COUNTER },
  { kTimeBaseScheduleDateCounterOid,
    (uint8_t) LWIP_ARRAYSIZE(kTimeBaseScheduleDateCounterOid),
    SNMP_ASN1_TYPE_COUNTER },
  { kControllerLocalTimeCounterOid,
    (uint8_t) LWIP_ARRAYSIZE(kControllerLocalTimeCounterOid),
    SNMP_ASN1_TYPE_COUNTER },
  { kDstBeginSecondsCounterOid,
    (uint8_t) LWIP_ARRAYSIZE(kDstBeginSecondsCounterOid),
    SNMP_ASN1_TYPE_COUNTER },
  { kDstEndSecondsCounterOid,
    (uint8_t) LWIP_ARRAYSIZE(kDstEndSecondsCounterOid),
    SNMP_ASN1_TYPE_COUNTER },
  { kDetectorSampleTimeCounterOid,
    (uint8_t) LWIP_ARRAYSIZE(kDetectorSampleTimeCounterOid),
    SNMP_ASN1_TYPE_COUNTER },
  { kPedestrianDetectorSampleTimeCounterOid,
    (uint8_t) LWIP_ARRAYSIZE(kPedestrianDetectorSampleTimeCounterOid),
    SNMP_ASN1_TYPE_COUNTER },
  { kUnitTimeNonSequentialChangeCounterOid,
    (uint8_t) LWIP_ARRAYSIZE(kUnitTimeNonSequentialChangeCounterOid),
    SNMP_ASN1_TYPE_COUNTER }
};

static uint32_t HashBytes(uint32_t hash, const uint8_t *bytes, uint16_t length)
{
  uint16_t index;

  if (bytes == NULL)
  {
    return hash;
  }

  for (index = 0U; index < length; index++)
  {
    hash ^= bytes[index];
    hash *= FNV1A_PRIME;
  }

  return hash;
}

static uint32_t BuildSessionKey(void)
{
  snmp_request_identity_t identity;
  uint32_t hash = FNV1A_OFFSET_BASIS;

  if (snmp_get_current_request_identity(&identity) == 0U)
  {
    return 0U;
  }

  hash = HashBytes(hash, &identity.version, 1U);
  hash = HashBytes(hash, &identity.security_level, 1U);
  hash = HashBytes(hash,
                   (const uint8_t *) &identity.source_ip,
                   (uint16_t) sizeof(identity.source_ip));
  hash = HashBytes(hash,
                   identity.security_name,
                   identity.security_name_len);

  if (hash == 0U)
  {
    hash = 1U;
  }

  return hash;
}

static uint8_t RequestAuthModelFromIdentity(const snmp_request_identity_t *identity)
{
  if (identity == NULL)
  {
    return (uint8_t) NTCIP_AUTH_MODEL_UNKNOWN;
  }

  switch (identity->version)
  {
    case SNMP_VERSION_1:
      return (uint8_t) NTCIP_AUTH_MODEL_SNMP_V1;

    case SNMP_VERSION_2c:
      return (uint8_t) NTCIP_AUTH_MODEL_SNMP_V2C;

    case SNMP_VERSION_3:
      return (uint8_t) NTCIP_AUTH_MODEL_SNMP_V3;

    default:
      return (uint8_t) NTCIP_AUTH_MODEL_UNKNOWN;
  }
}

static uint8_t RequestSecurityLevelFromIdentity(
  const snmp_request_identity_t *identity)
{
  if (identity == NULL)
  {
    return (uint8_t) NTCIP_SECURITY_LEVEL_NONE;
  }

  switch (identity->security_level)
  {
    case 0x00U:
      return (uint8_t) NTCIP_SECURITY_LEVEL_NOAUTH_NOPRIV;

    case 0x01U:
      return (uint8_t) NTCIP_SECURITY_LEVEL_AUTH_NOPRIV;

    case 0x03U:
      return (uint8_t) NTCIP_SECURITY_LEVEL_AUTH_PRIV;

    default:
      return (uint8_t) NTCIP_SECURITY_LEVEL_NONE;
  }
}

static void PopulateRequestAuthContext(NtcipRequestContext_t *requestContext)
{
  snmp_request_identity_t identity;

  if (requestContext == NULL)
  {
    return;
  }

  if (snmp_get_current_request_identity(&identity) == 0U)
  {
    return;
  }

  requestContext->authModel = RequestAuthModelFromIdentity(&identity);
  requestContext->securityLevel = RequestSecurityLevelFromIdentity(&identity);
}

static uint8_t DefaultAsn1TypeForDescriptor(
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *oid,
  uint8_t oidLength)
{
  uint8_t index;

  if ((descriptor == NULL) || (oid == NULL))
  {
    return 0U;
  }

  for (index = 0U; index < LWIP_ARRAYSIZE(kTypeOverrides); index++)
  {
    if (oidLength == kTypeOverrides[index].oidLength)
    {
      if (snmp_oid_equal(kTypeOverrides[index].oid,
                         kTypeOverrides[index].oidLength,
                         oid,
                         oidLength) != 0U)
      {
        return kTypeOverrides[index].asn1Type;
      }
    }
  }

  switch (descriptor->valueType)
  {
    case NTCIP_VALUE_TYPE_UNSIGNED32:
    case NTCIP_VALUE_TYPE_SIGNED32:
      return SNMP_ASN1_TYPE_INTEGER;

    case NTCIP_VALUE_TYPE_OBJECT_ID:
      return SNMP_ASN1_TYPE_OBJECT_ID;

    case NTCIP_VALUE_TYPE_OCTET_STRING:
      return SNMP_ASN1_TYPE_OCTET_STRING;

    case NTCIP_VALUE_TYPE_OPAQUE:
      return SNMP_ASN1_TYPE_OPAQUE;

    case NTCIP_VALUE_TYPE_IP_ADDRESS:
      return SNMP_ASN1_TYPE_IPADDR;

    default:
      return 0U;
  }
}

static uint8_t OidStartsWith(const uint32_t *oid,
                             uint8_t oidLength,
                             const uint32_t *prefix,
                             uint8_t prefixLength)
{
  if ((oid == NULL) || (prefix == NULL) || (oidLength < prefixLength))
  {
    return 0U;
  }

  return (uint8_t) (snmp_oid_compare(oid, prefixLength, prefix, prefixLength)
                    == 0);
}

static uint8_t DescriptorInSubtree(const NtcipObjectDescriptor_t *descriptor,
                                   const uint32_t *subtreeOid,
                                   uint8_t subtreeOidLen)
{
  if ((descriptor == NULL) || (descriptor->oid == NULL))
  {
    return 0U;
  }

  return OidStartsWith(descriptor->oid,
                       descriptor->oidLength,
                       subtreeOid,
                       subtreeOidLen);
}

static void BuildExactOidForDescriptor(const NtcipObjectDescriptor_t *descriptor,
                                       const uint32_t *indexes,
                                       struct snmp_obj_id *oid)
{
  uint8_t index;

  if ((descriptor == NULL) || (oid == NULL))
  {
    return;
  }

  snmp_oid_assign(oid, descriptor->oid, descriptor->oidLength);

  if (descriptor->kind == NTCIP_OBJECT_KIND_SCALAR)
  {
    oid->id[oid->len] = 0U;
    oid->len++;
    return;
  }

  for (index = 0U; index < descriptor->indexCount; index++)
  {
    oid->id[oid->len] = indexes[index];
    oid->len++;
  }
}

static uint8_t DescriptorInstanceExists(const NtcipObjectGroup_t *group,
                                        const NtcipObjectDescriptor_t *descriptor,
                                        const uint32_t *indexes)
{
  NtcipValue_t value;

  if ((group == NULL) || (descriptor == NULL) || (descriptor->get == NULL))
  {
    return 0U;
  }

  return (uint8_t) (descriptor->get(group->context,
                                    descriptor,
                                    indexes,
                                    descriptor->indexCount,
                                    NULL,
                                    &value) == NTCIP_ERROR_OK);
}

static void SearchDescriptorIndexesRecursive(
  const NtcipObjectGroup_t *group,
  const NtcipObjectDescriptor_t *descriptor,
  uint8_t depth,
  uint32_t *indexes,
  struct snmp_next_oid_state *state)
{
  struct snmp_obj_id candidate;
  uint32_t value;

  if ((group == NULL) || (descriptor == NULL) || (indexes == NULL)
      || (state == NULL))
  {
    return;
  }

  for (value = 1U; value <= LWIP_SNMP_INDEX_VALUE_LIMIT; value++)
  {
    indexes[depth] = value;
    BuildExactOidForDescriptor(descriptor, indexes, &candidate);

    if (snmp_next_oid_precheck(state,
                               candidate.id,
                               (u8_t) (descriptor->oidLength + depth + 1U))
        == 0U)
    {
      continue;
    }

    if ((depth + 1U) < descriptor->indexCount)
    {
      SearchDescriptorIndexesRecursive(group,
                                       descriptor,
                                       (uint8_t) (depth + 1U),
                                       indexes,
                                       state);
    }
    else if (DescriptorInstanceExists(group, descriptor, indexes) != 0U)
    {
      (void) snmp_next_oid_check(state,
                                 candidate.id,
                                 candidate.len,
                                 NULL);
    }
  }
}

static void FindNextForDescriptor(const NtcipObjectGroup_t *group,
                                  const NtcipObjectDescriptor_t *descriptor,
                                  struct snmp_next_oid_state *state)
{
  struct snmp_obj_id candidate;
  uint32_t indexes[NTCIP_OBJECT_DIRECTORY_INDEX_MAX] = { 0U };

  if ((group == NULL) || (descriptor == NULL) || (state == NULL)
      || (descriptor->get == NULL))
  {
    return;
  }

  if (descriptor->kind == NTCIP_OBJECT_KIND_SCALAR)
  {
    BuildExactOidForDescriptor(descriptor, NULL, &candidate);
    (void) snmp_next_oid_check(state, candidate.id, candidate.len, NULL);
    return;
  }

  SearchDescriptorIndexesRecursive(group, descriptor, 0U, indexes, state);
}

static snmp_err_t MapSetError(NtcipError_t error)
{
  switch (error)
  {
    case NTCIP_ERROR_OK:
      return SNMP_ERR_NOERROR;

    case NTCIP_ERROR_NOT_FOUND:
      return SNMP_ERR_NOSUCHINSTANCE;

    case NTCIP_ERROR_READ_ONLY:
      return SNMP_ERR_NOTWRITABLE;

    case NTCIP_ERROR_NO_ACCESS:
      return SNMP_ERR_NOACCESS;

    case NTCIP_ERROR_BAD_VALUE:
    case NTCIP_ERROR_RANGE_ERROR:
      return SNMP_ERR_WRONGVALUE;

    case NTCIP_ERROR_NO_TRANSACTION:
    case NTCIP_ERROR_OWNER_MISMATCH:
    case NTCIP_ERROR_TRANSACTION_ID_MISMATCH:
      return SNMP_ERR_INCONSISTENTVALUE;

    case NTCIP_ERROR_COMMIT_FAILED:
      return SNMP_ERR_COMMITFAILED;

    default:
      return SNMP_ERR_GENERROR;
  }
}

static uint8_t ResolveManagedDescriptor(struct snmp_node_instance *instance,
                                        const uint32_t **oid,
                                        uint8_t *oidLength,
                                        const NtcipObjectDescriptor_t **descriptor)
{
  if ((spAdapterCtx == NULL) || (instance == NULL) || (oid == NULL)
      || (oidLength == NULL) || (descriptor == NULL)
      || (instance->reference.const_ptr == NULL)
      || (instance->reference_len > UINT8_MAX))
  {
    return 0U;
  }

  *oid = (const uint32_t *) instance->reference.const_ptr;
  *oidLength = (uint8_t) instance->reference_len;

  return (uint8_t) (LWIPSNMPAdapterGetManagedState(spAdapterCtx,
                                                   *oid,
                                                   *oidLength,
                                                   descriptor)
                    == LWIP_SNMP_MANAGED_STATE_EXACT);
}

static uint8_t LoadUnsigned32FromValue(const NtcipValue_t *value,
                                       struct snmp_node_instance *instance,
                                       void *output,
                                       s16_t *length)
{
  u32_t *unsignedOutput = (u32_t *) output;
  s32_t *signedOutput = (s32_t *) output;

  if ((value == NULL) || (instance == NULL) || (output == NULL)
      || (length == NULL))
  {
    return 0U;
  }

  switch (instance->asn1_type)
  {
    case SNMP_ASN1_TYPE_INTEGER:
      if (value->data.unsigned32 > (uint32_t) INT32_MAX)
      {
        return 0U;
      }

      *signedOutput = (s32_t) value->data.unsigned32;
      *length = (s16_t) sizeof(*signedOutput);
      return 1U;

    case SNMP_ASN1_TYPE_COUNTER:
    case SNMP_ASN1_TYPE_GAUGE:
    case SNMP_ASN1_TYPE_TIMETICKS:
      *unsignedOutput = (u32_t) value->data.unsigned32;
      *length = (s16_t) sizeof(*unsignedOutput);
      return 1U;

    default:
      return 0U;
  }
}

static uint8_t LoadSigned32FromValue(const NtcipValue_t *value,
                                     struct snmp_node_instance *instance,
                                     void *output,
                                     s16_t *length)
{
  s32_t *signedOutput = (s32_t *) output;

  if ((value == NULL) || (instance == NULL) || (output == NULL)
      || (length == NULL)
      || (instance->asn1_type != SNMP_ASN1_TYPE_INTEGER))
  {
    return 0U;
  }

  *signedOutput = (s32_t) value->data.signed32;
  *length = (s16_t) sizeof(*signedOutput);
  return 1U;
}

static s16_t ManagedGetValue(struct snmp_node_instance *instance, void *value)
{
  const uint32_t *oid;
  const NtcipObjectDescriptor_t *descriptor;
  NtcipRequestContext_t requestContext;
  NtcipValue_t managedValue;
  uint8_t oidLength;
  s16_t length = -1;

  if ((value == NULL)
      || (ResolveManagedDescriptor(instance,
                                   &oid,
                                   &oidLength,
                                   &descriptor) == 0U))
  {
    return -1;
  }

  (void) descriptor;

  LWIPSNMPAdapterBuildRequestContext(spAdapterCtx,
                                     BuildSessionKey(),
                                     &requestContext);
  PopulateRequestAuthContext(&requestContext);

  if (LWIPSNMPAdapterGet(spAdapterCtx,
                         oid,
                         oidLength,
                         &requestContext,
                         &managedValue) != NTCIP_ERROR_OK)
  {
    return -1;
  }

  switch (managedValue.type)
  {
    case NTCIP_VALUE_TYPE_UNSIGNED32:
      if (LoadUnsigned32FromValue(&managedValue,
                                  instance,
                                  value,
                                  &length) == 0U)
      {
        return -1;
      }
      break;

    case NTCIP_VALUE_TYPE_SIGNED32:
      if (LoadSigned32FromValue(&managedValue,
                                instance,
                                value,
                                &length) == 0U)
      {
        return -1;
      }
      break;

    case NTCIP_VALUE_TYPE_OBJECT_ID:
      if (instance->asn1_type != SNMP_ASN1_TYPE_OBJECT_ID)
      {
        return -1;
      }

      MEMCPY(value,
             managedValue.data.objectId.elements,
             managedValue.data.objectId.length * sizeof(uint32_t));
      length = (s16_t) (managedValue.data.objectId.length * sizeof(uint32_t));
      break;

    case NTCIP_VALUE_TYPE_OCTET_STRING:
      if ((instance->asn1_type != SNMP_ASN1_TYPE_OCTET_STRING)
          && (instance->asn1_type != SNMP_ASN1_TYPE_OPAQUE))
      {
        return -1;
      }

      MEMCPY(value,
             managedValue.data.octetString.bytes,
             managedValue.data.octetString.length);
      length = (s16_t) managedValue.data.octetString.length;
      break;

    case NTCIP_VALUE_TYPE_OPAQUE:
      if (instance->asn1_type != SNMP_ASN1_TYPE_OPAQUE)
      {
        return -1;
      }

      MEMCPY(value,
             managedValue.data.opaque.bytes,
             managedValue.data.opaque.length);
      length = (s16_t) managedValue.data.opaque.length;
      break;

    case NTCIP_VALUE_TYPE_IP_ADDRESS:
      if (instance->asn1_type != SNMP_ASN1_TYPE_IPADDR)
      {
        return -1;
      }

      MEMCPY(value,
             managedValue.data.ipAddress.bytes,
             sizeof(managedValue.data.ipAddress.bytes));
      length = (s16_t) sizeof(managedValue.data.ipAddress.bytes);
      break;

    default:
      return -1;
  }

  return length;
}

static uint8_t BuildSetValue(struct snmp_node_instance *instance,
                             u16_t valueLength,
                             void *value,
                             NtcipValue_t *managedValue)
{
  if ((instance == NULL) || (value == NULL) || (managedValue == NULL))
  {
    return 0U;
  }

  switch (instance->asn1_type)
  {
    case SNMP_ASN1_TYPE_INTEGER:
      if (valueLength != sizeof(s32_t))
      {
        return 0U;
      }

      NtcipValueSetSigned32(managedValue, *((s32_t *) value));
      return 1U;

    case SNMP_ASN1_TYPE_COUNTER:
    case SNMP_ASN1_TYPE_GAUGE:
    case SNMP_ASN1_TYPE_TIMETICKS:
      if (valueLength != sizeof(u32_t))
      {
        return 0U;
      }

      NtcipValueSetUnsigned32(managedValue, *((u32_t *) value));
      return 1U;

    case SNMP_ASN1_TYPE_OBJECT_ID:
      if ((valueLength == 0U) || ((valueLength % sizeof(uint32_t)) != 0U))
      {
        return 0U;
      }

      return (uint8_t) (NtcipValueSetObjectId(managedValue,
                                              (const uint32_t *) value,
                                              (uint8_t) (valueLength
                                                         / sizeof(uint32_t)))
                        == NTCIP_ERROR_OK);

    case SNMP_ASN1_TYPE_OCTET_STRING:
      return (uint8_t) (NtcipValueSetOctetString(managedValue,
                                                 (const uint8_t *) value,
                                                 valueLength)
                        == NTCIP_ERROR_OK);

    case SNMP_ASN1_TYPE_OPAQUE:
      return (uint8_t) (NtcipValueSetOpaque(managedValue,
                                            (const uint8_t *) value,
                                            valueLength)
                        == NTCIP_ERROR_OK);

    case SNMP_ASN1_TYPE_IPADDR:
      if (valueLength != 4U)
      {
        return 0U;
      }

      return (uint8_t) (NtcipValueSetIpAddress(managedValue,
                                               (const uint8_t *) value)
                        == NTCIP_ERROR_OK);

    default:
      return 0U;
  }
}

static snmp_err_t ManagedSetCommon(struct snmp_node_instance *instance,
                                   u16_t valueLength,
                                   void *value,
                                   uint8_t commit)
{
  const uint32_t *oid;
  const NtcipObjectDescriptor_t *descriptor;
  NtcipRequestContext_t requestContext;
  NtcipValue_t managedValue;
  NtcipError_t error;
  uint8_t oidLength;

  if (ResolveManagedDescriptor(instance,
                               &oid,
                               &oidLength,
                               &descriptor) == 0U)
  {
    return SNMP_ERR_NOSUCHINSTANCE;
  }

  LWIPSNMPAdapterBuildRequestContext(spAdapterCtx,
                                     BuildSessionKey(),
                                     &requestContext);
  PopulateRequestAuthContext(&requestContext);

  if (descriptor->access != NTCIP_ACCESS_READ_WRITE)
  {
    return SNMP_ERR_NOTWRITABLE;
  }

  if (BuildSetValue(instance,
                    valueLength,
                    value,
                    &managedValue) == 0U)
  {
    return SNMP_ERR_WRONGVALUE;
  }

  if (descriptor->valueType != managedValue.type)
  {
    if (!((descriptor->valueType == NTCIP_VALUE_TYPE_UNSIGNED32)
          && (managedValue.type == NTCIP_VALUE_TYPE_SIGNED32)
          && (managedValue.data.signed32 >= 0)))
    {
      return SNMP_ERR_WRONGVALUE;
    }

    NtcipValueSetUnsigned32(&managedValue,
                            (uint32_t) managedValue.data.signed32);
  }

  if (commit != 0U)
  {
    error = LWIPSNMPAdapterSetValue(spAdapterCtx,
                                    oid,
                                    oidLength,
                                    &requestContext,
                                    &managedValue);
  }
  else
  {
    error = LWIPSNMPAdapterSetTest(spAdapterCtx,
                                   oid,
                                   oidLength,
                                   &requestContext,
                                   &managedValue);
  }

  return MapSetError(error);
}

static snmp_err_t ManagedSetTest(struct snmp_node_instance *instance,
                                 u16_t valueLength,
                                 void *value)
{
  return ManagedSetCommon(instance, valueLength, value, 0U);
}

static snmp_err_t ManagedSetValue(struct snmp_node_instance *instance,
                                  u16_t valueLength,
                                  void *value)
{
  return ManagedSetCommon(instance, valueLength, value, 1U);
}

void LWIPSNMPBridgeBindAdapter(LWIPSNMPAdapterCtx_t *ctx)
{
  spAdapterCtx = ctx;
}

u8_t LWIPSNMPBridgeGetManagedState(const u32_t *oid, u8_t oidLen)
{
  if (spAdapterCtx == NULL)
  {
    return 0U;
  }

  return (u8_t) LWIPSNMPAdapterGetManagedState(spAdapterCtx,
                                               oid,
                                               oidLen,
                                               NULL);
}

u8_t LWIPSNMPBridgeFindNextManagedOid(const u32_t *startOid,
                                      u8_t startOidLen,
                                      const u32_t *subtreeOid,
                                      u8_t subtreeOidLen,
                                      struct snmp_obj_id *nextOid)
{
  struct snmp_next_oid_state state;
  uint32_t nextBuffer[SNMP_MAX_OBJ_ID_LEN];
  uint8_t groupIndex;

  if ((spAdapterCtx == NULL) || (startOid == NULL) || (subtreeOid == NULL)
      || (nextOid == NULL))
  {
    return 0U;
  }

  snmp_next_oid_init(&state,
                     startOid,
                     startOidLen,
                     nextBuffer,
                     (u8_t) LWIP_ARRAYSIZE(nextBuffer));

  for (groupIndex = 0U;
       groupIndex < spAdapterCtx->objectDirectory.groupCount;
       groupIndex++)
  {
    const NtcipObjectGroup_t *group =
      &spAdapterCtx->objectDirectory.groups[groupIndex];
    uint16_t descriptorIndex;

    for (descriptorIndex = 0U; descriptorIndex < group->descriptorCount;
         descriptorIndex++)
    {
      const NtcipObjectDescriptor_t *descriptor =
        &group->descriptors[descriptorIndex];

      if (DescriptorInSubtree(descriptor, subtreeOid, subtreeOidLen) == 0U)
      {
        continue;
      }

      FindNextForDescriptor(group, descriptor, &state);
    }
  }

  if (state.status != SNMP_NEXT_OID_STATUS_SUCCESS)
  {
    return 0U;
  }

  snmp_oid_assign(nextOid, state.next_oid, state.next_oid_len);
  return 1U;
}

void LWIPSNMPBridgeBindNodeInstance(const u32_t *oid,
                                    u8_t oidLen,
                                    struct snmp_node_instance *nodeInstance)
{
  snmp_external_bind_node_instance(oid, oidLen, nodeInstance);
}

u8_t snmp_external_get_oid_state(const u32_t *oid, u8_t oid_len)
{
  return LWIPSNMPBridgeGetManagedState(oid, oid_len);
}

void snmp_external_bind_node_instance(const u32_t *oid,
                                      u8_t oid_len,
                                      struct snmp_node_instance *node_instance)
{
  const NtcipObjectDescriptor_t *descriptor = NULL;

  if ((spAdapterCtx == NULL) || (oid == NULL) || (node_instance == NULL))
  {
    return;
  }

  if (LWIPSNMPAdapterGetManagedState(spAdapterCtx,
                                     oid,
                                     oid_len,
                                     &descriptor) != LWIP_SNMP_MANAGED_STATE_EXACT)
  {
    return;
  }

  node_instance->asn1_type = DefaultAsn1TypeForDescriptor(descriptor,
                                                          oid,
                                                          oid_len);
  node_instance->access = (descriptor->access == NTCIP_ACCESS_READ_WRITE)
                          ? SNMP_NODE_INSTANCE_READ_WRITE
                          : SNMP_NODE_INSTANCE_READ_ONLY;
  node_instance->get_value = ManagedGetValue;
  node_instance->set_test = (descriptor->access == NTCIP_ACCESS_READ_WRITE)
                            ? ManagedSetTest
                            : NULL;
  node_instance->set_value = (descriptor->access == NTCIP_ACCESS_READ_WRITE)
                             ? ManagedSetValue
                             : NULL;
  node_instance->release_instance = NULL;
  node_instance->reference.const_ptr = oid;
  node_instance->reference_len = oid_len;
}
