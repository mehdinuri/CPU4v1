/* App/Domain/NTCIP/Mib1103v0352/SecurityObjects.c */
#include "SecurityObjects.h"

#include <string.h>

#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"

enum
{
  SECURITY_TAG_COMMUNITY_NAME_ADMIN = 1,
  SECURITY_TAG_COMMUNITY_NAMES_MAX = 2,
  SECURITY_TAG_COMMUNITY_NAME_INDEX = 3,
  SECURITY_TAG_COMMUNITY_NAME_USER = 4,
  SECURITY_TAG_COMMUNITY_NAME_ACCESS_MASK = 5
};

static const uint32_t kCommunityNameAdminOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 5U, 1U
};
static const uint32_t kCommunityNamesMaxOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 5U, 2U
};
static const uint32_t kCommunityNameIndexOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 5U, 3U, 1U, 1U
};
static const uint32_t kCommunityNameUserOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 5U, 3U, 1U, 2U
};
static const uint32_t kCommunityNameAccessMaskOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 5U, 3U, 1U, 3U
};

static EventReportService_t *GetService(NtcipContext_t *context)
{
  return (context == NULL) ? NULL : context->eventReportService;
}

static EventReportConfiguration_t *GetWorkingConfig(NtcipContext_t *context)
{
  EventReportService_t *service = GetService(context);

  return (service == NULL) ? NULL : EventReportServiceGetCandidateConfig(service);
}

