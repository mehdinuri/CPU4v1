/* App/Domain/NTCIP/Mib1103v0352/TrapObjects.c */
#include "TrapObjects.h"

#include <string.h>

#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"

enum
{
  TRAP_TAG_TRAP_CONTROL = 1,
  TRAP_TAG_TRAP_DATA = 2,
  TRAP_TAG_TRAP_MGMT_MAX_ENTRIES = 3,
  TRAP_TAG_TRAP_MAX_AGGREGATION_EVENTS = 4,
  TRAP_TAG_TRAP_MAX_AGGREGATION_SIZE = 5,
  TRAP_TAG_TRAP_MGMT_MANAGER_INDEX = 6,
  TRAP_TAG_TRAP_MGMT_MANAGER_POINTER = 7,
  TRAP_TAG_TRAP_MGMT_COMMUNITY_POINTER = 8,
  TRAP_TAG_TRAP_MGMT_APP_PROTOCOL = 9,
  TRAP_TAG_TRAP_MGMT_TRANSPORT_PROTOCOL = 10,
  TRAP_TAG_TRAP_MGMT_PORT = 11,
  TRAP_TAG_TRAP_MGMT_MAX_RETRIES = 12,
  TRAP_TAG_TRAP_MGMT_REPEAT_INTERVAL = 13,
  TRAP_TAG_TRAP_MGMT_DELTA = 14,
  TRAP_TAG_TRAP_MGMT_QUEUE_DEPTH = 15,
  TRAP_TAG_TRAP_MGMT_LINK_STATE = 16,
  TRAP_TAG_TRAP_MGMT_ANTI_STREAM_RATE = 17,
  TRAP_TAG_TRAP_MGMT_ERR_STATUS = 18,
  TRAP_TAG_TRAP_MGMT_LOST_TRAPS = 19,
  TRAP_TAG_TRAP_MGMT_ROW_STATUS = 20,
  TRAP_TAG_TRAP_MGMT_SEQ_NUM = 21,
  TRAP_TAG_TRAP_MGMT_SEQ_NUM_ACK = 22,
  TRAP_TAG_TRAP_DEST_ENABLE = 23,
  TRAP_TAG_TRAP_MODE = 24,
  TRAP_TAG_TRAP_AGGREGATION_TIME = 25,
  TRAP_TAG_TRAP_COUNTER = 26,
  TRAP_TAG_CLEAR_CLASSES = 27,
  TRAP_TAG_CLEAR_CONFIGURATION = 28,
  TRAP_TAG_CLEAR_LOG = 29,
  TRAP_TAG_CLEAR_REPORT_OBJECTS = 30,
  TRAP_TAG_CLEAR_REPORT_BLOCK_TABLE = 31,
  TRAP_TAG_CLEAR_WATCH_OBJECTS = 32,
  TRAP_TAG_CLEAR_WATCH_BLOCK_TABLE = 33,
  TRAP_TAG_CLEAR_TRAP_MGMT_TABLE = 34
};

