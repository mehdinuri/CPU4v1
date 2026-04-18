/* App/Domain/NTCIP/Mib1202v0335/UnitObjects.c
 *
 * Unit status projection rooted in the 1202 unit subtree. Values are derived
 * conservatively from engine runtime and controller module-bus health.
 */
#include "UnitObjects.h"

#include <stddef.h>

enum
{
  UNIT_OBJECT_TAG_STARTUP_FLASH = 1,
  UNIT_OBJECT_TAG_AUTO_PEDESTRIAN_CLEAR,
  UNIT_OBJECT_TAG_BACKUP_TIME,
  UNIT_OBJECT_TAG_RED_REVERT,
  UNIT_OBJECT_TAG_CONTROL_STATUS,
  UNIT_OBJECT_TAG_FLASH_STATUS,
  UNIT_OBJECT_TAG_CONTROL,
  UNIT_OBJECT_TAG_MAX_ALARM_GROUPS,
  UNIT_OBJECT_TAG_ALARM_GROUP_NUMBER,
  UNIT_OBJECT_TAG_ALARM_GROUP_STATE,
  UNIT_OBJECT_TAG_ALARM_STATUS2,
  UNIT_OBJECT_TAG_ALARM_STATUS1,
  UNIT_OBJECT_TAG_SHORT_ALARM_STATUS,
  UNIT_OBJECT_TAG_STARTUP_FLASH_MODE,
  UNIT_OBJECT_TAG_MCE_TIMEOUT,
  UNIT_OBJECT_TAG_MCE_INTERVAL_ADVANCE,
  UNIT_OBJECT_TAG_ASC_ELEVATION_OFFSET,
  UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_TIME,
  UNIT_OBJECT_TAG_MAX_USER_DEFINED_BACKUP_CONTENT,
  UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_NUMBER,
  UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_OID,
  UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_DESCRIPTION,
  UNIT_OBJECT_TAG_MAX_GLOBAL_SET_IDS,
  UNIT_OBJECT_TAG_GLOBAL_SET_ID_NUMBER,
  UNIT_OBJECT_TAG_GLOBAL_SET_ID_OID,
  UNIT_OBJECT_TAG_ALARM_STATUS3,
  UNIT_OBJECT_TAG_ALARM_STATUS4
};

enum
{
  UNIT_CONTROL_STATUS_OTHER = 1,
  UNIT_CONTROL_STATUS_SYSTEM_CONTROL = 2,
  UNIT_CONTROL_STATUS_BACKUP_MODE = 4,
  UNIT_CONTROL_STATUS_MANUAL = 5,
  UNIT_CONTROL_STATUS_TIMEBASE = 6,
  UNIT_CONTROL_STATUS_INTERCONNECT = 7,
  UNIT_CONTROL_STATUS_INTERCONNECT_BACKUP = 8,
  UNIT_CONTROL_STATUS_REMOTE_MANUAL_CONTROL = 9,
  UNIT_CONTROL_STATUS_LOCAL_MANUAL_CONTROL = 10
};

enum
{
  UNIT_FLASH_STATUS_OTHER = 1,
  UNIT_FLASH_STATUS_NOT_FLASH = 2,
  UNIT_FLASH_STATUS_AUTOMATIC = 3,
  UNIT_FLASH_STATUS_LOCAL_MANUAL = 4,
  UNIT_FLASH_STATUS_FAULT_MONITOR = 5,
  UNIT_FLASH_STATUS_MMU = 6,
  UNIT_FLASH_STATUS_STARTUP = 7,
  UNIT_FLASH_STATUS_PREEMPT = 8
};

static const uint32_t kUnitStartUpFlashOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                 1206U, 4U, 2U, 1U, 3U, 1U };
static const uint32_t kUnitAutoPedestrianClearOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 2U
};
static const uint32_t kUnitBackupTimeOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                               4U, 2U, 1U, 3U, 3U };
static const uint32_t kUnitRedRevertOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                              4U, 2U, 1U, 3U, 4U };
static const uint32_t kUnitControlStatusOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                  1206U, 4U, 2U, 1U, 3U, 5U };
static const uint32_t kUnitFlashStatusOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                4U, 2U, 1U, 3U, 6U };
static const uint32_t kUnitControlOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                            4U, 2U, 1U, 3U, 10U };
static const uint32_t kMaxAlarmGroupsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                               4U, 2U, 1U, 3U, 11U };
static const uint32_t kAlarmGroupNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                 4U, 2U, 1U, 3U, 12U, 1U, 1U };
static const uint32_t kAlarmGroupStateOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                4U, 2U, 1U, 3U, 12U, 1U, 2U };
static const uint32_t kUnitAlarmStatus2Oid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                 4U, 2U, 1U, 3U, 7U };
static const uint32_t kUnitAlarmStatus1Oid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                 4U, 2U, 1U, 3U, 8U };
static const uint32_t kShortAlarmStatusOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                 4U, 2U, 1U, 3U, 9U };
static const uint32_t kUnitStartUpFlashModeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 18U
};
static const uint32_t kUnitMceTimeoutOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                               4U, 2U, 1U, 3U, 15U };
static const uint32_t kUnitMceIntAdvOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                              4U, 2U, 1U, 3U, 16U };
static const uint32_t kAscElevationOffsetOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                   1206U, 4U, 2U, 1U, 3U, 17U };
static const uint32_t kUnitUserDefinedBackupTimeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 19U
};
static const uint32_t kMaxUserDefinedBackupTimeContentOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 20U
};
static const uint32_t kUnitUserDefinedBackupTimeContentNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 21U, 1U, 1U
};
static const uint32_t kUnitUserDefinedBackupTimeContentOidOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 21U, 1U, 2U
};
static const uint32_t kUnitUserDefinedBackupTimeContentDescriptionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 21U, 1U, 3U
};
static const uint32_t kMaxGlobalSetIdsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                4U, 2U, 1U, 3U, 24U };
static const uint32_t kGlobalSetIdNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                  1206U, 4U, 2U, 1U, 3U, 25U,
                                                  1U, 1U };
