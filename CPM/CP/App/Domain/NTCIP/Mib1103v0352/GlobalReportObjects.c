/* App/Domain/NTCIP/Mib1103v0352/GlobalReportObjects.c */
#include "GlobalReportObjects.h"

#include <string.h>

#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"

enum
{
  GLOBAL_REPORT_TAG_MAX_EVENT_LOG_CONFIGS = 1,
  GLOBAL_REPORT_TAG_EVENT_CONFIG_ID = 2,
  GLOBAL_REPORT_TAG_EVENT_CONFIG_CLASS = 3,
  GLOBAL_REPORT_TAG_EVENT_CONFIG_MODE = 4,
  GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_VALUE = 5,
  GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_VALUE2 = 6,
  GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_OID = 7,
  GLOBAL_REPORT_TAG_EVENT_CONFIG_LOG_OID = 8,
  GLOBAL_REPORT_TAG_EVENT_CONFIG_ACTION = 9,
  GLOBAL_REPORT_TAG_EVENT_CONFIG_STATUS = 10,
  GLOBAL_REPORT_TAG_MAX_EVENT_LOG_SIZE = 11,
  GLOBAL_REPORT_TAG_EVENT_LOG_CLASS = 12,
  GLOBAL_REPORT_TAG_EVENT_LOG_NUMBER = 13,
  GLOBAL_REPORT_TAG_EVENT_LOG_ID = 14,
  GLOBAL_REPORT_TAG_EVENT_LOG_TIME = 15,
  GLOBAL_REPORT_TAG_EVENT_LOG_VALUE = 16,
  GLOBAL_REPORT_TAG_EVENT_LOG_TIME_MS = 17,
  GLOBAL_REPORT_TAG_MAX_EVENT_CLASSES = 18,
  GLOBAL_REPORT_TAG_EVENT_CLASS_NUMBER = 19,
  GLOBAL_REPORT_TAG_EVENT_CLASS_LIMIT = 20,
  GLOBAL_REPORT_TAG_EVENT_CLASS_CLEAR_TIME = 21,
  GLOBAL_REPORT_TAG_EVENT_CLASS_DESCRIPTION = 22,
  GLOBAL_REPORT_TAG_EVENT_CLASS_ROWS = 23,
  GLOBAL_REPORT_TAG_EVENT_CLASS_NUM_EVENTS = 24,
  GLOBAL_REPORT_TAG_NUM_EVENTS = 25,
  GLOBAL_REPORT_TAG_EVENT_TIME_LATENCY = 26
};

static const uint32_t kMaxEventLogConfigsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 1U
};
static const uint32_t kEventConfigIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 2U, 1U, 1U
};
static const uint32_t kEventConfigClassOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 2U, 1U, 2U
};
static const uint32_t kEventConfigModeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 2U, 1U, 3U
};
static const uint32_t kEventConfigCompareValueOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 2U, 1U, 4U
};
static const uint32_t kEventConfigCompareValue2Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 2U, 1U, 5U
};
static const uint32_t kEventConfigCompareOidOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 2U, 1U, 6U
};
static const uint32_t kEventConfigLogOidOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 2U, 1U, 7U
};
static const uint32_t kEventConfigActionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 2U, 1U, 8U
};
static const uint32_t kEventConfigStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 2U, 1U, 9U
};
static const uint32_t kMaxEventLogSizeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 3U
};
static const uint32_t kEventLogClassOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 4U, 1U, 1U
};
static const uint32_t kEventLogNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 4U, 1U, 2U
};
static const uint32_t kEventLogIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 4U, 1U, 3U
};
static const uint32_t kEventLogTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 4U, 1U, 4U
};
static const uint32_t kEventLogValueOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 4U, 1U, 5U
};
static const uint32_t kEventLogTimeMsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 4U, 1U, 6U
};
static const uint32_t kMaxEventClassesOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 5U
};
static const uint32_t kEventClassNumberOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 6U, 1U, 1U
};
static const uint32_t kEventClassLimitOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 6U, 1U, 2U
};
static const uint32_t kEventClassClearTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 6U, 1U, 3U
};
static const uint32_t kEventClassDescriptionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 6U, 1U, 4U
};
static const uint32_t kEventClassRowsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 6U, 1U, 5U
};
static const uint32_t kEventClassNumEventsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 6U, 1U, 6U
};
static const uint32_t kNumEventsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 7U
};
static const uint32_t kEventTimeLatencyOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 4U, 8U
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