static const uint32_t kTrapControlOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 1U
};
static const uint32_t kTrapDataOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 2U
};
static const uint32_t kTrapMgmtMaxEntriesOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 3U
};
static const uint32_t kTrapMaxAggregationEventsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 4U
};
static const uint32_t kTrapMaxAggregationSizeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 5U
};
static const uint32_t kTrapMgmtManagerIndexOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 1U
};
static const uint32_t kTrapMgmtManagerPointerOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 2U
};
static const uint32_t kTrapMgmtCommunityPointerOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 3U
};
static const uint32_t kTrapMgmtApplicationProtocolOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 4U
};
static const uint32_t kTrapMgmtTransportProtocolOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 5U
};
static const uint32_t kTrapMgmtPortNumOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 6U
};
static const uint32_t kTrapMgmtMaxRetriesOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 7U
};
static const uint32_t kTrapMgmtRepeatIntervalOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 8U
};
static const uint32_t kTrapMgmtDeltaOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 9U
};
static const uint32_t kTrapMgmtQueueDepthOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 10U
};
static const uint32_t kTrapMgmtLinkStateOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 11U
};
static const uint32_t kTrapMgmtAntiStreamRateOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 12U
};
static const uint32_t kTrapMgmtErrStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 13U
};
static const uint32_t kTrapMgmtLostTrapsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 14U
};
static const uint32_t kTrapMgmtRowStatusOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 15U
};
static const uint32_t kTrapMgmtSeqNumOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 16U
};
static const uint32_t kTrapMgmtSeqNumAckOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 6U, 1U, 17U
};
static const uint32_t kTrapDestEnableOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 7U, 1U, 1U
};
static const uint32_t kTrapModeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 7U, 1U, 2U
};
static const uint32_t kTrapAggregationTimeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 7U, 1U, 3U
};
static const uint32_t kTrapCounterOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 7U, 1U, 4U
};
static const uint32_t kClearClassesOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 8U, 1U
};
static const uint32_t kClearConfigurationOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 8U, 2U
};
static const uint32_t kClearLogOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 8U, 3U
};
static const uint32_t kClearReportObjectsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 8U, 4U
};
static const uint32_t kClearReportBlockTableOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 8U, 5U
};
static const uint32_t kClearWatchObjectsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 8U, 6U
};
static const uint32_t kClearWatchBlockTableOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 8U, 7U
};
static const uint32_t kClearTrapMgmtTableOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 8U, 8U
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

static EventReportTrapMgmtRow_t *ResolveTrapMgmtRow(EventReportConfiguration_t *config,
                                                    const uint32_t *indexes,
                                                    uint8_t indexCount)
{
  if ((config == NULL) || (indexes == NULL) || (indexCount != 1U)
      || (indexes[0] != 1U))
  {
    return NULL;
  }

  return &config->trapMgmtRows[0];
}

static EventReportTrapRow_t *ResolveTrapRow(EventReportConfiguration_t *config,
                                            const uint32_t *indexes,
                                            uint8_t indexCount)
{
  if ((config == NULL) || (indexes == NULL) || (indexCount != 2U)
      || (indexes[0] == 0U)
      || (indexes[0] > EVENT_REPORT_MAX_EVENT_LOG_CONFIGS)
      || (indexes[1] != 1U))
  {
    return NULL;
  }

  return &config->trapRows[indexes[0] - 1U][0];
}

static EventReportConfigRow_t *ResolveConfigRow(EventReportConfiguration_t *config,
                                                uint32_t eventConfigId)
{
  if ((config == NULL) || (eventConfigId == 0U)
      || (eventConfigId > EVENT_REPORT_MAX_EVENT_LOG_CONFIGS))
  {
    return NULL;
  }

  return &config->configs[eventConfigId - 1U];
}

static void ResetTrapRows(EventReportConfiguration_t *config)
{
  uint16_t index;

  if (config == NULL)
  {
    return;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_EVENT_LOG_CONFIGS; index++)
  {
    (void) memset(&config->trapRows[index][0], 0, sizeof(config->trapRows[index][0]));
    config->trapRows[index][0].trapMode = 1U;
  }
}

static void ResetWatchObjectRows(EventReportConfiguration_t *config)
{
  uint8_t index;

  if (config == NULL)
  {
    return;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_WATCH_OBJECTS; index++)
  {
    (void) memset(&config->watchObjectRows[index], 0,
                  sizeof(config->watchObjectRows[index]));
    config->watchObjectRows[index].watchId = (uint8_t) (index + 1U);
    config->watchObjectRows[index].watchBlock = 1U;
  }
}

static void ResetWatchBlockRows(EventReportConfiguration_t *config)
{
  uint8_t index;

  if (config == NULL)
  {
    return;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_WATCH_BLOCKS; index++)
  {
    (void) memset(&config->watchBlockRows[index], 0,
                  sizeof(config->watchBlockRows[index]));
    config->watchBlockRows[index].watchBlockNumber = (uint8_t) (index + 1U);
  }
}

static void ResetReportObjectRows(EventReportConfiguration_t *config)
{
  uint8_t index;

  if (config == NULL)
  {
    return;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_REPORT_OBJECTS; index++)
  {
    (void) memset(&config->reportObjectRows[index], 0,
                  sizeof(config->reportObjectRows[index]));
    config->reportObjectRows[index].reportId = (uint8_t) (index + 1U);
    config->reportObjectRows[index].reportBlock = 1U;
  }
}

