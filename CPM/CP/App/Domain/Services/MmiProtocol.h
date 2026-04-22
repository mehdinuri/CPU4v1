/* App/Domain/Services/MmiProtocol.h
 *
 * Canonical MMI CAN protocol v2 contract for the touchscreen LCD controller.
 * This protocol is intentionally decoupled from the legacy ProgramTask/MMI
 * wire format and instead projects the new controller runtime and NTCIP model.
 */
#ifndef MMI_PROTOCOL_H
#define MMI_PROTOCOL_H

#include <stdint.h>

#include "Domain/Intersection/CpMpProtocol.h"
#include "Domain/Intersection/IntersectionRuntime.h"

#define MMI_PROTOCOL_V2_VERSION 2U

#define MMI_PROTOCOL_V2_CAN_ID_ACK            0x780U
#define MMI_PROTOCOL_V2_CAN_ID_EVENT_SEG      0x781U
#define MMI_PROTOCOL_V2_CAN_ID_PUBLISH_SEG    0x782U
#define MMI_PROTOCOL_V2_CAN_ID_SUBSCRIBE_SEG  0x783U
#define MMI_PROTOCOL_V2_CAN_ID_RESPONSE_SEG   0x784U
#define MMI_PROTOCOL_V2_CAN_ID_COMMAND_SEG    0x785U
#define MMI_PROTOCOL_V2_CAN_ID_HELLO_RSP      0x786U
#define MMI_PROTOCOL_V2_CAN_ID_HELLO_REQ      0x787U

#define MMI_PROTOCOL_V2_SEGMENT_DATA_BYTES    4U
#define MMI_PROTOCOL_V2_SEGMENT_INDEX_MAX     255U
#define MMI_PROTOCOL_V2_EVENT_PAGE_MAX_RECORDS 4U

typedef enum
{
  MMI_PROTOCOL_V2_MESSAGE_CLASS_NONE = 0U,
  MMI_PROTOCOL_V2_MESSAGE_CLASS_ACK = 1U,
  MMI_PROTOCOL_V2_MESSAGE_CLASS_EVENT = 2U,
  MMI_PROTOCOL_V2_MESSAGE_CLASS_PUBLISH = 3U,
  MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE = 4U,
  MMI_PROTOCOL_V2_MESSAGE_CLASS_RESPONSE = 5U,
  MMI_PROTOCOL_V2_MESSAGE_CLASS_COMMAND = 6U,
  MMI_PROTOCOL_V2_MESSAGE_CLASS_HELLO_RESPONSE = 7U,
  MMI_PROTOCOL_V2_MESSAGE_CLASS_HELLO_REQUEST = 8U
} MmiProtocolMessageClass_t;

typedef enum
{
  MMI_PROTOCOL_V2_SEGMENT_FLAG_FIRST = 0x01U,
  MMI_PROTOCOL_V2_SEGMENT_FLAG_LAST = 0x02U,
  MMI_PROTOCOL_V2_SEGMENT_FLAG_ABORT = 0x04U
} MmiProtocolSegmentFlags_t;

typedef enum
{
  MMI_PROTOCOL_V2_NAMESPACE_RUNTIME = 1U,
  MMI_PROTOCOL_V2_NAMESPACE_STANDARD_OBJECT = 2U,
  MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS = 3U,
  MMI_PROTOCOL_V2_NAMESPACE_VENDOR_PRIVATE = 4U,
  MMI_PROTOCOL_V2_NAMESPACE_EVENT_LOG = 5U,
  MMI_PROTOCOL_V2_NAMESPACE_MAINTENANCE = 6U
} MmiProtocolNamespace_t;

typedef enum
{
  MMI_PROTOCOL_V2_OPCODE_GET = 1U,
  MMI_PROTOCOL_V2_OPCODE_SET = 2U,
  MMI_PROTOCOL_V2_OPCODE_BEGIN_TRANSACTION = 3U,
  MMI_PROTOCOL_V2_OPCODE_VERIFY = 4U,
  MMI_PROTOCOL_V2_OPCODE_COMMIT = 5U,
  MMI_PROTOCOL_V2_OPCODE_ROLLBACK = 6U,
  MMI_PROTOCOL_V2_OPCODE_SUBSCRIBE = 7U,
  MMI_PROTOCOL_V2_OPCODE_UNSUBSCRIBE = 8U,
  MMI_PROTOCOL_V2_OPCODE_PING = 9U,
  MMI_PROTOCOL_V2_OPCODE_COMMAND = 10U
} MmiProtocolOpcode_t;

