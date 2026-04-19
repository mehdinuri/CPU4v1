/* App/Domain/NTCIP/Mib1202v0335/AscIoInputMappingObjects.c
 *
 * Canonical 1202 ascIOmapping input-side subtree. This pass implements the
 * controller-local active input map, live custom FEIG/ped pin status, and the
 * standard input function reference table.
 */
#include "AscIoInputMappingObjects.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

enum
{
  ASC_IO_INPUT_TAG_MAX_INPUTS = 1,
  ASC_IO_INPUT_TAG_MAP_IO_INDEX,
  ASC_IO_INPUT_TAG_MAP_DEVICE_TYPE,
  ASC_IO_INPUT_TAG_MAP_DEVICE_PNN,
  ASC_IO_INPUT_TAG_MAP_DEVICE_PTYPE,
  ASC_IO_INPUT_TAG_MAP_DEVICE_ADDR,
  ASC_IO_INPUT_TAG_MAP_DEVICE_PIN,
  ASC_IO_INPUT_TAG_MAP_FUNC_TYPE,
  ASC_IO_INPUT_TAG_MAP_FUNC_PTYPE,
  ASC_IO_INPUT_TAG_MAP_FUNCTION,
  ASC_IO_INPUT_TAG_MAP_FUNCTION_INDEX,
  ASC_IO_INPUT_TAG_PIN_DESCRIPTION,
  ASC_IO_INPUT_TAG_PIN_STATUS,
  ASC_IO_INPUT_TAG_MAX_INPUT_FUNCTIONS,
  ASC_IO_INPUT_TAG_FUNCTION_INDEX,
  ASC_IO_INPUT_TAG_FUNCTION_MAX_INDEX,
  ASC_IO_INPUT_TAG_FUNCTION_NAME
};

static const uint32_t kAscIOmapMaxInputsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 2U
};
static const uint32_t kAscIOinputMapIOindexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 4U, 1U, 2U
};
static const uint32_t kAscIOinputMapDeviceTypeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 4U, 1U, 3U
};
static const uint32_t kAscIOinputMapDevicePNNOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 4U, 1U, 4U
};
static const uint32_t kAscIOinputMapDevicePtypeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 4U, 1U, 5U
};
static const uint32_t kAscIOinputMapDeviceAddrOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 4U, 1U, 6U
};
static const uint32_t kAscIOinputMapDevicePinOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 4U, 1U, 7U
};
static const uint32_t kAscIOinputMapFuncTypeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 4U, 1U, 8U
};
static const uint32_t kAscIOinputMapFuncPtypeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 4U, 1U, 9U
};
static const uint32_t kAscIOinputMapFunctionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 4U, 1U, 10U
};
static const uint32_t kAscIOinputMapFuncIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 4U, 1U, 11U
};
static const uint32_t kAscIOinputMapDevPinDescrOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 5U, 1U, 1U
};
static const uint32_t kAscIOinputMapDevPinStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 5U, 1U, 2U
};
static const uint32_t kAscIOmapMaxInputFunctionsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 9U, 1U
};
static const uint32_t kAscIOinputIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 9U, 2U, 1U, 1U
};
static const uint32_t kAscIOinputMaxFuncIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 9U, 2U, 1U, 2U
};
static const uint32_t kAscIOinputFunctionNameOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 9U, 2U, 1U, 3U
};

