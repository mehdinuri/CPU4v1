/* App/Domain/NTCIP/Mib1103v0352/BlockObjects.c */
#include "BlockObjects.h"

#include <string.h>

#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"

enum
{
  BLOCK_TAG_MAX_WATCH_OBJECTS = 1,
  BLOCK_TAG_MAX_WATCH_BLOCKS = 2,
  BLOCK_TAG_WATCH_ID = 3,
  BLOCK_TAG_WATCH_STATUS = 4,
  BLOCK_TAG_WATCH_BLOCK = 5,
  BLOCK_TAG_WATCH_OID = 6,
  BLOCK_TAG_WATCH_BLOCK_NUMBER = 7,
  BLOCK_TAG_WATCH_BLOCK_STATUS = 8,
  BLOCK_TAG_WATCH_BLOCK_DESCRIPTION = 9,
  BLOCK_TAG_WATCH_BLOCK_VALUE = 10,
  BLOCK_TAG_MAX_REPORT_OBJECTS = 11,
  BLOCK_TAG_MAX_REPORT_BLOCKS = 12,
  BLOCK_TAG_REPORT_ID = 13,
  BLOCK_TAG_REPORT_STATUS = 14,
  BLOCK_TAG_REPORT_BLOCK = 15,
  BLOCK_TAG_REPORT_OID = 16,
  BLOCK_TAG_REPORT_BLOCK_NUMBER = 17,
  BLOCK_TAG_REPORT_BLOCK_STATUS = 18,
  BLOCK_TAG_REPORT_BLOCK_DESCRIPTION = 19,
  BLOCK_TAG_REPORT_BLOCK_VALUE = 20
};

static const uint32_t kMaxWatchObjectsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U, 1U
};
static const uint32_t kMaxWatchBlocksOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U, 2U
};
static const uint32_t kWatchIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U, 3U, 1U, 1U
};
static const uint32_t kWatchStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U, 3U, 1U, 2U
};
static const uint32_t kWatchBlockOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U, 3U, 1U, 3U
};
static const uint32_t kWatchOidOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U, 3U, 1U, 4U
};
static const uint32_t kWatchBlockNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U, 4U, 1U, 1U
};
static const uint32_t kWatchBlockStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U, 4U, 1U, 2U
};
static const uint32_t kWatchBlockDescriptionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U, 4U, 1U, 3U
};
static const uint32_t kWatchBlockValueOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U, 4U, 1U, 4U
};
static const uint32_t kMaxReportObjectsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U, 1U
};
static const uint32_t kMaxReportBlocksOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U, 2U
};
static const uint32_t kReportIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U, 3U, 1U, 1U
};
static const uint32_t kReportStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U, 3U, 1U, 2U
};
static const uint32_t kReportBlockOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U, 3U, 1U, 3U
};
static const uint32_t kReportOidOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U, 3U, 1U, 4U
};
static const uint32_t kReportBlockNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U, 4U, 1U, 1U
};
static const uint32_t kReportBlockStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U, 4U, 1U, 2U
};
static const uint32_t kReportBlockDescriptionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U, 4U, 1U, 3U
};
static const uint32_t kReportBlockValueOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U, 4U, 1U, 4U
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

static EventReportWatchObjectRow_t *ResolveWatchObjectRow(
  EventReportConfiguration_t *config,
  const uint32_t *indexes,
  uint8_t indexCount)
{
  if ((config == NULL) || (indexes == NULL) || (indexCount != 1U)
      || (indexes[0] == 0U)
      || (indexes[0] > EVENT_REPORT_MAX_WATCH_OBJECTS))
  {
    return NULL;
  }

  return &config->watchObjectRows[indexes[0] - 1U];
}

static EventReportWatchBlockRow_t *ResolveWatchBlockRow(
  EventReportConfiguration_t *config,
  const uint32_t *indexes,
  uint8_t indexCount)
{
  if ((config == NULL) || (indexes == NULL) || (indexCount != 1U)
      || (indexes[0] == 0U)
      || (indexes[0] > EVENT_REPORT_MAX_WATCH_BLOCKS))
  {
    return NULL;
  }

  return &config->watchBlockRows[indexes[0] - 1U];
}