static void ResetReportBlockRows(EventReportConfiguration_t *config)
{
  uint8_t index;

  if (config == NULL)
  {
    return;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_REPORT_BLOCKS; index++)
  {
    (void) memset(&config->reportBlockRows[index], 0,
                  sizeof(config->reportBlockRows[index]));
    config->reportBlockRows[index].reportBlockNumber = (uint8_t) (index + 1U);
  }
}

static void ResetConfigRow(EventReportConfiguration_t *config, uint16_t eventConfigId)
{
  EventReportConfigRow_t *row;

  row = ResolveConfigRow(config, eventConfigId);
  if (row == NULL)
  {
    return;
  }

  (void) memset(row, 0, sizeof(*row));
  row->eventConfigID = eventConfigId;
  row->eventConfigClass = 1U;
  row->eventConfigMode = EVENT_REPORT_MODE_ON_CHANGE;
  row->eventConfigAction = EVENT_REPORT_ACTION_DISABLED;
  row->eventConfigStatus = EVENT_REPORT_STATUS_DISABLED;
}

static void ClearConfigSelection(EventReportService_t *service,
                                 EventReportConfiguration_t *config,
                                 uint16_t eventConfigId)
{
  uint16_t index;

  if ((service == NULL) || (config == NULL))
  {
    return;
  }

  if (eventConfigId == 0U)
  {
    for (index = 1U; index <= EVENT_REPORT_MAX_EVENT_LOG_CONFIGS; index++)
    {
      if (config->configs[index - 1U].preconfigured == 0U)
      {
        ResetConfigRow(config, index);
        (void) memset(&config->trapRows[index - 1U][0],
                      0,
                      sizeof(config->trapRows[index - 1U][0]));
        config->trapRows[index - 1U][0].trapMode = 1U;
        (void) EventReportServiceClearEventConfig(service, index);
      }
    }
    return;
  }

  if ((eventConfigId <= EVENT_REPORT_MAX_EVENT_LOG_CONFIGS)
      && (config->configs[eventConfigId - 1U].preconfigured == 0U))
  {
    ResetConfigRow(config, eventConfigId);
    (void) memset(&config->trapRows[eventConfigId - 1U][0],
                  0,
                  sizeof(config->trapRows[eventConfigId - 1U][0]));
    config->trapRows[eventConfigId - 1U][0].trapMode = 1U;
    (void) EventReportServiceClearEventConfig(service, eventConfigId);
  }
}

static void ClearClassSelection(EventReportService_t *service,
                                EventReportConfiguration_t *config,
                                uint8_t eventClass)
{
  uint8_t classIndex;

  if ((service == NULL) || (config == NULL))
  {
    return;
  }

  if (eventClass == 0U)
  {
    for (classIndex = 5U; classIndex <= EVENT_REPORT_MAX_EVENT_CLASSES; classIndex++)
    {
      ClearClassSelection(service, config, classIndex);
    }
    return;
  }

  if ((eventClass <= 4U) || (eventClass > EVENT_REPORT_MAX_EVENT_CLASSES))
  {
    return;
  }

  config->classes[eventClass - 1U].eventClassLimit = 0U;
  config->classes[eventClass - 1U].eventClassClearTime = 0U;
  config->classes[eventClass - 1U].eventClassDescriptionLength = 0U;
  (void) memset(&config->classes[eventClass - 1U].eventClassDescription[0],
                0,
                sizeof(config->classes[eventClass - 1U].eventClassDescription));

  for (classIndex = 1U; classIndex <= EVENT_REPORT_MAX_EVENT_LOG_CONFIGS; classIndex++)
  {
    if ((config->configs[classIndex - 1U].preconfigured == 0U)
        && (config->configs[classIndex - 1U].eventConfigClass == eventClass))
    {
      ClearConfigSelection(service, config, classIndex);
    }
  }

  (void) EventReportServiceClearEventClass(service, eventClass);
}