typedef enum
{
  MMI_PROTOCOL_V2_STATUS_OK = 0U,
  MMI_PROTOCOL_V2_STATUS_MORE_SEGMENTS = 1U,
  MMI_PROTOCOL_V2_STATUS_BAD_NAMESPACE = 2U,
  MMI_PROTOCOL_V2_STATUS_BAD_RESOURCE = 3U,
  MMI_PROTOCOL_V2_STATUS_BAD_INDEX = 4U,
  MMI_PROTOCOL_V2_STATUS_NOT_FOUND = 5U,
  MMI_PROTOCOL_V2_STATUS_NOT_WRITABLE = 6U,
  MMI_PROTOCOL_V2_STATUS_TRANSACTION_REQUIRED = 7U,
  MMI_PROTOCOL_V2_STATUS_INVALID_VALUE = 8U,
  MMI_PROTOCOL_V2_STATUS_BUSY = 9U,
  MMI_PROTOCOL_V2_STATUS_UNSUPPORTED = 10U,
  MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR = 11U
} MmiProtocolStatus_t;

typedef enum
{
  MMI_PROTOCOL_V2_UI_TYPE_TOUCH_LCD = 1U
} MmiProtocolUiType_t;

typedef enum
{
  MMI_PROTOCOL_V2_CAPABILITY_RUNTIME = 0x01U,
  MMI_PROTOCOL_V2_CAPABILITY_STANDARD_OBJECTS = 0x02U,
  MMI_PROTOCOL_V2_CAPABILITY_LOCAL_SETTINGS = 0x04U,
  MMI_PROTOCOL_V2_CAPABILITY_VENDOR_PRIVATE = 0x08U,
  MMI_PROTOCOL_V2_CAPABILITY_SUBSCRIPTIONS = 0x10U,
  MMI_PROTOCOL_V2_CAPABILITY_EVENT_LOG = 0x20U,
  MMI_PROTOCOL_V2_CAPABILITY_MAINTENANCE = 0x40U
} MmiProtocolCapabilityFlags_t;

typedef enum
{
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_SUMMARY = 1U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_RINGS = 2U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_PHASES = 3U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_CHANNELS = 4U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_OVERLAPS = 5U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_RAW_INPUTS = 6U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_VEHICLE_DETECTORS = 7U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_PEDESTRIAN_DETECTORS = 8U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_MODULE_STATUS = 9U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_SAFETY_SUMMARY = 10U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_SAFETY_CHANNELS = 11U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_CLOCK = 12U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_POWER = 13U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_COMMS = 14U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_RELAY = 15U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_OUTPUT_TEST = 16U,
  MMI_PROTOCOL_V2_RUNTIME_TOPIC_DOOR = 17U
} MmiProtocolRuntimeTopic_t;

typedef enum
{
  MMI_PROTOCOL_V2_STANDARD_RESOURCE_NTCIP_OBJECT = 1U
} MmiProtocolStandardResource_t;

typedef enum
{
  MMI_PROTOCOL_V2_LOCAL_RESOURCE_MODEM = 1U,
  MMI_PROTOCOL_V2_LOCAL_RESOURCE_GPS = 2U,
  MMI_PROTOCOL_V2_LOCAL_RESOURCE_USER_FLAGS = 3U,
  MMI_PROTOCOL_V2_LOCAL_RESOURCE_BROKEN_INPUT = 4U,
  MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN = 5U,
  MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN_PASSWORD_CHANGE = 6U,
  MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS = 7U
} MmiProtocolLocalResource_t;

typedef enum
{
  MMI_PROTOCOL_V2_VENDOR_RESOURCE_CPMP_LINK = 1U,
  MMI_PROTOCOL_V2_VENDOR_RESOURCE_CPMP_FAULT_SUMMARY = 2U,
  MMI_PROTOCOL_V2_VENDOR_RESOURCE_CPMP_FAULT_CHANNELS = 3U
} MmiProtocolVendorResource_t;

typedef enum
{
  MMI_PROTOCOL_V2_EVENT_RESOURCE_PAGE = 1U,
  MMI_PROTOCOL_V2_EVENT_RESOURCE_CURSOR = 2U
} MmiProtocolEventResource_t;

#define MMI_PROTOCOL_V2_EVENT_CURSOR_NONE 0xFFFFU

