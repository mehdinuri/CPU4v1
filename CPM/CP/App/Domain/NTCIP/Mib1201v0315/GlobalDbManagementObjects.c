/* App/Domain/NTCIP/Mib1201v0315/GlobalDbManagementObjects.c
 *
 * 1201 database transaction and verify-status objects.
 */
#include "GlobalDbManagementObjects.h"

#include <stddef.h>

enum
{
  GLOBAL_DB_MANAGEMENT_TAG_CREATE_TRANSACTION = 1,
  GLOBAL_DB_MANAGEMENT_TAG_ERROR_TYPE,
  GLOBAL_DB_MANAGEMENT_TAG_ERROR_ID,
  GLOBAL_DB_MANAGEMENT_TAG_TRANSACTION_ID,
  GLOBAL_DB_MANAGEMENT_TAG_MAKE_ID,
  GLOBAL_DB_MANAGEMENT_TAG_VERIFY_STATUS,
  GLOBAL_DB_MANAGEMENT_TAG_VERIFY_ERROR
};

static const uint32_t kDbCreateTransactionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 1U
};
static const uint32_t kDbErrorTypeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 2U
};
static const uint32_t kDbErrorIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 3U
};
static const uint32_t kDbTransactionIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 4U
};
static const uint32_t kDbMakeIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 5U
};
static const uint32_t kDbVerifyStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 6U
};
static const uint32_t kDbVerifyErrorOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 7U
};