static EventReportClassConfig_t *ResolveClassRow(EventReportConfiguration_t *config,
                                                 const uint32_t *indexes,
                                                 uint8_t indexCount)
{
  uint32_t rowIndex;

  if ((config == NULL) || (indexes == NULL) || (indexCount != 1U))
  {
    return NULL;
  }

  rowIndex = indexes[0];
  if ((rowIndex == 0U) || (rowIndex > EVENT_REPORT_MAX_EVENT_CLASSES))
  {
    return NULL;
  }

  return &config->classes[rowIndex - 1U];
}

static EventReportConfigRow_t *ResolveConfigRow(EventReportConfiguration_t *config,
                                                const uint32_t *indexes,
                                                uint8_t indexCount)
{
  uint32_t rowIndex;

  if ((config == NULL) || (indexes == NULL) || (indexCount != 1U))
  {
    return NULL;
  }

  rowIndex = indexes[0];
  if ((rowIndex == 0U) || (rowIndex > EVENT_REPORT_MAX_EVENT_LOG_CONFIGS))
  {
    return NULL;
  }

  return &config->configs[rowIndex - 1U];
}

static uint8_t LoadLogRecord(const EventReportService_t *service,
                             const uint32_t *indexes,
                             uint8_t indexCount,
                             EventReportLogRecord_t *record)
{
  if ((service == NULL) || (indexes == NULL) || (record == NULL)
      || (indexCount != 2U))
  {
    return 0U;
  }

  if ((indexes[0] == 0U) || (indexes[0] > EVENT_REPORT_MAX_EVENT_CLASSES)
      || (indexes[1] == 0U))
  {
    return 0U;
  }

  return EventReportServiceReadLogByClassNumber(service,
                                                (uint8_t) indexes[0],
                                                (uint8_t) indexes[1],
                                                record);
}

static uint16_t ComputeClassLimitSum(const EventReportConfiguration_t *config,
                                     uint8_t replacedClass,
                                     uint8_t replacementLimit)
{
  uint16_t sum = 0U;
  uint8_t index;

  if (config == NULL)
  {
    return 0U;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_EVENT_CLASSES; index++)
  {
    if ((uint8_t) (index + 1U) == replacedClass)
    {
      sum = (uint16_t) (sum + replacementLimit);
    }
    else
    {
      sum = (uint16_t) (sum + config->classes[index].eventClassLimit);
    }
  }

  return sum;
}

static void CopyOid(NtcipOid_t *target, const NtcipOid_t *source)
{
  if ((target == NULL) || (source == NULL))
  {
    return;
  }

  (void) memset(target, 0, sizeof(*target));
  target->length = source->length;
  if (source->length > 0U)
  {
    (void) memcpy(&target->elements[0],
                  &source->elements[0],
                  (size_t) source->length * sizeof(uint32_t));
  }
}

