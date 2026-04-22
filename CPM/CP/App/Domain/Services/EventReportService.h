/* App/Domain/Services/EventReportService.h
 *
 * CP-owned NTCIP 1103 event reporting state, evaluation, and log retention.
 */
#ifndef EVENT_REPORT_SERVICE_H
#define EVENT_REPORT_SERVICE_H

#include <stdint.h>

#include "Domain/Intersection/GlobalTimeManagementService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Ports/ILogRepositoryPort.h"

#define EVENT_REPORT_MAX_EVENT_CLASSES 16U
#define EVENT_REPORT_MAX_EVENT_LOG_CONFIGS 64U
#define EVENT_REPORT_MAX_EVENT_LOG_SIZE 256U
#define EVENT_REPORT_EVENT_VALUE_MAX_LENGTH 46U
#define EVENT_REPORT_COMMUNITY_NAMES_MAX 3U
#define EVENT_REPORT_LOGICAL_NAME_MAX_ENTRIES 1U
#define EVENT_REPORT_TRAP_MGMT_MAX_ENTRIES 1U
#define EVENT_REPORT_TRAP_MAX_AGGREGATION_EVENTS 1U
#define EVENT_REPORT_TRAP_MAX_AGGREGATION_SIZE 64U
#define EVENT_REPORT_MAX_WATCH_OBJECTS 32U
#define EVENT_REPORT_MAX_WATCH_BLOCKS 8U
#define EVENT_REPORT_MAX_REPORT_OBJECTS 32U
#define EVENT_REPORT_MAX_REPORT_BLOCKS 8U
#define EVENT_REPORT_BLOCK_DESCRIPTION_MAX_LENGTH 20U
#define EVENT_REPORT_PERSIST_QUEUE_CAPACITY 16U

#define EVENT_REPORT_EVENT_ID_POWER_ON 1U
#define EVENT_REPORT_EVENT_ID_STANDBY 2U
#define EVENT_REPORT_EVENT_ID_DOOR_OPEN 3U
#define EVENT_REPORT_EVENT_ID_DOOR_CLOSED 4U
#define EVENT_REPORT_EVENT_ID_CPMP_LINK_DEGRADED 5U
#define EVENT_REPORT_EVENT_ID_CPMP_LINK_RESTORED 6U
#define EVENT_REPORT_EVENT_ID_MP_EVENT 7U

typedef enum
{
  EVENT_REPORT_MODE_OTHER = 1U,
  EVENT_REPORT_MODE_ON_CHANGE = 2U,
  EVENT_REPORT_MODE_GREATER_THAN_VALUE = 3U,
  EVENT_REPORT_MODE_SMALLER_THAN_VALUE = 4U,
  EVENT_REPORT_MODE_HYSTERESIS_BOUND = 5U,
  EVENT_REPORT_MODE_PERIODIC = 6U,
  EVENT_REPORT_MODE_ANDED_WITH_VALUE = 7U
} EventReportMode_t;

typedef enum
{
  EVENT_REPORT_ACTION_OTHER = 1U,
  EVENT_REPORT_ACTION_DISABLED = 2U,
  EVENT_REPORT_ACTION_LOG = 3U
} EventReportAction_t;

typedef enum
{
  EVENT_REPORT_STATUS_OTHER = 1U,
  EVENT_REPORT_STATUS_DISABLED = 2U,
  EVENT_REPORT_STATUS_LOG = 3U,
  EVENT_REPORT_STATUS_ERROR = 4U
} EventReportStatus_t;

typedef enum
{
  EVENT_REPORT_ROW_STATUS_INVALID = 1U,
  EVENT_REPORT_ROW_STATUS_ACTIVE = 2U
} EventReportRowStatus_t;

typedef enum
{
  EVENT_REPORT_TRAP_LINK_OTHER = 1U,
  EVENT_REPORT_TRAP_LINK_READY = 2U,
  EVENT_REPORT_TRAP_LINK_PENDING = 3U,
  EVENT_REPORT_TRAP_LINK_ERROR = 4U
} EventReportTrapLinkState_t;

