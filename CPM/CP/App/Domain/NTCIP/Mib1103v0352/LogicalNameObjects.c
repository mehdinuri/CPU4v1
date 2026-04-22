/* App/Domain/NTCIP/Mib1103v0352/LogicalNameObjects.c */
#include "LogicalNameObjects.h"

#include <string.h>

#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"

enum
{
  LOGICAL_NAME_TAG_MAX_ENTRIES = 1,
  LOGICAL_NAME_TAG_INDEX = 2,
  LOGICAL_NAME_TAG_NAME = 3,
  LOGICAL_NAME_TAG_ADDRESS = 4,
  LOGICAL_NAME_TAG_STATUS = 5
};

static const uint32_t kLogicalNameMaxEntriesOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 4U, 1U
};
static const uint32_t kLogicalNameIndexOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 4U, 2U, 1U, 1U
};
static const uint32_t kLogicalNameOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 4U, 2U, 1U, 2U
};
static const uint32_t kLogicalNameAddressOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 4U, 2U, 1U, 3U
};
static const uint32_t kLogicalNameStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 4U, 2U, 1U, 4U
};

static EventReportConfiguration_t *GetWorkingConfig(NtcipContext_t *context)
{
  if ((context == NULL) || (context->eventReportService == NULL))
  {
    return NULL;
  }

  return EventReportServiceGetCandidateConfig(context->eventReportService);
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

static EventReportLogicalNameRow_t *ResolveLogicalNameRow(
  EventReportConfiguration_t *config,
  const uint32_t *indexes,
  uint8_t indexCount)
{
  if ((config == NULL) || (indexes == NULL) || (indexCount != 1U)
      || (indexes[0] != 1U))
  {
    return NULL;
  }

  return &config->logicalNameRows[0];
}

static NtcipError_t GetLogicalNameObject(void *groupContext,
                                         const NtcipObjectDescriptor_t *descriptor,
                                         const uint32_t *indexes,
                                         uint8_t indexCount,
                                         const NtcipRequestContext_t *requestContext,
                                         NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportLogicalNameRow_t *row;

  (void) requestContext;

  if ((descriptor == NULL) || (value == NULL) || (config == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
    case LOGICAL_NAME_TAG_MAX_ENTRIES:
      NtcipValueSetUnsigned32(value, EVENT_REPORT_LOGICAL_NAME_MAX_ENTRIES);
      return NTCIP_ERROR_OK;

    case LOGICAL_NAME_TAG_INDEX:
      row = ResolveLogicalNameRow(config, indexes, indexCount);
      if (row == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      NtcipValueSetUnsigned32(value, row->logicalNameTranslationIndex);
      return NTCIP_ERROR_OK;

    case LOGICAL_NAME_TAG_NAME:
      row = ResolveLogicalNameRow(config, indexes, indexCount);
      if (row == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      return NtcipValueSetOctetString(value,
                                      &row->logicalName[0],
                                      row->logicalNameLength);

    case LOGICAL_NAME_TAG_ADDRESS:
      row = ResolveLogicalNameRow(config, indexes, indexCount);
      if (row == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      return NtcipValueSetIpAddress(value, &row->networkAddress[0]);

    case LOGICAL_NAME_TAG_STATUS:
      row = ResolveLogicalNameRow(config, indexes, indexCount);
      if (row == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      NtcipValueSetUnsigned32(value, row->status);
      return NTCIP_ERROR_OK;

    default:
      return NTCIP_ERROR_NOT_FOUND;
  }
}

static NtcipError_t SetTestLogicalNameObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportLogicalNameRow_t *row;
  NtcipError_t error;

  if ((descriptor == NULL) || (value == NULL) || (config == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  error = ValidateWrite(context, requestContext);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  row = ResolveLogicalNameRow(config, indexes, indexCount);
  if ((descriptor->tag != LOGICAL_NAME_TAG_MAX_ENTRIES) && (row == NULL))
  {
    return NTCIP_ERROR_NOT_FOUND;
  }

  switch (descriptor->tag)
  {
    case LOGICAL_NAME_TAG_NAME:
      return (value->data.octetString.length <= sizeof(row->logicalName))
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case LOGICAL_NAME_TAG_ADDRESS:
      return NTCIP_ERROR_OK;

    case LOGICAL_NAME_TAG_STATUS:
      return ((value->data.unsigned32 == EVENT_REPORT_ROW_STATUS_INVALID)
              || (value->data.unsigned32 == EVENT_REPORT_ROW_STATUS_ACTIVE))
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    default:
      return NTCIP_ERROR_READ_ONLY;
  }
}

static NtcipError_t SetValueLogicalNameObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportLogicalNameRow_t *row;
  NtcipError_t error;

  error = SetTestLogicalNameObject(groupContext,
                                   descriptor,
                                   indexes,
                                   indexCount,
                                   requestContext,
                                   value);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  row = ResolveLogicalNameRow(config, indexes, indexCount);
  if ((descriptor == NULL) || (row == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
    case LOGICAL_NAME_TAG_NAME:
      (void) memset(&row->logicalName[0], 0, sizeof(row->logicalName));
      row->logicalNameLength = (uint8_t) value->data.octetString.length;
      if (row->logicalNameLength > 0U)
      {
        (void) memcpy(&row->logicalName[0],
                      &value->data.octetString.bytes[0],
                      row->logicalNameLength);
      }
      return NTCIP_ERROR_OK;

    case LOGICAL_NAME_TAG_ADDRESS:
      (void) memcpy(&row->networkAddress[0],
                    &value->data.ipAddress.bytes[0],
                    sizeof(row->networkAddress));
      return NTCIP_ERROR_OK;

    case LOGICAL_NAME_TAG_STATUS:
      row->status = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;

    default:
      return NTCIP_ERROR_READ_ONLY;
  }
}

static const NtcipObjectDescriptor_t kLogicalNameObjects[] =
{
  { kLogicalNameMaxEntriesOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    LOGICAL_NAME_TAG_MAX_ENTRIES, GetLogicalNameObject, NULL, NULL },
  { kLogicalNameIndexOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    LOGICAL_NAME_TAG_INDEX, GetLogicalNameObject, NULL, NULL },
  { kLogicalNameOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    LOGICAL_NAME_TAG_NAME, GetLogicalNameObject,
    SetTestLogicalNameObject, SetValueLogicalNameObject },
  { kLogicalNameAddressOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_IP_ADDRESS,
    LOGICAL_NAME_TAG_ADDRESS, GetLogicalNameObject,
    SetTestLogicalNameObject, SetValueLogicalNameObject },
  { kLogicalNameStatusOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    LOGICAL_NAME_TAG_STATUS, GetLogicalNameObject,
    SetTestLogicalNameObject, SetValueLogicalNameObject }
};

void LogicalNameObjectsRegister(NtcipObjectDirectory_t *directory,
                                NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1103.logicalNames",
    kLogicalNameObjects,
    (uint16_t) (sizeof(kLogicalNameObjects) / sizeof(kLogicalNameObjects[0])),
    context);
}
