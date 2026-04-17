/* App/Domain/NTCIP/Mib1202v0335/DetectorObjects.c
 *
 * 1202 detector subtree projection backed by canonical detector config,
 * engine runtime, and module-bus health.
 */
#include "DetectorObjects.h"

#include <stddef.h>

enum
{
  DETECTOR_OBJECT_TAG_MAX_VEHICLE_DETECTORS = 1,
  DETECTOR_OBJECT_TAG_VEHICLE_NUMBER,
  DETECTOR_OBJECT_TAG_VEHICLE_OPTIONS,
  DETECTOR_OBJECT_TAG_VEHICLE_CALL_PHASE,
  DETECTOR_OBJECT_TAG_VEHICLE_SWITCH_PHASE,
  DETECTOR_OBJECT_TAG_VEHICLE_DELAY,
  DETECTOR_OBJECT_TAG_VEHICLE_EXTEND,
  DETECTOR_OBJECT_TAG_VEHICLE_QUEUE_LIMIT,
  DETECTOR_OBJECT_TAG_VEHICLE_NO_ACTIVITY,
  DETECTOR_OBJECT_TAG_VEHICLE_MAX_PRESENCE,
  DETECTOR_OBJECT_TAG_VEHICLE_ERRATIC_COUNTS,
  DETECTOR_OBJECT_TAG_VEHICLE_FAIL_TIME,
  DETECTOR_OBJECT_TAG_VEHICLE_ALARMS,
  DETECTOR_OBJECT_TAG_VEHICLE_REPORTED_ALARMS,
  DETECTOR_OBJECT_TAG_VEHICLE_RESET,
  DETECTOR_OBJECT_TAG_VEHICLE_OPTIONS2,
  DETECTOR_OBJECT_TAG_VEHICLE_PAIRED_DETECTOR,
  DETECTOR_OBJECT_TAG_VEHICLE_PAIRED_DETECTOR_SPACING,
  DETECTOR_OBJECT_TAG_VEHICLE_AVG_VEHICLE_LENGTH,
  DETECTOR_OBJECT_TAG_VEHICLE_LENGTH,
  DETECTOR_OBJECT_TAG_VEHICLE_TRAVEL_MODE,
  DETECTOR_OBJECT_TAG_MAX_VEHICLE_STATUS_GROUPS,
  DETECTOR_OBJECT_TAG_VEHICLE_STATUS_GROUP_NUMBER,
  DETECTOR_OBJECT_TAG_VEHICLE_STATUS_GROUP_ACTIVE,
  DETECTOR_OBJECT_TAG_VEHICLE_STATUS_GROUP_ALARMS,
  DETECTOR_OBJECT_TAG_MAX_PEDESTRIAN_DETECTORS,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_NUMBER,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_CALL_PHASE,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_NO_ACTIVITY,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_MAX_PRESENCE,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_ERRATIC_COUNTS,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_ALARMS,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_RESET,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_BUTTON_PUSH_TIME,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_OPTIONS,
  DETECTOR_OBJECT_TAG_MAX_PEDESTRIAN_STATUS_GROUPS,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_STATUS_GROUP_NUMBER,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_STATUS_GROUP_ACTIVE,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_STATUS_GROUP_ALARMS,
  DETECTOR_OBJECT_TAG_MAX_VEHICLE_CONTROL_GROUPS,
  DETECTOR_OBJECT_TAG_VEHICLE_CONTROL_GROUP_NUMBER,
  DETECTOR_OBJECT_TAG_VEHICLE_CONTROL_GROUP_ACTUATION,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_CONTROL_GROUP_NUMBER,
  DETECTOR_OBJECT_TAG_PEDESTRIAN_CONTROL_GROUP_ACTUATION
};

enum
{
  DETECTOR_ALARM_BIT_NO_ACTIVITY = 0x01U,
  DETECTOR_ALARM_BIT_MAX_PRESENCE = 0x02U,
  DETECTOR_ALARM_BIT_ERRATIC = 0x04U,
  DETECTOR_ALARM_BIT_COMMUNICATIONS = 0x08U,
  DETECTOR_ALARM_BIT_CONFIGURATION = 0x10U,
  DETECTOR_ALARM_BIT_OTHER = 0x80U
};

static const uint32_t kMaxVehicleDetectorsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                    1206U, 4U, 2U, 1U, 2U,
                                                    1U };
static const uint32_t kVehicleDetectorNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                      1206U, 4U, 2U, 1U, 2U,
                                                      2U, 1U, 1U };
static const uint32_t kVehicleDetectorOptionsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                       1206U, 4U, 2U, 1U, 2U,
                                                       2U, 1U, 2U };
static const uint32_t kVehicleDetectorCallPhaseOid[] = { 1U, 3U, 6U, 1U, 4U,
                                                         1U, 1206U, 4U, 2U,
                                                         1U, 2U, 2U, 1U, 4U };
static const uint32_t kVehicleDetectorSwitchPhaseOid[] = { 1U, 3U, 6U, 1U,
                                                           4U, 1U, 1206U, 4U,
                                                           2U, 1U, 2U, 2U, 1U,
                                                           5U };
static const uint32_t kVehicleDetectorDelayOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                     1206U, 4U, 2U, 1U, 2U,
                                                     2U, 1U, 6U };
static const uint32_t kVehicleDetectorExtendOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                      1206U, 4U, 2U, 1U, 2U,
                                                      2U, 1U, 7U };
static const uint32_t kVehicleDetectorQueueLimitOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 8U
};
static const uint32_t kVehicleDetectorNoActivityOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 9U
};
static const uint32_t kVehicleDetectorMaxPresenceOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 10U
};
static const uint32_t kVehicleDetectorErraticCountsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 11U
};
static const uint32_t kVehicleDetectorFailTimeOid[] = { 1U, 3U, 6U, 1U, 4U,
                                                        1U, 1206U, 4U, 2U,
                                                        1U, 2U, 2U, 1U, 12U };
static const uint32_t kVehicleDetectorAlarmsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                      1206U, 4U, 2U, 1U, 2U,
                                                      2U, 1U, 13U };
static const uint32_t kVehicleDetectorReportedAlarmsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 14U
};
static const uint32_t kVehicleDetectorResetOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                     1206U, 4U, 2U, 1U, 2U,
                                                     2U, 1U, 15U };
