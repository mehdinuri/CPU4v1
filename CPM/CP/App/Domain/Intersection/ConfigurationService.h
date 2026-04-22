/* App/Domain/Intersection/ConfigurationService.h
 *
 * Transactional configuration service for persisted NTCIP/NEMA controller
 * data. Owns active config, candidate overlay, verify status, and slot
 * commits.
 */
#ifndef CONFIGURATION_SERVICE_H
#define CONFIGURATION_SERVICE_H

#include <stdint.h>

#include "Domain/Intersection/IntersectionConfig.h"
#include "Ports/IConfigRepositoryPort.h"

#define CONFIGURATION_IMAGE_MAGIC 0x43464731UL
#define CONFIGURATION_IMAGE_SCHEMA_VERSION 22UL
#define CONFIGURATION_IMAGE_HEADER_SIZE 40U
#define CONFIGURATION_PHASE_IMAGE_SIZE 56U
#define CONFIGURATION_CHANNEL_IMAGE_SIZE \
        (8U + INTERSECTION_CHANNEL_COUNT_MAX)
#define CONFIGURATION_OVERLAP_IMAGE_SIZE 38U
#define CONFIGURATION_PREEMPT_IMAGE_SIZE \
        (13U \
         + (4U * (1U + INTERSECTION_PHASE_COUNT_MAX)) \
         + (3U * (1U + INTERSECTION_OVERLAP_COUNT_MAX)) \
         + (2U * (1U + INTERSECTION_PHASE_COUNT_MAX)) \
         + 6U)
#define CONFIGURATION_VEHICLE_DETECTOR_IMAGE_SIZE 20U
#define CONFIGURATION_PED_DETECTOR_IMAGE_SIZE 6U
#define CONFIGURATION_TIMEBASE_ACTION_IMAGE_SIZE 4U
#define CONFIGURATION_UNIT_IMAGE_SIZE 13U
#define CONFIGURATION_GLOBAL_TIME_MANAGEMENT_IMAGE_SIZE 91U
#define CONFIGURATION_CABINET_ENVIRONMENT_IMAGE_SIZE \
        (1U \
         + (INTERSECTION_CABINET_ENVIRONMENT_DEVICE_COUNT_MAX \
            * (1U + INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX)) \
         + (INTERSECTION_CABINET_TEMP_SENSOR_COUNT_MAX \
            * (INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX + 2U)) \
         + (INTERSECTION_CABINET_HUMIDITY_SENSOR_COUNT_MAX \
            * (INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX + 1U)))
#define CONFIGURATION_IO_MAP_INPUT_ROW_IMAGE_SIZE 11U
#define CONFIGURATION_IO_MAP_OUTPUT_ROW_IMAGE_SIZE 11U
#define CONFIGURATION_IO_MAP_IMAGE_SIZE \
        (INTERSECTION_IO_MAP_DESCRIPTION_MAX \
         + (INTERSECTION_IO_MAP_MAX_INPUTS \
            * CONFIGURATION_IO_MAP_INPUT_ROW_IMAGE_SIZE) \
         + (INTERSECTION_IO_MAP_MAX_OUTPUTS \
            * CONFIGURATION_IO_MAP_OUTPUT_ROW_IMAGE_SIZE))
#define CONFIGURATION_USER_DEFINED_BACKUP_CONTENT_IMAGE_SIZE \
        (2U + (INTERSECTION_USER_DEFINED_BACKUP_OID_COMPONENT_COUNT_MAX * 4U) \
         + INTERSECTION_USER_DEFINED_BACKUP_DESCRIPTION_MAX)
#define CONFIGURATION_IMAGE_PAYLOAD_SIZE \
        (4U + (INTERSECTION_PHASE_COUNT_MAX * CONFIGURATION_PHASE_IMAGE_SIZE) \
         + (INTERSECTION_SEQUENCE_COUNT_MAX * INTERSECTION_RING_COUNT_MAX * 8U) \
         + 8U \
         + (INTERSECTION_PATTERN_COUNT_MAX * 8U) \
         + (INTERSECTION_SPLIT_COUNT_MAX * INTERSECTION_PHASE_COUNT_MAX * 4U) \
         + (INTERSECTION_PREEMPT_COUNT_MAX * CONFIGURATION_PREEMPT_IMAGE_SIZE) \
         + (INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX \
            * CONFIGURATION_VEHICLE_DETECTOR_IMAGE_SIZE) \
         + (INTERSECTION_PED_INPUT_COUNT_MAX \
            * CONFIGURATION_PED_DETECTOR_IMAGE_SIZE) \
         + 8U \
         + (INTERSECTION_PREEMPT_COUNT_MAX \
            * INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX * 2U) \
         + (INTERSECTION_PREEMPT_GATE_COUNT_MAX \
            * (1U + INTERSECTION_PREEMPT_GATE_DESCRIPTION_MAX)) \
         + (INTERSECTION_PHASE_COUNT_MAX * 2U) \
         + (INTERSECTION_PREEMPT_COUNT_MAX * 2U) \
         + (INTERSECTION_CHANNEL_COUNT_MAX * CONFIGURATION_CHANNEL_IMAGE_SIZE) \
         + (INTERSECTION_OVERLAP_COUNT_MAX * CONFIGURATION_OVERLAP_IMAGE_SIZE) \
         + 2U \
         + (INTERSECTION_TIMEBASE_ACTION_COUNT_MAX \
            * CONFIGURATION_TIMEBASE_ACTION_IMAGE_SIZE) \
         + CONFIGURATION_GLOBAL_TIME_MANAGEMENT_IMAGE_SIZE \
         + CONFIGURATION_UNIT_IMAGE_SIZE \
         + (INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX \
            * CONFIGURATION_USER_DEFINED_BACKUP_CONTENT_IMAGE_SIZE) \
         + CONFIGURATION_CABINET_ENVIRONMENT_IMAGE_SIZE \
         + CONFIGURATION_IO_MAP_IMAGE_SIZE)