typedef enum
{
  MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_MODE_CONTROL = 1U,
  MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_TIME_SET = 2U,
  MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_DETECTOR_RESET = 3U,
  MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_OUTPUT_TEST = 4U,
  MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_FACTORY_RESET = 5U,
  MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_RELAY_COMMAND = 6U
} MmiProtocolMaintenanceResource_t;

typedef struct
{
  uint8_t sessionId;
  uint8_t transferId;
  uint8_t segmentIndex;
  uint8_t flags;
  uint8_t bytes[MMI_PROTOCOL_V2_SEGMENT_DATA_BYTES];
} __attribute__((packed)) MmiProtocolSegmentV2_t;

typedef struct
{
  uint8_t protocolVersion;
  uint8_t uiType;
  uint8_t capabilityFlags;
  uint8_t requestedSessionId;
  uint16_t displayWidthPixels;
  uint16_t displayHeightPixels;
} __attribute__((packed)) MmiProtocolHelloRequestV2_t;

typedef struct
{
  uint8_t protocolVersion;
  uint8_t controllerRole;
  uint8_t capabilityFlags;
  uint8_t assignedSessionId;
  uint16_t activeConfigSetId;
  uint16_t activeGenerationLow;
  uint8_t authorityReady;
  uint8_t peerHealthy;
  uint8_t reserved0[2];
} __attribute__((packed)) MmiProtocolHelloResponseV2_t;

typedef struct
{
  uint8_t opcode;
  uint8_t namespaceId;
  uint8_t resourceId;
  uint8_t recordIndex;
  uint8_t transactionId;
  uint8_t flags;
  uint16_t payloadLength;
} __attribute__((packed)) MmiProtocolCommandHeaderV2_t;

typedef struct
{
  uint8_t opcode;
  uint8_t namespaceId;
  uint8_t resourceId;
  uint8_t recordIndex;
  uint8_t transactionId;
  uint8_t status;
  uint16_t payloadLength;
} __attribute__((packed)) MmiProtocolResponseHeaderV2_t;

typedef struct
{
  uint8_t topicId;
  uint8_t recordIndex;
  uint8_t periodDeciseconds;
  uint8_t flags;
  uint16_t leaseSeconds;
  uint16_t cursor;
} __attribute__((packed)) MmiProtocolSubscribeRequestV2_t;

typedef struct
{
  uint8_t topicId;
  uint8_t recordIndex;
  uint8_t sequence;
  uint8_t flags;
  uint16_t payloadLength;
  uint16_t cursor;
} __attribute__((packed)) MmiProtocolPublishHeaderV2_t;

typedef struct
{
  uint8_t messageClass;
  uint8_t transferId;
  uint8_t status;
  uint8_t reserved0;
  uint32_t cursor;
} __attribute__((packed)) MmiProtocolAckV2_t;

typedef struct
{
  uint8_t oidLength;
  uint8_t valueEncoding;
  uint16_t valueLength;
} __attribute__((packed)) MmiProtocolObjectPrefixV2_t;

typedef struct
{
  uint8_t mode;
  uint8_t localFreeStatus;
  uint8_t unitControlStatus;
  uint8_t coordPatternStatus;
  uint8_t actionPlanControl;
  uint8_t timebaseActionStatus;
  uint8_t preemptStatus;
  uint8_t mmuFlashActive;
  uint8_t startUpFlashActive;
  uint8_t dimmingActive;
  uint8_t safetyAction;
  uint8_t safetyReasonCode;
  uint8_t activeSequenceNumber;
  uint8_t configLoaded;
  uint16_t coordCycleStatusSeconds;
  uint16_t coordSyncStatusSeconds;
  uint32_t monotonicTicks;
} __attribute__((packed)) MmiRuntimeSummaryV2_t;

typedef struct
{
  uint8_t ringNumber;
  uint8_t activePhaseNumber;
  uint8_t stage;
  uint8_t statusCode;
  uint8_t terminationReasonBits;
  uint8_t barrierWaiting;
  uint8_t activePosition;
  uint8_t pendingPosition;
  uint32_t stageElapsedTicks;
} __attribute__((packed)) MmiRuntimeRingRecordV2_t;

typedef struct
{
  uint8_t phaseNumber;
  uint8_t interval;
  uint8_t pedInterval;
  uint8_t detectorActive;
  uint8_t callLatched;
  uint8_t pedInputActive;
  uint8_t pedCallLatched;
  uint8_t nextPhase;
  uint32_t intervalElapsedTicks;
  uint16_t pedIntervalElapsedTicks;
} __attribute__((packed)) MmiRuntimePhaseRecordV2_t;