static NtcipError_t GetTrapObject(void *groupContext,
                                  const NtcipObjectDescriptor_t *descriptor,
                                  const uint32_t *indexes,
                                  uint8_t indexCount,
                                  const NtcipRequestContext_t *requestContext,
                                  NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportService_t *service = GetService(context);
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportTrapMgmtRow_t *trapMgmtRow;
  EventReportTrapRow_t *trapRow;
  const NtcipOctetString_t *trapData;

  (void) requestContext;

  if ((descriptor == NULL) || (value == NULL) || (service == NULL)
      || (config == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
    case TRAP_TAG_TRAP_CONTROL:
      NtcipValueSetUnsigned32(value, config->trapControl);
      return NTCIP_ERROR_OK;

    case TRAP_TAG_TRAP_DATA:
      trapData = EventReportServiceGetLatestTrapData(service);
      return (trapData == NULL)
             ? NTCIP_ERROR_GEN_ERROR
             : NtcipValueSetOctetString(value,
                                        &trapData->bytes[0],
                                        trapData->length);

    case TRAP_TAG_TRAP_MGMT_MAX_ENTRIES:
      NtcipValueSetUnsigned32(value, EVENT_REPORT_TRAP_MGMT_MAX_ENTRIES);
      return NTCIP_ERROR_OK;

    case TRAP_TAG_TRAP_MAX_AGGREGATION_EVENTS:
      NtcipValueSetUnsigned32(value, EVENT_REPORT_TRAP_MAX_AGGREGATION_EVENTS);
      return NTCIP_ERROR_OK;

    case TRAP_TAG_TRAP_MAX_AGGREGATION_SIZE:
      NtcipValueSetUnsigned32(value, EVENT_REPORT_TRAP_MAX_AGGREGATION_SIZE);
      return NTCIP_ERROR_OK;

    case TRAP_TAG_CLEAR_CLASSES:
    case TRAP_TAG_CLEAR_CONFIGURATION:
    case TRAP_TAG_CLEAR_LOG:
    case TRAP_TAG_CLEAR_REPORT_OBJECTS:
    case TRAP_TAG_CLEAR_REPORT_BLOCK_TABLE:
    case TRAP_TAG_CLEAR_WATCH_OBJECTS:
    case TRAP_TAG_CLEAR_WATCH_BLOCK_TABLE:
    case TRAP_TAG_CLEAR_TRAP_MGMT_TABLE:
      NtcipValueSetUnsigned32(value, 0U);
      return NTCIP_ERROR_OK;

    case TRAP_TAG_TRAP_MGMT_MANAGER_INDEX:
    case TRAP_TAG_TRAP_MGMT_MANAGER_POINTER:
    case TRAP_TAG_TRAP_MGMT_COMMUNITY_POINTER:
    case TRAP_TAG_TRAP_MGMT_APP_PROTOCOL:
    case TRAP_TAG_TRAP_MGMT_TRANSPORT_PROTOCOL:
    case TRAP_TAG_TRAP_MGMT_PORT:
    case TRAP_TAG_TRAP_MGMT_MAX_RETRIES:
    case TRAP_TAG_TRAP_MGMT_REPEAT_INTERVAL:
    case TRAP_TAG_TRAP_MGMT_DELTA:
    case TRAP_TAG_TRAP_MGMT_QUEUE_DEPTH:
    case TRAP_TAG_TRAP_MGMT_LINK_STATE:
    case TRAP_TAG_TRAP_MGMT_ANTI_STREAM_RATE:
    case TRAP_TAG_TRAP_MGMT_ERR_STATUS:
    case TRAP_TAG_TRAP_MGMT_LOST_TRAPS:
    case TRAP_TAG_TRAP_MGMT_ROW_STATUS:
    case TRAP_TAG_TRAP_MGMT_SEQ_NUM:
    case TRAP_TAG_TRAP_MGMT_SEQ_NUM_ACK:
      trapMgmtRow = ResolveTrapMgmtRow(config, indexes, indexCount);
      if (trapMgmtRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      switch (descriptor->tag)
      {
        case TRAP_TAG_TRAP_MGMT_MANAGER_INDEX:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtManagerIndex);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_MANAGER_POINTER:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtManagerPointer);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_COMMUNITY_POINTER:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtCommunityNamePointer);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_APP_PROTOCOL:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtApplicationProtocol);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_TRANSPORT_PROTOCOL:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtTransportProtocol);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_PORT:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtPortNum);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_MAX_RETRIES:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtMaxRetries);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_REPEAT_INTERVAL:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtRepeatInterval);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_DELTA:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtDelta);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_QUEUE_DEPTH:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtQueueDepth);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_LINK_STATE:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtLinkStateStatus);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_ANTI_STREAM_RATE:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtAntiStreamRate);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_ERR_STATUS:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtErrStatus);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_LOST_TRAPS:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtLostTraps);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_ROW_STATUS:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtRowStatus);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_SEQ_NUM:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtSeqNum);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MGMT_SEQ_NUM_ACK:
          NtcipValueSetUnsigned32(value, trapMgmtRow->trapMgmtSeqNumAck);
          return NTCIP_ERROR_OK;
        default:
          return NTCIP_ERROR_NOT_FOUND;
      }

    case TRAP_TAG_TRAP_DEST_ENABLE:
    case TRAP_TAG_TRAP_MODE:
    case TRAP_TAG_TRAP_AGGREGATION_TIME:
    case TRAP_TAG_TRAP_COUNTER:
      trapRow = ResolveTrapRow(config, indexes, indexCount);
      if (trapRow == NULL)
      {
        return NTCIP_ERROR_NOT_FOUND;
      }

      switch (descriptor->tag)
      {
        case TRAP_TAG_TRAP_DEST_ENABLE:
          NtcipValueSetUnsigned32(value, trapRow->trapDestEnable);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_MODE:
          NtcipValueSetUnsigned32(value, trapRow->trapMode);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_AGGREGATION_TIME:
          NtcipValueSetUnsigned32(value, trapRow->trapAggregationTime);
          return NTCIP_ERROR_OK;
        case TRAP_TAG_TRAP_COUNTER:
          NtcipValueSetUnsigned32(value, trapRow->trapCounter);
          return NTCIP_ERROR_OK;
        default:
          return NTCIP_ERROR_NOT_FOUND;
      }

    default:
      return NTCIP_ERROR_NOT_FOUND;
  }
}