static NtcipError_t GetGlobalDbManagementObject(void *groupContext,
                                                const NtcipObjectDescriptor_t *
                                                descriptor,
                                                const uint32_t *indexes,
                                                uint8_t indexCount,
                                                const NtcipRequestContext_t *
                                                requestContext,
                                                NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  const NtcipOid_t *errorObjectId;
  const NtcipOctetString_t *verifyError;

  (void) indexes;
  (void) indexCount;
  (void) requestContext;

  if ((context == NULL) || (context->dbTransactionService == NULL)
      || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case GLOBAL_DB_MANAGEMENT_TAG_CREATE_TRANSACTION:
      {
        NtcipValueSetUnsigned32(
          value,
          (uint32_t) NtcipDbTransactionServiceGetState(
            context->dbTransactionService));

        return NTCIP_ERROR_OK;
      }

      case GLOBAL_DB_MANAGEMENT_TAG_ERROR_TYPE:
      {
        NtcipValueSetUnsigned32(
          value,
          (uint32_t) NtcipDbTransactionServiceGetErrorType(
            context->dbTransactionService));

        return NTCIP_ERROR_OK;
      }

      case GLOBAL_DB_MANAGEMENT_TAG_ERROR_ID:
      {
        errorObjectId = NtcipDbTransactionServiceGetErrorObjectId(
          context->dbTransactionService);

        if (errorObjectId == NULL)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        return NtcipValueSetObjectId(value,
                                     errorObjectId->elements,
                                     errorObjectId->length);
      }

      case GLOBAL_DB_MANAGEMENT_TAG_TRANSACTION_ID:
      {
        NtcipValueSetUnsigned32(
          value,
          (uint32_t) NtcipDbTransactionServiceGetTransactionId(
            context->dbTransactionService));

        return NTCIP_ERROR_OK;
      }

      case GLOBAL_DB_MANAGEMENT_TAG_MAKE_ID:
      {
        NtcipValueSetUnsigned32(
          value,
          (uint32_t) NtcipDbTransactionServiceReadMakeId(
            context->dbTransactionService));

        return NTCIP_ERROR_OK;
      }

      case GLOBAL_DB_MANAGEMENT_TAG_VERIFY_STATUS:
      {
        NtcipValueSetUnsigned32(
          value,
          (uint32_t) NtcipDbTransactionServiceGetVerifyStatus(
            context->dbTransactionService));

        return NTCIP_ERROR_OK;
      }

      case GLOBAL_DB_MANAGEMENT_TAG_VERIFY_ERROR:
      {
        verifyError = NtcipDbTransactionServiceGetVerifyError(
          context->dbTransactionService);

        if (verifyError == NULL)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        return NtcipValueSetOctetString(value,
                                        verifyError->bytes,
                                        verifyError->length);
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  } /* switch */
} /* GetGlobalDbManagementObject */

static NtcipError_t SetTestGlobalDbManagementObject(void *groupContext,
                                                    const
                                                    NtcipObjectDescriptor_t *
                                                    descriptor,
                                                    const uint32_t *indexes,
                                                    uint8_t indexCount,
                                                    const NtcipRequestContext_t
                                                    *requestContext,
                                                    const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;

  (void) indexes;
  (void) indexCount;

  if ((context == NULL) || (context->dbTransactionService == NULL)
      || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case GLOBAL_DB_MANAGEMENT_TAG_CREATE_TRANSACTION:
      {
        return NtcipDbTransactionServiceSetCreateTransactionTest(
          context->dbTransactionService,
          requestContext,
          value->data.unsigned32);
      }

      case GLOBAL_DB_MANAGEMENT_TAG_TRANSACTION_ID:
      {
        return NtcipDbTransactionServiceSetTransactionIdTest(
          context->dbTransactionService,
          requestContext,
          value->data.unsigned32);
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }
}

static NtcipError_t SetValueGlobalDbManagementObject(void *groupContext,
                                                     const
                                                     NtcipObjectDescriptor_t *
                                                     descriptor,
                                                     const uint32_t *indexes,
                                                     uint8_t indexCount,
                                                     const NtcipRequestContext_t
                                                     *requestContext,
                                                     const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;

  (void) indexes;
  (void) indexCount;

  if ((context == NULL) || (context->dbTransactionService == NULL)
      || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case GLOBAL_DB_MANAGEMENT_TAG_CREATE_TRANSACTION:
      {
        return NtcipDbTransactionServiceSetCreateTransactionValue(
          context->dbTransactionService,
          requestContext,
          value->data.unsigned32);
      }

      case GLOBAL_DB_MANAGEMENT_TAG_TRANSACTION_ID:
      {
        return NtcipDbTransactionServiceSetTransactionIdValue(
          context->dbTransactionService,
          requestContext,
          value->data.unsigned32);
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }
}

static const NtcipObjectDescriptor_t kGlobalDbManagementObjects[] =
{
  { kDbCreateTransactionOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_DB_MANAGEMENT_TAG_CREATE_TRANSACTION,
    GetGlobalDbManagementObject, SetTestGlobalDbManagementObject,
    SetValueGlobalDbManagementObject },
  { kDbErrorTypeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_DB_MANAGEMENT_TAG_ERROR_TYPE,
    GetGlobalDbManagementObject, NULL, NULL },
  { kDbErrorIdOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OBJECT_ID,
    GLOBAL_DB_MANAGEMENT_TAG_ERROR_ID,
    GetGlobalDbManagementObject, NULL, NULL },
  { kDbTransactionIdOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_DB_MANAGEMENT_TAG_TRANSACTION_ID,
    GetGlobalDbManagementObject, SetTestGlobalDbManagementObject,
    SetValueGlobalDbManagementObject },
  { kDbMakeIdOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_DB_MANAGEMENT_TAG_MAKE_ID,
    GetGlobalDbManagementObject, NULL, NULL },
  { kDbVerifyStatusOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_DB_MANAGEMENT_TAG_VERIFY_STATUS,
    GetGlobalDbManagementObject, NULL, NULL },
  { kDbVerifyErrorOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OCTET_STRING,
    GLOBAL_DB_MANAGEMENT_TAG_VERIFY_ERROR,
    GetGlobalDbManagementObject, NULL, NULL }
};

void GlobalDbManagementObjectsRegister(NtcipObjectDirectory_t *directory,
                                       NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1201.globalDbManagement",
    kGlobalDbManagementObjects,
    (uint16_t) (sizeof(kGlobalDbManagementObjects)
                / sizeof(kGlobalDbManagementObjects[0])),
    context);
}