static const uint32_t kVehicleDetectorOptions2Oid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 16U
};
static const uint32_t kVehicleDetectorPairedDetectorOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 17U
};
static const uint32_t kVehicleDetectorPairedDetectorSpacingOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 18U
};
static const uint32_t kVehicleDetectorAvgVehicleLengthOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 19U
};
static const uint32_t kVehicleDetectorLengthOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                      1206U, 4U, 2U, 1U, 2U,
                                                      2U, 1U, 20U };
static const uint32_t kVehicleDetectorTravelModeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 21U
};
static const uint32_t kMaxVehicleDetectorStatusGroupsOid[] = { 1U, 3U, 6U, 1U,
                                                               4U, 1U, 1206U,
                                                               4U, 2U, 1U, 2U,
                                                               3U };
static const uint32_t kVehicleDetectorStatusGroupNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 4U, 1U, 1U
};
static const uint32_t kVehicleDetectorStatusGroupActiveOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 4U, 1U, 2U
};
static const uint32_t kVehicleDetectorStatusGroupAlarmsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 4U, 1U, 3U
};
static const uint32_t kMaxPedestrianDetectorsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                       1206U, 4U, 2U, 1U, 2U,
                                                       6U };
static const uint32_t kPedestrianDetectorNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 1U
};
static const uint32_t kPedestrianDetectorCallPhaseOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 2U
};
static const uint32_t kPedestrianDetectorNoActivityOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 3U
};
static const uint32_t kPedestrianDetectorMaxPresenceOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 4U
};
static const uint32_t kPedestrianDetectorErraticCountsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 5U
};
static const uint32_t kPedestrianDetectorAlarmsOid[] = { 1U, 3U, 6U, 1U, 4U,
                                                         1U, 1206U, 4U, 2U,
                                                         1U, 2U, 7U, 1U, 6U };
static const uint32_t kPedestrianDetectorResetOid[] = { 1U, 3U, 6U, 1U, 4U,
                                                        1U, 1206U, 4U, 2U,
                                                        1U, 2U, 7U, 1U, 7U };
static const uint32_t kPedestrianButtonPushTimeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 8U
};
static const uint32_t kPedestrianDetectorOptionsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 9U
};
static const uint32_t kMaxPedestrianDetectorGroupsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 8U
};
static const uint32_t kPedestrianDetectorStatusGroupNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 9U, 1U, 1U
};
static const uint32_t kPedestrianDetectorStatusGroupActiveOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 9U, 1U, 2U
};
static const uint32_t kPedestrianDetectorStatusGroupAlarmsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 9U, 1U, 3U
};
static const uint32_t kMaxVehicleDetectorControlGroupsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 11U
};
static const uint32_t kVehicleDetectorControlGroupNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 12U, 1U, 1U
};
static const uint32_t kVehicleDetectorControlGroupActuationOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 12U, 1U, 2U
};
static const uint32_t kPedestrianDetectorControlGroupNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 13U, 1U, 1U
};
static const uint32_t kPedestrianDetectorControlGroupActuationOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 13U, 1U, 2U
};

static uint8_t GetGroupBitMask(uint8_t oneBasedNumber)
{
  return (uint8_t) (1U << ((oneBasedNumber - 1U) % 8U));
}

static uint8_t GetStatusGroupCount(uint8_t objectCount)
{
  return (uint8_t) ((objectCount + 7U) / 8U);
}

static const IntersectionRuntime_t *GetRuntime(const NtcipContext_t *context)
{
  if ((context == NULL) || (context->intersectionEngine == NULL))
  {
    return NULL;
  }

  return IntersectionEngineGetRuntime(context->intersectionEngine);
}

static uint8_t GetControllerSnapshot(const NtcipContext_t *context,
                                     ModuleBusSnapshot_t *snapshot)
{
  if ((context == NULL) || (snapshot == NULL)
      || (context->intersectionController == NULL))
  {
    return 0U;
  }

  return IntersectionControllerGetLastSnapshot(context->intersectionController,
                                               snapshot);
}