static NtcipError_t GetGlobalReportObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportService_t *service = GetService(context);
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportClassConfig_t *classRow;
  EventReportConfigRow_t *configRow;
  EventReportLogRecord_t logRecord;

  (void) requestContext;

  if ((descriptor == NULL) || (value == NULL) || (service == NULL)
      || (config == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
    case GLOBAL_REPORT_TAG_MAX_EVENT_LOG_CONFIGS:
      NtcipValueSetUnsigned32(value, EVENT_REPORT_MAX_EVENT_LOG_CONFIGS);
      return NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_MAX_EVENT_LOG_SIZE:
      NtcipValueSetUnsigned32(value, EVENT_REPORT_MAX_EVENT_LOG_SIZE);
      return NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_MAX_EVENT_CLASSES:
      NtcipValueSetUnsigned32(value, EVENT_REPORT_MAX_EVENT_CLASSES);
      return NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_NUM_EVENTS:
      NtcipValueSetUnsigned32(value, EventReportServiceGetNumEvents(service));
      return NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_TIME_LATENCY:
      NtcipValueSetUnsigned32(value, 1000U);
      return NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CLASS_NUMBER:
    case GLOBAL_REPORT_TAG_EVENT_CLASS_LIMIT:
    case GLOBAL_REPORT_TAG_EVENT_CLASS_CLEAR_TIME:
    case GLOBAL_REPORT_TAG_EVENT_CLASS_DESCRIPTION:
    case GLOBAL_REPORT_TAG_EVENT_CLASS_ROWS:
    case GLOBAL_REPORT_TAG_EVENT_CLASS_NUM_EVENTS:
      classRow = ResolveClassRow(config, indexes, indexCount);
      if (classRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      switch (descriptor->tag)
      {
        case GLOBAL_REPORT_TAG_EVENT_CLASS_NUMBER:
          NtcipValueSetUnsigned32(value, classRow->eventClassNumber);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_CLASS_LIMIT:
          NtcipValueSetUnsigned32(value, classRow->eventClassLimit);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_CLASS_CLEAR_TIME:
          NtcipValueSetUnsigned32(value, classRow->eventClassClearTime);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_CLASS_DESCRIPTION:
          return NtcipValueSetOctetString(value,
                                          &classRow->eventClassDescription[0],
                                          classRow->eventClassDescriptionLength);

        case GLOBAL_REPORT_TAG_EVENT_CLASS_ROWS:
          NtcipValueSetUnsigned32(value,
                                  EventReportServiceGetClassCount(
                                    service,
                                    classRow->eventClassNumber));
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_CLASS_NUM_EVENTS:
          NtcipValueSetUnsigned32(
            value,
            (uint16_t) service->classEventCounters[classRow->eventClassNumber - 1U]);
          return NTCIP_ERROR_OK;

        default:
          return NTCIP_ERROR_NOT_FOUND;
      }

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_ID:
    case GLOBAL_REPORT_TAG_EVENT_CONFIG_CLASS:
    case GLOBAL_REPORT_TAG_EVENT_CONFIG_MODE:
    case GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_VALUE:
    case GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_VALUE2:
    case GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_OID:
    case GLOBAL_REPORT_TAG_EVENT_CONFIG_LOG_OID:
    case GLOBAL_REPORT_TAG_EVENT_CONFIG_ACTION:
    case GLOBAL_REPORT_TAG_EVENT_CONFIG_STATUS:
      configRow = ResolveConfigRow(config, indexes, indexCount);
      if (configRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      switch (descriptor->tag)
      {
        case GLOBAL_REPORT_TAG_EVENT_CONFIG_ID:
          NtcipValueSetUnsigned32(value, configRow->eventConfigID);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_CONFIG_CLASS:
          NtcipValueSetUnsigned32(value, configRow->eventConfigClass);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_CONFIG_MODE:
          NtcipValueSetUnsigned32(value, configRow->eventConfigMode);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_VALUE:
          NtcipValueSetSigned32(value, configRow->eventConfigCompareValue);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_VALUE2:
          NtcipValueSetSigned32(value, configRow->eventConfigCompareValue2);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_OID:
          return NtcipValueSetObjectId(value,
                                       &configRow->eventConfigCompareOid.elements[0],
                                       configRow->eventConfigCompareOid.length);

        case GLOBAL_REPORT_TAG_EVENT_CONFIG_LOG_OID:
          return NtcipValueSetObjectId(value,
                                       &configRow->eventConfigLogOid.elements[0],
                                       configRow->eventConfigLogOid.length);

        case GLOBAL_REPORT_TAG_EVENT_CONFIG_ACTION:
          NtcipValueSetUnsigned32(value, configRow->eventConfigAction);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_CONFIG_STATUS:
          NtcipValueSetUnsigned32(value, configRow->eventConfigStatus);
          return NTCIP_ERROR_OK;

        default:
          return NTCIP_ERROR_NOT_FOUND;
      }

    case GLOBAL_REPORT_TAG_EVENT_LOG_CLASS:
    case GLOBAL_REPORT_TAG_EVENT_LOG_NUMBER:
    case GLOBAL_REPORT_TAG_EVENT_LOG_ID:
    case GLOBAL_REPORT_TAG_EVENT_LOG_TIME:
    case GLOBAL_REPORT_TAG_EVENT_LOG_VALUE:
    case GLOBAL_REPORT_TAG_EVENT_LOG_TIME_MS:
      if (LoadLogRecord(service, indexes, indexCount, &logRecord) == 0U)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      switch (descriptor->tag)
      {
        case GLOBAL_REPORT_TAG_EVENT_LOG_CLASS:
          NtcipValueSetUnsigned32(value, logRecord.eventLogClass);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_LOG_NUMBER:
          NtcipValueSetUnsigned32(value, indexes[1]);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_LOG_ID:
          NtcipValueSetUnsigned32(value, logRecord.eventLogID);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_LOG_TIME:
          NtcipValueSetUnsigned32(value, logRecord.eventLogTime);
          return NTCIP_ERROR_OK;

        case GLOBAL_REPORT_TAG_EVENT_LOG_VALUE:
          return NtcipValueSetOpaque(value,
                                     &logRecord.eventLogValue[0],
                                     logRecord.eventLogValueLength);

        case GLOBAL_REPORT_TAG_EVENT_LOG_TIME_MS:
          NtcipValueSetUnsigned32(value, logRecord.eventLogTimeMilliseconds);
          return NTCIP_ERROR_OK;

        default:
          return NTCIP_ERROR_NOT_FOUND;
      }

    default:
      return NTCIP_ERROR_NOT_FOUND;
  }
}

static NtcipError_t SetTestGlobalReportObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportClassConfig_t *classRow;
  EventReportConfigRow_t *configRow;
  NtcipError_t error;
  uint32_t unsignedValue;

  if ((descriptor == NULL) || (value == NULL) || (config == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  error = ValidateWrite(context, requestContext);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  unsignedValue = value->data.unsigned32;

  switch (descriptor->tag)
  {
    case GLOBAL_REPORT_TAG_EVENT_CLASS_LIMIT:
      classRow = ResolveClassRow(config, indexes, indexCount);
      if (classRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      if ((unsignedValue > 255U)
          || (ComputeClassLimitSum(config,
                                   classRow->eventClassNumber,
                                   (uint8_t) unsignedValue)
              > EVENT_REPORT_MAX_EVENT_LOG_SIZE))
      {
        return NTCIP_ERROR_GEN_ERROR;
      }
      return NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CLASS_CLEAR_TIME:
      return (ResolveClassRow(config, indexes, indexCount) == NULL)
             ? NTCIP_ERROR_NOT_FOUND : NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CLASS_DESCRIPTION:
      classRow = ResolveClassRow(config, indexes, indexCount);
      if (classRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      return (value->data.octetString.length
              <= sizeof(classRow->eventClassDescription))
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_CLASS:
      configRow = ResolveConfigRow(config, indexes, indexCount);
      if ((configRow == NULL) || (unsignedValue == 0U)
          || (unsignedValue > EVENT_REPORT_MAX_EVENT_CLASSES))
      {
        return (configRow == NULL) ? NTCIP_ERROR_NOT_FOUND : NTCIP_ERROR_BAD_VALUE;
      }
      return NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_MODE:
      configRow = ResolveConfigRow(config, indexes, indexCount);
      if (configRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
      return ((unsignedValue >= EVENT_REPORT_MODE_ON_CHANGE)
              && (unsignedValue <= EVENT_REPORT_MODE_ANDED_WITH_VALUE))
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_VALUE:
    case GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_VALUE2:
      return (ResolveConfigRow(config, indexes, indexCount) == NULL)
             ? NTCIP_ERROR_NOT_FOUND : NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_OID:
    case GLOBAL_REPORT_TAG_EVENT_CONFIG_LOG_OID:
      return (ResolveConfigRow(config, indexes, indexCount) == NULL)
             ? NTCIP_ERROR_NOT_FOUND : NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_ACTION:
      configRow = ResolveConfigRow(config, indexes, indexCount);
      if (configRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
      return ((unsignedValue == EVENT_REPORT_ACTION_DISABLED)
              || (unsignedValue == EVENT_REPORT_ACTION_LOG))
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    default:
      return NTCIP_ERROR_READ_ONLY;
  }
}

static NtcipError_t SetValueGlobalReportObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportService_t *service = GetService(context);
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportClassConfig_t *classRow;
  EventReportConfigRow_t *configRow;
  NtcipError_t error;

  error = SetTestGlobalReportObject(groupContext,
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
    case GLOBAL_REPORT_TAG_EVENT_CLASS_LIMIT:
      classRow = ResolveClassRow(config, indexes, indexCount);
      classRow->eventClassLimit = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CLASS_CLEAR_TIME:
      classRow = ResolveClassRow(config, indexes, indexCount);
      classRow->eventClassClearTime = value->data.unsigned32;
      return NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CLASS_DESCRIPTION:
      classRow = ResolveClassRow(config, indexes, indexCount);
      (void) memset(&classRow->eventClassDescription[0],
                    0,
                    sizeof(classRow->eventClassDescription));
      classRow->eventClassDescriptionLength =
        (uint8_t) value->data.octetString.length;
      if (classRow->eventClassDescriptionLength > 0U)
      {
        (void) memcpy(&classRow->eventClassDescription[0],
                      &value->data.octetString.bytes[0],
                      classRow->eventClassDescriptionLength);
      }
      return NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_CLASS:
      configRow = ResolveConfigRow(config, indexes, indexCount);
      configRow->eventConfigClass = (uint8_t) value->data.unsigned32;
      EventReportServiceRefreshWorkingConfig(service);
      return (configRow->eventConfigStatus == EVENT_REPORT_STATUS_ERROR)
             ? NTCIP_ERROR_BAD_VALUE : NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_MODE:
      configRow = ResolveConfigRow(config, indexes, indexCount);
      configRow->eventConfigMode = (uint8_t) value->data.unsigned32;
      EventReportServiceRefreshWorkingConfig(service);
      return (configRow->eventConfigStatus == EVENT_REPORT_STATUS_ERROR)
             ? NTCIP_ERROR_BAD_VALUE : NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_VALUE:
      configRow = ResolveConfigRow(config, indexes, indexCount);
      configRow->eventConfigCompareValue = value->data.signed32;
      EventReportServiceRefreshWorkingConfig(service);
      return (configRow->eventConfigStatus == EVENT_REPORT_STATUS_ERROR)
             ? NTCIP_ERROR_BAD_VALUE : NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_VALUE2:
      configRow = ResolveConfigRow(config, indexes, indexCount);
      configRow->eventConfigCompareValue2 = value->data.signed32;
      EventReportServiceRefreshWorkingConfig(service);
      return (configRow->eventConfigStatus == EVENT_REPORT_STATUS_ERROR)
             ? NTCIP_ERROR_BAD_VALUE : NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_OID:
      configRow = ResolveConfigRow(config, indexes, indexCount);
      CopyOid(&configRow->eventConfigCompareOid, &value->data.objectId);
      EventReportServiceRefreshWorkingConfig(service);
      return (configRow->eventConfigStatus == EVENT_REPORT_STATUS_ERROR)
             ? NTCIP_ERROR_BAD_VALUE : NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_LOG_OID:
      configRow = ResolveConfigRow(config, indexes, indexCount);
      CopyOid(&configRow->eventConfigLogOid, &value->data.objectId);
      EventReportServiceRefreshWorkingConfig(service);
      return (configRow->eventConfigStatus == EVENT_REPORT_STATUS_ERROR)
             ? NTCIP_ERROR_BAD_VALUE : NTCIP_ERROR_OK;

    case GLOBAL_REPORT_TAG_EVENT_CONFIG_ACTION:
      configRow = ResolveConfigRow(config, indexes, indexCount);
      configRow->eventConfigAction = (uint8_t) value->data.unsigned32;
      EventReportServiceRefreshWorkingConfig(service);
      return (configRow->eventConfigStatus == EVENT_REPORT_STATUS_ERROR)
             ? NTCIP_ERROR_BAD_VALUE : NTCIP_ERROR_OK;

    default:
      return NTCIP_ERROR_READ_ONLY;
  }
}

static const NtcipObjectDescriptor_t kGlobalReportObjects[] =
{
  { kMaxEventLogConfigsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_MAX_EVENT_LOG_CONFIGS, GetGlobalReportObject, NULL, NULL },
  { kEventConfigIdOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_CONFIG_ID, GetGlobalReportObject, NULL, NULL },
  { kEventConfigClassOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_CONFIG_CLASS, GetGlobalReportObject,
    SetTestGlobalReportObject, SetValueGlobalReportObject },
  { kEventConfigModeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_CONFIG_MODE, GetGlobalReportObject,
    SetTestGlobalReportObject, SetValueGlobalReportObject },
  { kEventConfigCompareValueOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_SIGNED32,
    GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_VALUE, GetGlobalReportObject,
    SetTestGlobalReportObject, SetValueGlobalReportObject },
  { kEventConfigCompareValue2Oid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_SIGNED32,
    GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_VALUE2, GetGlobalReportObject,
    SetTestGlobalReportObject, SetValueGlobalReportObject },
  { kEventConfigCompareOidOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OBJECT_ID,
    GLOBAL_REPORT_TAG_EVENT_CONFIG_COMPARE_OID, GetGlobalReportObject,
    SetTestGlobalReportObject, SetValueGlobalReportObject },
  { kEventConfigLogOidOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OBJECT_ID,
    GLOBAL_REPORT_TAG_EVENT_CONFIG_LOG_OID, GetGlobalReportObject,
    SetTestGlobalReportObject, SetValueGlobalReportObject },
  { kEventConfigActionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_CONFIG_ACTION, GetGlobalReportObject,
    SetTestGlobalReportObject, SetValueGlobalReportObject },
  { kEventConfigStatusOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_CONFIG_STATUS, GetGlobalReportObject, NULL, NULL },
  { kMaxEventLogSizeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_MAX_EVENT_LOG_SIZE, GetGlobalReportObject, NULL, NULL },
  { kEventLogClassOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_LOG_CLASS, GetGlobalReportObject, NULL, NULL },
  { kEventLogNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_LOG_NUMBER, GetGlobalReportObject, NULL, NULL },
  { kEventLogIdOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_LOG_ID, GetGlobalReportObject, NULL, NULL },
  { kEventLogTimeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_LOG_TIME, GetGlobalReportObject, NULL, NULL },
  { kEventLogValueOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OPAQUE,
    GLOBAL_REPORT_TAG_EVENT_LOG_VALUE, GetGlobalReportObject, NULL, NULL },
  { kEventLogTimeMsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_LOG_TIME_MS, GetGlobalReportObject, NULL, NULL },
  { kMaxEventClassesOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_MAX_EVENT_CLASSES, GetGlobalReportObject, NULL, NULL },
  { kEventClassNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_CLASS_NUMBER, GetGlobalReportObject, NULL, NULL },
  { kEventClassLimitOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_CLASS_LIMIT, GetGlobalReportObject,
    SetTestGlobalReportObject, SetValueGlobalReportObject },
  { kEventClassClearTimeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_CLASS_CLEAR_TIME, GetGlobalReportObject,
    SetTestGlobalReportObject, SetValueGlobalReportObject },
  { kEventClassDescriptionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    GLOBAL_REPORT_TAG_EVENT_CLASS_DESCRIPTION, GetGlobalReportObject,
    SetTestGlobalReportObject, SetValueGlobalReportObject },
  { kEventClassRowsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_CLASS_ROWS, GetGlobalReportObject, NULL, NULL },
  { kEventClassNumEventsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_CLASS_NUM_EVENTS, GetGlobalReportObject, NULL,
    NULL },
  { kNumEventsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_NUM_EVENTS, GetGlobalReportObject, NULL, NULL },
  { kEventTimeLatencyOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    GLOBAL_REPORT_TAG_EVENT_TIME_LATENCY, GetGlobalReportObject, NULL, NULL }
};

void GlobalReportObjectsRegister(NtcipObjectDirectory_t *directory,
                                 NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1103.globalReport",
    kGlobalReportObjects,
    (uint16_t) (sizeof(kGlobalReportObjects) / sizeof(kGlobalReportObjects[0])),
    context);
}