typedef struct
{
  uint8_t eventLogClass;
  uint16_t eventLogID;
  uint32_t eventLogTime;
  uint16_t eventLogTimeMilliseconds;
  uint8_t eventLogValueLength;
  uint8_t eventLogValue[EVENT_REPORT_EVENT_VALUE_MAX_LENGTH];
} EventReportLogRecord_t;

typedef struct
{
  uint8_t eventClassNumber;
  uint8_t eventClassLimit;
  uint32_t eventClassClearTime;
  uint8_t eventClassDescriptionLength;
  uint8_t eventClassDescription[32];
} EventReportClassConfig_t;

typedef struct
{
  uint16_t eventConfigID;
  uint8_t eventConfigClass;
  uint8_t eventConfigMode;
  int32_t eventConfigCompareValue;
  int32_t eventConfigCompareValue2;
  NtcipOid_t eventConfigCompareOid;
  NtcipOid_t eventConfigLogOid;
  uint8_t eventConfigAction;
  uint8_t eventConfigStatus;
  uint8_t preconfigured;
} EventReportConfigRow_t;

typedef struct
{
  uint8_t communityNameIndex;
  uint8_t communityNameLength;
  uint8_t communityNameUser[16];
  uint32_t communityNameAccessMask;
} EventReportCommunityRow_t;

typedef struct
{
  uint8_t logicalNameTranslationIndex;
  uint8_t logicalNameLength;
  uint8_t logicalName[32];
  uint8_t networkAddress[4];
  uint8_t status;
} EventReportLogicalNameRow_t;

typedef struct
{
  uint8_t trapDestEnable;
  uint8_t trapMode;
  uint16_t trapAggregationTime;
  uint32_t trapCounter;
} EventReportTrapRow_t;

typedef struct
{
  uint8_t watchId;
  uint8_t watchStatus;
  uint8_t watchBlock;
  NtcipOid_t watchOid;
} EventReportWatchObjectRow_t;

typedef struct
{
  uint8_t watchBlockNumber;
  uint8_t watchBlockStatus;
  uint8_t watchBlockDescriptionLength;
  uint8_t watchBlockDescription[EVENT_REPORT_BLOCK_DESCRIPTION_MAX_LENGTH];
} EventReportWatchBlockRow_t;

typedef struct
{
  uint8_t reportId;
  uint8_t reportStatus;
  uint8_t reportBlock;
  NtcipOid_t reportOid;
} EventReportReportObjectRow_t;

typedef struct
{
  uint8_t reportBlockNumber;
  uint8_t reportBlockStatus;
  uint8_t reportBlockDescriptionLength;
  uint8_t reportBlockDescription[EVENT_REPORT_BLOCK_DESCRIPTION_MAX_LENGTH];
} EventReportReportBlockRow_t;

typedef struct
{
  uint8_t trapMgmtManagerIndex;
  uint8_t trapMgmtManagerPointer;
  uint8_t trapMgmtCommunityNamePointer;
  uint8_t trapMgmtApplicationProtocol;
  uint8_t trapMgmtTransportProtocol;
  uint16_t trapMgmtPortNum;
  uint8_t trapMgmtMaxRetries;
  uint8_t trapMgmtRepeatInterval;
  uint8_t trapMgmtDelta;
  uint8_t trapMgmtQueueDepth;
  uint8_t trapMgmtLinkStateStatus;
  uint8_t trapMgmtAntiStreamRate;
  uint8_t trapMgmtErrStatus;
  uint32_t trapMgmtLostTraps;
  uint8_t trapMgmtRowStatus;
  uint8_t trapMgmtSeqNum;
  uint8_t trapMgmtSeqNumAck;
} EventReportTrapMgmtRow_t;