static EventReportReportObjectRow_t *ResolveReportObjectRow(
  EventReportConfiguration_t *config,
  const uint32_t *indexes,
  uint8_t indexCount)
{
  if ((config == NULL) || (indexes == NULL) || (indexCount != 1U)
      || (indexes[0] == 0U)
      || (indexes[0] > EVENT_REPORT_MAX_REPORT_OBJECTS))
  {
    return NULL;
  }

  return &config->reportObjectRows[indexes[0] - 1U];
}

static EventReportReportBlockRow_t *ResolveReportBlockRow(
  EventReportConfiguration_t *config,
  const uint32_t *indexes,
  uint8_t indexCount)
{
  if ((config == NULL) || (indexes == NULL) || (indexCount != 1U)
      || (indexes[0] == 0U)
      || (indexes[0] > EVENT_REPORT_MAX_REPORT_BLOCKS))
  {
    return NULL;
  }

  return &config->reportBlockRows[indexes[0] - 1U];
}

static void CopyDescription(uint8_t *target,
                            uint8_t *targetLength,
                            const NtcipOctetString_t *source,
                            uint8_t maxLength)
{
  if ((target == NULL) || (targetLength == NULL) || (source == NULL))
  {
    return;
  }

  (void) memset(target, 0, maxLength);
  *targetLength = (uint8_t) source->length;
  if (*targetLength > 0U)
  {
    (void) memcpy(target, &source->bytes[0], *targetLength);
  }
}