#define CONFIGURATION_SLOT_STATE_ERASED 0xFFFFFFFFUL
#define CONFIGURATION_SLOT_STATE_WRITING 0x7FFFFFFFUL
#define CONFIGURATION_SLOT_STATE_VALID 0x3FFFFFFFUL
#define CONFIGURATION_SLOT_STATE_RETIRED 0x1FFFFFFFUL

#define CONFIGURATION_MIGRATION_JOURNAL_MAGIC 0x4D494731UL
#define CONFIGURATION_MIGRATION_JOURNAL_SIZE 24U

typedef enum
{
  CONFIGURATION_SLOT_NONE = 0,
  CONFIGURATION_SLOT_A,
  CONFIGURATION_SLOT_B
} ConfigurationSlotId_t;

typedef enum
{
  CONFIGURATION_VERIFY_STATUS_IDLE = 0,
  CONFIGURATION_VERIFY_STATUS_SUCCESS,
  CONFIGURATION_VERIFY_STATUS_FAILED,
  CONFIGURATION_VERIFY_STATUS_COMMIT_FAILED
} ConfigurationVerifyStatus_t;

typedef enum
{
  CONFIGURATION_MIGRATION_STATE_NONE = 0,
  CONFIGURATION_MIGRATION_STATE_STARTED,
  CONFIGURATION_MIGRATION_STATE_COMPLETED
} ConfigurationMigrationState_t;

typedef struct
{
  uint32_t magic;
  uint32_t schemaVersion;
  uint32_t payloadLength;
  uint32_t generation;
  uint32_t payloadCrc32;
  uint32_t headerCrc32;
  uint32_t state;
  uint32_t migrationSourceVersion;
  uint32_t migrationSourceCrc32;
  uint32_t reserved;
} ConfigurationImageHeader_t;

typedef struct
{
  uint32_t magic;
  uint32_t state;
  uint32_t sourceVersion;
  uint32_t sourceCrc32;
  uint32_t targetSlot;
  uint32_t targetGeneration;
} ConfigurationMigrationJournal_t;

typedef struct
{
  uint8_t inUse;
  uint8_t dirty;
  uint8_t phaseCountValid;
  uint8_t ringCountValid;
  uint8_t barrierCountValid;
  uint8_t reserved[3];
  uint8_t phaseValid[INTERSECTION_PHASE_COUNT_MAX];
  union
  {
    uint8_t ringValid[INTERSECTION_RING_COUNT_MAX];
    uint8_t sequencePlanValid[INTERSECTION_SEQUENCE_COUNT_MAX][
      INTERSECTION_RING_COUNT_MAX];
  };
  uint8_t channelValid[INTERSECTION_CHANNEL_COUNT_MAX];
  uint8_t overlapValid[INTERSECTION_OVERLAP_COUNT_MAX];
  uint8_t vehicleDetectorValid[INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX];
  uint8_t pedestrianDetectorValid[INTERSECTION_PED_INPUT_COUNT_MAX];
  uint8_t coordinationValid;
  uint8_t timebaseValid;
  uint8_t globalTimeManagementValid;
  uint8_t cabinetEnvironmentValid;
  uint8_t ioMapValid;
  uint8_t unitValid;
  uint8_t inputMappingValid;
  uint8_t detectorReportsValid;
  uint8_t preemptValid[INTERSECTION_PREEMPT_COUNT_MAX];
  uint8_t preemptQueueDelayWeightValid[INTERSECTION_PREEMPT_COUNT_MAX][
    INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX];
  uint8_t preemptGateValid[INTERSECTION_PREEMPT_GATE_COUNT_MAX];
  uint8_t userDefinedBackupContentValid[
    INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX];
  IntersectionPhaseConfig_t phases[INTERSECTION_PHASE_COUNT_MAX];
  union
  {
    IntersectionRingPlan_t rings[INTERSECTION_RING_COUNT_MAX];
    IntersectionRingPlan_t sequencePlans[INTERSECTION_SEQUENCE_COUNT_MAX][
      INTERSECTION_RING_COUNT_MAX];
  };
  IntersectionChannelConfig_t channels[INTERSECTION_CHANNEL_COUNT_MAX];
  IntersectionOverlapConfig_t overlaps[INTERSECTION_OVERLAP_COUNT_MAX];
  IntersectionVehicleDetectorConfig_t vehicleDetectors[
    INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX];
  IntersectionPedestrianDetectorConfig_t pedestrianDetectors[
    INTERSECTION_PED_INPUT_COUNT_MAX];
  IntersectionDetectorReportConfig_t detectorReports;
  IntersectionCoordinationConfig_t coordination;
  IntersectionTimebaseConfig_t timebase;
  IntersectionGlobalTimeManagementConfig_t globalTimeManagement;
  IntersectionCabinetEnvironmentConfig_t cabinetEnvironment;
  IntersectionIoMapConfig_t ioMap;
  IntersectionUnitConfig_t unit;
  IntersectionInputMappingConfig_t inputMapping;
  IntersectionPreemptConfig_t preempts[INTERSECTION_PREEMPT_COUNT_MAX];
  uint16_t preemptQueueDelayWeights[INTERSECTION_PREEMPT_COUNT_MAX][
    INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX];
  IntersectionPreemptGateConfig_t preemptGates[
    INTERSECTION_PREEMPT_GATE_COUNT_MAX];
  IntersectionUserDefinedBackupContentConfig_t userDefinedBackupContents[
    INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX];
  uint8_t phaseCount;
  uint8_t ringCount;
  uint8_t barrierCount;
} ConfigurationCandidateOverlay_t;