static NtcipError_t SetTestTrapObject(void *groupContext,
                                      const NtcipObjectDescriptor_t *descriptor,
                                      const uint32_t *indexes,
                                      uint8_t indexCount,
                                      const NtcipRequestContext_t *requestContext,
                                      const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  NtcipError_t error;

  if ((descriptor == NULL) || (value == NULL) || (config == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag == TRAP_TAG_TRAP_MGMT_SEQ_NUM_ACK)
  {
    return (ResolveTrapMgmtRow(config, indexes, indexCount) == NULL)
           ? NTCIP_ERROR_NOT_FOUND : NTCIP_ERROR_OK;
  }

  if ((descriptor->tag >= TRAP_TAG_CLEAR_CLASSES)
      && (descriptor->tag <= TRAP_TAG_CLEAR_TRAP_MGMT_TABLE))
  {
    return NTCIP_ERROR_OK;
  }

  error = ValidateWrite(context, requestContext);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
    case TRAP_TAG_TRAP_CONTROL:
      return (value->data.unsigned32 <= 1U) ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case TRAP_TAG_TRAP_MGMT_MANAGER_POINTER:
      return (value->data.unsigned32 == 1U) ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case TRAP_TAG_TRAP_MGMT_COMMUNITY_POINTER:
      return (value->data.unsigned32 >= 1U)
             && (value->data.unsigned32 <= EVENT_REPORT_COMMUNITY_NAMES_MAX)
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case TRAP_TAG_TRAP_MGMT_APP_PROTOCOL:
    case TRAP_TAG_TRAP_MGMT_TRANSPORT_PROTOCOL:
      return (value->data.unsigned32 >= 1U)
             && (value->data.unsigned32 <= 3U)
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case TRAP_TAG_TRAP_MGMT_PORT:
      return NTCIP_ERROR_OK;

    case TRAP_TAG_TRAP_MGMT_MAX_RETRIES:
    case TRAP_TAG_TRAP_MGMT_REPEAT_INTERVAL:
    case TRAP_TAG_TRAP_MGMT_DELTA:
      return (ResolveTrapMgmtRow(config, indexes, indexCount) == NULL)
             ? NTCIP_ERROR_NOT_FOUND : NTCIP_ERROR_OK;

    case TRAP_TAG_TRAP_MGMT_QUEUE_DEPTH:
      return (value->data.unsigned32 <= 50U)
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case TRAP_TAG_TRAP_MGMT_ANTI_STREAM_RATE:
      return ((value->data.unsigned32 >= 1U)
              && (value->data.unsigned32 <= 255U))
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case TRAP_TAG_TRAP_MGMT_ROW_STATUS:
      return ((value->data.unsigned32 == EVENT_REPORT_ROW_STATUS_INVALID)
              || (value->data.unsigned32 == EVENT_REPORT_ROW_STATUS_ACTIVE))
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case TRAP_TAG_TRAP_DEST_ENABLE:
      return (value->data.unsigned32 <= 1U) ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case TRAP_TAG_TRAP_MODE:
      return ((value->data.unsigned32 >= 1U)
              && (value->data.unsigned32 <= 7U))
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case TRAP_TAG_TRAP_AGGREGATION_TIME:
      return NTCIP_ERROR_OK;

    case TRAP_TAG_CLEAR_CLASSES:
      return ((value->data.unsigned32 == 0U)
              || ((value->data.unsigned32 >= 5U)
                  && (value->data.unsigned32 <= EVENT_REPORT_MAX_EVENT_CLASSES)))
             ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    case TRAP_TAG_CLEAR_CONFIGURATION:
      if ((value->data.unsigned32 == 0U)
          || ((value->data.unsigned32 <= EVENT_REPORT_MAX_EVENT_LOG_CONFIGS)
              && (config->configs[value->data.unsigned32 - 1U].preconfigured == 0U)))
      {
        return NTCIP_ERROR_OK;
      }
      return NTCIP_ERROR_BAD_VALUE;

    case TRAP_TAG_CLEAR_LOG:
    case TRAP_TAG_CLEAR_REPORT_OBJECTS:
    case TRAP_TAG_CLEAR_REPORT_BLOCK_TABLE:
    case TRAP_TAG_CLEAR_WATCH_OBJECTS:
    case TRAP_TAG_CLEAR_WATCH_BLOCK_TABLE:
    case TRAP_TAG_CLEAR_TRAP_MGMT_TABLE:
      return (value->data.unsigned32 <= 1U) ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;

    default:
      return NTCIP_ERROR_READ_ONLY;
  }
}