static NtcipError_t ValidateWrite(const NtcipContext_t *context,
                                  const NtcipRequestContext_t *requestContext)
{
  if ((context == NULL) || (context->dbTransactionService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return NtcipDbTransactionServiceValidateDatabaseWrite(
    context->dbTransactionService,
    requestContext);
}

static EventReportCommunityRow_t *ResolveCommunityRow(
  EventReportConfiguration_t *config,
  const uint32_t *indexes,
  uint8_t indexCount)
{
  uint32_t rowIndex;

  if ((config == NULL) || (indexes == NULL) || (indexCount != 1U))
  {
    return NULL;
  }

  rowIndex = indexes[0];
  if ((rowIndex == 0U) || (rowIndex > EVENT_REPORT_COMMUNITY_NAMES_MAX))
  {
    return NULL;
  }

  return &config->communityRows[rowIndex - 1U];
}

static uint8_t StrictReleasePolicyEnabled(const NtcipContext_t *context)
{
  if ((context != NULL) && (context->snmpSecurityPort != NULL))
  {
    return SnmpSecurityPortGetStrictReleasePolicy(context->snmpSecurityPort);
  }

#if CP_SNMP_STRICT_RELEASE
  return 1U;
#else
  return 0U;
#endif
}

static uint8_t RequestUsesCommunitySecurity(
  const NtcipRequestContext_t *requestContext)
{
  if (requestContext == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((requestContext->authModel
                     == (uint8_t) NTCIP_AUTH_MODEL_SNMP_V1)
                    || (requestContext->authModel
                        == (uint8_t) NTCIP_AUTH_MODEL_SNMP_V2C));
}

static NtcipError_t GetSecurityObject(void *groupContext,
                                      const NtcipObjectDescriptor_t *descriptor,
                                      const uint32_t *indexes,
                                      uint8_t indexCount,
                                      const NtcipRequestContext_t *requestContext,
                                      NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportCommunityRow_t *row;

  (void) requestContext;

  if ((descriptor == NULL) || (value == NULL) || (config == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
    case SECURITY_TAG_COMMUNITY_NAME_ADMIN:
      return NtcipValueSetOctetString(value,
                                      &config->communityNameAdmin[0],
                                      config->communityNameAdminLength);

    case SECURITY_TAG_COMMUNITY_NAMES_MAX:
      NtcipValueSetUnsigned32(value, EVENT_REPORT_COMMUNITY_NAMES_MAX);
      return NTCIP_ERROR_OK;

    case SECURITY_TAG_COMMUNITY_NAME_INDEX:
      row = ResolveCommunityRow(config, indexes, indexCount);
      if (row == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      NtcipValueSetUnsigned32(value, row->communityNameIndex);
      return NTCIP_ERROR_OK;

    case SECURITY_TAG_COMMUNITY_NAME_USER:
      if ((StrictReleasePolicyEnabled(context) != 0U)
          && (RequestUsesCommunitySecurity(requestContext) != 0U))
      {
        return NTCIP_ERROR_NO_ACCESS;
      }

      row = ResolveCommunityRow(config, indexes, indexCount);
      if (row == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      return NtcipValueSetOctetString(value,
                                      &row->communityNameUser[0],
                                      row->communityNameLength);

    case SECURITY_TAG_COMMUNITY_NAME_ACCESS_MASK:
      row = ResolveCommunityRow(config, indexes, indexCount);
      if (row == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      NtcipValueSetUnsigned32(value, row->communityNameAccessMask);
      return NTCIP_ERROR_OK;

    default:
      return NTCIP_ERROR_NOT_FOUND;
  }
}

static NtcipError_t SetTestSecurityObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportCommunityRow_t *row;
  NtcipError_t error;

  if ((descriptor == NULL) || (value == NULL) || (config == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if ((StrictReleasePolicyEnabled(context) != 0U)
      && (RequestUsesCommunitySecurity(requestContext) != 0U))
  {
    return NTCIP_ERROR_NO_ACCESS;
  }

  error = ValidateWrite(context, requestContext);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
    case SECURITY_TAG_COMMUNITY_NAME_ADMIN:
      return (value->data.octetString.length >= 8U)
             && (value->data.octetString.length
                 <= sizeof(config->communityNameAdmin))
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case SECURITY_TAG_COMMUNITY_NAME_USER:
      row = ResolveCommunityRow(config, indexes, indexCount);
      if (row == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      return (value->data.octetString.length >= 6U)
             && (value->data.octetString.length
                 <= sizeof(row->communityNameUser))
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case SECURITY_TAG_COMMUNITY_NAME_ACCESS_MASK:
      row = ResolveCommunityRow(config, indexes, indexCount);
      return (row == NULL) ? NTCIP_ERROR_NOT_FOUND : NTCIP_ERROR_OK;

    default:
      return NTCIP_ERROR_READ_ONLY;
  }
}

static NtcipError_t SetValueSecurityObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportCommunityRow_t *row;
  NtcipError_t error;

  error = SetTestSecurityObject(groupContext,
                                descriptor,
                                indexes,
                                indexCount,
                                requestContext,
                                value);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if ((descriptor == NULL) || (config == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
    case SECURITY_TAG_COMMUNITY_NAME_ADMIN:
      (void) memset(&config->communityNameAdmin[0],
                    0,
                    sizeof(config->communityNameAdmin));
      config->communityNameAdminLength = (uint8_t) value->data.octetString.length;
      if (config->communityNameAdminLength > 0U)
      {
        (void) memcpy(&config->communityNameAdmin[0],
                      &value->data.octetString.bytes[0],
                      config->communityNameAdminLength);
      }
      return NTCIP_ERROR_OK;

    case SECURITY_TAG_COMMUNITY_NAME_USER:
      row = ResolveCommunityRow(config, indexes, indexCount);
      if (row == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      (void) memset(&row->communityNameUser[0], 0, sizeof(row->communityNameUser));
      row->communityNameLength = (uint8_t) value->data.octetString.length;
      if (row->communityNameLength > 0U)
      {
        (void) memcpy(&row->communityNameUser[0],
                      &value->data.octetString.bytes[0],
                      row->communityNameLength);
      }
      return NTCIP_ERROR_OK;

    case SECURITY_TAG_COMMUNITY_NAME_ACCESS_MASK:
      row = ResolveCommunityRow(config, indexes, indexCount);
      if (row == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      row->communityNameAccessMask = value->data.unsigned32;
      return NTCIP_ERROR_OK;

    default:
      return NTCIP_ERROR_READ_ONLY;
  }
}

static const NtcipObjectDescriptor_t kSecurityObjects[] =
{
  { kCommunityNameAdminOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    SECURITY_TAG_COMMUNITY_NAME_ADMIN, GetSecurityObject,
    SetTestSecurityObject, SetValueSecurityObject },
  { kCommunityNamesMaxOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    SECURITY_TAG_COMMUNITY_NAMES_MAX, GetSecurityObject, NULL, NULL },
  { kCommunityNameIndexOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    SECURITY_TAG_COMMUNITY_NAME_INDEX, GetSecurityObject, NULL, NULL },
  { kCommunityNameUserOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    SECURITY_TAG_COMMUNITY_NAME_USER, GetSecurityObject,
    SetTestSecurityObject, SetValueSecurityObject },
  { kCommunityNameAccessMaskOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    SECURITY_TAG_COMMUNITY_NAME_ACCESS_MASK, GetSecurityObject,
    SetTestSecurityObject, SetValueSecurityObject }
};

void SecurityObjectsRegister(NtcipObjectDirectory_t *directory,
                             NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1103.security",
    kSecurityObjects,
    (uint16_t) (sizeof(kSecurityObjects) / sizeof(kSecurityObjects[0])),
    context);
}
