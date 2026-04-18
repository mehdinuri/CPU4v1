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

static LWIPSNMPAdapterCtx_t *spAdapterCtx;

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
    case SNMP_ASN1_TYPE_OPAQUE:
      return (uint8_t) (NtcipValueSetOctetString(managedValue,
                                                 (const uint8_t *) value,
                                                 valueLength)
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

u8_t snmp_external_get_oid_state(const u32_t *oid, u8_t oid_len)
{
  if (spAdapterCtx == NULL)
  {
    return 0U;
  }

  return (u8_t) LWIPSNMPAdapterGetManagedState(spAdapterCtx,
                                               oid,
                                               oid_len,
                                               NULL);
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