static NtcipError_t SetValueTrapObject(void *groupContext,
                                       const NtcipObjectDescriptor_t *descriptor,
                                       const uint32_t *indexes,
                                       uint8_t indexCount,
                                       const NtcipRequestContext_t *requestContext,
                                       const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  EventReportService_t *service = GetService(context);
  EventReportConfiguration_t *config = GetWorkingConfig(context);
  EventReportTrapMgmtRow_t *trapMgmtRow;
  EventReportTrapRow_t *trapRow;
  NtcipError_t error;

  error = SetTestTrapObject(groupContext,
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

  trapMgmtRow = ResolveTrapMgmtRow(config, indexes, indexCount);
  trapRow = ResolveTrapRow(config, indexes, indexCount);

  switch (descriptor->tag)
  {
    case TRAP_TAG_TRAP_CONTROL:
      config->trapControl = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;

    case TRAP_TAG_TRAP_MGMT_MANAGER_POINTER:
      trapMgmtRow->trapMgmtManagerPointer = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_MGMT_COMMUNITY_POINTER:
      trapMgmtRow->trapMgmtCommunityNamePointer = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_MGMT_APP_PROTOCOL:
      trapMgmtRow->trapMgmtApplicationProtocol = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_MGMT_TRANSPORT_PROTOCOL:
      trapMgmtRow->trapMgmtTransportProtocol = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_MGMT_PORT:
      trapMgmtRow->trapMgmtPortNum = (uint16_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_MGMT_MAX_RETRIES:
      trapMgmtRow->trapMgmtMaxRetries = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_MGMT_REPEAT_INTERVAL:
      trapMgmtRow->trapMgmtRepeatInterval = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_MGMT_DELTA:
      trapMgmtRow->trapMgmtDelta = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_MGMT_QUEUE_DEPTH:
      trapMgmtRow->trapMgmtQueueDepth = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_MGMT_ANTI_STREAM_RATE:
      trapMgmtRow->trapMgmtAntiStreamRate = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_MGMT_ROW_STATUS:
      trapMgmtRow->trapMgmtRowStatus = (uint8_t) value->data.unsigned32;
      if ((trapMgmtRow->trapMgmtRowStatus == EVENT_REPORT_ROW_STATUS_ACTIVE)
          && (trapMgmtRow->trapMgmtSeqNum == 0U))
      {
        trapMgmtRow->trapMgmtSeqNum = 1U;
      }
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_MGMT_SEQ_NUM_ACK:
      if ((value->data.unsigned32 == 0U)
          || (value->data.unsigned32 == trapMgmtRow->trapMgmtSeqNumAck))
      {
        trapMgmtRow->trapMgmtSeqNumAck = 0U;
        trapMgmtRow->trapMgmtLinkStateStatus = EVENT_REPORT_TRAP_LINK_READY;
        trapMgmtRow->trapMgmtErrStatus = 0U;
      }
      return NTCIP_ERROR_OK;

    case TRAP_TAG_TRAP_DEST_ENABLE:
      trapRow->trapDestEnable = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_MODE:
      trapRow->trapMode = (uint8_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;
    case TRAP_TAG_TRAP_AGGREGATION_TIME:
      trapRow->trapAggregationTime = (uint16_t) value->data.unsigned32;
      return NTCIP_ERROR_OK;

    case TRAP_TAG_CLEAR_CLASSES:
      ClearClassSelection(service, config, (uint8_t) value->data.unsigned32);
      EventReportServiceRefreshWorkingConfig(service);
      return NTCIP_ERROR_OK;

    case TRAP_TAG_CLEAR_CONFIGURATION:
      ClearConfigSelection(service, config, (uint16_t) value->data.unsigned32);
      EventReportServiceRefreshWorkingConfig(service);
      return NTCIP_ERROR_OK;

    case TRAP_TAG_CLEAR_LOG:
      if (value->data.unsigned32 == 1U)
      {
        (void) EventReportServiceClearLog(service);
      }
      return NTCIP_ERROR_OK;

    case TRAP_TAG_CLEAR_REPORT_OBJECTS:
      if (value->data.unsigned32 == 1U)
      {
        ResetReportObjectRows(config);
        ResetReportBlockRows(config);
        EventReportServiceRefreshWorkingConfig(service);
      }
      return NTCIP_ERROR_OK;

    case TRAP_TAG_CLEAR_REPORT_BLOCK_TABLE:
      if (value->data.unsigned32 == 1U)
      {
        ResetReportBlockRows(config);
        EventReportServiceRefreshWorkingConfig(service);
      }
      return NTCIP_ERROR_OK;

    case TRAP_TAG_CLEAR_WATCH_OBJECTS:
      if (value->data.unsigned32 == 1U)
      {
        ResetWatchObjectRows(config);
        ResetWatchBlockRows(config);
        EventReportServiceRefreshWorkingConfig(service);
      }
      return NTCIP_ERROR_OK;

    case TRAP_TAG_CLEAR_WATCH_BLOCK_TABLE:
      if (value->data.unsigned32 == 1U)
      {
        ResetWatchBlockRows(config);
        EventReportServiceRefreshWorkingConfig(service);
      }
      return NTCIP_ERROR_OK;

    case TRAP_TAG_CLEAR_TRAP_MGMT_TABLE:
      if (value->data.unsigned32 == 1U)
      {
        (void) memset(&config->trapMgmtRows[0], 0, sizeof(config->trapMgmtRows[0]));
        ResetTrapRows(config);
      }
      return NTCIP_ERROR_OK;

    default:
      return NTCIP_ERROR_READ_ONLY;
  }
}

static const NtcipObjectDescriptor_t kTrapObjects[] =
{
  { kTrapControlOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_CONTROL, GetTrapObject, SetTestTrapObject, SetValueTrapObject },
  { kTrapDataOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OCTET_STRING,
    TRAP_TAG_TRAP_DATA, GetTrapObject, NULL, NULL },
  { kTrapMgmtMaxEntriesOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_MAX_ENTRIES, GetTrapObject, NULL, NULL },
  { kTrapMaxAggregationEventsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MAX_AGGREGATION_EVENTS, GetTrapObject, NULL, NULL },
  { kTrapMaxAggregationSizeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MAX_AGGREGATION_SIZE, GetTrapObject, NULL, NULL },
  { kTrapMgmtManagerIndexOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_MANAGER_INDEX, GetTrapObject, NULL, NULL },
  { kTrapMgmtManagerPointerOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_MANAGER_POINTER, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kTrapMgmtCommunityPointerOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_COMMUNITY_POINTER, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kTrapMgmtApplicationProtocolOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_APP_PROTOCOL, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kTrapMgmtTransportProtocolOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_TRANSPORT_PROTOCOL, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kTrapMgmtPortNumOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_PORT, GetTrapObject, SetTestTrapObject, SetValueTrapObject },
  { kTrapMgmtMaxRetriesOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_MAX_RETRIES, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kTrapMgmtRepeatIntervalOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_REPEAT_INTERVAL, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kTrapMgmtDeltaOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_DELTA, GetTrapObject, SetTestTrapObject, SetValueTrapObject },
  { kTrapMgmtQueueDepthOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_QUEUE_DEPTH, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kTrapMgmtLinkStateOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_LINK_STATE, GetTrapObject, NULL, NULL },
  { kTrapMgmtAntiStreamRateOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_ANTI_STREAM_RATE, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kTrapMgmtErrStatusOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_ERR_STATUS, GetTrapObject, NULL, NULL },
  { kTrapMgmtLostTrapsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_LOST_TRAPS, GetTrapObject, NULL, NULL },
  { kTrapMgmtRowStatusOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_ROW_STATUS, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kTrapMgmtSeqNumOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_SEQ_NUM, GetTrapObject, NULL, NULL },
  { kTrapMgmtSeqNumAckOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MGMT_SEQ_NUM_ACK, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kTrapDestEnableOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_DEST_ENABLE, GetTrapObject, SetTestTrapObject, SetValueTrapObject },
  { kTrapModeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_MODE, GetTrapObject, SetTestTrapObject, SetValueTrapObject },
  { kTrapAggregationTimeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_AGGREGATION_TIME, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kTrapCounterOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_TRAP_COUNTER, GetTrapObject, NULL, NULL },
  { kClearClassesOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_CLEAR_CLASSES, GetTrapObject, SetTestTrapObject, SetValueTrapObject },
  { kClearConfigurationOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_CLEAR_CONFIGURATION, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kClearLogOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_CLEAR_LOG, GetTrapObject, SetTestTrapObject, SetValueTrapObject },
  { kClearReportObjectsOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_CLEAR_REPORT_OBJECTS, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kClearReportBlockTableOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_CLEAR_REPORT_BLOCK_TABLE, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kClearWatchObjectsOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_CLEAR_WATCH_OBJECTS, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kClearWatchBlockTableOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_CLEAR_WATCH_BLOCK_TABLE, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject },
  { kClearTrapMgmtTableOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    TRAP_TAG_CLEAR_TRAP_MGMT_TABLE, GetTrapObject,
    SetTestTrapObject, SetValueTrapObject }
};

void TrapObjectsRegister(NtcipObjectDirectory_t *directory,
                         NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1103.traps",
    kTrapObjects,
    (uint16_t) (sizeof(kTrapObjects) / sizeof(kTrapObjects[0])),
    context);
}