typedef struct
{
  uint8_t channelNumber;
  uint8_t requestedAspect;
  uint8_t appliedAspect;
  uint8_t dimmed;
  uint8_t dimAlternateHalfCycle;
} __attribute__((packed)) MmiRuntimeChannelRecordV2_t;

typedef struct
{
  uint8_t overlapNumber;
  uint8_t aspect;
  uint8_t reserved0[2];
} __attribute__((packed)) MmiRuntimeOverlapRecordV2_t;

typedef struct
{
  uint32_t rawVehicleMask;
  uint32_t rawPedestrianMask;
  uint8_t preemptInputs;
  uint8_t preemptControls;
  uint8_t validMask;
  uint8_t healthMask;
  uint8_t staleMask;
  uint8_t contextFaultMask;
  uint8_t sequenceFaultMask;
  uint8_t sequence;
  uint16_t configEpoch;
  uint32_t loadSwitchReds;
  uint32_t loadSwitchYellows;
  uint32_t loadSwitchGreens;
} __attribute__((packed)) MmiRuntimeRawInputsV2_t;

typedef struct
{
  uint8_t detectorNumber;
  uint8_t inputActive;
  uint8_t remoteActuation;
  uint8_t recognitionActive;
  uint8_t callPhase;
  uint8_t delayTimerDeciseconds;
  uint8_t extendTimerDeciseconds;
  uint8_t volume;
  uint8_t occupancy;
  uint16_t averageSpeed;
  uint8_t alarm;
  uint8_t reportedAlarm;
} __attribute__((packed)) MmiRuntimeVehicleDetectorRecordV2_t;

typedef struct
{
  uint8_t detectorNumber;
  uint8_t inputActive;
  uint8_t remoteActuation;
  uint8_t alternateTimingRequest;
  uint8_t callPhase;
  uint8_t volume;
  uint8_t actuations;
  uint8_t services;
  uint8_t alarm;
  uint8_t reserved0[3];
} __attribute__((packed)) MmiRuntimePedestrianDetectorRecordV2_t;

typedef struct
{
  uint8_t validMask;
  uint8_t healthMask;
  uint8_t staleMask;
  uint8_t contextFaultMask;
  uint8_t sequenceFaultMask;
  uint8_t sequence;
  uint16_t configEpoch;
  uint32_t loadSwitchReds;
  uint32_t loadSwitchYellows;
  uint32_t loadSwitchGreens;
} __attribute__((packed)) MmiRuntimeModuleStatusV2_t;

typedef struct
{
  uint8_t peerHealthy;
  uint8_t authorityReady;
  uint8_t safetyAction;
  uint8_t safetyReasonCode;
  uint8_t configState;
  uint8_t mmuFlashActive;
  uint8_t startUpFlashActive;
  uint8_t reserved0;
  uint32_t faultSequence;
  uint32_t globalFaultFlags;
} __attribute__((packed)) MmiRuntimeSafetySummaryV2_t;

typedef struct
{
  uint8_t channelNumber;
  uint8_t reserved0;
  uint16_t faultFlags;
} __attribute__((packed)) MmiRuntimeSafetyChannelRecordV2_t;

typedef struct
{
  uint32_t globalTimeSeconds;
  uint32_t localTimeSeconds;
  int32_t globalLocalDifferentialSeconds;
  uint8_t scheduleStatus;
  uint8_t dayPlanStatus;
  uint8_t lastAppliedActionNumber;
  uint8_t reserved0;
} __attribute__((packed)) MmiRuntimeClockSummaryV2_t;

typedef struct
{
  uint16_t psmVoltageRaw[2];
  uint16_t psmVoltageTenthsVrms[2];
  uint8_t psmFrequencyRaw[2];
  uint8_t psmIsolatedVoltage[2];
} __attribute__((packed)) MmiRuntimePowerSummaryV2_t;

typedef struct
{
  uint8_t networkType;
  uint8_t bearerState;
  uint8_t signalQuality;
  uint8_t transportReady;
  uint8_t snmpReady;
  uint8_t modemAlive;
  uint8_t simReady;
  char imei[16];
  char ethernetMac[13];
  char operatorName[21];
  char localIp[16];
  char managerIp[16];
} __attribute__((packed)) MmiRuntimeCommsSummaryV2_t;

typedef struct
{
  uint8_t userOutputPowerEnabled;
  uint8_t permitOutputPower;
  uint8_t relayDrive;
  uint8_t relayTopology;
  uint8_t safetyAction;
  uint8_t reserved0[3];
} __attribute__((packed)) MmiRuntimeRelaySummaryV2_t;

