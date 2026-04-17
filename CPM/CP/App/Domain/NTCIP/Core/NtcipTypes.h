/* App/Domain/NTCIP/Core/NtcipTypes.h
 *
 * Common NTCIP object and value types shared across the new domain-side
 * object directory and group handlers.
 */
#ifndef NTCIP_TYPES_H
#define NTCIP_TYPES_H

#include <stdint.h>

#define NTCIP_OID_MAX_LENGTH 32U
#define NTCIP_OCTET_STRING_MAX_LENGTH 255U

typedef enum
{
  NTCIP_OBJECT_KIND_SCALAR = 0,
  NTCIP_OBJECT_KIND_TABLE_COLUMN
} NtcipObjectKind_t;

typedef enum
{
  NTCIP_ACCESS_READ_ONLY = 0,
  NTCIP_ACCESS_READ_WRITE
} NtcipAccess_t;

typedef enum
{
  NTCIP_VALUE_TYPE_UNSIGNED32 = 0,
  NTCIP_VALUE_TYPE_SIGNED32,
  NTCIP_VALUE_TYPE_OBJECT_ID,
  NTCIP_VALUE_TYPE_OCTET_STRING
} NtcipValueType_t;

typedef enum
{
  NTCIP_ERROR_OK = 0,
  NTCIP_ERROR_NOT_FOUND,
  NTCIP_ERROR_READ_ONLY,
  NTCIP_ERROR_BAD_VALUE,
  NTCIP_ERROR_RANGE_ERROR,
  NTCIP_ERROR_NO_TRANSACTION,
  NTCIP_ERROR_OWNER_MISMATCH,
  NTCIP_ERROR_TRANSACTION_ID_MISMATCH,
  NTCIP_ERROR_GEN_ERROR,
  NTCIP_ERROR_COMMIT_FAILED
} NtcipError_t;

typedef struct
{
  uint32_t elements[NTCIP_OID_MAX_LENGTH];
  uint8_t length;
} NtcipOid_t;

typedef struct
{
  uint8_t bytes[NTCIP_OCTET_STRING_MAX_LENGTH];
  uint16_t length;
} NtcipOctetString_t;

typedef struct
{
  NtcipValueType_t type;

  union
  {
    uint32_t unsigned32;
    int32_t signed32;
    NtcipOid_t objectId;
    NtcipOctetString_t octetString;
  } data;
} NtcipValue_t;

typedef struct
{
  uint32_t sessionKey;
  uint8_t transactionIdValid;
  uint8_t transactionId;
} NtcipRequestContext_t;

void NtcipValueClear(NtcipValue_t *value);
void NtcipValueSetUnsigned32(NtcipValue_t *value, uint32_t data);
void NtcipValueSetSigned32(NtcipValue_t *value, int32_t data);
NtcipError_t NtcipValueSetObjectId(NtcipValue_t *value,
                                   const uint32_t *oid,
                                   uint8_t oidLength);
NtcipError_t NtcipValueSetOctetString(NtcipValue_t *value,
                                      const uint8_t *bytes,
                                      uint16_t length);
NtcipError_t NtcipValueSetCString(NtcipValue_t *value, const char *text);

#endif /* NTCIP_TYPES_H */