static const uint32_t kGlobalSetIdOidOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                               4U, 2U, 1U, 3U, 25U, 1U, 2U };
static const uint32_t kUnitAlarmStatus3Oid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                 4U, 2U, 1U, 3U, 26U };
static const uint32_t kUnitAlarmStatus4Oid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                 4U, 2U, 1U, 3U, 27U };

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

static const IntersectionRuntime_t *GetRuntime(const NtcipContext_t *context)
{
  if ((context == NULL) || (context->intersectionEngine == NULL))
  {
    return NULL;
  }

  return IntersectionEngineGetRuntime(context->intersectionEngine);
}

static uint8_t AnyAlarmGroupStateActive(const NtcipContext_t *context);
static uint32_t GetUnitAlarmStatus2(const NtcipContext_t *context);

static NtcipError_t ValidateDatabaseWrite(const NtcipContext_t *context,
                                          const NtcipRequestContext_t *request)
{
  if ((context == NULL) || (context->dbTransactionService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return NtcipDbTransactionServiceValidateDatabaseWrite(
    context->dbTransactionService,
    request);
}

static uint8_t GetUnitConfig(const NtcipContext_t *context,
                             IntersectionUnitConfig_t *unitConfig)
{
  if ((context == NULL) || (unitConfig == NULL)
      || (context->configurationService == NULL))
  {
    return 0U;
  }

  return ConfigurationServiceGetActiveUnitConfig(context->configurationService,
                                                 unitConfig);
}

static NtcipError_t GetUserDefinedBackupContent(
  const NtcipContext_t *context,
  const uint32_t *indexes,
  uint8_t indexCount,
  uint8_t *contentIndex,
  IntersectionUserDefinedBackupContentConfig_t *content)
{
  uint8_t contentCount;

  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (contentIndex == NULL) || (content == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  contentCount = ConfigurationServiceGetUserDefinedBackupContentCount(
    context->configurationService);
  if ((contentCount == 0U) || (indexes[0] > contentCount))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *contentIndex = (uint8_t) (indexes[0] - 1U);

  if (ConfigurationServiceGetActiveUserDefinedBackupContentConfig(
        context->configurationService,
        *contentIndex,
        content) == 0U)
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static uint8_t RuntimeHasPreemptDemand(const IntersectionRuntime_t *runtime)
{
  uint8_t preemptIndex;

  if (runtime == NULL)
  {
    return 0U;
  }

  if (runtime->preemptStatus != 0U)
  {
    return 1U;
  }

  for (preemptIndex = 0U; preemptIndex < INTERSECTION_PREEMPT_COUNT_MAX;
       ++preemptIndex)
  {
    if ((runtime->preemptInputStatus[preemptIndex] != 0U)
        || (runtime->preemptControlState[preemptIndex] != 0U))
    {
      return 1U;
    }
  }

  return 0U;
}

static uint8_t RuntimeHasFlashingOutputs(const IntersectionRuntime_t *runtime)
{
  uint8_t channelIndex;

  if (runtime == NULL)
  {
    return 0U;
  }

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       ++channelIndex)
  {
    if ((runtime->outputIntentImage.channels[channelIndex]
         == INTERSECTION_OUTPUT_ASPECT_FLASH_RED)
        || (runtime->outputIntentImage.channels[channelIndex]
            == INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW)
        || (runtime->outputIntentImage.channels[channelIndex]
            == INTERSECTION_OUTPUT_ASPECT_FLASH_GREEN))
    {
      return 1U;
    }
  }

  return 0U;
}

static uint8_t AnyDetectorFaultActive(const NtcipContext_t *context)
{
  ModuleBusSnapshot_t snapshot;
  uint8_t detectorIndex;
  uint8_t detectorCount;
  uint8_t pedDetectorIndex;
  uint8_t pedDetectorCount;
  uint8_t phaseNumber;

  if ((context == NULL) || (context->configurationService == NULL))
  {
    return 0U;
  }

  if (GetControllerSnapshot(context, &snapshot) == 0U)
  {
    return 0U;
  }

  detectorCount =
    ConfigurationServiceGetVehicleDetectorCount(context->configurationService);

  for (detectorIndex = 0U; detectorIndex < detectorCount; ++detectorIndex)
  {
    if ((ConfigurationServiceGetVehicleDetectorCallPhase(
           context->configurationService,
           detectorIndex,
           &phaseNumber) != 0U)
        && (phaseNumber != 0U)
        && (ModuleBusSnapshotSourceReady(&snapshot,
                                         MODULE_BUS_SNAPSHOT_VALID_DETECTORS)
            == 0U))
    {
      return 1U;
    }
  }

  pedDetectorCount = ConfigurationServiceGetPedestrianDetectorCount(
    context->configurationService);

  for (pedDetectorIndex = 0U;
       pedDetectorIndex < pedDetectorCount;
       ++pedDetectorIndex)
  {
    if ((ConfigurationServiceGetPedestrianDetectorCallPhase(
           context->configurationService,
           pedDetectorIndex,
           &phaseNumber) != 0U)
        && (phaseNumber != 0U)
        && (ModuleBusSnapshotSourceReady(&snapshot,
                                         MODULE_BUS_SNAPSHOT_VALID_PEDS) == 0U))
    {
      return 1U;
    }
  }

  return 0U;
} /* AnyDetectorFaultActive */

static uint32_t GetUnitControlStatus(const NtcipContext_t *context)
{
  const IntersectionRuntime_t *runtime = GetRuntime(context);

  if (runtime == NULL)
  {
    return UNIT_CONTROL_STATUS_OTHER;
  }

  if ((runtime->unitControlStatus >= UNIT_CONTROL_STATUS_OTHER)
      && (runtime->unitControlStatus <= UNIT_CONTROL_STATUS_LOCAL_MANUAL_CONTROL))
  {
    return runtime->unitControlStatus;
  }

  return UNIT_CONTROL_STATUS_OTHER;
}

static uint32_t GetUnitFlashStatus(const NtcipContext_t *context)
{
  const IntersectionRuntime_t *runtime = GetRuntime(context);

  if (runtime == NULL)
  {
    return UNIT_FLASH_STATUS_OTHER;
  }

  if (runtime->startUpFlashActive != 0U)
  {
    return UNIT_FLASH_STATUS_STARTUP;
  }

  if (runtime->mmuFlashActive != 0U)
  {
    return UNIT_FLASH_STATUS_MMU;
  }

  if ((runtime->preemptStatus != 0U)
      && (RuntimeHasFlashingOutputs(runtime) != 0U))
  {
    return UNIT_FLASH_STATUS_PREEMPT;
  }

  if (runtime->mode == INTERSECTION_CONTROL_MODE_FLASH)
  {
    return UNIT_FLASH_STATUS_AUTOMATIC;
  }

  return UNIT_FLASH_STATUS_NOT_FLASH;
}

static uint32_t GetUnitControl(const NtcipContext_t *context)
{
  uint8_t unitControl = 0U;

  if ((context == NULL) || (context->intersectionEngine == NULL))
  {
    return 0U;
  }

  if (IntersectionEngineGetUnitControl(context->intersectionEngine,
                                       &unitControl) == 0U)
  {
    return 0U;
  }

  return unitControl;
}

static uint32_t GetRemoteManualControlTimeout(const NtcipContext_t *context)
{
  uint8_t timeoutSeconds = 0U;

  if ((context == NULL) || (context->intersectionEngine == NULL))
  {
    return 0U;
  }

  if (IntersectionEngineGetRemoteManualControlTimeout(context->intersectionEngine,
                                                      &timeoutSeconds) == 0U)
  {
    return 0U;
  }

  return timeoutSeconds;
}

static uint32_t GetRemoteManualIntervalAdvance(const NtcipContext_t *context)
{
  uint8_t active = 0U;

  if ((context == NULL) || (context->intersectionEngine == NULL))
  {
    return 0U;
  }

  if (IntersectionEngineGetRemoteManualIntervalAdvance(context->intersectionEngine,
                                                       &active) == 0U)
  {
    return 0U;
  }

  return active;
}

static uint32_t GetUnitAlarmStatus1(const NtcipContext_t *context)
{
  const IntersectionRuntime_t *runtime = GetRuntime(context);
  uint32_t status = 0U;
  uint8_t externalStatus = 0U;

  if (runtime == NULL)
  {
    status = 0U;
  }
  else
  {
    if ((runtime->mode == INTERSECTION_CONTROL_MODE_COORDINATED)
        && (runtime->preemptStatus == 0U) && (runtime->mmuFlashActive == 0U))
    {
      status |= 0x80U;
    }

    if (runtime->localFreeStatus
        != (uint8_t) INTERSECTION_LOCAL_FREE_STATUS_NOT_FREE)
    {
      status |= 0x40U;
    }

    if ((runtime->mmuFlashActive != 0U)
        && (runtime->startUpFlashActive == 0U))
    {
      status |= 0x10U;
    }

    if (runtime->cycleFailActive != 0U)
    {
      status |= 0x08U;
    }

    if (runtime->coordFailActive != 0U)
    {
      status |= 0x04U;
    }

    if (runtime->coordFaultActive != 0U)
    {
      status |= 0x02U;
    }

    if (runtime->coordCycleFaultActive != 0U)
    {
      status |= 0x01U;
    }
  }

  if ((context != NULL) && (context->unitAlarmPort != NULL)
      && (UnitAlarmPortGetUnitAlarmStatus1(context->unitAlarmPort,
                                           &externalStatus) != 0U))
  {
    status |= externalStatus;
  }

  return status;
}

static uint32_t GetShortAlarmStatus(const NtcipContext_t *context)
{
  const IntersectionRuntime_t *runtime = GetRuntime(context);
  uint32_t status = 0U;
  uint32_t unitAlarmStatus1;
  uint32_t unitAlarmStatus2;
  uint8_t localCycleZero = 0U;

  unitAlarmStatus1 = GetUnitAlarmStatus1(context);
  unitAlarmStatus2 = GetUnitAlarmStatus2(context);

  if ((unitAlarmStatus2 & 0x10U) != 0U)
  {
    status |= 0x80U;
  }

  if (AnyAlarmGroupStateActive(context) != 0U)
  {
    status |= 0x40U;
  }

  if (AnyDetectorFaultActive(context) != 0U)
  {
    status |= 0x20U;
  }

  if ((runtime != NULL) && (runtime->coordinationAlarmActive != 0U))
  {
    status |= 0x10U;
  }

  if ((unitAlarmStatus1 & 0x40U) != 0U)
  {
    status |= 0x08U;
  }

  if ((context != NULL) && (context->intersectionEngine != NULL)
      && (IntersectionEngineGetShortAlarmCycleZeroLatched(
            context->intersectionEngine,
            &localCycleZero) != 0U)
      && (localCycleZero != 0U))
  {
    status |= 0x04U;
  }

  if ((unitAlarmStatus1 & 0x30U) != 0U)
  {
    status |= 0x02U;
  }

  if ((runtime != NULL) && (RuntimeHasPreemptDemand(runtime) != 0U))
  {
    status |= 0x01U;
  }

  return status;
}

static uint32_t GetUnitAlarmStatus3(const NtcipContext_t *context)
{
  uint8_t externalStatus = 0U;

  if ((context != NULL) && (context->unitAlarmPort != NULL)
      && (UnitAlarmPortGetUnitAlarmStatus3(context->unitAlarmPort,
                                           &externalStatus) != 0U))
  {
    return externalStatus;
  }

  return 0U;
}

static uint8_t GetMaxAlarmGroups(const NtcipContext_t *context)
{
  uint8_t maxAlarmGroups = 1U;

  if ((context == NULL) || (context->unitAlarmPort == NULL))
  {
    return 1U;
  }

  if ((UnitAlarmPortGetMaxAlarmGroups(context->unitAlarmPort,
                                      &maxAlarmGroups) == 0U)
      || (maxAlarmGroups == 0U))
  {
    return 1U;
  }

  return maxAlarmGroups;
}

static NtcipError_t GetAlarmGroupNumber(const NtcipContext_t *context,
                                        const uint32_t *indexes,
                                        uint8_t indexCount,
                                        uint8_t *groupNumber)
{
  uint8_t maxAlarmGroups;

  if ((indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (indexes[0] > 255U) || (groupNumber == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  maxAlarmGroups = GetMaxAlarmGroups(context);

  if (indexes[0] > maxAlarmGroups)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *groupNumber = (uint8_t) indexes[0];

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetGlobalSetIdRow(const uint32_t *indexes,
                                      uint8_t indexCount,
                                      uint16_t *rowNumber)
{
  if ((indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (rowNumber == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return NTCIP_ERROR_RANGE_ERROR;
}

static uint32_t GetAlarmGroupStateValue(const NtcipContext_t *context,
                                        uint8_t groupNumber)
{
  uint8_t alarmGroupState = 0U;

  if ((context == NULL) || (context->unitAlarmPort == NULL)
      || (groupNumber == 0U))
  {
    return 0U;
  }

  if (UnitAlarmPortGetAlarmGroupState(context->unitAlarmPort,
                                      (uint8_t) (groupNumber - 1U),
                                      &alarmGroupState) == 0U)
  {
    return 0U;
  }

  return alarmGroupState;
}

static uint8_t AnyAlarmGroupStateActive(const NtcipContext_t *context)
{
  uint8_t groupNumber;
  uint8_t maxAlarmGroups;

  maxAlarmGroups = GetMaxAlarmGroups(context);

  for (groupNumber = 1U; groupNumber <= maxAlarmGroups; ++groupNumber)
  {
    if (GetAlarmGroupStateValue(context, groupNumber) != 0U)
    {
      return 1U;
    }
  }

  return 0U;
}

static uint32_t GetUnitAlarmStatus2(const NtcipContext_t *context)
{
  const IntersectionRuntime_t *runtime = GetRuntime(context);
  uint8_t unitAlarmStatus2 = 0U;
  uint32_t status = 0U;

  if ((runtime != NULL) && (runtime->coordSyncStatusSeconds != 0U))
  {
    status |= 0x20U;
  }

  if ((context == NULL) || (context->unitAlarmPort == NULL))
  {
    return status;
  }

  if (UnitAlarmPortGetUnitAlarmStatus2(context->unitAlarmPort,
                                       &unitAlarmStatus2) == 0U)
  {
    return status;
  }

  status |= unitAlarmStatus2;

  return status;
}

static uint32_t GetUnitAlarmStatus4(const NtcipContext_t *context)
{
  uint8_t unitAlarmStatus4 = 0U;

  if ((context == NULL) || (context->unitAlarmPort == NULL))
  {
    return 0U;
  }

  if (UnitAlarmPortGetUnitAlarmStatus4(context->unitAlarmPort,
                                       &unitAlarmStatus4) == 0U)
  {
    return 0U;
  }

  return unitAlarmStatus4;
}

static NtcipError_t SetTestUnitObject(void *groupContext,
                                      const NtcipObjectDescriptor_t *descriptor,
                                      const uint32_t *indexes,
                                      uint8_t indexCount,
                                      const NtcipRequestContext_t *requestContext,
                                      const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  NtcipError_t error;

  (void) indexes;
  (void) indexCount;

  if ((descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if ((descriptor->tag == UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_OID)
      || (descriptor->tag
          == UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_DESCRIPTION))
  {
    IntersectionUserDefinedBackupContentConfig_t content;
    uint8_t contentIndex = 0U;

    if (GetUserDefinedBackupContent(context,
                                    indexes,
                                    indexCount,
                                    &contentIndex,
                                    &content) != NTCIP_ERROR_OK)
    {
      return NTCIP_ERROR_RANGE_ERROR;
    }

    error = ValidateDatabaseWrite(context, requestContext);

    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }

    if (descriptor->tag == UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_OID)
    {
      if (value->type != NTCIP_VALUE_TYPE_OBJECT_ID)
      {
        return NTCIP_ERROR_BAD_VALUE;
      }

      if (value->data.objectId.length
          > INTERSECTION_USER_DEFINED_BACKUP_OID_COMPONENT_COUNT_MAX)
      {
        return NTCIP_ERROR_RANGE_ERROR;
      }
    }
    else
    {
      if (value->type != NTCIP_VALUE_TYPE_OCTET_STRING)
      {
        return NTCIP_ERROR_BAD_VALUE;
      }

      if (value->data.octetString.length
          > INTERSECTION_USER_DEFINED_BACKUP_DESCRIPTION_MAX)
      {
        return NTCIP_ERROR_RANGE_ERROR;
      }
    }

    return NTCIP_ERROR_OK;
  }

  if (value->type != NTCIP_VALUE_TYPE_UNSIGNED32)
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if ((descriptor->tag != UNIT_OBJECT_TAG_BACKUP_TIME)
      && (descriptor->tag != UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_TIME)
      && (value->data.unsigned32 > 255U))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  switch (descriptor->tag)
  {
      case UNIT_OBJECT_TAG_STARTUP_FLASH:
      case UNIT_OBJECT_TAG_BACKUP_TIME:
      case UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_TIME:
      case UNIT_OBJECT_TAG_RED_REVERT:
      case UNIT_OBJECT_TAG_ASC_ELEVATION_OFFSET:
      {
        error = ValidateDatabaseWrite(context, requestContext);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        if ((descriptor->tag == UNIT_OBJECT_TAG_BACKUP_TIME)
            && (value->data.unsigned32 > 65535U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        if ((descriptor->tag == UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_TIME)
            && (value->data.unsigned32 > 16777216UL))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        if ((descriptor->tag == UNIT_OBJECT_TAG_ASC_ELEVATION_OFFSET)
            && (value->data.unsigned32
                > INTERSECTION_UNIT_ELEVATION_OFFSET_UNKNOWN))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_MAX_USER_DEFINED_BACKUP_CONTENT:
      case UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_NUMBER:
      {
        return NTCIP_ERROR_READ_ONLY;
      }

      case UNIT_OBJECT_TAG_AUTO_PEDESTRIAN_CLEAR:
      case UNIT_OBJECT_TAG_STARTUP_FLASH_MODE:
      {
        error = ValidateDatabaseWrite(context, requestContext);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        if ((descriptor->tag == UNIT_OBJECT_TAG_AUTO_PEDESTRIAN_CLEAR)
            && (value->data.unsigned32
                != (uint32_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_DISABLE)
            && (value->data.unsigned32
                != (uint32_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_ENABLE))
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        if ((descriptor->tag == UNIT_OBJECT_TAG_STARTUP_FLASH_MODE)
            && ((value->data.unsigned32
                 < (uint32_t) INTERSECTION_UNIT_STARTUP_FLASH_MODE_AUTO_FLASH)
                || (value->data.unsigned32
                    > (uint32_t)
                      INTERSECTION_UNIT_STARTUP_FLASH_MODE_ALL_RED_CONTROLLER_FLASH)))
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_CONTROL:
      {
        if ((value->data.unsigned32 & 0x01U) != 0U)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_MCE_TIMEOUT:
      {
        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_MCE_INTERVAL_ADVANCE:
      {
        if (value->data.unsigned32 > 1U)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        if ((value->data.unsigned32 == 1U)
            && (GetRemoteManualControlTimeout(context) == 0U))
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }
}

static NtcipError_t SetValueUnitObject(void *groupContext,
                                       const NtcipObjectDescriptor_t *descriptor,
                                       const uint32_t *indexes,
                                       uint8_t indexCount,
                                       const NtcipRequestContext_t *requestContext,
                                       const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  NtcipError_t error;

  error = SetTestUnitObject(groupContext,
                            descriptor,
                            indexes,
                            indexCount,
                            requestContext,
                            value);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if (context == NULL)
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case UNIT_OBJECT_TAG_STARTUP_FLASH:
      {
        if (context->configurationService == NULL)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (ConfigurationServiceSetUnitStartUpFlashSeconds(
                  context->configurationService,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case UNIT_OBJECT_TAG_AUTO_PEDESTRIAN_CLEAR:
      {
        if (context->configurationService == NULL)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (ConfigurationServiceSetUnitAutoPedestrianClear(
                  context->configurationService,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case UNIT_OBJECT_TAG_BACKUP_TIME:
      {
        if (context->configurationService == NULL)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (ConfigurationServiceSetUnitBackupTimeSeconds(
                  context->configurationService,
                  (uint16_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_TIME:
      {
        if (context->configurationService == NULL)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (ConfigurationServiceSetUnitUserDefinedBackupTimeSeconds(
                  context->configurationService,
                  value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case UNIT_OBJECT_TAG_RED_REVERT:
      {
        if (context->configurationService == NULL)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (ConfigurationServiceSetUnitRedRevertDs(
                  context->configurationService,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case UNIT_OBJECT_TAG_ASC_ELEVATION_OFFSET:
      {
        if (context->configurationService == NULL)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (ConfigurationServiceSetUnitElevationOffsetMeters(
                  context->configurationService,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case UNIT_OBJECT_TAG_CONTROL:
      {
        if (context->intersectionEngine == NULL)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (IntersectionEngineSetUnitControl(
                  context->intersectionEngine,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case UNIT_OBJECT_TAG_STARTUP_FLASH_MODE:
      {
        if (context->configurationService == NULL)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (ConfigurationServiceSetUnitStartUpFlashMode(
                  context->configurationService,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case UNIT_OBJECT_TAG_MCE_TIMEOUT:
      {
        if (context->intersectionEngine == NULL)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (IntersectionEngineSetRemoteManualControlTimeout(
                  context->intersectionEngine,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case UNIT_OBJECT_TAG_MCE_INTERVAL_ADVANCE:
      {
        if (context->intersectionEngine == NULL)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (IntersectionEngineSetRemoteManualIntervalAdvance(
                  context->intersectionEngine,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_OID:
      {
        if ((context->configurationService == NULL) || (indexes == NULL)
            || (indexCount != 1U))
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (ConfigurationServiceSetUserDefinedBackupContentOid(
                  context->configurationService,
                  (uint8_t) (indexes[0] - 1U),
                  value->data.objectId.elements,
                  value->data.objectId.length) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_DESCRIPTION:
      {
        if ((context->configurationService == NULL) || (indexes == NULL)
            || (indexCount != 1U))
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return (ConfigurationServiceSetUserDefinedBackupContentDescription(
                  context->configurationService,
                  (uint8_t) (indexes[0] - 1U),
                  value->data.octetString.bytes,
                  (uint8_t) value->data.octetString.length) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }
}

static NtcipError_t GetUnitObject(void *groupContext,
                                  const NtcipObjectDescriptor_t *descriptor,
                                  const uint32_t *indexes,
                                  uint8_t indexCount,
                                  const NtcipRequestContext_t *requestContext,
                                  NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionUnitConfig_t unitConfig;
  IntersectionUserDefinedBackupContentConfig_t content;
  uint8_t contentIndex = 0U;
  uint8_t alarmGroupNumber = 0U;
  uint16_t globalSetIdRow = 0U;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case UNIT_OBJECT_TAG_STARTUP_FLASH:
      case UNIT_OBJECT_TAG_AUTO_PEDESTRIAN_CLEAR:
      case UNIT_OBJECT_TAG_BACKUP_TIME:
      case UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_TIME:
      case UNIT_OBJECT_TAG_RED_REVERT:
      case UNIT_OBJECT_TAG_ASC_ELEVATION_OFFSET:
      case UNIT_OBJECT_TAG_STARTUP_FLASH_MODE:
      {
        if (GetUnitConfig(context, &unitConfig) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        switch (descriptor->tag)
        {
            case UNIT_OBJECT_TAG_STARTUP_FLASH:
            {
              NtcipValueSetUnsigned32(value, unitConfig.startUpFlashSeconds);

              return NTCIP_ERROR_OK;
            }

            case UNIT_OBJECT_TAG_AUTO_PEDESTRIAN_CLEAR:
            {
              NtcipValueSetUnsigned32(value, unitConfig.autoPedestrianClear);

              return NTCIP_ERROR_OK;
            }

            case UNIT_OBJECT_TAG_BACKUP_TIME:
            {
              NtcipValueSetUnsigned32(value, unitConfig.backupTimeSeconds);

              return NTCIP_ERROR_OK;
            }

            case UNIT_OBJECT_TAG_RED_REVERT:
            {
              NtcipValueSetUnsigned32(value, unitConfig.redRevertDs);

              return NTCIP_ERROR_OK;
            }

            case UNIT_OBJECT_TAG_ASC_ELEVATION_OFFSET:
            {
              NtcipValueSetUnsigned32(value,
                                      unitConfig.elevationOffsetMeters);

              return NTCIP_ERROR_OK;
            }

            case UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_TIME:
            {
              NtcipValueSetUnsigned32(value,
                                      unitConfig.userDefinedBackupTimeSeconds);

              return NTCIP_ERROR_OK;
            }

            case UNIT_OBJECT_TAG_STARTUP_FLASH_MODE:
            {
              NtcipValueSetUnsigned32(value, unitConfig.startUpFlashMode);

              return NTCIP_ERROR_OK;
            }

            default:
            {
              return NTCIP_ERROR_NOT_FOUND;
            }
        }
      }

      case UNIT_OBJECT_TAG_CONTROL_STATUS:
      {
        NtcipValueSetUnsigned32(value, GetUnitControlStatus(context));

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_FLASH_STATUS:
      {
        NtcipValueSetUnsigned32(value, GetUnitFlashStatus(context));

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_CONTROL:
      {
        NtcipValueSetUnsigned32(value, GetUnitControl(context));

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_MAX_ALARM_GROUPS:
      {
        NtcipValueSetUnsigned32(value, GetMaxAlarmGroups(context));

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_MAX_GLOBAL_SET_IDS:
      {
        NtcipValueSetUnsigned32(value, 0U);

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_ALARM_GROUP_NUMBER:
      case UNIT_OBJECT_TAG_ALARM_GROUP_STATE:
      {
        error = GetAlarmGroupNumber(context,
                                    indexes,
                                    indexCount,
                                    &alarmGroupNumber);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        if (descriptor->tag == UNIT_OBJECT_TAG_ALARM_GROUP_NUMBER)
        {
          NtcipValueSetUnsigned32(value, alarmGroupNumber);
        }
        else
        {
          NtcipValueSetUnsigned32(value,
                                  GetAlarmGroupStateValue(context,
                                                         alarmGroupNumber));
        }

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_GLOBAL_SET_ID_NUMBER:
      case UNIT_OBJECT_TAG_GLOBAL_SET_ID_OID:
      {
        error = GetGlobalSetIdRow(indexes, indexCount, &globalSetIdRow);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        (void) globalSetIdRow;

        return NTCIP_ERROR_NOT_FOUND;
      }

      case UNIT_OBJECT_TAG_MCE_TIMEOUT:
      {
        NtcipValueSetUnsigned32(value, GetRemoteManualControlTimeout(context));

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_MCE_INTERVAL_ADVANCE:
      {
        NtcipValueSetUnsigned32(value, GetRemoteManualIntervalAdvance(context));

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_MAX_USER_DEFINED_BACKUP_CONTENT:
      {
        NtcipValueSetUnsigned32(
          value,
          ConfigurationServiceGetUserDefinedBackupContentCount(
            context->configurationService));

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_NUMBER:
      case UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_OID:
      case UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_DESCRIPTION:
      {
        error = GetUserDefinedBackupContent(context,
                                            indexes,
                                            indexCount,
                                            &contentIndex,
                                            &content);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        if (descriptor->tag == UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_NUMBER)
        {
          NtcipValueSetUnsigned32(value, (uint32_t) (contentIndex + 1U));

          return NTCIP_ERROR_OK;
        }

        if (descriptor->tag == UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_OID)
        {
          return NtcipValueSetObjectId(value, content.oid, content.oidLength);
        }

        return NtcipValueSetOctetString(value,
                                        content.description,
                                        content.descriptionLength);
      }

      case UNIT_OBJECT_TAG_ALARM_STATUS2:
      {
        NtcipValueSetUnsigned32(value, GetUnitAlarmStatus2(context));
        UnitAlarmPortAcknowledgeUnitAlarmStatus2Read(context->unitAlarmPort);

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_ALARM_STATUS1:
      {
        NtcipValueSetUnsigned32(value, GetUnitAlarmStatus1(context));

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_SHORT_ALARM_STATUS:
      {
        NtcipValueSetUnsigned32(value, GetShortAlarmStatus(context));
        IntersectionEngineAcknowledgeShortAlarmStatusRead(
          context->intersectionEngine);

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_ALARM_STATUS3:
      {
        NtcipValueSetUnsigned32(value, GetUnitAlarmStatus3(context));

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_ALARM_STATUS4:
      {
        NtcipValueSetUnsigned32(value, GetUnitAlarmStatus4(context));

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  } /* switch */
} /* GetUnitObject */

static const NtcipObjectDescriptor_t kUnitObjects[] =
{
  { kUnitStartUpFlashOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_STARTUP_FLASH, GetUnitObject, SetTestUnitObject,
    SetValueUnitObject },
  { kUnitAutoPedestrianClearOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_AUTO_PEDESTRIAN_CLEAR, GetUnitObject, SetTestUnitObject,
    SetValueUnitObject },
  { kUnitBackupTimeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_BACKUP_TIME, GetUnitObject, SetTestUnitObject,
    SetValueUnitObject },
  { kUnitRedRevertOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_RED_REVERT, GetUnitObject, SetTestUnitObject,
    SetValueUnitObject },
  { kUnitControlStatusOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_CONTROL_STATUS, GetUnitObject, NULL, NULL },
  { kUnitFlashStatusOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_FLASH_STATUS, GetUnitObject, NULL, NULL },
  { kUnitControlOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_CONTROL, GetUnitObject, SetTestUnitObject,
    SetValueUnitObject },
  { kMaxAlarmGroupsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_MAX_ALARM_GROUPS, GetUnitObject, NULL, NULL },
  { kAlarmGroupNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_ALARM_GROUP_NUMBER, GetUnitObject, NULL, NULL },
  { kAlarmGroupStateOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_ALARM_GROUP_STATE, GetUnitObject, NULL, NULL },
  { kMaxGlobalSetIdsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_MAX_GLOBAL_SET_IDS, GetUnitObject, NULL, NULL },
  { kGlobalSetIdNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_GLOBAL_SET_ID_NUMBER, GetUnitObject, NULL, NULL },
  { kGlobalSetIdOidOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OBJECT_ID,
    UNIT_OBJECT_TAG_GLOBAL_SET_ID_OID, GetUnitObject, NULL, NULL },
  { kUnitAlarmStatus2Oid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_ALARM_STATUS2, GetUnitObject, NULL, NULL },
  { kUnitAlarmStatus1Oid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_ALARM_STATUS1, GetUnitObject, NULL, NULL },
  { kShortAlarmStatusOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_SHORT_ALARM_STATUS, GetUnitObject, NULL, NULL },
  { kUnitStartUpFlashModeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_STARTUP_FLASH_MODE, GetUnitObject, SetTestUnitObject,
    SetValueUnitObject },
  { kUnitMceTimeoutOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_MCE_TIMEOUT, GetUnitObject, SetTestUnitObject,
    SetValueUnitObject },
  { kUnitMceIntAdvOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_MCE_INTERVAL_ADVANCE, GetUnitObject, SetTestUnitObject,
    SetValueUnitObject },
  { kAscElevationOffsetOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_ASC_ELEVATION_OFFSET, GetUnitObject, SetTestUnitObject,
    SetValueUnitObject },
  { kUnitUserDefinedBackupTimeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_TIME, GetUnitObject,
    SetTestUnitObject, SetValueUnitObject },
  { kMaxUserDefinedBackupTimeContentOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_MAX_USER_DEFINED_BACKUP_CONTENT, GetUnitObject, NULL,
    NULL },
  { kUnitUserDefinedBackupTimeContentNumberOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_NUMBER,
    GetUnitObject, NULL, NULL },
  { kUnitUserDefinedBackupTimeContentOidOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OBJECT_ID, UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_OID,
    GetUnitObject, SetTestUnitObject, SetValueUnitObject },
  { kUnitUserDefinedBackupTimeContentDescriptionOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING,
    UNIT_OBJECT_TAG_USER_DEFINED_BACKUP_CONTENT_DESCRIPTION, GetUnitObject,
    SetTestUnitObject, SetValueUnitObject },
  { kUnitAlarmStatus3Oid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_ALARM_STATUS3, GetUnitObject, NULL, NULL }
  ,
  { kUnitAlarmStatus4Oid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_ALARM_STATUS4, GetUnitObject, NULL, NULL }
};

void UnitObjectsRegister(NtcipObjectDirectory_t *directory,
                         NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.unit",
    kUnitObjects,
    (uint16_t) (sizeof(kUnitObjects) / sizeof(kUnitObjects[0])),
    context);
}