static const char *const kInputFunctionNames[] = {
  NULL,
  "unusedInput",
  "ioUsedAsOutput",
  "logicInput",
  "addressBit",
  "alarmInput",
  "alternateSequence",
  "autoFlashRequest",
  "cabinetDoorOpen",
  "callToNonActuated",
  "clockUpdate",
  "conflictMonitorStatus",
  "cycleAdvance",
  "dimmingEnable",
  "externalStart",
  "forceOffRing",
  "freeRequest",
  "hardwareControl",
  "indicatorLampControl",
  "inhibitMaxRing",
  "intervalAdvance",
  "localFlashSense",
  "manualControlEnable",
  "max2Ring",
  "max3AllRings",
  "max4AllRings",
  "maxRecall",
  "maxWalk",
  "minRecall",
  "mmuCmuFlashSense",
  "modeSelectBit",
  "offsetInput",
  "omitRedClearRing",
  "patternSelect",
  "pedestrianDetector",
  "pedestrianOmit",
  "pedestrianRecycleRing",
  "phaseHold",
  "phaseOmit",
  "preemptGateDown",
  "preemptGateUp",
  "preemptHealthy",
  "preemptInput",
  "preemptInputAdvanced",
  "priorityCheckout",
  "priorityRequest",
  "redRestRing",
  "specialFunctionInput",
  "stopTimeAllRings",
  "stopTimeRing",
  "tbcOnline",
  "testInput",
  "timingPlanInput",
  "vehicleDetector",
  "vehicleDetectorFault",
  "walkRestModifier"
};

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