typedef struct
{
  uint8_t enabled;
  uint8_t reserved0;
  uint32_t forcedMask;
  uint32_t redMask;
  uint32_t yellowMask;
  uint32_t greenMask;
} __attribute__((packed)) MmiRuntimeOutputTestSummaryV2_t;

typedef struct
{
  uint8_t open;
  uint8_t reserved0;
  uint16_t latestOpenLogIndex;
  uint16_t latestCloseLogIndex;
  uint32_t changeSequence;
} __attribute__((packed)) MmiRuntimeDoorSummaryV2_t;

typedef struct
{
  uint8_t modemType;
  uint8_t reserved0[3];
} __attribute__((packed)) MmiLocalModemSettingsV2_t;

typedef struct
{
  uint8_t gpsPortType;
  uint8_t gpsBaudRateIndex;
  uint8_t reserved0[2];
} __attribute__((packed)) MmiLocalGpsSettingsV2_t;

typedef struct
{
  uint8_t configFlag;
  uint8_t logFlag;
  uint8_t trafficCountsFlag;
  uint8_t standbyInfoFlag;
} __attribute__((packed)) MmiLocalUserFlagsV2_t;

typedef struct
{
  uint8_t loopInputFlag;
  uint8_t digitalInputFlag;
  uint8_t reserved0[2];
} __attribute__((packed)) MmiLocalBrokenInputSettingsV2_t;

typedef struct
{
  uint16_t adminUsername;
  uint8_t adminValidity;
  uint8_t reserved0;
} __attribute__((packed)) MmiLocalAdminInfoV2_t;

typedef struct
{
  uint16_t currentPassword;
  uint16_t newPassword;
} __attribute__((packed)) MmiLocalAdminPasswordChangeV2_t;

typedef struct
{
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint8_t year;
  uint8_t century;
  uint8_t globalDaylightSaving;
} __attribute__((packed)) MmiLocalClockSettingsV2_t;

typedef struct
{
  uint16_t startIndex;
  uint8_t maxCount;
  uint8_t flags;
} __attribute__((packed)) MmiEventPageRequestV2_t;

typedef struct
{
  uint16_t startIndex;
  uint8_t count;
  uint8_t moreAvailable;
} __attribute__((packed)) MmiEventPageHeaderV2_t;

typedef struct
{
  uint16_t logIndex;
  uint8_t eventClass;
  uint8_t eventNumber;
  uint16_t eventId;
  uint32_t eventTime;
  uint16_t eventTimeMilliseconds;
  uint8_t valueLength;
  uint8_t value[46];
} __attribute__((packed)) MmiEventRecord_t;

typedef MmiEventRecord_t MmiEventRecordV2_t;

typedef struct
{
  uint8_t requestedControl;
  uint8_t reserved0[3];
} __attribute__((packed)) MmiMaintenanceModeCommandV2_t;

typedef struct
{
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint8_t year;
  uint8_t reserved0[2];
} __attribute__((packed)) MmiMaintenanceTimeSetCommandV2_t;

typedef struct
{
  uint8_t detectorClass;
  uint8_t detectorNumber;
  uint8_t reserved0[2];
} __attribute__((packed)) MmiMaintenanceDetectorResetCommandV2_t;

typedef struct
{
  uint8_t command;
  uint8_t outputNumber;
  uint8_t aspect;
  uint8_t reserved0;
} __attribute__((packed)) MmiMaintenanceOutputTestCommandV2_t;

typedef struct
{
  uint8_t magic0;
  uint8_t magic1;
  uint8_t scope;
  uint8_t reserved0;
} __attribute__((packed)) MmiMaintenanceFactoryResetCommandV2_t;

typedef struct
{
  uint8_t requestedState;
  uint8_t reserved0[3];
} __attribute__((packed)) MmiMaintenanceRelayCommandV2_t;

uint8_t MmiProtocolV2CanIdToMessageClass(uint16_t canId,
                                         MmiProtocolMessageClass_t *messageClass);
uint8_t MmiProtocolV2MessageClassToCanId(MmiProtocolMessageClass_t messageClass,
                                         uint16_t *canId);
uint8_t MmiProtocolV2SegmentIsFirst(const MmiProtocolSegmentV2_t *segment);
uint8_t MmiProtocolV2SegmentIsLast(const MmiProtocolSegmentV2_t *segment);
uint16_t MmiProtocolV2ObjectPrefixLength(void);

#endif /* MMI_PROTOCOL_H */