typedef struct
{
  uint8_t communityNameAdminLength;
  uint8_t communityNameAdmin[16];
  uint8_t trapControl;
  EventReportClassConfig_t classes[EVENT_REPORT_MAX_EVENT_CLASSES];
  EventReportConfigRow_t configs[EVENT_REPORT_MAX_EVENT_LOG_CONFIGS];
  EventReportCommunityRow_t communityRows[EVENT_REPORT_COMMUNITY_NAMES_MAX];
  EventReportLogicalNameRow_t logicalNameRows[
    EVENT_REPORT_LOGICAL_NAME_MAX_ENTRIES];
  EventReportWatchObjectRow_t watchObjectRows[EVENT_REPORT_MAX_WATCH_OBJECTS];
  EventReportWatchBlockRow_t watchBlockRows[EVENT_REPORT_MAX_WATCH_BLOCKS];
  EventReportReportObjectRow_t reportObjectRows[EVENT_REPORT_MAX_REPORT_OBJECTS];
  EventReportReportBlockRow_t reportBlockRows[EVENT_REPORT_MAX_REPORT_BLOCKS];
  EventReportTrapMgmtRow_t trapMgmtRows[EVENT_REPORT_TRAP_MGMT_MAX_ENTRIES];
  EventReportTrapRow_t trapRows[EVENT_REPORT_MAX_EVENT_LOG_CONFIGS]
                              [EVENT_REPORT_TRAP_MGMT_MAX_ENTRIES];
} EventReportConfiguration_t;

typedef struct
{
  uint32_t powerOnCount;
  uint32_t resetCause;
  uint32_t standbyCount;
  uint32_t doorOpenCount;
  uint32_t doorClosedCount;
  uint32_t cpMpLinkDegradedCount;
  uint32_t cpMpLinkRestoredCount;
  uint32_t mpEventCount;
  uint8_t mpEventData[7];
} EventReportEventSourceState_t;

typedef struct
{
  uint8_t lastStatus;
  uint8_t lastValueValid;
  uint32_t lastTrueSinceMs;
  uint32_t lastPeriodicMs;
  NtcipValue_t lastValue;
} EventReportRuntimeRow_t;

typedef struct
{
  uint8_t kind;
  EventReportLogRecord_t record;
} EventReportPersistenceOp_t;

typedef struct
{
  EventReportConfiguration_t activeConfig;
  EventReportConfiguration_t candidateConfig;
  EventReportLogRecord_t logRecords[EVENT_REPORT_MAX_EVENT_LOG_SIZE];
  EventReportEventSourceState_t eventSources;
  EventReportRuntimeRow_t runtimeRows[EVENT_REPORT_MAX_EVENT_LOG_CONFIGS];
  EventReportPersistenceOp_t persistQueue[EVENT_REPORT_PERSIST_QUEUE_CAPACITY];
  ILogRepositoryPort_t *logRepositoryPort;
  const NtcipObjectDirectory_t *objectDirectory;
  GlobalTimeManagementService_t *globalTimeManagementService;
  NtcipOctetString_t latestTrapData;
  uint32_t classEventCounters[EVENT_REPORT_MAX_EVENT_CLASSES];
  uint16_t writeIndex;
  uint16_t count;
  uint16_t totalEvents;
  uint8_t transactionActive;
  uint8_t transactionVerified;
  uint8_t primed;
  uint8_t doorStateValid;
  uint8_t doorOpen;
  uint8_t cpMpLinkHealthyValid;
  uint8_t cpMpLinkHealthy;
  uint8_t persistHead;
  uint8_t persistTail;
  uint8_t persistCount;
  uint32_t persistDropped;
  uint8_t fullSyncPending;
  uint8_t trapPending;
} EventReportService_t;

void EventReportServiceInit(EventReportService_t *service);
void EventReportServiceBindLogRepository(EventReportService_t *service,
                                         ILogRepositoryPort_t *logRepositoryPort);
void EventReportServiceBindObjectDirectory(
  EventReportService_t *service,
  const NtcipObjectDirectory_t *objectDirectory);
void EventReportServiceBindGlobalTimeManagementService(
  EventReportService_t *service,
  GlobalTimeManagementService_t *globalTimeManagementService);
