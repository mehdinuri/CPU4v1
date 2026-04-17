/* App/Domain/NTCIP/Core/NtcipTypes.c
 *
 * Helpers for moving integer, string, and OID values across the NTCIP
 * object dispatch boundary.
 */
#include "NtcipTypes.h"

#include <stddef.h>
#include <string.h>

void NtcipValueClear(NtcipValue_t *value)
{
  if (value != NULL)
  {
    memset(value, 0, sizeof(*value));
  }
}

void NtcipValueSetUnsigned32(NtcipValue_t *value, uint32_t data)
{
  if (value != NULL)
  {
    NtcipValueClear(value);
    value->type = NTCIP_VALUE_TYPE_UNSIGNED32;
    value->data.unsigned32 = data;
  }
}

void NtcipValueSetSigned32(NtcipValue_t *value, int32_t data)
{
  if (value != NULL)
  {
    NtcipValueClear(value);
    value->type = NTCIP_VALUE_TYPE_SIGNED32;
    value->data.signed32 = data;
  }
}

NtcipError_t NtcipValueSetObjectId(NtcipValue_t *value,
                                   const uint32_t *oid,
                                   uint8_t oidLength)
{
  if ((value == NULL) || ((oid == NULL) && (oidLength != 0U))
      || (oidLength > NTCIP_OID_MAX_LENGTH))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  NtcipValueClear(value);
  value->type = NTCIP_VALUE_TYPE_OBJECT_ID;
  value->data.objectId.length = oidLength;

  if (oidLength != 0U)
  {
    memcpy(value->data.objectId.elements, oid,
           (size_t) oidLength * sizeof(uint32_t));
  }

  return NTCIP_ERROR_OK;
}

NtcipError_t NtcipValueSetOctetString(NtcipValue_t *value,
                                      const uint8_t *bytes,
                                      uint16_t length)
{
  if ((value == NULL) || ((bytes == NULL) && (length != 0U))
      || (length > NTCIP_OCTET_STRING_MAX_LENGTH))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  NtcipValueClear(value);
  value->type = NTCIP_VALUE_TYPE_OCTET_STRING;
  value->data.octetString.length = length;

  if (length != 0U)
  {
    memcpy(value->data.octetString.bytes, bytes, length);
  }

  return NTCIP_ERROR_OK;
}

NtcipError_t NtcipValueSetCString(NtcipValue_t *value, const char *text)
{
  size_t length;

  if (text == NULL)
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  length = strlen(text);

  if (length > NTCIP_OCTET_STRING_MAX_LENGTH)
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return NtcipValueSetOctetString(value,
                                  (const uint8_t *) text,
                                  (uint16_t) length);
}