static NtcipError_t GetBlockObject(void *groupContext,
                                   const NtcipObjectDescriptor_t *descriptor,
                                   const uint32_t *indexes,
                                   uint8_t indexCount,
                                   const NtcipRequestContext_t *requestContext,
                                   NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportService_t *service = GetService(context);
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportWatchObjectRow_t *watchObjectRow;
  EventReportWatchBlockRow_t *watchBlockRow;
  EventReportReportObjectRow_t *reportObjectRow;
  EventReportReportBlockRow_t *reportBlockRow;
  NtcipOctetString_t encodedValue;

  (void) requestContext;

  if ((descriptor == NULL) || (value == NULL) || (service == NULL)
      || (config == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
    case BLOCK_TAG_MAX_WATCH_OBJECTS:
      NtcipValueSetUnsigned32(value, EVENT_REPORT_MAX_WATCH_OBJECTS);
      return NTCIP_ERROR_OK;

    case BLOCK_TAG_MAX_WATCH_BLOCKS:
      NtcipValueSetUnsigned32(value, EVENT_REPORT_MAX_WATCH_BLOCKS);
      return NTCIP_ERROR_OK;

    case BLOCK_TAG_WATCH_ID:
    case BLOCK_TAG_WATCH_STATUS:
    case BLOCK_TAG_WATCH_BLOCK:
    case BLOCK_TAG_WATCH_OID:
      watchObjectRow = ResolveWatchObjectRow(config, indexes, indexCount);
      if (watchObjectRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      switch (descriptor->tag)
      {
        case BLOCK_TAG_WATCH_ID:
          NtcipValueSetUnsigned32(value, watchObjectRow->watchId);
          return NTCIP_ERROR_OK;
        case BLOCK_TAG_WATCH_STATUS:
          NtcipValueSetUnsigned32(value, watchObjectRow->watchStatus);
          return NTCIP_ERROR_OK;
        case BLOCK_TAG_WATCH_BLOCK:
          NtcipValueSetUnsigned32(value, watchObjectRow->watchBlock);
          return NTCIP_ERROR_OK;
        case BLOCK_TAG_WATCH_OID:
          return NtcipValueSetObjectId(value,
                                       &watchObjectRow->watchOid.elements[0],
                                       watchObjectRow->watchOid.length);
        default:
          return NTCIP_ERROR_NOT_FOUND;
      }

    case BLOCK_TAG_WATCH_BLOCK_NUMBER:
    case BLOCK_TAG_WATCH_BLOCK_STATUS:
    case BLOCK_TAG_WATCH_BLOCK_DESCRIPTION:
    case BLOCK_TAG_WATCH_BLOCK_VALUE:
      watchBlockRow = ResolveWatchBlockRow(config, indexes, indexCount);
      if (watchBlockRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      switch (descriptor->tag)
      {
        case BLOCK_TAG_WATCH_BLOCK_NUMBER:
          NtcipValueSetUnsigned32(value, watchBlockRow->watchBlockNumber);
          return NTCIP_ERROR_OK;
        case BLOCK_TAG_WATCH_BLOCK_STATUS:
          NtcipValueSetUnsigned32(value, watchBlockRow->watchBlockStatus);
          return NTCIP_ERROR_OK;
        case BLOCK_TAG_WATCH_BLOCK_DESCRIPTION:
          return NtcipValueSetOctetString(
            value,
            &watchBlockRow->watchBlockDescription[0],
            watchBlockRow->watchBlockDescriptionLength);
        case BLOCK_TAG_WATCH_BLOCK_VALUE:
          return (EventReportServiceReadWatchBlockValue(
                    service,
                    watchBlockRow->watchBlockNumber,
                    &encodedValue) != 0U)
                 ? NtcipValueSetOctetString(value,
                                            &encodedValue.bytes[0],
                                            encodedValue.length)
                 : NTCIP_ERROR_GEN_ERROR;
        default:
          return NTCIP_ERROR_NOT_FOUND;
      }

    case BLOCK_TAG_MAX_REPORT_OBJECTS:
      NtcipValueSetUnsigned32(value, EVENT_REPORT_MAX_REPORT_OBJECTS);
      return NTCIP_ERROR_OK;

    case BLOCK_TAG_MAX_REPORT_BLOCKS:
      NtcipValueSetUnsigned32(value, EVENT_REPORT_MAX_REPORT_BLOCKS);
      return NTCIP_ERROR_OK;

    case BLOCK_TAG_REPORT_ID:
    case BLOCK_TAG_REPORT_STATUS:
    case BLOCK_TAG_REPORT_BLOCK:
    case BLOCK_TAG_REPORT_OID:
      reportObjectRow = ResolveReportObjectRow(config, indexes, indexCount);
      if (reportObjectRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      switch (descriptor->tag)
      {
        case BLOCK_TAG_REPORT_ID:
          NtcipValueSetUnsigned32(value, reportObjectRow->reportId);
          return NTCIP_ERROR_OK;
        case BLOCK_TAG_REPORT_STATUS:
          NtcipValueSetUnsigned32(value, reportObjectRow->reportStatus);
          return NTCIP_ERROR_OK;
        case BLOCK_TAG_REPORT_BLOCK:
          NtcipValueSetUnsigned32(value, reportObjectRow->reportBlock);
          return NTCIP_ERROR_OK;
        case BLOCK_TAG_REPORT_OID:
          return NtcipValueSetObjectId(value,
                                       &reportObjectRow->reportOid.elements[0],
                                       reportObjectRow->reportOid.length);
        default:
          return NTCIP_ERROR_NOT_FOUND;
      }

    case BLOCK_TAG_REPORT_BLOCK_NUMBER:
    case BLOCK_TAG_REPORT_BLOCK_STATUS:
    case BLOCK_TAG_REPORT_BLOCK_DESCRIPTION:
    case BLOCK_TAG_REPORT_BLOCK_VALUE:
      reportBlockRow = ResolveReportBlockRow(config, indexes, indexCount);
      if (reportBlockRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      switch (descriptor->tag)
      {
        case BLOCK_TAG_REPORT_BLOCK_NUMBER:
          NtcipValueSetUnsigned32(value, reportBlockRow->reportBlockNumber);
          return NTCIP_ERROR_OK;
        case BLOCK_TAG_REPORT_BLOCK_STATUS:
          NtcipValueSetUnsigned32(value, reportBlockRow->reportBlockStatus);
          return NTCIP_ERROR_OK;
        case BLOCK_TAG_REPORT_BLOCK_DESCRIPTION:
          return NtcipValueSetOctetString(
            value,
            &reportBlockRow->reportBlockDescription[0],
            reportBlockRow->reportBlockDescriptionLength);
        case BLOCK_TAG_REPORT_BLOCK_VALUE:
          return (EventReportServiceReadReportBlockValue(
                    service,
                    reportBlockRow->reportBlockNumber,
                    &encodedValue) != 0U)
                 ? NtcipValueSetOctetString(value,
                                            &encodedValue.bytes[0],
                                            encodedValue.length)
                 : NTCIP_ERROR_GEN_ERROR;
        default:
          return NTCIP_ERROR_NOT_FOUND;
      }

    default:
      return NTCIP_ERROR_NOT_FOUND;
  }
}

static NtcipError_t SetTestBlockObject(void *groupContext,
                                       const NtcipObjectDescriptor_t *descriptor,
                                       const uint32_t *indexes,
                                       uint8_t indexCount,
                                       const NtcipRequestContext_t *requestContext,
                                       const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportService_t *service = GetService(context);
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportWatchObjectRow_t *watchObjectRow;
  EventReportWatchBlockRow_t *watchBlockRow;
  EventReportReportObjectRow_t *reportObjectRow;
  EventReportReportBlockRow_t *reportBlockRow;
  NtcipError_t error;

  if ((descriptor == NULL) || (value == NULL) || (service == NULL)
      || (config == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  error = ValidateWrite(context, requestContext);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
    case BLOCK_TAG_WATCH_STATUS:
      watchObjectRow = ResolveWatchObjectRow(config, indexes, indexCount);
      if (watchObjectRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      if ((value->data.unsigned32 != EVENT_REPORT_ROW_STATUS_INVALID)
          && (value->data.unsigned32 != EVENT_REPORT_ROW_STATUS_ACTIVE))
      {
        return NTCIP_ERROR_BAD_VALUE;
      }

      if ((value->data.unsigned32 == EVENT_REPORT_ROW_STATUS_ACTIVE)
          && (EventReportServiceValidateWatchObjectOid(service,
                                                      &watchObjectRow->watchOid)
              == 0U))
      {
        return NTCIP_ERROR_BAD_VALUE;
      }

      return NTCIP_ERROR_OK;

    case BLOCK_TAG_WATCH_BLOCK:
      return (ResolveWatchObjectRow(config, indexes, indexCount) == NULL)
             ? NTCIP_ERROR_NOT_FOUND
             : ((value->data.unsigned32 >= 1U)
                && (value->data.unsigned32 <= EVENT_REPORT_MAX_WATCH_BLOCKS))
             ? NTCIP_ERROR_OK
             : NTCIP_ERROR_BAD_VALUE;

    case BLOCK_TAG_WATCH_OID:
      watchObjectRow = ResolveWatchObjectRow(config, indexes, indexCount);
      if (watchObjectRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      if (value->data.objectId.length == 0U)
      {
        return (watchObjectRow->watchStatus == EVENT_REPORT_ROW_STATUS_ACTIVE)
               ? NTCIP_ERROR_BAD_VALUE
               : NTCIP_ERROR_OK;
      }

      return (EventReportServiceValidateWatchObjectOid(service,
                                                       &value->data.objectId)
              != 0U)
             ? NTCIP_ERROR_OK
             : NTCIP_ERROR_BAD_VALUE;

    case BLOCK_TAG_WATCH_BLOCK_STATUS:
      watchBlockRow = ResolveWatchBlockRow(config, indexes, indexCount);
      if (watchBlockRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      return ((value->data.unsigned32 == EVENT_REPORT_ROW_STATUS_INVALID)
              || (value->data.unsigned32 == EVENT_REPORT_ROW_STATUS_ACTIVE))
             ? NTCIP_ERROR_OK
             : NTCIP_ERROR_BAD_VALUE;

    case BLOCK_TAG_WATCH_BLOCK_DESCRIPTION:
      return (ResolveWatchBlockRow(config, indexes, indexCount) == NULL)
             ? NTCIP_ERROR_NOT_FOUND
             : (value->data.octetString.length
                <= EVENT_REPORT_BLOCK_DESCRIPTION_MAX_LENGTH)
             ? NTCIP_ERROR_OK
             : NTCIP_ERROR_BAD_VALUE;

    case BLOCK_TAG_REPORT_STATUS:
      reportObjectRow = ResolveReportObjectRow(config, indexes, indexCount);
      if (reportObjectRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      if ((value->data.unsigned32 != EVENT_REPORT_ROW_STATUS_INVALID)
          && (value->data.unsigned32 != EVENT_REPORT_ROW_STATUS_ACTIVE))
      {
        return NTCIP_ERROR_BAD_VALUE;
      }

      if ((value->data.unsigned32 == EVENT_REPORT_ROW_STATUS_ACTIVE)
          && (EventReportServiceValidateReportObjectOid(
                service,
                &reportObjectRow->reportOid) == 0U))
      {
        return NTCIP_ERROR_BAD_VALUE;
      }

      return NTCIP_ERROR_OK;

    case BLOCK_TAG_REPORT_BLOCK:
      return (ResolveReportObjectRow(config, indexes, indexCount) == NULL)
             ? NTCIP_ERROR_NOT_FOUND
             : ((value->data.unsigned32 >= 1U)
                && (value->data.unsigned32 <= EVENT_REPORT_MAX_REPORT_BLOCKS))
             ? NTCIP_ERROR_OK
             : NTCIP_ERROR_BAD_VALUE;

    case BLOCK_TAG_REPORT_OID:
      reportObjectRow = ResolveReportObjectRow(config, indexes, indexCount);
      if (reportObjectRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      if (value->data.objectId.length == 0U)
      {
        return (reportObjectRow->reportStatus == EVENT_REPORT_ROW_STATUS_ACTIVE)
               ? NTCIP_ERROR_BAD_VALUE
               : NTCIP_ERROR_OK;
      }

      return (EventReportServiceValidateReportObjectOid(service,
                                                        &value->data.objectId)
              != 0U)
             ? NTCIP_ERROR_OK
             : NTCIP_ERROR_BAD_VALUE;

    case BLOCK_TAG_REPORT_BLOCK_STATUS:
      reportBlockRow = ResolveReportBlockRow(config, indexes, indexCount);
      if (reportBlockRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      return ((value->data.unsigned32 == EVENT_REPORT_ROW_STATUS_INVALID)
              || (value->data.unsigned32 == EVENT_REPORT_ROW_STATUS_ACTIVE))
             ? NTCIP_ERROR_OK
             : NTCIP_ERROR_BAD_VALUE;

    case BLOCK_TAG_REPORT_BLOCK_DESCRIPTION:
      return (ResolveReportBlockRow(config, indexes, indexCount) == NULL)
             ? NTCIP_ERROR_NOT_FOUND
             : (value->data.octetString.length
                <= EVENT_REPORT_BLOCK_DESCRIPTION_MAX_LENGTH)
             ? NTCIP_ERROR_OK
             : NTCIP_ERROR_BAD_VALUE;

    default:
      return NTCIP_ERROR_READ_ONLY;
  }
}

static NtcipError_t SetValueBlockObject(void *groupContext,
                                        const NtcipObjectDescriptor_t *descriptor,
                                        const uint32_t *indexes,
                                        uint8_t indexCount,
                                        const NtcipRequestContext_t *requestContext,
                                        const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportService_t *service = GetService(context);
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportWatchObjectRow_t *watchObjectRow;
  EventReportWatchBlockRow_t *watchBlockRow;
  EventReportReportObjectRow_t *reportObjectRow;
  EventReportReportBlockRow_t *reportBlockRow;
  NtcipError_t error;

  error = SetTestBlockObject(groupContext,
                             descriptor,
                             indexes,
                             indexCount,
                             requestContext,
                             value);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if ((descriptor == NULL) || (service == NULL) || (config == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
    case BLOCK_TAG_WATCH_STATUS:
      watchObjectRow = ResolveWatchObjectRow(config, indexes, indexCount);
      watchObjectRow->watchStatus = (uint8_t) value->data.unsigned32;
      break;

    case BLOCK_TAG_WATCH_BLOCK:
      watchObjectRow = ResolveWatchObjectRow(config, indexes, indexCount);
      watchObjectRow->watchBlock = (uint8_t) value->data.unsigned32;
      break;

    case BLOCK_TAG_WATCH_OID:
      watchObjectRow = ResolveWatchObjectRow(config, indexes, indexCount);
      watchObjectRow->watchOid = value->data.objectId;
      break;

    case BLOCK_TAG_WATCH_BLOCK_STATUS:
      watchBlockRow = ResolveWatchBlockRow(config, indexes, indexCount);
      watchBlockRow->watchBlockStatus = (uint8_t) value->data.unsigned32;
      break;

    case BLOCK_TAG_WATCH_BLOCK_DESCRIPTION:
      watchBlockRow = ResolveWatchBlockRow(config, indexes, indexCount);
      CopyDescription(&watchBlockRow->watchBlockDescription[0],
                      &watchBlockRow->watchBlockDescriptionLength,
                      &value->data.octetString,
                      sizeof(watchBlockRow->watchBlockDescription));
      break;

    case BLOCK_TAG_REPORT_STATUS:
      reportObjectRow = ResolveReportObjectRow(config, indexes, indexCount);
      reportObjectRow->reportStatus = (uint8_t) value->data.unsigned32;
      break;

    case BLOCK_TAG_REPORT_BLOCK:
      reportObjectRow = ResolveReportObjectRow(config, indexes, indexCount);
      reportObjectRow->reportBlock = (uint8_t) value->data.unsigned32;
      break;

    case BLOCK_TAG_REPORT_OID:
      reportObjectRow = ResolveReportObjectRow(config, indexes, indexCount);
      reportObjectRow->reportOid = value->data.objectId;
      break;

    case BLOCK_TAG_REPORT_BLOCK_STATUS:
      reportBlockRow = ResolveReportBlockRow(config, indexes, indexCount);
      reportBlockRow->reportBlockStatus = (uint8_t) value->data.unsigned32;
      break;

    case BLOCK_TAG_REPORT_BLOCK_DESCRIPTION:
      reportBlockRow = ResolveReportBlockRow(config, indexes, indexCount);
      CopyDescription(&reportBlockRow->reportBlockDescription[0],
                      &reportBlockRow->reportBlockDescriptionLength,
                      &value->data.octetString,
                      sizeof(reportBlockRow->reportBlockDescription));
      break;

    default:
      return NTCIP_ERROR_READ_ONLY;
  }

  EventReportServiceRefreshWorkingConfig(service);
  return NTCIP_ERROR_OK;
}

static const NtcipObjectDescriptor_t kBlockObjects[] =
{
  { kMaxWatchObjectsOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_MAX_WATCH_OBJECTS, GetBlockObject, NULL, NULL },
  { kMaxWatchBlocksOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_MAX_WATCH_BLOCKS, GetBlockObject, NULL, NULL },
  { kWatchIdOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_WATCH_ID, GetBlockObject, NULL, NULL },
  { kWatchStatusOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_WATCH_STATUS, GetBlockObject, SetTestBlockObject,
    SetValueBlockObject },
  { kWatchBlockOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_WATCH_BLOCK, GetBlockObject, SetTestBlockObject,
    SetValueBlockObject },
  { kWatchOidOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OBJECT_ID,
    BLOCK_TAG_WATCH_OID, GetBlockObject, SetTestBlockObject,
    SetValueBlockObject },
  { kWatchBlockNumberOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_WATCH_BLOCK_NUMBER, GetBlockObject, NULL, NULL },
  { kWatchBlockStatusOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_WATCH_BLOCK_STATUS, GetBlockObject, SetTestBlockObject,
    SetValueBlockObject },
  { kWatchBlockDescriptionOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    BLOCK_TAG_WATCH_BLOCK_DESCRIPTION, GetBlockObject, SetTestBlockObject,
    SetValueBlockObject },
  { kWatchBlockValueOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OCTET_STRING,
    BLOCK_TAG_WATCH_BLOCK_VALUE, GetBlockObject, NULL, NULL },
  { kMaxReportObjectsOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_MAX_REPORT_OBJECTS, GetBlockObject, NULL, NULL },
  { kMaxReportBlocksOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_MAX_REPORT_BLOCKS, GetBlockObject, NULL, NULL },
  { kReportIdOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_REPORT_ID, GetBlockObject, NULL, NULL },
  { kReportStatusOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_REPORT_STATUS, GetBlockObject, SetTestBlockObject,
    SetValueBlockObject },
  { kReportBlockOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_REPORT_BLOCK, GetBlockObject, SetTestBlockObject,
    SetValueBlockObject },
  { kReportOidOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OBJECT_ID,
    BLOCK_TAG_REPORT_OID, GetBlockObject, SetTestBlockObject,
    SetValueBlockObject },
  { kReportBlockNumberOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_REPORT_BLOCK_NUMBER, GetBlockObject, NULL, NULL },
  { kReportBlockStatusOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    BLOCK_TAG_REPORT_BLOCK_STATUS, GetBlockObject, SetTestBlockObject,
    SetValueBlockObject },
  { kReportBlockDescriptionOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    BLOCK_TAG_REPORT_BLOCK_DESCRIPTION, GetBlockObject, SetTestBlockObject,
    SetValueBlockObject },
  { kReportBlockValueOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OCTET_STRING,
    BLOCK_TAG_REPORT_BLOCK_VALUE, GetBlockObject, NULL, NULL }
};

void BlockObjectsRegister(NtcipObjectDirectory_t *directory,
                          NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1103.blocks",
    kBlockObjects,
    (uint16_t) (sizeof(kBlockObjects) / sizeof(kBlockObjects[0])),
    context);
}