void EventReportServiceLoadPersistedLog(EventReportService_t *service);
void EventReportServicePrime(EventReportService_t *service);
void EventReportServiceStep(EventReportService_t *service, uint32_t nowMs);
void EventReportServiceRefreshWorkingConfig(EventReportService_t *service);
void EventReportServiceApplySnmpCommunities(EventReportService_t *service,
                                            const char *readCommunity,
                                            const char *writeCommunity,
                                            const char *trapCommunity);
void EventReportServiceCreateTransaction(EventReportService_t *service);
void EventReportServiceVerifyTransaction(EventReportService_t *service);
void EventReportServiceCommitTransaction(EventReportService_t *service);
void EventReportServiceRollbackTransaction(EventReportService_t *service);
void EventReportServiceAppendLegacyEvent(EventReportService_t *service,
                                         uint8_t eventCode,
                                         uint8_t eventParam,
                                         uint16_t eventShortParam,
                                         uint32_t eventLongParam);
void EventReportServiceUpdateDoorState(EventReportService_t *service,
                                       uint8_t open);
void EventReportServiceUpdateCpMpLinkHealthy(EventReportService_t *service,
                                             uint8_t healthy);
uint8_t EventReportServiceValidateWatchObjectOid(
  const EventReportService_t *service,
  const NtcipOid_t *oid);
uint8_t EventReportServiceValidateReportObjectOid(
  const EventReportService_t *service,
  const NtcipOid_t *oid);
uint8_t EventReportServiceReadWatchBlockValue(
  const EventReportService_t *service,
  uint8_t blockNumber,
  NtcipOctetString_t *value);
uint8_t EventReportServiceReadReportBlockValue(
  const EventReportService_t *service,
  uint8_t blockNumber,
  NtcipOctetString_t *value);
uint8_t EventReportServiceProcessPersistence(EventReportService_t *service);
const EventReportConfiguration_t *EventReportServiceGetActiveConfig(
  const EventReportService_t *service);
EventReportConfiguration_t *EventReportServiceGetCandidateConfig(
  EventReportService_t *service);
uint16_t EventReportServiceGetLatestLogIndex(const EventReportService_t *service);
uint8_t EventReportServiceCanReadLogIndex(const EventReportService_t *service,
                                          uint16_t index);
uint8_t EventReportServiceReadLogRecord(const EventReportService_t *service,
                                        uint16_t index,
                                        EventReportLogRecord_t *record);
uint8_t EventReportServiceGetEventNumberForIndex(
  const EventReportService_t *service,
  uint16_t index,
  uint8_t *eventNumber);
uint8_t EventReportServiceFindLatestEventId(const EventReportService_t *service,
                                            uint16_t eventId,
                                            uint16_t *index);
uint8_t EventReportServiceReadLogByClassNumber(
  const EventReportService_t *service,
  uint8_t eventClass,
  uint8_t eventNumber,
  EventReportLogRecord_t *record);
uint8_t EventReportServiceClearLog(EventReportService_t *service);
uint8_t EventReportServiceClearEventClass(EventReportService_t *service,
                                          uint8_t eventClass);
uint8_t EventReportServiceClearEventConfig(EventReportService_t *service,
                                           uint16_t eventConfigId);
uint8_t EventReportServiceGetClassCount(const EventReportService_t *service,
                                        uint8_t eventClass);
uint16_t EventReportServiceGetNumEvents(const EventReportService_t *service);
uint16_t EventReportServiceGetPersistDropped(
  const EventReportService_t *service);
const NtcipOctetString_t *EventReportServiceGetLatestTrapData(
  const EventReportService_t *service);
uint8_t EventReportServiceCopyPendingTrap(const EventReportService_t *service,
                                          NtcipOctetString_t *trapData);
void EventReportServiceAcknowledgeTrapDispatch(EventReportService_t *service,
                                               uint8_t success);
const EventReportEventSourceState_t *EventReportServiceGetEventSources(
  const EventReportService_t *service);

#endif /* EVENT_REPORT_SERVICE_H */