static NtcipError_t GetActiveIoMapConfig(const NtcipContext_t *context,
                                         IntersectionIoMapConfig_t *ioMap)
{
  if ((context == NULL) || (ioMap == NULL)
      || (context->configurationService == NULL)
      || (ConfigurationServiceGetActiveIoMapConfig(context->configurationService,
                                                   ioMap) == 0U))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetCandidateIoMapConfig(NtcipContext_t *context,
                                            IntersectionIoMapConfig_t *ioMap)
{
  if ((context == NULL) || (ioMap == NULL)
      || (context->configurationService == NULL)
      || (ConfigurationServiceGetCandidateIoMapConfig(
            context->configurationService,
            ioMap) == 0U))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t SaveCandidateIoMapConfig(
  NtcipContext_t *context,
  const IntersectionIoMapConfig_t *ioMap)
{
  if ((context == NULL) || (ioMap == NULL)
      || (context->configurationService == NULL)
      || (ConfigurationServiceSetIoMapConfig(context->configurationService,
                                             ioMap) == 0U))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static uint8_t IoMapFamilyMatches(const IntersectionIoInputMapRowConfig_t *row,
                                  uint16_t pnn,
                                  uint8_t ptype)
{
  return (uint8_t) ((row != NULL)
                    && (pnn != 0U)
                    && (ptype != 0U)
                    && (row->deviceType
                        == (uint8_t) INTERSECTION_IO_MAP_DEVICE_CUSTOM)
                    && (row->devicePnn == pnn)
                    && (row->devicePtype == ptype));
}

static uint8_t IoMapInputRowIsFeigFamily(
  const IntersectionIoInputMapRowConfig_t *row)
{
  return IoMapFamilyMatches(row,
                            INTERSECTION_IO_MAP_FEIG_DEVICE_PNN,
                            INTERSECTION_IO_MAP_FEIG_DEVICE_PTYPE);
}

static uint8_t IoMapInputRowIsPedFamily(
  const IntersectionIoInputMapRowConfig_t *row)
{
  return IoMapFamilyMatches(row,
                            INTERSECTION_IO_MAP_PED_DEVICE_PNN,
                            INTERSECTION_IO_MAP_PED_DEVICE_PTYPE);
}

static uint8_t GetMapIndex(const uint32_t *indexes,
                           uint8_t indexCount,
                           uint8_t *mapIndex)
{
  if ((indexes == NULL) || (indexCount != 1U) || (mapIndex == NULL)
      || (indexes[0] != 1U))
  {
    return 0U;
  }

  *mapIndex = 0U;

  return 1U;
}

static uint8_t GetInputRowIndex(const uint32_t *indexes,
                                uint8_t indexCount,
                                uint8_t *rowIndex)
{
  if ((indexes == NULL) || (indexCount != 2U) || (rowIndex == NULL)
      || (indexes[0] != 1U) || (indexes[1] == 0U)
      || (indexes[1] > INTERSECTION_IO_MAP_MAX_INPUTS))
  {
    return 0U;
  }

  *rowIndex = (uint8_t) (indexes[1] - 1U);

  return 1U;
}

static uint8_t GetInputFunctionIndex(const uint32_t *indexes,
                                     uint8_t indexCount,
                                     uint8_t *functionIndex)
{
  if ((indexes == NULL) || (indexCount != 1U) || (functionIndex == NULL)
      || (indexes[0] == 0U)
      || (indexes[0]
          >= (sizeof(kInputFunctionNames) / sizeof(kInputFunctionNames[0]))))
  {
    return 0U;
  }

  *functionIndex = (uint8_t) indexes[0];

  return 1U;
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

static uint8_t GetFeigPhysicalInputNumber(
  const IntersectionIoInputMapRowConfig_t *row,
  uint8_t *physicalInputNumber)
{
  uint8_t inputNumber;

  if ((row == NULL) || (physicalInputNumber == NULL)
      || (IoMapInputRowIsFeigFamily(row) == 0U)
      || (row->deviceAddr == 0U) || (row->deviceAddr > 8U)
      || (row->devicePin == 0U) || (row->devicePin > 4U))
  {
    return 0U;
  }

  inputNumber = (uint8_t) (((row->deviceAddr - 1U) * 4U) + row->devicePin);
  if (inputNumber > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
  {
    return 0U;
  }

  *physicalInputNumber = inputNumber;

  return 1U;
}

static uint8_t GetPedPhysicalInputNumber(
  const IntersectionIoInputMapRowConfig_t *row,
  uint8_t *physicalInputNumber)
{
  if ((row == NULL) || (physicalInputNumber == NULL)
      || (IoMapInputRowIsPedFamily(row) == 0U)
      || (row->deviceAddr != 1U)
      || (row->devicePin == 0U)
      || (row->devicePin > INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  *physicalInputNumber = row->devicePin;

  return 1U;
}

static uint8_t GetInputPinStatus(const NtcipContext_t *context,
                                 const IntersectionIoInputMapRowConfig_t *row)
{
  ModuleBusSnapshot_t snapshot;
  uint8_t physicalInputNumber;

  if ((context == NULL) || (row == NULL)
      || (GetControllerSnapshot(context, &snapshot) == 0U))
  {
    return 0U;
  }

  if ((ModuleBusSnapshotSourceReady(&snapshot,
                                    MODULE_BUS_SNAPSHOT_VALID_DETECTORS)
       != 0U)
      && (GetFeigPhysicalInputNumber(row, &physicalInputNumber) != 0U))
  {
    return ModuleBusSnapshotRawVehicleDetectorInputActive(&snapshot,
                                                          physicalInputNumber);
  }

  if ((ModuleBusSnapshotSourceReady(&snapshot,
                                    MODULE_BUS_SNAPSHOT_VALID_PEDS)
       != 0U)
      && (GetPedPhysicalInputNumber(row, &physicalInputNumber) != 0U))
  {
    return ModuleBusSnapshotRawPedestrianInputActive(&snapshot,
                                                     physicalInputNumber);
  }

  return 0U;
}

static NtcipError_t BuildInputPinDescription(
  const IntersectionIoInputMapRowConfig_t *row,
  NtcipValue_t *value)
{
  char buffer[INTERSECTION_IO_MAP_DESCRIPTION_MAX];
  int written;

  if ((row == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (IoMapInputRowIsFeigFamily(row) != 0U)
  {
    written = snprintf(buffer,
                       sizeof(buffer),
                       "FEIG%u-IN%u",
                       row->deviceAddr,
                       row->devicePin);
  }
  else if (IoMapInputRowIsPedFamily(row) != 0U)
  {
    written = snprintf(buffer,
                       sizeof(buffer),
                       "PED%u-IN%u",
                       row->deviceAddr,
                       row->devicePin);
  }
  else
  {
    written = 0;
    buffer[0] = '\0';
  }

  if (written <= 0)
  {
    return NtcipValueSetOctetString(value, NULL, 0U);
  }

  if ((uint32_t) written >= sizeof(buffer))
  {
    written = (int) (sizeof(buffer) - 1U);
  }

  return NtcipValueSetOctetString(value,
                                  (const uint8_t *) buffer,
                                  (uint16_t) written);
}

static uint32_t GetInputFunctionMaxIndex(uint8_t functionIndex)
{
  switch (functionIndex)
  {
      case 6U:
      {
        return INTERSECTION_SEQUENCE_COUNT_MAX;
      }

      case 9U:
      case 15U:
      case 19U:
      case 23U:
      case 26U:
      case 27U:
      case 28U:
      case 32U:
      case 35U:
      case 36U:
      case 37U:
      case 38U:
      case 46U:
      case 49U:
      case 55U:
      {
        return INTERSECTION_PHASE_COUNT_MAX;
      }

      case 33U:
      case 52U:
      {
        return INTERSECTION_PATTERN_COUNT_MAX;
      }

      case 39U:
      case 40U:
      case 41U:
      case 42U:
      case 43U:
      {
        return INTERSECTION_PREEMPT_COUNT_MAX;
      }

      case 34U:
      {
        return INTERSECTION_PED_INPUT_COUNT_MAX;
      }

      case 53U:
      case 54U:
      {
        return INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
      }

      default:
      {
        return 1U;
      }
  }
}

static NtcipError_t GetAscIoInputObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionIoMapConfig_t ioMap;
  uint8_t rowIndex;
  uint8_t functionIndex = 0U;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case ASC_IO_INPUT_TAG_MAX_INPUTS:
      {
        NtcipValueSetUnsigned32(value, INTERSECTION_IO_MAP_MAX_INPUTS);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_MAX_INPUT_FUNCTIONS:
      {
        NtcipValueSetUnsigned32(
          value,
          (uint32_t) ((sizeof(kInputFunctionNames)
                       / sizeof(kInputFunctionNames[0]))
                      - 1U));
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_MAP_IO_INDEX:
      case ASC_IO_INPUT_TAG_MAP_DEVICE_TYPE:
      case ASC_IO_INPUT_TAG_MAP_DEVICE_PNN:
      case ASC_IO_INPUT_TAG_MAP_DEVICE_PTYPE:
      case ASC_IO_INPUT_TAG_MAP_DEVICE_ADDR:
      case ASC_IO_INPUT_TAG_MAP_DEVICE_PIN:
      case ASC_IO_INPUT_TAG_MAP_FUNC_TYPE:
      case ASC_IO_INPUT_TAG_MAP_FUNC_PTYPE:
      case ASC_IO_INPUT_TAG_MAP_FUNCTION:
      case ASC_IO_INPUT_TAG_MAP_FUNCTION_INDEX:
      case ASC_IO_INPUT_TAG_PIN_DESCRIPTION:
      case ASC_IO_INPUT_TAG_PIN_STATUS:
      {
        if (GetInputRowIndex(indexes, indexCount, &rowIndex) == 0U)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        if (GetActiveIoMapConfig(context, &ioMap) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        break;
      }

      case ASC_IO_INPUT_TAG_FUNCTION_INDEX:
      case ASC_IO_INPUT_TAG_FUNCTION_MAX_INDEX:
      case ASC_IO_INPUT_TAG_FUNCTION_NAME:
      {
        if (GetInputFunctionIndex(indexes, indexCount, &functionIndex) == 0U)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        break;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }

  switch (descriptor->tag)
  {
      case ASC_IO_INPUT_TAG_MAP_IO_INDEX:
      {
        NtcipValueSetUnsigned32(value, indexes[1]);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_MAP_DEVICE_TYPE:
      {
        NtcipValueSetUnsigned32(value, ioMap.inputs[rowIndex].deviceType);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_MAP_DEVICE_PNN:
      {
        NtcipValueSetUnsigned32(value, ioMap.inputs[rowIndex].devicePnn);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_MAP_DEVICE_PTYPE:
      {
        NtcipValueSetUnsigned32(value, ioMap.inputs[rowIndex].devicePtype);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_MAP_DEVICE_ADDR:
      {
        NtcipValueSetUnsigned32(value, ioMap.inputs[rowIndex].deviceAddr);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_MAP_DEVICE_PIN:
      {
        NtcipValueSetUnsigned32(value, ioMap.inputs[rowIndex].devicePin);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_MAP_FUNC_TYPE:
      {
        NtcipValueSetUnsigned32(value, ioMap.inputs[rowIndex].functionType);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_MAP_FUNC_PTYPE:
      {
        NtcipValueSetUnsigned32(value, ioMap.inputs[rowIndex].functionPtype);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_MAP_FUNCTION:
      {
        NtcipValueSetUnsigned32(value, ioMap.inputs[rowIndex].function);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_MAP_FUNCTION_INDEX:
      {
        NtcipValueSetUnsigned32(value, ioMap.inputs[rowIndex].functionIndex);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_PIN_DESCRIPTION:
      {
        return BuildInputPinDescription(&ioMap.inputs[rowIndex], value);
      }

      case ASC_IO_INPUT_TAG_PIN_STATUS:
      {
        NtcipValueSetUnsigned32(value,
                                GetInputPinStatus(context,
                                                  &ioMap.inputs[rowIndex])
                                != 0U ? 1U : 0U);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_FUNCTION_INDEX:
      {
        NtcipValueSetUnsigned32(value, functionIndex);
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_FUNCTION_MAX_INDEX:
      {
        NtcipValueSetUnsigned32(value, GetInputFunctionMaxIndex(functionIndex));
        return NTCIP_ERROR_OK;
      }

      case ASC_IO_INPUT_TAG_FUNCTION_NAME:
      {
        return NtcipValueSetOctetString(
          value,
          (const uint8_t *) kInputFunctionNames[functionIndex],
          (uint16_t) strlen(kInputFunctionNames[functionIndex]));
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetTestAscIoInputObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  uint8_t mapIndex;
  uint8_t rowIndex;
  NtcipError_t error;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  error = ValidateDatabaseWrite(context, requestContext);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case ASC_IO_INPUT_TAG_MAP_DEVICE_TYPE:
      {
        if (GetInputRowIndex(indexes, indexCount, &rowIndex) == 0U)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        (void) rowIndex;
        return (value->type == NTCIP_VALUE_TYPE_UNSIGNED32)
               && ((value->data.unsigned32
                    == (uint32_t) INTERSECTION_IO_MAP_DEVICE_UNUSED)
                   || (value->data.unsigned32
                       == (uint32_t) INTERSECTION_IO_MAP_DEVICE_CUSTOM))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_BAD_VALUE;
      }

      case ASC_IO_INPUT_TAG_MAP_DEVICE_PNN:
      case ASC_IO_INPUT_TAG_MAP_FUNC_TYPE:
      {
        if (GetInputRowIndex(indexes, indexCount, &rowIndex) == 0U)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        (void) rowIndex;
        return (value->type == NTCIP_VALUE_TYPE_UNSIGNED32)
               && (value->data.unsigned32 <= 65535U)
               && ((descriptor->tag != ASC_IO_INPUT_TAG_MAP_FUNC_TYPE)
                   || (value->data.unsigned32 == 0U))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_BAD_VALUE;
      }

      case ASC_IO_INPUT_TAG_MAP_DEVICE_PTYPE:
      case ASC_IO_INPUT_TAG_MAP_DEVICE_ADDR:
      case ASC_IO_INPUT_TAG_MAP_DEVICE_PIN:
      case ASC_IO_INPUT_TAG_MAP_FUNC_PTYPE:
      case ASC_IO_INPUT_TAG_MAP_FUNCTION_INDEX:
      {
        if (GetInputRowIndex(indexes, indexCount, &rowIndex) == 0U)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        (void) rowIndex;
        return (value->type == NTCIP_VALUE_TYPE_UNSIGNED32)
               && (value->data.unsigned32 <= 255U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_BAD_VALUE;
      }

      case ASC_IO_INPUT_TAG_MAP_FUNCTION:
      {
        if (GetInputRowIndex(indexes, indexCount, &rowIndex) == 0U)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        (void) rowIndex;

        if (value->type != NTCIP_VALUE_TYPE_UNSIGNED32)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        switch ((uint8_t) value->data.unsigned32)
        {
            case (uint8_t) INTERSECTION_IO_MAP_INPUT_FUNCTION_UNUSED_INPUT:
            case (uint8_t) INTERSECTION_IO_MAP_INPUT_FUNCTION_PEDESTRIAN_DETECTOR:
            case (uint8_t) INTERSECTION_IO_MAP_INPUT_FUNCTION_VEHICLE_DETECTOR:
            {
              return NTCIP_ERROR_OK;
            }

            default:
            {
              return NTCIP_ERROR_BAD_VALUE;
            }
        }
      }

      case ASC_IO_INPUT_TAG_FUNCTION_INDEX:
      case ASC_IO_INPUT_TAG_FUNCTION_MAX_INDEX:
      case ASC_IO_INPUT_TAG_FUNCTION_NAME:
      {
        return NTCIP_ERROR_READ_ONLY;
      }

      case ASC_IO_INPUT_TAG_MAX_INPUTS:
      case ASC_IO_INPUT_TAG_PIN_DESCRIPTION:
      case ASC_IO_INPUT_TAG_PIN_STATUS:
      {
        return NTCIP_ERROR_READ_ONLY;
      }

      case ASC_IO_INPUT_TAG_MAP_IO_INDEX:
      {
        return NTCIP_ERROR_READ_ONLY;
      }

      default:
      {
        if (GetMapIndex(indexes, indexCount, &mapIndex) != 0U)
        {
          (void) mapIndex;
        }

        return NTCIP_ERROR_READ_ONLY;
      }
  }
}

static NtcipError_t SetValueAscIoInputObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionIoMapConfig_t ioMap;
  uint8_t rowIndex;
  NtcipError_t error;

  error = SetTestAscIoInputObject(groupContext,
                                  descriptor,
                                  indexes,
                                  indexCount,
                                  requestContext,
                                  value);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if ((context == NULL) || (GetCandidateIoMapConfig(context, &ioMap)
                            != NTCIP_ERROR_OK))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  if (GetInputRowIndex(indexes, indexCount, &rowIndex) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  switch (descriptor->tag)
  {
      case ASC_IO_INPUT_TAG_MAP_DEVICE_TYPE:
      {
        ioMap.inputs[rowIndex].deviceType = (uint8_t) value->data.unsigned32;
        break;
      }

      case ASC_IO_INPUT_TAG_MAP_DEVICE_PNN:
      {
        ioMap.inputs[rowIndex].devicePnn = (uint16_t) value->data.unsigned32;
        break;
      }

      case ASC_IO_INPUT_TAG_MAP_DEVICE_PTYPE:
      {
        ioMap.inputs[rowIndex].devicePtype = (uint8_t) value->data.unsigned32;
        break;
      }

      case ASC_IO_INPUT_TAG_MAP_DEVICE_ADDR:
      {
        ioMap.inputs[rowIndex].deviceAddr = (uint8_t) value->data.unsigned32;
        break;
      }

      case ASC_IO_INPUT_TAG_MAP_DEVICE_PIN:
      {
        ioMap.inputs[rowIndex].devicePin = (uint8_t) value->data.unsigned32;
        break;
      }

      case ASC_IO_INPUT_TAG_MAP_FUNC_TYPE:
      {
        ioMap.inputs[rowIndex].functionType = (uint16_t) value->data.unsigned32;
        break;
      }

      case ASC_IO_INPUT_TAG_MAP_FUNC_PTYPE:
      {
        ioMap.inputs[rowIndex].functionPtype = (uint8_t) value->data.unsigned32;
        break;
      }

      case ASC_IO_INPUT_TAG_MAP_FUNCTION:
      {
        ioMap.inputs[rowIndex].function = (uint8_t) value->data.unsigned32;
        break;
      }

      case ASC_IO_INPUT_TAG_MAP_FUNCTION_INDEX:
      {
        ioMap.inputs[rowIndex].functionIndex = (uint8_t) value->data.unsigned32;
        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }

  return SaveCandidateIoMapConfig(context, &ioMap);
}

static const NtcipObjectDescriptor_t kAscIoInputMappingObjects[] = {
  { kAscIOmapMaxInputsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_MAX_INPUTS, GetAscIoInputObject, NULL, NULL },
  { kAscIOinputMapIOindexOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_MAP_IO_INDEX, GetAscIoInputObject, NULL, NULL },
  { kAscIOinputMapDeviceTypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_MAP_DEVICE_TYPE, GetAscIoInputObject,
    SetTestAscIoInputObject, SetValueAscIoInputObject },
  { kAscIOinputMapDevicePNNOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_MAP_DEVICE_PNN, GetAscIoInputObject,
    SetTestAscIoInputObject, SetValueAscIoInputObject },
  { kAscIOinputMapDevicePtypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_MAP_DEVICE_PTYPE, GetAscIoInputObject,
    SetTestAscIoInputObject, SetValueAscIoInputObject },
  { kAscIOinputMapDeviceAddrOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_MAP_DEVICE_ADDR, GetAscIoInputObject,
    SetTestAscIoInputObject, SetValueAscIoInputObject },
  { kAscIOinputMapDevicePinOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_MAP_DEVICE_PIN, GetAscIoInputObject,
    SetTestAscIoInputObject, SetValueAscIoInputObject },
  { kAscIOinputMapFuncTypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_MAP_FUNC_TYPE, GetAscIoInputObject,
    SetTestAscIoInputObject, SetValueAscIoInputObject },
  { kAscIOinputMapFuncPtypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_MAP_FUNC_PTYPE, GetAscIoInputObject,
    SetTestAscIoInputObject, SetValueAscIoInputObject },
  { kAscIOinputMapFunctionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_MAP_FUNCTION, GetAscIoInputObject,
    SetTestAscIoInputObject, SetValueAscIoInputObject },
  { kAscIOinputMapFuncIndexOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_MAP_FUNCTION_INDEX, GetAscIoInputObject,
    SetTestAscIoInputObject, SetValueAscIoInputObject },
  { kAscIOinputMapDevPinDescrOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OCTET_STRING,
    ASC_IO_INPUT_TAG_PIN_DESCRIPTION, GetAscIoInputObject, NULL, NULL },
  { kAscIOinputMapDevPinStatusOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_PIN_STATUS, GetAscIoInputObject, NULL, NULL },
  { kAscIOmapMaxInputFunctionsOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_MAX_INPUT_FUNCTIONS, GetAscIoInputObject, NULL, NULL },
  { kAscIOinputIndexOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_FUNCTION_INDEX, GetAscIoInputObject, NULL, NULL },
  { kAscIOinputMaxFuncIndexOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_INPUT_TAG_FUNCTION_MAX_INDEX, GetAscIoInputObject, NULL, NULL },
  { kAscIOinputFunctionNameOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OCTET_STRING,
    ASC_IO_INPUT_TAG_FUNCTION_NAME, GetAscIoInputObject, NULL, NULL }
};

void AscIoInputMappingObjectsRegister(NtcipObjectDirectory_t *directory,
                                      NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.ascIOmapping.input",
    kAscIoInputMappingObjects,
    (uint16_t) (sizeof(kAscIoInputMappingObjects)
                / sizeof(kAscIoInputMappingObjects[0])),
    context);
}