typedef struct
{
  IConfigRepositoryPort_t *repositoryPort;
  IntersectionConfig_t activeConfig;
  uint32_t activeGeneration;
  ConfigurationSlotId_t activeSlot;
  ConfigurationVerifyStatus_t verifyStatus;
  IntersectionConfigErrorInfo_t lastError;
  uint8_t loadedFromRepository;
  ConfigurationCandidateOverlay_t candidate;
} ConfigurationService_t;

void ConfigurationServiceInit(ConfigurationService_t *service,
                              IConfigRepositoryPort_t *repositoryPort);
uint8_t ConfigurationServiceLoad(ConfigurationService_t *service);

const IntersectionConfig_t *ConfigurationServiceGetActiveConfig(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetActivePhaseConfig(
  const ConfigurationService_t *service,
  uint8_t phaseIndex,
  IntersectionPhaseConfig_t *
  phaseConfig);
uint8_t ConfigurationServiceGetPhaseCount(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetRingCount(const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetSequenceCount(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetActiveRingPlan(
  const ConfigurationService_t *service,
  uint8_t ringIndex,
  IntersectionRingPlan_t *ringPlan);
uint8_t ConfigurationServiceGetActiveSequenceRingPlan(
  const ConfigurationService_t *service,
  uint8_t sequenceNumber,
  uint8_t ringIndex,
  IntersectionRingPlan_t *ringPlan);
uint8_t ConfigurationServiceGetCandidateRingPlan(
  ConfigurationService_t *service,
  uint8_t ringIndex,
  IntersectionRingPlan_t *ringPlan);
uint8_t ConfigurationServiceGetCandidateSequenceRingPlan(
  ConfigurationService_t *service,
  uint8_t sequenceNumber,
  uint8_t ringIndex,
  IntersectionRingPlan_t *ringPlan);
uint8_t ConfigurationServiceGetCandidateGlobalTimeManagementConfig(
  ConfigurationService_t *service,
  IntersectionGlobalTimeManagementConfig_t *globalTimeManagementConfig);
uint8_t ConfigurationServiceGetCandidateCabinetEnvironmentConfig(
  ConfigurationService_t *service,
  IntersectionCabinetEnvironmentConfig_t *cabinetEnvironmentConfig);
uint8_t ConfigurationServiceGetCandidateIoMapConfig(
  ConfigurationService_t *service,
  IntersectionIoMapConfig_t *ioMapConfig);
uint8_t ConfigurationServiceGetActiveChannelConfig(
  const ConfigurationService_t *service,
  uint8_t channelIndex,
  IntersectionChannelConfig_t *
  channelConfig);
uint8_t ConfigurationServiceGetChannelCount(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetActiveOverlapConfig(
  const ConfigurationService_t *service,
  uint8_t overlapIndex,
  IntersectionOverlapConfig_t *
  overlapConfig);
uint8_t ConfigurationServiceGetOverlapCount(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetActiveCoordinationConfig(
  const ConfigurationService_t *service,
  IntersectionCoordinationConfig_t
  *coordinationConfig);
uint8_t ConfigurationServiceGetActiveTimebaseConfig(
  const ConfigurationService_t *service,
  IntersectionTimebaseConfig_t *timebaseConfig);
uint8_t ConfigurationServiceGetActiveGlobalTimeManagementConfig(
  const ConfigurationService_t *service,
  IntersectionGlobalTimeManagementConfig_t *globalTimeManagementConfig);
uint8_t ConfigurationServiceGetActiveCabinetEnvironmentConfig(
  const ConfigurationService_t *service,
  IntersectionCabinetEnvironmentConfig_t *cabinetEnvironmentConfig);
uint8_t ConfigurationServiceGetActiveIoMapConfig(
  const ConfigurationService_t *service,
  IntersectionIoMapConfig_t *ioMapConfig);
uint8_t ConfigurationServiceGetActiveUnitConfig(
  const ConfigurationService_t *service,
  IntersectionUnitConfig_t *unitConfig);
uint8_t ConfigurationServiceGetUserDefinedBackupContentCount(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetActiveUserDefinedBackupContentConfig(
  const ConfigurationService_t *service,
  uint8_t contentIndex,
  IntersectionUserDefinedBackupContentConfig_t *contentConfig);
uint8_t ConfigurationServiceMatchesUserDefinedBackupContentOid(
  const ConfigurationService_t *service,
  const uint32_t *oid,
  uint8_t oidLength);
uint8_t ConfigurationServiceGetActiveTimebaseActionConfig(
  const ConfigurationService_t *service,
  uint8_t actionIndex,
  IntersectionTimebaseActionConfig_t *actionConfig);
uint8_t ConfigurationServiceGetActivePatternConfig(
  const ConfigurationService_t *service,
  uint8_t patternIndex,
  IntersectionPatternConfig_t *
  patternConfig);
uint8_t ConfigurationServiceGetPatternCount(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetActiveSplitPhaseConfig(
  const ConfigurationService_t *service,
  uint8_t splitIndex,
  uint8_t phaseIndex,
  IntersectionSplitPhaseConfig_t
  *splitConfig);
uint8_t ConfigurationServiceGetSplitCount(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetTimebaseActionCount(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetActivePreemptConfig(
  const ConfigurationService_t *service,
  uint8_t preemptIndex,
  IntersectionPreemptConfig_t *
  preemptConfig);
uint8_t ConfigurationServiceGetPreemptCount(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetVehicleDetectorCount(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetPedestrianDetectorCount(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetActiveVehicleDetectorConfig(
  const ConfigurationService_t *service,
  uint8_t detectorIndex,
  IntersectionVehicleDetectorConfig_t *detectorConfig);
uint8_t ConfigurationServiceGetActivePedestrianDetectorConfig(
  const ConfigurationService_t *service,
  uint8_t detectorIndex,
  IntersectionPedestrianDetectorConfig_t *detectorConfig);
uint8_t ConfigurationServiceGetActiveDetectorReportConfig(
  const ConfigurationService_t *service,
  IntersectionDetectorReportConfig_t *detectorReportConfig);
uint8_t ConfigurationServiceGetActiveInputMappingConfig(
  const ConfigurationService_t *service,
  IntersectionInputMappingConfig_t
  *inputMapping);
uint8_t ConfigurationServiceGetVehicleDetectorCallPhase(
  const ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t *phaseNumber);
uint8_t ConfigurationServiceGetPedestrianDetectorCallPhase(
  const ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t *phaseNumber);
uint8_t ConfigurationServiceGetPreemptQueueDelayWeight(
  const ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t detectorIndex,
  uint16_t *detectorWeight);
uint8_t ConfigurationServiceGetPreemptGateCount(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceGetActivePreemptGateConfig(
  const ConfigurationService_t *service,
  uint8_t gateIndex,
  IntersectionPreemptGateConfig_t
  *gateConfig);
uint16_t ConfigurationServiceGetActiveSetId(
  const ConfigurationService_t *service);
uint32_t ConfigurationServiceGetActiveGeneration(
  const ConfigurationService_t *service);
ConfigurationSlotId_t ConfigurationServiceGetActiveSlot(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceIsLoadedFromRepository(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceHasOpenTransaction(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceHasPendingChanges(
  const ConfigurationService_t *service);
ConfigurationVerifyStatus_t ConfigurationServiceGetVerifyStatus(
  const ConfigurationService_t *service);
IntersectionConfigErrorInfo_t ConfigurationServiceGetLastError(
  const ConfigurationService_t *service);

uint8_t ConfigurationServiceCreateTransaction(ConfigurationService_t *service);
void ConfigurationServiceRollback(ConfigurationService_t *service);
uint8_t ConfigurationServiceVerify(ConfigurationService_t *service);
uint8_t ConfigurationServiceCommit(ConfigurationService_t *service);

uint8_t ConfigurationServiceSetPhaseMinGreenDs(ConfigurationService_t *service,
                                               uint8_t phaseIndex,
                                               uint16_t valueDs);
uint8_t ConfigurationServiceSetPhaseWalkSeconds(ConfigurationService_t *service,
                                                uint8_t phaseIndex,
                                                uint16_t walkSeconds);
uint8_t ConfigurationServiceSetPhasePedClearSeconds(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint16_t pedClearSeconds);
uint8_t ConfigurationServiceSetPhaseMaxGreenDs(ConfigurationService_t *service,
                                               uint8_t phaseIndex,
                                               uint16_t valueDs);
uint8_t ConfigurationServiceSetPhaseYellowChangeDs(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint16_t valueDs);
uint8_t ConfigurationServiceSetPhaseRedClearDs(ConfigurationService_t *service,
                                               uint8_t phaseIndex,
                                               uint16_t valueDs);
uint8_t ConfigurationServiceSetPhasePassageDs(ConfigurationService_t *service,
                                              uint8_t phaseIndex,
                                              uint16_t valueDs);
uint8_t ConfigurationServiceSetPhaseMaxInitialDs(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint16_t valueDs);
uint8_t ConfigurationServiceSetPhasePedAdvanceWalkDs(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint16_t valueDs);
uint8_t ConfigurationServiceSetPhasePedDelayDs(ConfigurationService_t *service,
                                               uint8_t phaseIndex,
                                               uint16_t valueDs);
uint8_t ConfigurationServiceSetPhaseOptions(ConfigurationService_t *service,
                                            uint8_t phaseIndex,
                                            uint16_t options);
uint16_t ConfigurationServiceGetPhaseOptions(
  const ConfigurationService_t *service,
  uint8_t phaseIndex);
uint8_t ConfigurationServiceSetPhaseMaximum2Ds(ConfigurationService_t *service,
                                               uint8_t phaseIndex,
                                               uint16_t valueDs);
uint8_t ConfigurationServiceSetPhaseMaximum3Ds(ConfigurationService_t *service,
                                               uint8_t phaseIndex,
                                               uint16_t valueDs);
uint8_t ConfigurationServiceSetPhaseRedRevertDs(ConfigurationService_t *service,
                                                uint8_t phaseIndex,
                                                uint8_t valueDs);
uint8_t ConfigurationServiceSetPhaseAddedInitialDs(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t valueDs);
uint8_t ConfigurationServiceSetPhaseTimeBeforeReductionSec(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t timeSeconds);
uint8_t ConfigurationServiceSetPhaseCarsBeforeReduction(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t vehicles);
uint8_t ConfigurationServiceSetPhaseTimeToReduceSec(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t timeSeconds);
uint8_t ConfigurationServiceSetPhaseReduceByDs(ConfigurationService_t *service,
                                               uint8_t phaseIndex,
                                               uint8_t valueDs);
uint8_t ConfigurationServiceSetPhaseMinimumGapDs(ConfigurationService_t *service,
                                                 uint8_t phaseIndex,
                                                 uint8_t valueDs);
uint8_t ConfigurationServiceSetPhaseDynamicMaxLimitSeconds(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t valueSeconds);
uint8_t ConfigurationServiceSetPhaseDynamicMaxStepDs(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t valueDs);
uint8_t ConfigurationServiceSetPhaseStartup(ConfigurationService_t *service,
                                            uint8_t phaseIndex,
                                            uint8_t startup);
uint8_t ConfigurationServiceSetPhaseConcurrency(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  const uint8_t *phaseNumbers,
  uint8_t count);
uint8_t ConfigurationServiceSetPhaseYellowRedBeforeEndPedClearDs(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t valueDs);
uint8_t ConfigurationServiceSetPhasePedWalkService(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t serviceCount);
uint8_t ConfigurationServiceSetPhaseDontWalkRevertDs(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t valueDs);
uint8_t ConfigurationServiceSetPhasePedAlternateClearSeconds(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint16_t seconds);
uint8_t ConfigurationServiceSetPhasePedAlternateWalkSeconds(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint16_t seconds);
uint8_t ConfigurationServiceSetPhaseAdvWarnGrnStartTimeDs(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t valueDs);
uint8_t ConfigurationServiceSetPhaseAdvWarnRedStartTimeDs(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t valueDs);
uint8_t ConfigurationServiceSetPhaseAltMinTimeTransitionSeconds(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t seconds);
uint8_t ConfigurationServiceSetPhaseRing(ConfigurationService_t *service,
                                         uint8_t phaseIndex,
                                         uint8_t ringIndex);
uint8_t ConfigurationServiceSetRingSequenceData(ConfigurationService_t *service,
                                                uint8_t sequenceNumber,
                                                uint8_t ringIndex,
                                                const uint8_t *phaseNumbers,
                                                uint8_t length);
uint8_t ConfigurationServiceSetPhaseEnabled(ConfigurationService_t *service,
                                            uint8_t phaseIndex,
                                            uint8_t enabled);
uint8_t ConfigurationServiceSetPhaseDetectorInput(
  ConfigurationService_t *service,
  uint8_t phaseIndex,
  uint8_t detectorNumber);
uint8_t ConfigurationServiceSetPhasePedInput(ConfigurationService_t *service,
                                             uint8_t phaseIndex,
                                             uint8_t pedInputNumber);
uint8_t ConfigurationServiceSetPreemptInputSource(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t sourceNumber);
uint8_t ConfigurationServiceSetPreemptControlSource(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t sourceNumber);
uint8_t ConfigurationServiceSetVehicleDetectorCallPhase(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t phaseNumber);
uint8_t ConfigurationServiceSetVehicleDetectorOptions(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t options);
uint8_t ConfigurationServiceSetVehicleDetectorSwitchPhase(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t phaseNumber);
uint8_t ConfigurationServiceSetVehicleDetectorDelayDs(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint16_t valueDs);
uint8_t ConfigurationServiceSetVehicleDetectorExtendDs(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t valueDs);
uint8_t ConfigurationServiceSetVehicleDetectorQueueLimitSeconds(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t valueSeconds);
uint8_t ConfigurationServiceSetVehicleDetectorNoActivityMinutes(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t valueMinutes);
uint8_t ConfigurationServiceSetVehicleDetectorMaxPresenceMinutes(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t valueMinutes);
uint8_t ConfigurationServiceSetVehicleDetectorErraticCountsPerMinute(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t valueCounts);
uint8_t ConfigurationServiceSetVehicleDetectorFailTimeSeconds(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t valueSeconds);
uint8_t ConfigurationServiceSetVehicleDetectorOptions2(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t options);
uint8_t ConfigurationServiceSetVehicleDetectorPairedDetector(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t pairedDetectorNumber);
uint8_t ConfigurationServiceSetVehicleDetectorPairedDetectorSpacingCm(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint16_t spacingCm);
uint8_t ConfigurationServiceSetVehicleDetectorAvgVehicleLengthCm(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint16_t lengthCm);
uint8_t ConfigurationServiceSetVehicleDetectorLengthCm(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint16_t lengthCm);
uint8_t ConfigurationServiceSetVehicleDetectorTravelMode(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t travelMode);
uint8_t ConfigurationServiceSetPedestrianDetectorCallPhase(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t phaseNumber);
uint8_t ConfigurationServiceSetPedestrianDetectorNoActivityMinutes(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t valueMinutes);
uint8_t ConfigurationServiceSetPedestrianDetectorMaxPresenceMinutes(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t valueMinutes);
uint8_t ConfigurationServiceSetPedestrianDetectorErraticCountsPerMinute(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t valueCounts);
uint8_t ConfigurationServiceSetPedestrianDetectorApsMinimumActuationDs(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t valueDs);
uint8_t ConfigurationServiceSetPedestrianDetectorOptions(
  ConfigurationService_t *service,
  uint8_t detectorIndex,
  uint8_t options);
uint8_t ConfigurationServiceSetVolumeOccupancyPeriodSeconds(
  ConfigurationService_t *service,
  uint8_t seconds);
uint8_t ConfigurationServiceSetVolumeOccupancyPeriodV3Seconds(
  ConfigurationService_t *service,
  uint16_t seconds);
uint8_t ConfigurationServiceSetPedestrianDetectorPeriodSeconds(
  ConfigurationService_t *service,
  uint16_t seconds);
uint8_t ConfigurationServiceSetChannelControlSource(
  ConfigurationService_t *service,
  uint8_t channelIndex,
  uint8_t controlSource);
uint8_t ConfigurationServiceSetChannelControlType(
  ConfigurationService_t *service,
  uint8_t channelIndex,
  uint8_t controlType);
uint8_t ConfigurationServiceSetChannelFlashMask(ConfigurationService_t *service,
                                                uint8_t channelIndex,
                                                uint8_t flashMask);
uint8_t ConfigurationServiceSetChannelDimMask(ConfigurationService_t *service,
                                              uint8_t channelIndex,
                                              uint8_t dimMask);
uint8_t ConfigurationServiceSetChannelGreenType(ConfigurationService_t *service,
                                                uint8_t channelIndex,
                                                uint8_t greenType);
uint8_t ConfigurationServiceSetChannelGreenIncluded(
  ConfigurationService_t *service,
  uint8_t channelIndex,
  const uint8_t *
  channelNumbers,
  uint8_t channelCount);
uint8_t ConfigurationServiceSetChannelIntersectionId(
  ConfigurationService_t *service,
  uint8_t channelIndex,
  uint16_t intersectionId);
uint8_t ConfigurationServiceSetOverlapType(ConfigurationService_t *service,
                                           uint8_t overlapIndex,
                                           uint8_t overlapType);
uint8_t ConfigurationServiceSetOverlapIncludedPhases(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount);
uint8_t ConfigurationServiceSetOverlapModifierPhases(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount);
uint8_t ConfigurationServiceSetOverlapTrailGreenDs(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  uint16_t trailGreenDs);
uint8_t ConfigurationServiceSetOverlapTrailYellowDs(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  uint16_t trailYellowDs);
uint8_t ConfigurationServiceSetOverlapTrailRedDs(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  uint16_t trailRedDs);
uint8_t ConfigurationServiceSetOverlapWalkSeconds(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  uint16_t walkSeconds);
uint8_t ConfigurationServiceSetOverlapPedClearSeconds(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  uint16_t pedClearSeconds);
uint8_t ConfigurationServiceSetOverlapConflictingPedPhases(
  ConfigurationService_t *service,
  uint8_t overlapIndex,
  const uint8_t *
  phaseNumbers,
  uint8_t phaseCount);
uint8_t ConfigurationServiceSetCoordOperationalMode(
  ConfigurationService_t *service,
  uint8_t operationalMode);
uint8_t ConfigurationServiceSetCoordCorrectionMode(
  ConfigurationService_t *service,
  uint8_t correctionMode);
uint8_t ConfigurationServiceSetCoordMaximumMode(ConfigurationService_t *service,
                                                uint8_t maximumMode);
uint8_t ConfigurationServiceSetCoordForceMode(ConfigurationService_t *service,
                                              uint8_t forceMode);
uint8_t ConfigurationServiceSetCoordUnitSyncPoint(
  ConfigurationService_t *service,
  uint8_t unitCoordSyncPoint);
uint8_t ConfigurationServiceSetPatternCycleTimeSeconds(
  ConfigurationService_t *service,
  uint8_t patternIndex,
  uint8_t cycleTimeSeconds);
uint8_t ConfigurationServiceSetPatternOffsetTimeSeconds(
  ConfigurationService_t *service,
  uint8_t patternIndex,
  uint8_t
  offsetTimeSeconds);
uint8_t ConfigurationServiceSetPatternSplitNumber(
  ConfigurationService_t *service,
  uint8_t patternIndex,
  uint8_t splitNumber);
uint8_t ConfigurationServiceSetPatternSequenceNumber(
  ConfigurationService_t *service,
  uint8_t patternIndex,
  uint8_t sequenceNumber);
uint8_t ConfigurationServiceSetPatternCoordSyncPoint(
  ConfigurationService_t *service,
  uint8_t patternIndex,
  uint8_t coordSyncPoint);
uint8_t ConfigurationServiceSetPatternOptions(ConfigurationService_t *service,
                                              uint8_t patternIndex,
                                              uint8_t options);
uint8_t ConfigurationServiceSetSplitTimeSeconds(ConfigurationService_t *service,
                                                uint8_t splitIndex,
                                                uint8_t phaseIndex,
                                                uint8_t timeSeconds);
uint8_t ConfigurationServiceSetSplitMode(ConfigurationService_t *service,
                                         uint8_t splitIndex,
                                         uint8_t phaseIndex,
                                         uint8_t mode);
uint8_t ConfigurationServiceSetSplitCoordPhase(ConfigurationService_t *service,
                                               uint8_t splitIndex,
                                               uint8_t phaseIndex,
                                               uint8_t coordPhase);
uint8_t ConfigurationServiceSetSplitOptions(ConfigurationService_t *service,
                                            uint8_t splitIndex,
                                            uint8_t phaseIndex,
                                            uint8_t options);
uint8_t ConfigurationServiceSetTimebasePatternSyncMinutes(
  ConfigurationService_t *service,
  uint16_t patternSyncMinutes);
uint8_t ConfigurationServiceSetTimebaseActionPattern(
  ConfigurationService_t *service,
  uint8_t actionIndex,
  uint8_t pattern);
uint8_t ConfigurationServiceSetTimebaseActionAuxiliaryFunction(
  ConfigurationService_t *service,
  uint8_t actionIndex,
  uint8_t auxiliaryFunction);
uint8_t ConfigurationServiceSetTimebaseActionSpecialFunction(
  ConfigurationService_t *service,
  uint8_t actionIndex,
  uint8_t specialFunction);
uint8_t ConfigurationServiceSetTimebaseActionEnabledLane(
  ConfigurationService_t *service,
  uint8_t actionIndex,
  uint8_t enabledLane);
uint8_t ConfigurationServiceSetGlobalTimeManagementConfig(
  ConfigurationService_t *service,
  const IntersectionGlobalTimeManagementConfig_t *globalTimeManagementConfig);
uint8_t ConfigurationServiceSetCabinetEnvironmentConfig(
  ConfigurationService_t *service,
  const IntersectionCabinetEnvironmentConfig_t *cabinetEnvironmentConfig);
uint8_t ConfigurationServiceSetIoMapConfig(
  ConfigurationService_t *service,
  const IntersectionIoMapConfig_t *ioMapConfig);
uint8_t ConfigurationServiceSetUnitTimeSourceCommanded(
  ConfigurationService_t *service,
  uint8_t timeSourceCommanded);
uint8_t ConfigurationServiceSetUnitElevationOffsetMeters(
  ConfigurationService_t *service,
  uint8_t elevationOffsetMeters);
uint8_t ConfigurationServiceSetUnitStartUpFlashSeconds(
  ConfigurationService_t *service,
  uint8_t startUpFlashSeconds);
uint8_t ConfigurationServiceSetUnitAutoPedestrianClear(
  ConfigurationService_t *service,
  uint8_t autoPedestrianClear);
uint8_t ConfigurationServiceSetUnitBackupTimeSeconds(
  ConfigurationService_t *service,
  uint16_t backupTimeSeconds);
uint8_t ConfigurationServiceSetUnitUserDefinedBackupTimeSeconds(
  ConfigurationService_t *service,
  uint32_t backupTimeSeconds);
uint8_t ConfigurationServiceSetUnitRedRevertDs(ConfigurationService_t *service,
                                               uint8_t redRevertDs);
uint8_t ConfigurationServiceSetUnitStartUpFlashMode(
  ConfigurationService_t *service,
  uint8_t startUpFlashMode);
uint8_t ConfigurationServiceSetUnitFailureFlashPeriodDs(
  ConfigurationService_t *service,
  uint8_t failureFlashPeriodDs);
uint8_t ConfigurationServiceSetUserDefinedBackupContentOid(
  ConfigurationService_t *service,
  uint8_t contentIndex,
  const uint32_t *oid,
  uint8_t oidLength);
uint8_t ConfigurationServiceSetUserDefinedBackupContentDescription(
  ConfigurationService_t *service,
  uint8_t contentIndex,
  const uint8_t *description,
  uint8_t descriptionLength);
uint8_t ConfigurationServiceSetPreemptControl(ConfigurationService_t *service,
                                              uint8_t preemptIndex,
                                              uint8_t control);
uint8_t ConfigurationServiceSetPreemptLink(ConfigurationService_t *service,
                                           uint8_t preemptIndex,
                                           uint8_t link);
uint8_t ConfigurationServiceSetPreemptDelaySeconds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint16_t delaySeconds);
uint8_t ConfigurationServiceSetPreemptMinimumDurationSeconds(
  ConfigurationService_t *service,
  uint8_t
  preemptIndex,
  uint16_t
  minimumDurationSeconds);
uint8_t ConfigurationServiceSetPreemptMinimumGreenSeconds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  minimumGreenSeconds);
uint8_t ConfigurationServiceSetPreemptMinimumWalkSeconds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  minimumWalkSeconds);
uint8_t ConfigurationServiceSetPreemptEnterPedClearSeconds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  enterPedClearSeconds);
uint8_t ConfigurationServiceSetPreemptTrackGreenSeconds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  trackGreenSeconds);
uint8_t ConfigurationServiceSetPreemptDwellGreenSeconds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  dwellGreenSeconds);
uint8_t ConfigurationServiceSetPreemptMaximumPresenceSeconds(
  ConfigurationService_t *service,
  uint8_t
  preemptIndex,
  uint16_t
  maximumPresenceSeconds);
uint8_t ConfigurationServiceSetPreemptTrackPhases(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount);
uint8_t ConfigurationServiceSetPreemptDwellPhases(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount);
uint8_t ConfigurationServiceSetPreemptDwellPeds(ConfigurationService_t *service,
                                                uint8_t preemptIndex,
                                                const uint8_t *phaseNumbers,
                                                uint8_t phaseCount);
uint8_t ConfigurationServiceSetPreemptExitPhases(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount);
uint8_t ConfigurationServiceSetPreemptTrackOverlaps(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *
  overlapNumbers,
  uint8_t overlapCount);
uint8_t ConfigurationServiceSetPreemptDwellOverlaps(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *
  overlapNumbers,
  uint8_t overlapCount);
uint8_t ConfigurationServiceSetPreemptCyclingPhases(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount);
uint8_t ConfigurationServiceSetPreemptCyclingPeds(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *phaseNumbers,
  uint8_t phaseCount);
uint8_t ConfigurationServiceSetPreemptCyclingOverlaps(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  const uint8_t *
  overlapNumbers,
  uint8_t overlapCount);
uint8_t ConfigurationServiceSetPreemptEnterYellowChangeDs(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  enterYellowChangeDs);
uint8_t ConfigurationServiceSetPreemptEnterRedClearDs(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t enterRedClearDs);
uint8_t ConfigurationServiceSetPreemptTrackYellowChangeDs(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t
  trackYellowChangeDs);
uint8_t ConfigurationServiceSetPreemptTrackRedClearDs(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t trackRedClearDs);
uint8_t ConfigurationServiceSetPreemptSequenceNumber(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t sequenceNumber);
uint8_t ConfigurationServiceSetPreemptExitType(ConfigurationService_t *service,
                                               uint8_t preemptIndex,
                                               uint8_t exitType);
uint8_t ConfigurationServiceSetPreemptQueueDelayWeight(
  ConfigurationService_t *service,
  uint8_t preemptIndex,
  uint8_t detectorIndex,
  uint16_t detectorWeight);
uint8_t ConfigurationServiceSetPreemptGateDescription(
  ConfigurationService_t *service,
  uint8_t gateIndex,
  const uint8_t *description,
  uint8_t descriptionLength);

uint8_t ConfigurationServiceReadMigrationJournal(
  ConfigurationService_t *service,
  ConfigurationMigrationJournal_t
  *journal);
uint8_t ConfigurationServiceWriteMigrationJournal(
  ConfigurationService_t *service,
  const
  ConfigurationMigrationJournal_t
  *journal);

#endif /* CONFIGURATION_SERVICE_H */