static NtcipError_t ValidateDatabaseWrite(const NtcipContext_t *context,
                                          const NtcipRequestContext_t *
                                          requestContext)
{
  if ((context == NULL) || (context->dbTransactionService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return NtcipDbTransactionServiceValidateDatabaseWrite(
    context->dbTransactionService,
    requestContext);
}

static NtcipError_t GetVehicleDetectorIndex(const NtcipContext_t *context,
                                            const uint32_t *indexes,
                                            uint8_t indexCount,
                                            uint8_t *detectorIndex,
                                            IntersectionVehicleDetectorConfig_t *
                                            detector)
{
  uint8_t detectorCount;

  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 1U)
      || (detectorIndex == NULL) || (detector == NULL) || (indexes[0] == 0U))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  detectorCount =
    ConfigurationServiceGetVehicleDetectorCount(context->configurationService);

  if (indexes[0] > detectorCount)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *detectorIndex = (uint8_t) (indexes[0] - 1U);

  if (ConfigurationServiceGetActiveVehicleDetectorConfig(
        context->configurationService,
        *detectorIndex,
        detector) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetPedestrianDetectorIndex(
  const NtcipContext_t *context,
  const uint32_t *indexes,
  uint8_t indexCount,
  uint8_t *detectorIndex,
  IntersectionPedestrianDetectorConfig_t *detector)
{
  uint8_t detectorCount;

  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 1U)
      || (detectorIndex == NULL) || (detector == NULL) || (indexes[0] == 0U))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  detectorCount = ConfigurationServiceGetPedestrianDetectorCount(
    context->configurationService);

  if (indexes[0] > detectorCount)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *detectorIndex = (uint8_t) (indexes[0] - 1U);

  if (ConfigurationServiceGetActivePedestrianDetectorConfig(
        context->configurationService,
        *detectorIndex,
        detector) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetStatusGroupNumber(uint8_t objectCount,
                                         const uint32_t *indexes,
                                         uint8_t indexCount,
                                         uint8_t *groupNumber)
{
  uint8_t maxGroups;

  if ((indexes == NULL) || (indexCount != 1U) || (groupNumber == NULL)
      || (indexes[0] == 0U))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  maxGroups = GetStatusGroupCount(objectCount);

  if ((maxGroups == 0U) || (indexes[0] > maxGroups))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *groupNumber = (uint8_t) indexes[0];

  return NTCIP_ERROR_OK;
}

static uint8_t ReadVehicleDetectorReportedAlarm(const NtcipContext_t *context,
                                                uint8_t detectorIndex)
{
  ModuleBusSnapshot_t snapshot;

  if ((context == NULL)
      || (GetControllerSnapshot(context, &snapshot) == 0U)
      || (ModuleBusSnapshotSourceReady(&snapshot,
                                       MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS)
          == 0U))
  {
    return 0U;
  }

  return ModuleBusSnapshotVehicleDetectorReportedAlarmBits(
    &snapshot,
    (uint8_t) (detectorIndex + 1U));
}

static uint8_t IssueDetectorReset(const NtcipContext_t *context,
                                  ModuleBusDetectorClass_t detectorClass,
                                  uint8_t detectorNumber)
{
  if ((context == NULL) || (context->intersectionController == NULL)
      || (context->intersectionController->moduleBusPort == NULL))
  {
    return 0U;
  }

  return ModuleBusCommandDetectorReset(
    context->intersectionController->moduleBusPort,
    detectorClass,
    detectorNumber);
}

static uint8_t ComputeVehicleDetectorAlarm(const NtcipContext_t *context,
                                           uint8_t detectorIndex)
{
  ModuleBusSnapshot_t snapshot;
  IntersectionVehicleDetectorConfig_t detector;
  uint8_t alarmBits = 0U;

  if ((context == NULL) || (context->configurationService == NULL)
      || (ConfigurationServiceGetActiveVehicleDetectorConfig(
            context->configurationService,
            detectorIndex,
            &detector) == 0U))
  {
    return DETECTOR_ALARM_BIT_OTHER;
  }

  if ((GetControllerSnapshot(context, &snapshot) != 0U)
      && (ModuleBusSnapshotSourceReady(&snapshot,
                                       MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS)
          != 0U))
  {
    alarmBits |= ModuleBusSnapshotVehicleDetectorAlarmBits(
      &snapshot,
      (uint8_t) (detectorIndex + 1U));
  }

  if ((detector.callPhase != 0U) && (GetControllerSnapshot(context, &snapshot)
                                     != 0U)
      && (ModuleBusSnapshotSourceReady(&snapshot,
                                       MODULE_BUS_SNAPSHOT_VALID_DETECTORS)
          == 0U))
  {
    alarmBits |= DETECTOR_ALARM_BIT_COMMUNICATIONS;
  }

  return alarmBits;
}

static uint8_t ComputePedestrianDetectorAlarm(const NtcipContext_t *context,
                                              uint8_t detectorIndex)
{
  ModuleBusSnapshot_t snapshot;
  IntersectionPedestrianDetectorConfig_t detector;
  uint8_t alarmBits = 0U;

  if ((context == NULL) || (context->configurationService == NULL)
      || (ConfigurationServiceGetActivePedestrianDetectorConfig(
            context->configurationService,
            detectorIndex,
            &detector) == 0U))
  {
    return DETECTOR_ALARM_BIT_OTHER;
  }

  if ((GetControllerSnapshot(context, &snapshot) != 0U)
      && (ModuleBusSnapshotSourceReady(&snapshot,
                                       MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS)
          != 0U))
  {
    alarmBits |= ModuleBusSnapshotPedestrianDetectorAlarmBits(
      &snapshot,
      (uint8_t) (detectorIndex + 1U));
  }

  if ((detector.callPhase != 0U) && (GetControllerSnapshot(context, &snapshot)
                                     != 0U)
      && (ModuleBusSnapshotSourceReady(&snapshot,
                                       MODULE_BUS_SNAPSHOT_VALID_PEDS) == 0U))
  {
    alarmBits |= DETECTOR_ALARM_BIT_COMMUNICATIONS;
  }

  return alarmBits;
}

static void ReadVehicleStatusGroup(const NtcipContext_t *context,
                                   uint8_t groupNumber,
                                   uint8_t *activeMask,
                                   uint8_t *alarmMask)
{
  uint8_t detectorNumber;
  uint8_t detectorCount;

  *activeMask = 0U;
  *alarmMask = 0U;

  if ((context == NULL) || (context->configurationService == NULL)
      || (context->intersectionEngine == NULL))
  {
    return;
  }

  detectorCount =
    ConfigurationServiceGetVehicleDetectorCount(context->configurationService);
  (void) IntersectionEngineGetVehicleDetectorStatusGroup(context->
                                                         intersectionEngine,
                                                         groupNumber,
                                                         activeMask);

  for (detectorNumber = (uint8_t) (((groupNumber - 1U) * 8U) + 1U);
       (detectorNumber <= detectorCount)
       && (detectorNumber <= (uint8_t) (groupNumber * 8U));
       ++detectorNumber)
  {
    if (ComputeVehicleDetectorAlarm(context,
                                    (uint8_t) (detectorNumber - 1U)) != 0U)
    {
      *alarmMask |= GetGroupBitMask(detectorNumber);
    }
  }
}

static void ReadPedestrianStatusGroup(const NtcipContext_t *context,
                                      uint8_t groupNumber,
                                      uint8_t *activeMask,
                                      uint8_t *alarmMask)
{
  uint8_t detectorNumber;
  uint8_t detectorCount;

  *activeMask = 0U;
  *alarmMask = 0U;

  if ((context == NULL) || (context->configurationService == NULL)
      || (context->intersectionEngine == NULL))
  {
    return;
  }

  detectorCount = ConfigurationServiceGetPedestrianDetectorCount(
    context->configurationService);
  (void) IntersectionEngineGetPedestrianDetectorStatusGroup(
    context->intersectionEngine,
    groupNumber,
    activeMask);

  for (detectorNumber = (uint8_t) (((groupNumber - 1U) * 8U) + 1U);
       (detectorNumber <= detectorCount)
       && (detectorNumber <= (uint8_t) (groupNumber * 8U));
       ++detectorNumber)
  {
    if (ComputePedestrianDetectorAlarm(context,
                                       (uint8_t) (detectorNumber - 1U)) != 0U)
    {
      *alarmMask |= GetGroupBitMask(detectorNumber);
    }
  }
}

static void ReadVehicleControlGroupActuation(const NtcipContext_t *context,
                                             uint8_t groupNumber,
                                             uint8_t *mask)
{
  const IntersectionRuntime_t *runtime = GetRuntime(context);
  uint8_t detectorCount;
  uint8_t detectorNumber;

  *mask = 0U;

  if ((runtime == NULL) || (context == NULL)
      || (context->configurationService == NULL))
  {
    return;
  }

  detectorCount =
    ConfigurationServiceGetVehicleDetectorCount(context->configurationService);

  for (detectorNumber = (uint8_t) (((groupNumber - 1U) * 8U) + 1U);
       (detectorNumber <= detectorCount)
       && (detectorNumber <= (uint8_t) (groupNumber * 8U));
       ++detectorNumber)
  {
    if (runtime->vehicleDetectors[detectorNumber - 1U].remoteActuation != 0U)
    {
      *mask |= GetGroupBitMask(detectorNumber);
    }
  }
}

static void ReadPedestrianControlGroupActuation(const NtcipContext_t *context,
                                                uint8_t groupNumber,
                                                uint8_t *mask)
{
  const IntersectionRuntime_t *runtime = GetRuntime(context);
  uint8_t detectorCount;
  uint8_t detectorNumber;

  *mask = 0U;

  if ((runtime == NULL) || (context == NULL)
      || (context->configurationService == NULL))
  {
    return;
  }

  detectorCount = ConfigurationServiceGetPedestrianDetectorCount(
    context->configurationService);

  for (detectorNumber = (uint8_t) (((groupNumber - 1U) * 8U) + 1U);
       (detectorNumber <= detectorCount)
       && (detectorNumber <= (uint8_t) (groupNumber * 8U));
       ++detectorNumber)
  {
    if (runtime->pedestrianDetectors[detectorNumber - 1U].remoteActuation
        != 0U)
    {
      *mask |= GetGroupBitMask(detectorNumber);
    }
  }
}

static NtcipError_t ApplyVehicleControlGroupActuation(
  const NtcipContext_t *context,
  uint8_t groupNumber,
  uint8_t mask)
{
  uint8_t detectorCount;
  uint8_t detectorNumber;

  if ((context == NULL) || (context->configurationService == NULL)
      || (context->intersectionEngine == NULL))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  detectorCount =
    ConfigurationServiceGetVehicleDetectorCount(context->configurationService);

  for (detectorNumber = (uint8_t) (((groupNumber - 1U) * 8U) + 1U);
       (detectorNumber <= detectorCount)
       && (detectorNumber <= (uint8_t) (groupNumber * 8U));
       ++detectorNumber)
  {
    uint8_t active = (uint8_t) ((mask & GetGroupBitMask(detectorNumber)) != 0U);

    if (IntersectionEngineSetVehicleDetectorRemoteActuation(
          context->intersectionEngine,
          detectorNumber,
          active) == 0U)
    {
      return NTCIP_ERROR_GEN_ERROR;
    }
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t ApplyPedestrianControlGroupActuation(
  const NtcipContext_t *context,
  uint8_t groupNumber,
  uint8_t mask)
{
  uint8_t detectorCount;
  uint8_t detectorNumber;

  if ((context == NULL) || (context->configurationService == NULL)
      || (context->intersectionEngine == NULL))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  detectorCount = ConfigurationServiceGetPedestrianDetectorCount(
    context->configurationService);

  for (detectorNumber = (uint8_t) (((groupNumber - 1U) * 8U) + 1U);
       (detectorNumber <= detectorCount)
       && (detectorNumber <= (uint8_t) (groupNumber * 8U));
       ++detectorNumber)
  {
    uint8_t active = (uint8_t) ((mask & GetGroupBitMask(detectorNumber)) != 0U);

    if (IntersectionEngineSetPedestrianDetectorRemoteActuation(
          context->intersectionEngine,
          detectorNumber,
          active) == 0U)
    {
      return NTCIP_ERROR_GEN_ERROR;
    }
  }

  return NTCIP_ERROR_OK;
}

static uint8_t IsVehicleTableTag(uint16_t tag)
{
  return (uint8_t) ((tag >= DETECTOR_OBJECT_TAG_VEHICLE_NUMBER)
                    && (tag <= DETECTOR_OBJECT_TAG_VEHICLE_TRAVEL_MODE));
}

static uint8_t IsPedestrianTableTag(uint16_t tag)
{
  return (uint8_t) ((tag >= DETECTOR_OBJECT_TAG_PEDESTRIAN_NUMBER)
                    && (tag <= DETECTOR_OBJECT_TAG_PEDESTRIAN_OPTIONS));
}

static NtcipError_t GetDetectorObject(void *groupContext,
                                      const NtcipObjectDescriptor_t *descriptor,
                                      const uint32_t *indexes,
                                      uint8_t indexCount,
                                      const NtcipRequestContext_t *requestContext,
                                      NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionVehicleDetectorConfig_t vehicleDetector;
  IntersectionPedestrianDetectorConfig_t pedestrianDetector;
  uint8_t detectorIndex = 0U;
  uint8_t groupNumber = 0U;
  uint8_t activeMask = 0U;
  uint8_t alarmMask = 0U;
  NtcipError_t error = NTCIP_ERROR_OK;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL)
      || (context->configurationService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case DETECTOR_OBJECT_TAG_MAX_VEHICLE_DETECTORS:
      case DETECTOR_OBJECT_TAG_MAX_VEHICLE_STATUS_GROUPS:
      case DETECTOR_OBJECT_TAG_MAX_PEDESTRIAN_DETECTORS:
      case DETECTOR_OBJECT_TAG_MAX_PEDESTRIAN_STATUS_GROUPS:
      case DETECTOR_OBJECT_TAG_MAX_VEHICLE_CONTROL_GROUPS:
      {
        break;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_STATUS_GROUP_NUMBER:
      case DETECTOR_OBJECT_TAG_VEHICLE_STATUS_GROUP_ACTIVE:
      case DETECTOR_OBJECT_TAG_VEHICLE_STATUS_GROUP_ALARMS:
      case DETECTOR_OBJECT_TAG_VEHICLE_CONTROL_GROUP_NUMBER:
      case DETECTOR_OBJECT_TAG_VEHICLE_CONTROL_GROUP_ACTUATION:
      {
        error = GetStatusGroupNumber(
          ConfigurationServiceGetVehicleDetectorCount(
            context->configurationService),
          indexes,
          indexCount,
          &groupNumber);
        break;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_STATUS_GROUP_NUMBER:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_STATUS_GROUP_ACTIVE:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_STATUS_GROUP_ALARMS:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_CONTROL_GROUP_NUMBER:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_CONTROL_GROUP_ACTUATION:
      {
        error = GetStatusGroupNumber(
          ConfigurationServiceGetPedestrianDetectorCount(
            context->configurationService),
          indexes,
          indexCount,
          &groupNumber);
        break;
      }

      default:
      {
        if (IsVehicleTableTag(descriptor->tag) != 0U)
        {
          error = GetVehicleDetectorIndex(context,
                                          indexes,
                                          indexCount,
                                          &detectorIndex,
                                          &vehicleDetector);
        }
        else if (IsPedestrianTableTag(descriptor->tag) != 0U)
        {
          error = GetPedestrianDetectorIndex(context,
                                             indexes,
                                             indexCount,
                                             &detectorIndex,
                                             &pedestrianDetector);
        }
        else
        {
          return NTCIP_ERROR_NOT_FOUND;
        }

        break;
      }
  }

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case DETECTOR_OBJECT_TAG_MAX_VEHICLE_DETECTORS:
      {
        NtcipValueSetUnsigned32(value,
                                ConfigurationServiceGetVehicleDetectorCount(
                                  context->configurationService));
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_MAX_VEHICLE_STATUS_GROUPS:
      case DETECTOR_OBJECT_TAG_MAX_VEHICLE_CONTROL_GROUPS:
      {
        NtcipValueSetUnsigned32(
          value,
          GetStatusGroupCount(ConfigurationServiceGetVehicleDetectorCount(
                                context->configurationService)));
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_MAX_PEDESTRIAN_DETECTORS:
      {
        NtcipValueSetUnsigned32(value,
                                ConfigurationServiceGetPedestrianDetectorCount(
                                  context->configurationService));
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_MAX_PEDESTRIAN_STATUS_GROUPS:
      {
        NtcipValueSetUnsigned32(
          value,
          GetStatusGroupCount(ConfigurationServiceGetPedestrianDetectorCount(
                                context->configurationService)));
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_NUMBER:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_NUMBER:
      case DETECTOR_OBJECT_TAG_VEHICLE_STATUS_GROUP_NUMBER:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_STATUS_GROUP_NUMBER:
      case DETECTOR_OBJECT_TAG_VEHICLE_CONTROL_GROUP_NUMBER:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_CONTROL_GROUP_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_OPTIONS:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.options);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_CALL_PHASE:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.callPhase);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_SWITCH_PHASE:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.switchPhase);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_DELAY:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.delayDs);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_EXTEND:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.extendDs);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_QUEUE_LIMIT:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.queueLimitSeconds);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_NO_ACTIVITY:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.noActivityMinutes);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_MAX_PRESENCE:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.maxPresenceMinutes);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_ERRATIC_COUNTS:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.erraticCountsPerMinute);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_FAIL_TIME:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.failTimeSeconds);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_ALARMS:
      {
        NtcipValueSetUnsigned32(value,
                                ComputeVehicleDetectorAlarm(context,
                                                            detectorIndex));
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_REPORTED_ALARMS:
      {
        NtcipValueSetUnsigned32(value,
                                ReadVehicleDetectorReportedAlarm(context,
                                                                 detectorIndex));
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_RESET:
      {
        NtcipValueSetUnsigned32(value, 0U);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_OPTIONS2:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.options2);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_PAIRED_DETECTOR:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.pairedDetector);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_PAIRED_DETECTOR_SPACING:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.pairedDetectorSpacingCm);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_AVG_VEHICLE_LENGTH:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.avgVehicleLengthCm);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_LENGTH:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.detectorLengthCm);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_TRAVEL_MODE:
      {
        NtcipValueSetUnsigned32(value, vehicleDetector.travelMode);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_STATUS_GROUP_ACTIVE:
      {
        ReadVehicleStatusGroup(context, groupNumber, &activeMask, &alarmMask);
        NtcipValueSetUnsigned32(value, activeMask);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_STATUS_GROUP_ALARMS:
      {
        ReadVehicleStatusGroup(context, groupNumber, &activeMask, &alarmMask);
        NtcipValueSetUnsigned32(value, alarmMask);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_CALL_PHASE:
      {
        NtcipValueSetUnsigned32(value, pedestrianDetector.callPhase);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_NO_ACTIVITY:
      {
        NtcipValueSetUnsigned32(value, pedestrianDetector.noActivityMinutes);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_MAX_PRESENCE:
      {
        NtcipValueSetUnsigned32(value, pedestrianDetector.maxPresenceMinutes);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_ERRATIC_COUNTS:
      {
        NtcipValueSetUnsigned32(value, pedestrianDetector.erraticCountsPerMinute);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_ALARMS:
      {
        NtcipValueSetUnsigned32(value,
                                ComputePedestrianDetectorAlarm(context,
                                                               detectorIndex));
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_RESET:
      {
        NtcipValueSetUnsigned32(value, 0U);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_BUTTON_PUSH_TIME:
      {
        NtcipValueSetUnsigned32(value, pedestrianDetector.apsMinimumActuationDs);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_OPTIONS:
      {
        NtcipValueSetUnsigned32(value, pedestrianDetector.options);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_STATUS_GROUP_ACTIVE:
      {
        ReadPedestrianStatusGroup(context, groupNumber, &activeMask,
                                  &alarmMask);
        NtcipValueSetUnsigned32(value, activeMask);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_STATUS_GROUP_ALARMS:
      {
        ReadPedestrianStatusGroup(context, groupNumber, &activeMask,
                                  &alarmMask);
        NtcipValueSetUnsigned32(value, alarmMask);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_CONTROL_GROUP_ACTUATION:
      {
        ReadVehicleControlGroupActuation(context, groupNumber, &activeMask);
        NtcipValueSetUnsigned32(value, activeMask);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_CONTROL_GROUP_ACTUATION:
      {
        ReadPedestrianControlGroupActuation(context, groupNumber, &activeMask);
        NtcipValueSetUnsigned32(value, activeMask);
        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetTestDetectorObject(void *groupContext,
                                          const NtcipObjectDescriptor_t *
                                          descriptor,
                                          const uint32_t *indexes,
                                          uint8_t indexCount,
                                          const NtcipRequestContext_t *
                                          requestContext,
                                          const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionVehicleDetectorConfig_t vehicleDetector;
  IntersectionPedestrianDetectorConfig_t pedestrianDetector;
  uint8_t detectorIndex;
  uint8_t groupNumber;
  uint8_t detectorCount;
  NtcipError_t error = NTCIP_ERROR_OK;
  uint32_t maxPhaseNumber;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL)
      || (context->configurationService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  maxPhaseNumber =
    ConfigurationServiceGetPhaseCount(context->configurationService);

  switch (descriptor->tag)
  {
      case DETECTOR_OBJECT_TAG_VEHICLE_OPTIONS:
      case DETECTOR_OBJECT_TAG_VEHICLE_CALL_PHASE:
      case DETECTOR_OBJECT_TAG_VEHICLE_SWITCH_PHASE:
      case DETECTOR_OBJECT_TAG_VEHICLE_DELAY:
      case DETECTOR_OBJECT_TAG_VEHICLE_EXTEND:
      case DETECTOR_OBJECT_TAG_VEHICLE_QUEUE_LIMIT:
      case DETECTOR_OBJECT_TAG_VEHICLE_NO_ACTIVITY:
      case DETECTOR_OBJECT_TAG_VEHICLE_MAX_PRESENCE:
      case DETECTOR_OBJECT_TAG_VEHICLE_ERRATIC_COUNTS:
      case DETECTOR_OBJECT_TAG_VEHICLE_FAIL_TIME:
      case DETECTOR_OBJECT_TAG_VEHICLE_OPTIONS2:
      case DETECTOR_OBJECT_TAG_VEHICLE_PAIRED_DETECTOR:
      case DETECTOR_OBJECT_TAG_VEHICLE_PAIRED_DETECTOR_SPACING:
      case DETECTOR_OBJECT_TAG_VEHICLE_AVG_VEHICLE_LENGTH:
      case DETECTOR_OBJECT_TAG_VEHICLE_LENGTH:
      case DETECTOR_OBJECT_TAG_VEHICLE_TRAVEL_MODE:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_CALL_PHASE:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_NO_ACTIVITY:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_MAX_PRESENCE:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_ERRATIC_COUNTS:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_BUTTON_PUSH_TIME:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_OPTIONS:
      {
        error = ValidateDatabaseWrite(context, requestContext);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }
        break;
      }

      default:
      {
        break;
      }
  }

  switch (descriptor->tag)
  {
      case DETECTOR_OBJECT_TAG_VEHICLE_OPTIONS:
      {
        error = GetVehicleDetectorIndex(context,
                                        indexes,
                                        indexCount,
                                        &detectorIndex,
                                        &vehicleDetector);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 > 255U))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_CALL_PHASE:
      case DETECTOR_OBJECT_TAG_VEHICLE_SWITCH_PHASE:
      {
        error = GetVehicleDetectorIndex(context,
                                        indexes,
                                        indexCount,
                                        &detectorIndex,
                                        &vehicleDetector);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 > maxPhaseNumber))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_DELAY:
      {
        error = GetVehicleDetectorIndex(context,
                                        indexes,
                                        indexCount,
                                        &detectorIndex,
                                        &vehicleDetector);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 > 2550U))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_EXTEND:
      case DETECTOR_OBJECT_TAG_VEHICLE_QUEUE_LIMIT:
      case DETECTOR_OBJECT_TAG_VEHICLE_NO_ACTIVITY:
      case DETECTOR_OBJECT_TAG_VEHICLE_MAX_PRESENCE:
      case DETECTOR_OBJECT_TAG_VEHICLE_ERRATIC_COUNTS:
      case DETECTOR_OBJECT_TAG_VEHICLE_FAIL_TIME:
      {
        error = GetVehicleDetectorIndex(context,
                                        indexes,
                                        indexCount,
                                        &detectorIndex,
                                        &vehicleDetector);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 > 255U))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_OPTIONS2:
      {
        uint32_t allowedMask = (uint32_t) (VEHICLE_DETECTOR_OPTIONS2_SPEED_ENABLED
                                           | VEHICLE_DETECTOR_OPTIONS2_PLACEMENT_LEAD
                                           | VEHICLE_DETECTOR_OPTIONS2_SPEED_NTCIP);
        error = GetVehicleDetectorIndex(context,
                                        indexes,
                                        indexCount,
                                        &detectorIndex,
                                        &vehicleDetector);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        if ((value->data.unsigned32 > 255U)
            || ((value->data.unsigned32 & (~allowedMask)) != 0U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_PAIRED_DETECTOR:
      {
        error = GetVehicleDetectorIndex(context,
                                        indexes,
                                        indexCount,
                                        &detectorIndex,
                                        &vehicleDetector);
        detectorCount = ConfigurationServiceGetVehicleDetectorCount(
          context->configurationService);

        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 > detectorCount)
            || (value->data.unsigned32 == (uint32_t) (detectorIndex + 1U)))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_PAIRED_DETECTOR_SPACING:
      {
        error = GetVehicleDetectorIndex(context,
                                        indexes,
                                        indexCount,
                                        &detectorIndex,
                                        &vehicleDetector);
        return error;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_AVG_VEHICLE_LENGTH:
      {
        error = GetVehicleDetectorIndex(context,
                                        indexes,
                                        indexCount,
                                        &detectorIndex,
                                        &vehicleDetector);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 == 0U)
            || (value->data.unsigned32 > 4000U))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_LENGTH:
      {
        error = GetVehicleDetectorIndex(context,
                                        indexes,
                                        indexCount,
                                        &detectorIndex,
                                        &vehicleDetector);
        if ((error != NTCIP_ERROR_OK)
            || ((value->data.unsigned32 != 65535U)
                && ((value->data.unsigned32 == 0U)
                    || (value->data.unsigned32 > 4000U))))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_TRAVEL_MODE:
      {
        error = GetVehicleDetectorIndex(context,
                                        indexes,
                                        indexCount,
                                        &detectorIndex,
                                        &vehicleDetector);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 < 1U)
            || (value->data.unsigned32 > 4U))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_CALL_PHASE:
      {
        error = GetPedestrianDetectorIndex(context,
                                           indexes,
                                           indexCount,
                                           &detectorIndex,
                                           &pedestrianDetector);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 > maxPhaseNumber))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_NO_ACTIVITY:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_MAX_PRESENCE:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_ERRATIC_COUNTS:
      case DETECTOR_OBJECT_TAG_PEDESTRIAN_BUTTON_PUSH_TIME:
      {
        error = GetPedestrianDetectorIndex(context,
                                           indexes,
                                           indexCount,
                                           &detectorIndex,
                                           &pedestrianDetector);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 > 255U))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_OPTIONS:
      {
        error = GetPedestrianDetectorIndex(context,
                                           indexes,
                                           indexCount,
                                           &detectorIndex,
                                           &pedestrianDetector);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 > 255U)
            || ((value->data.unsigned32
                 & (uint32_t) (~(uint32_t) (PED_DETECTOR_OPTIONS_PRESENCE
                                            | PED_DETECTOR_OPTIONS_ALT_TIMING
                                            | PED_DETECTOR_OPTIONS_NON_LOCKING)))
                != 0U))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_RESET:
      {
        error = GetVehicleDetectorIndex(context,
                                        indexes,
                                        indexCount,
                                        &detectorIndex,
                                        &vehicleDetector);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 > 1U))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_RESET:
      {
        error = GetPedestrianDetectorIndex(context,
                                           indexes,
                                           indexCount,
                                           &detectorIndex,
                                           &pedestrianDetector);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 > 1U))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_CONTROL_GROUP_ACTUATION:
      {
        error = GetStatusGroupNumber(
          ConfigurationServiceGetVehicleDetectorCount(
            context->configurationService),
          indexes,
          indexCount,
          &groupNumber);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 > 255U))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_CONTROL_GROUP_ACTUATION:
      {
        error = GetStatusGroupNumber(
          ConfigurationServiceGetPedestrianDetectorCount(
            context->configurationService),
          indexes,
          indexCount,
          &groupNumber);
        if ((error != NTCIP_ERROR_OK) || (value->data.unsigned32 > 255U))
        {
          return (error == NTCIP_ERROR_OK) ? NTCIP_ERROR_RANGE_ERROR : error;
        }
        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetValueDetectorObject(void *groupContext,
                                           const NtcipObjectDescriptor_t *
                                           descriptor,
                                           const uint32_t *indexes,
                                           uint8_t indexCount,
                                           const NtcipRequestContext_t *
                                           requestContext,
                                           const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionVehicleDetectorConfig_t vehicleDetector;
  IntersectionPedestrianDetectorConfig_t pedestrianDetector;
  uint8_t detectorIndex = 0U;
  uint8_t groupNumber = 0U;
  NtcipError_t error;

  error = SetTestDetectorObject(groupContext,
                                descriptor,
                                indexes,
                                indexCount,
                                requestContext,
                                value);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case DETECTOR_OBJECT_TAG_VEHICLE_OPTIONS:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorOptions(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_CALL_PHASE:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorCallPhase(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_SWITCH_PHASE:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorSwitchPhase(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_DELAY:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorDelayDs(
                  context->configurationService,
                  detectorIndex,
                  (uint16_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_EXTEND:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorExtendDs(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_QUEUE_LIMIT:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorQueueLimitSeconds(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_NO_ACTIVITY:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorNoActivityMinutes(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_MAX_PRESENCE:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorMaxPresenceMinutes(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_ERRATIC_COUNTS:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorErraticCountsPerMinute(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_FAIL_TIME:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorFailTimeSeconds(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_OPTIONS2:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorOptions2(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_PAIRED_DETECTOR:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorPairedDetector(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_PAIRED_DETECTOR_SPACING:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorPairedDetectorSpacingCm(
                  context->configurationService,
                  detectorIndex,
                  (uint16_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_AVG_VEHICLE_LENGTH:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorAvgVehicleLengthCm(
                  context->configurationService,
                  detectorIndex,
                  (uint16_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_LENGTH:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorLengthCm(
                  context->configurationService,
                  detectorIndex,
                  (uint16_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_TRAVEL_MODE:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        return (ConfigurationServiceSetVehicleDetectorTravelMode(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_CALL_PHASE:
      {
        (void) GetPedestrianDetectorIndex(context,
                                          indexes,
                                          indexCount,
                                          &detectorIndex,
                                          &pedestrianDetector);
        return (ConfigurationServiceSetPedestrianDetectorCallPhase(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_NO_ACTIVITY:
      {
        (void) GetPedestrianDetectorIndex(context,
                                          indexes,
                                          indexCount,
                                          &detectorIndex,
                                          &pedestrianDetector);
        return (ConfigurationServiceSetPedestrianDetectorNoActivityMinutes(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_MAX_PRESENCE:
      {
        (void) GetPedestrianDetectorIndex(context,
                                          indexes,
                                          indexCount,
                                          &detectorIndex,
                                          &pedestrianDetector);
        return (ConfigurationServiceSetPedestrianDetectorMaxPresenceMinutes(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_ERRATIC_COUNTS:
      {
        (void) GetPedestrianDetectorIndex(context,
                                          indexes,
                                          indexCount,
                                          &detectorIndex,
                                          &pedestrianDetector);
        return (ConfigurationServiceSetPedestrianDetectorErraticCountsPerMinute(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_BUTTON_PUSH_TIME:
      {
        (void) GetPedestrianDetectorIndex(context,
                                          indexes,
                                          indexCount,
                                          &detectorIndex,
                                          &pedestrianDetector);
        return (ConfigurationServiceSetPedestrianDetectorApsMinimumActuationDs(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_OPTIONS:
      {
        (void) GetPedestrianDetectorIndex(context,
                                          indexes,
                                          indexCount,
                                          &detectorIndex,
                                          &pedestrianDetector);
        return (ConfigurationServiceSetPedestrianDetectorOptions(
                  context->configurationService,
                  detectorIndex,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_RESET:
      {
        (void) GetVehicleDetectorIndex(context,
                                       indexes,
                                       indexCount,
                                       &detectorIndex,
                                       &vehicleDetector);
        if (value->data.unsigned32 == 0U)
        {
          return NTCIP_ERROR_OK;
        }

        return (IssueDetectorReset(context,
                                   MODULE_BUS_DETECTOR_CLASS_VEHICLE,
                                   (uint8_t) (detectorIndex + 1U)) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_RESET:
      {
        (void) GetPedestrianDetectorIndex(context,
                                          indexes,
                                          indexCount,
                                          &detectorIndex,
                                          &pedestrianDetector);
        if (value->data.unsigned32 == 0U)
        {
          return NTCIP_ERROR_OK;
        }

        return (IssueDetectorReset(context,
                                   MODULE_BUS_DETECTOR_CLASS_PEDESTRIAN,
                                   (uint8_t) (detectorIndex + 1U)) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_OBJECT_TAG_VEHICLE_CONTROL_GROUP_ACTUATION:
      {
        (void) GetStatusGroupNumber(
          ConfigurationServiceGetVehicleDetectorCount(
            context->configurationService),
          indexes,
          indexCount,
          &groupNumber);
        return ApplyVehicleControlGroupActuation(context,
                                                 groupNumber,
                                                 (uint8_t) value->data.unsigned32);
      }

      case DETECTOR_OBJECT_TAG_PEDESTRIAN_CONTROL_GROUP_ACTUATION:
      {
        (void) GetStatusGroupNumber(
          ConfigurationServiceGetPedestrianDetectorCount(
            context->configurationService),
          indexes,
          indexCount,
          &groupNumber);
        return ApplyPedestrianControlGroupActuation(
          context,
          groupNumber,
          (uint8_t) value->data.unsigned32);
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static const NtcipObjectDescriptor_t kDetectorObjects[] =
{
  { kMaxVehicleDetectorsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_MAX_VEHICLE_DETECTORS, GetDetectorObject, NULL, NULL },
  { kVehicleDetectorNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_NUMBER, GetDetectorObject, NULL, NULL },
  { kVehicleDetectorOptionsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_OPTIONS, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorCallPhaseOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_CALL_PHASE, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorSwitchPhaseOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_SWITCH_PHASE, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorDelayOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_DELAY, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorExtendOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_EXTEND, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorQueueLimitOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_QUEUE_LIMIT, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorNoActivityOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_NO_ACTIVITY, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorMaxPresenceOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_MAX_PRESENCE, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorErraticCountsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_ERRATIC_COUNTS, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorFailTimeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_FAIL_TIME, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorAlarmsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_ALARMS, GetDetectorObject, NULL, NULL },
  { kVehicleDetectorReportedAlarmsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_REPORTED_ALARMS, GetDetectorObject, NULL,
    NULL },
  { kVehicleDetectorResetOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_RESET, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorOptions2Oid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_OPTIONS2, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorPairedDetectorOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_PAIRED_DETECTOR, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorPairedDetectorSpacingOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_PAIRED_DETECTOR_SPACING, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorAvgVehicleLengthOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, DETECTOR_OBJECT_TAG_VEHICLE_AVG_VEHICLE_LENGTH,
    GetDetectorObject, SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorLengthOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_LENGTH, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kVehicleDetectorTravelModeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_TRAVEL_MODE, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kMaxVehicleDetectorStatusGroupsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_MAX_VEHICLE_STATUS_GROUPS, GetDetectorObject, NULL,
    NULL },
  { kVehicleDetectorStatusGroupNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN,
    1U, NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_STATUS_GROUP_NUMBER, GetDetectorObject, NULL,
    NULL },
  { kVehicleDetectorStatusGroupActiveOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN,
    1U, NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_STATUS_GROUP_ACTIVE, GetDetectorObject, NULL,
    NULL },
  { kVehicleDetectorStatusGroupAlarmsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN,
    1U, NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_STATUS_GROUP_ALARMS, GetDetectorObject, NULL,
    NULL },
  { kMaxPedestrianDetectorsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_MAX_PEDESTRIAN_DETECTORS, GetDetectorObject, NULL,
    NULL },
  { kPedestrianDetectorNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_NUMBER, GetDetectorObject, NULL, NULL },
  { kPedestrianDetectorCallPhaseOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_CALL_PHASE, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kPedestrianDetectorNoActivityOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_NO_ACTIVITY, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kPedestrianDetectorMaxPresenceOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_MAX_PRESENCE, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kPedestrianDetectorErraticCountsOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, DETECTOR_OBJECT_TAG_PEDESTRIAN_ERRATIC_COUNTS,
    GetDetectorObject, SetTestDetectorObject, SetValueDetectorObject },
  { kPedestrianDetectorAlarmsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_ALARMS, GetDetectorObject, NULL, NULL },
  { kPedestrianDetectorResetOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_RESET, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kPedestrianButtonPushTimeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_BUTTON_PUSH_TIME, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kPedestrianDetectorOptionsOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_OPTIONS, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kMaxPedestrianDetectorGroupsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_MAX_PEDESTRIAN_STATUS_GROUPS, GetDetectorObject, NULL,
    NULL },
  { kPedestrianDetectorStatusGroupNumberOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_STATUS_GROUP_NUMBER, GetDetectorObject,
    NULL, NULL },
  { kPedestrianDetectorStatusGroupActiveOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_STATUS_GROUP_ACTIVE, GetDetectorObject,
    NULL, NULL },
  { kPedestrianDetectorStatusGroupAlarmsOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_STATUS_GROUP_ALARMS, GetDetectorObject,
    NULL, NULL },
  { kMaxVehicleDetectorControlGroupsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_MAX_VEHICLE_CONTROL_GROUPS, GetDetectorObject, NULL,
    NULL },
  { kVehicleDetectorControlGroupNumberOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_CONTROL_GROUP_NUMBER, GetDetectorObject, NULL,
    NULL },
  { kVehicleDetectorControlGroupActuationOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_VEHICLE_CONTROL_GROUP_ACTUATION, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject },
  { kPedestrianDetectorControlGroupNumberOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_CONTROL_GROUP_NUMBER, GetDetectorObject,
    NULL, NULL },
  { kPedestrianDetectorControlGroupActuationOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_OBJECT_TAG_PEDESTRIAN_CONTROL_GROUP_ACTUATION, GetDetectorObject,
    SetTestDetectorObject, SetValueDetectorObject }
};

void DetectorObjectsRegister(NtcipObjectDirectory_t *directory,
                             NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.detector",
    kDetectorObjects,
    (uint16_t) (sizeof(kDetectorObjects) / sizeof(kDetectorObjects[0])),
    context);
}
