/* App/Domain/NTCIP/Mib1202v0335/AscIoOutputMappingObjects.c
 *
 * Canonical 1202 ascIOmapping output-side subtree. This pass implements the
 * controller-local active output map, live pin status, and the standard output
 * function reference table.
 */
#include "AscIoOutputMappingObjects.h"

#include <stddef.h>
#include <string.h>

#include "Domain/Intersection/IntersectionOutputDispatcher.h"

enum
{
  ASC_IO_TAG_MAX_MAPS = 1,
  ASC_IO_TAG_ACTIVE_MAP,
  ASC_IO_TAG_ACTIVATE_REQUIREMENT,
  ASC_IO_TAG_MAX_OUTPUTS,
  ASC_IO_TAG_OUTPUT_MAP_IO_INDEX,
  ASC_IO_TAG_OUTPUT_MAP_DEVICE_TYPE,
  ASC_IO_TAG_OUTPUT_MAP_DEVICE_PNN,
  ASC_IO_TAG_OUTPUT_MAP_DEVICE_PTYPE,
  ASC_IO_TAG_OUTPUT_MAP_DEVICE_ADDR,
  ASC_IO_TAG_OUTPUT_MAP_DEVICE_PIN,
  ASC_IO_TAG_OUTPUT_MAP_FUNC_TYPE,
  ASC_IO_TAG_OUTPUT_MAP_FUNC_PTYPE,
  ASC_IO_TAG_OUTPUT_MAP_FUNCTION,
  ASC_IO_TAG_OUTPUT_MAP_FUNCTION_INDEX,
  ASC_IO_TAG_OUTPUT_PIN_DESCRIPTION,
  ASC_IO_TAG_OUTPUT_PIN_STATUS,
  ASC_IO_TAG_MAP_DESCRIPTION,
  ASC_IO_TAG_MAX_OUTPUT_FUNCTIONS,
  ASC_IO_TAG_OUTPUT_FUNCTION_INDEX,
  ASC_IO_TAG_OUTPUT_FUNCTION_MAX_INDEX,
  ASC_IO_TAG_OUTPUT_FUNCTION_NAME
};

static const uint32_t kAscIOmaxMapsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 1U, 1U
};
static const uint32_t kAscIOactiveMapOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 1U, 2U
};
static const uint32_t kAscIOactivateRequirementOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 1U, 3U
};
static const uint32_t kAscIOmapMaxOutputsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 3U
};
static const uint32_t kAscIOoutputMapIOindexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 1U
};
static const uint32_t kAscIOoutputMapDeviceTypeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 2U
};
static const uint32_t kAscIOoutputMapDevicePNNOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 3U
};
static const uint32_t kAscIOoutputMapDevicePtypeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 4U
};
static const uint32_t kAscIOoutputMapDeviceAddrOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 5U
};
static const uint32_t kAscIOoutputMapDevicePinOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 6U
};
static const uint32_t kAscIOoutputMapFuncTypeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 7U
};
static const uint32_t kAscIOoutputMapFuncPtypeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 8U
};
static const uint32_t kAscIOoutputMapFunctionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 9U
};
static const uint32_t kAscIOoutputMapFuncIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 6U, 1U, 11U
};
static const uint32_t kAscIOoutputMapDevPinDescrOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 7U, 1U, 1U
};
static const uint32_t kAscIOoutputMapDevPinStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 7U, 1U, 2U
};
static const uint32_t kAscIOmapDescriptionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 8U, 1U, 1U
};
static const uint32_t kAscIOmapMaxOutputFunctionsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 10U, 1U
};
static const uint32_t kAscIOoutputIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 10U, 2U, 1U, 1U
};
static const uint32_t kAscIOoutputMaxFuncIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 10U, 2U, 1U, 2U
};
static const uint32_t kAscIOoutputFunctionNameOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 13U, 10U, 2U, 1U, 3U
};

static const char *const kOutputFunctionNames[] = {
  NULL,
  "unusedOutput",
  "ioUsedAsInput",
  "logicOutput",
  "advWarnGrn",
  "advWarnRed",
  "alarmOutput",
  "automaticFlashStatus",
  "channelGreen",
  "channelRed",
  "channelYellow",
  "codedStatusBitA",
  "codedStatusBitB",
  "codedStatusBitC",
  "detectorResetSlots",
  "detectorReset",
  "faultMonitor",
  "flashingLogic",
  "freeStatus",
  "offsetOutput",
  "phaseCheck",
  "phaseNext",
  "phaseOn",
  "preemptActive",
  "preemptActiveAdvanced",
  "specialFunctionOutput",
  "tbcAuxOutput",
  "timingPlanOutput",
  "voltageMonitor",
  "watchdog"
};

static const char *const kFioOutputPins[] = {
  "pinC1-2", "pinC1-3", "pinC1-4", "pinC1-5", "pinC1-6", "pinC1-7",
  "pinC1-8", "pinC1-9", "pinC1-10", "pinC1-11", "pinC1-12", "pinC1-13",
  "pinC1-15", "pinC1-16", "pinC1-17", "pinC1-18", "pinC1-19", "pinC1-20",
  "pinC1-21", "pinC1-22", "pinC1-23", "pinC1-24", "pinC1-25", "pinC1-26",
  "pinC1-27", "pinC1-28", "pinC1-29", "pinC1-30", "pinC1-31", "pinC1-32",
  "pinC1-33", "pinC1-34", "pinC1-35", "pinC1-36", "pinC1-37", "pinC1-38",
  "pinC1-83", "pinC1-84", "pinC1-85", "pinC1-86", "pinC1-87", "pinC1-88",
  "pinC1-89", "pinC1-90", "pinC1-91", "pinC1-93", "pinC1-94", "pinC1-95",
  "pinC1-96", "pinC1-97", "pinC1-98", "pinC1-99", "pinC1-100", "pinC1-101",
  "pinC1-102", "pinC1-103", "pinC11-1", "pinC11-2", "pinC11-3", "pinC11-4",
  "pinC11-5", "pinC11-6", "pinC11-7", "pinC11-8"
};

static const char *const kTs1OutputPins[] = {
  "pinA-a", "pinA-b", "pinA-c", "pinA-d", "pinA-e", "pinA-r", "pinA-s",
  "pinA-t", "pinA-u", "pinA-A", "pinA-C", "pinA-D", "pinA-E", "pinA-F",
  "pinA-G", "pinA-H", "pinA-J", "pinA-X", "pinA-Y", "pinA-Z", "pinA-CC",
  "pinA-DD", "pinB-a", "pinB-b", "pinB-c", "pinB-d", "pinB-e", "pinB-f",
  "pinB-p", "pinB-q", "pinB-r", "pinB-s", "pinB-t", "pinB-u", "pinB-w",
  "pinB-A", "pinB-C", "pinB-D", "pinB-E", "pinB-F", "pinB-G", "pinB-H",
  "pinB-J", "pinB-K", "pinB-Y", "pinB-Z", "pinB-AA", "pinB-BB", "pinB-CC",
  "pinB-DD", "pinB-EE", "pinB-FF", "pinB-GG", "pinB-HH", "pinC-c", "pinC-d",
  "pinC-e", "pinC-f", "pinC-g", "pinC-h", "pinC-i", "pinC-j", "pinC-k",
  "pinC-w", "pinC-x", "pinC-y", "pinC-z", "pinC-A", "pinC-B", "pinC-C",
  "pinC-D", "pinC-E", "pinC-F", "pinC-G", "pinC-H", "pinC-J", "pinC-K",
  "pinC-L", "pinC-M", "pinC-N", "pinC-AA", "pinC-BB", "pinC-CC", "pinC-DD",
  "pinC-FF", "pinC-GG", "pinC-HH", "pinC-JJ", "pinC-KK", "pinC-LL",
  "pinC-MM", "pinC-NN", "pinC-PP", "pinD-z", "pinD-AA", "pinD-BB",
  "pinD-CC", "pinD-DD", "pinD-EE", "pinD-FF", "pinD-GG", "pinD-HH",
  "pinD-JJ", "pinD-LL"
};

static const char *const kBiuOutputPins[] = {
  "biuOutputO1", "biuOutputO2", "biuOutputO3", "biuOutputO4", "biuOutputO5",
  "biuOutputO6", "biuOutputO7", "biuOutputO8", "biuOutputO9", "biuOutputO10",
  "biuOutputO11", "biuOutputO12", "biuOutputO13", "biuOutputO14",
  "biuOutputO15", "biuOutputIO1", "biuOutputIO2", "biuOutputIO3",
  "biuOutputIO4", "biuOutputIO5", "biuOutputIO6", "biuOutputIO7",
  "biuOutputIO8", "biuOutputIO9", "biuOutputIO10", "biuOutputIO11",
  "biuOutputIO12", "biuOutputIO13", "biuOutputIO14", "biuOutputIO15",
  "biuOutputIO16", "biuOutputIO17", "biuOutputIO18", "biuOutputIO19",
  "biuOutputIO20", "biuOutputIO21", "biuOutputIO22", "biuOutputIO23",
  "biuOutputIO24"
};

static const char *const kSiuOutputPins[] = {
  "siuOutputIO0", "siuOutputIO1", "siuOutputIO2", "siuOutputIO3",
  "siuOutputIO4", "siuOutputIO5", "siuOutputIO6", "siuOutputIO7",
  "siuOutputIO8", "siuOutputIO9", "siuOutputIO10", "siuOutputIO11",
  "siuOutputIO12", "siuOutputIO13", "siuOutputIO14", "siuOutputIO15",
  "siuOutputIO16", "siuOutputIO17", "siuOutputIO18", "siuOutputIO19",
  "siuOutputIO20", "siuOutputIO21", "siuOutputIO22", "siuOutputIO23",
  "siuOutputIO24", "siuOutputIO25", "siuOutputIO26", "siuOutputIO27",
  "siuOutputIO28", "siuOutputIO29", "siuOutputIO30", "siuOutputIO31",
  "siuOutputIO32", "siuOutputIO33", "siuOutputIO34", "siuOutputIO35",
  "siuOutputIO36", "siuOutputIO37", "siuOutputIO38", "siuOutputIO39",
  "siuOutputIO40", "siuOutputIO41", "siuOutputIO42", "siuOutputIO43",
  "siuOutputIO44", "siuOutputIO45", "siuOutputIO46", "siuOutputIO47",
  "siuOutputIO48", "siuOutputIO49", "siuOutputIO50", "siuOutputIO51",
  "siuOutputIO52", "siuOutputIO53"
};

static const char *const kAuxOutputPins[] = {
  "auxOutputIO0", "auxOutputIO1", "auxOutputIO2", "auxOutputIO3",
  "auxOutputIO4", "auxOutputIO5", "auxOutputIO6", "auxOutputIO7"
};

static uint16_t DescriptionLength(const uint8_t *description,
                                  uint16_t maxLength)
{
  uint16_t length = 0U;

  if (description == NULL)
  {
    return 0U;
  }

  while ((length < maxLength) && (description[length] != 0U))
  {
    length++;
  }

  return length;
}

static void SetDescription(uint8_t *target,
                           uint16_t maxLength,
                           const NtcipValue_t *value)
{
  uint16_t index;

  if ((target == NULL) || (value == NULL))
  {
    return;
  }

  for (index = 0U; index < maxLength; index++)
  {
    target[index] = 0U;
  }

  for (index = 0U; (index < value->data.octetString.length)
       && (index < maxLength); index++)
  {
    target[index] = value->data.octetString.bytes[index];
  }
}

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

static NtcipError_t GetMapIndex(const uint32_t *indexes,
                                uint8_t indexCount,
                                uint8_t *mapIndex)
{
  if ((indexes == NULL) || (indexCount != 1U) || (mapIndex == NULL)
      || (indexes[0] != 1U))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *mapIndex = 0U;

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetOutputRowIndex(const uint32_t *indexes,
                                      uint8_t indexCount,
                                      uint8_t *rowIndex)
{
  if ((indexes == NULL) || (indexCount != 2U) || (rowIndex == NULL)
      || (indexes[0] != 1U) || (indexes[1] == 0U)
      || (indexes[1] > INTERSECTION_IO_MAP_MAX_OUTPUTS))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *rowIndex = (uint8_t) (indexes[1] - 1U);

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetOutputFunctionIndex(const uint32_t *indexes,
                                           uint8_t indexCount,
                                           uint8_t *functionIndex)
{
  if ((indexes == NULL) || (indexCount != 1U) || (functionIndex == NULL)
      || (indexes[0] == 0U)
      || (indexes[0]
          >= (sizeof(kOutputFunctionNames) / sizeof(kOutputFunctionNames[0]))))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *functionIndex = (uint8_t) indexes[0];

  return NTCIP_ERROR_OK;
}

static const char *GetOutputPinName(uint8_t deviceType, uint8_t pinNumber)
{
  if (pinNumber == 0U)
  {
    return NULL;
  }

  switch ((IntersectionIoMapDeviceType_t) deviceType)
  {
      case INTERSECTION_IO_MAP_DEVICE_FIO:
      {
        return (pinNumber <= (sizeof(kFioOutputPins) / sizeof(kFioOutputPins[0])))
                 ? kFioOutputPins[pinNumber - 1U]
                 : NULL;
      }

      case INTERSECTION_IO_MAP_DEVICE_TS1:
      {
        return (pinNumber <= (sizeof(kTs1OutputPins) / sizeof(kTs1OutputPins[0])))
                 ? kTs1OutputPins[pinNumber - 1U]
                 : NULL;
      }

      case INTERSECTION_IO_MAP_DEVICE_BIU:
      {
        return (pinNumber <= (sizeof(kBiuOutputPins) / sizeof(kBiuOutputPins[0])))
                 ? kBiuOutputPins[pinNumber - 1U]
                 : NULL;
      }

      case INTERSECTION_IO_MAP_DEVICE_SIU:
      {
        return (pinNumber <= (sizeof(kSiuOutputPins) / sizeof(kSiuOutputPins[0])))
                 ? kSiuOutputPins[pinNumber - 1U]
                 : NULL;
      }

      case INTERSECTION_IO_MAP_DEVICE_AUX:
      {
        return (pinNumber <= (sizeof(kAuxOutputPins) / sizeof(kAuxOutputPins[0])))
                 ? kAuxOutputPins[pinNumber - 1U]
                 : NULL;
      }

      case INTERSECTION_IO_MAP_DEVICE_UNUSED:
      case INTERSECTION_IO_MAP_DEVICE_CUSTOM:
      default:
      {
        return NULL;
      }
  }
}

static uint8_t OutputAspectMatchesFunction(OutputDriverAspect_t aspect,
                                           uint8_t function)
{
  switch ((IntersectionIoMapOutputFunction_t) function)
  {
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_RED:
      {
        return (uint8_t) ((aspect == OUTPUT_DRIVER_ASPECT_RED)
                          || (aspect == OUTPUT_DRIVER_ASPECT_FLASH_RED));
      }

      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_YELLOW:
      {
        return (uint8_t) ((aspect == OUTPUT_DRIVER_ASPECT_YELLOW)
                          || (aspect == OUTPUT_DRIVER_ASPECT_FLASH_YELLOW));
      }

      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_GREEN:
      {
        return (uint8_t) ((aspect == OUTPUT_DRIVER_ASPECT_GREEN)
                          || (aspect == OUTPUT_DRIVER_ASPECT_FLASH_GREEN));
      }

      default:
      {
        return 0U;
      }
  }
}

static uint8_t GetOutputPinStatus(const NtcipContext_t *context,
                                  const IntersectionIoOutputMapRowConfig_t *row)
{
  OutputDriverImage_t image;

  if ((context == NULL) || (row == NULL)
      || (context->intersectionController == NULL)
      || (context->intersectionController->outputDispatcher == NULL)
      || (row->functionIndex == 0U)
      || (row->functionIndex > INTERSECTION_CHANNEL_COUNT_MAX)
      || (IntersectionOutputDispatcherGetLastAppliedImage(
            context->intersectionController->outputDispatcher,
            &image) == 0U))
  {
    return 0U;
  }

  if ((row->function
       != (uint8_t) INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_RED)
      && (row->function
          != (uint8_t) INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_YELLOW)
      && (row->function
          != (uint8_t) INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_GREEN))
  {
    return 0U;
  }

  return OutputAspectMatchesFunction(
    image.channels[row->functionIndex - 1U],
    row->function);
}

static uint32_t GetOutputFunctionMaxIndex(uint8_t functionIndex)
{
  switch ((IntersectionIoMapOutputFunction_t) functionIndex)
  {
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_GREEN:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_RED:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_YELLOW:
      {
        return INTERSECTION_CHANNEL_COUNT_MAX;
      }

      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_PHASE_CHECK:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_PHASE_NEXT:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_PHASE_ON:
      {
        return INTERSECTION_PHASE_COUNT_MAX;
      }

      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_PREEMPT_ACTIVE:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_PREEMPT_ACTIVE_ADVANCED:
      {
        return INTERSECTION_PREEMPT_COUNT_MAX;
      }

      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_SPECIAL_FUNCTION_OUTPUT:
      {
        return INTERSECTION_SPECIAL_FUNCTION_OUTPUT_COUNT;
      }

      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_TIMING_PLAN_OUTPUT:
      {
        return INTERSECTION_PATTERN_COUNT_MAX;
      }

      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_UNUSED_OUTPUT:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_IO_USED_AS_INPUT:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_LOGIC_OUTPUT:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_ADV_WARN_GREEN:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_ADV_WARN_RED:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_ALARM_OUTPUT:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_AUTOMATIC_FLASH_STATUS:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CODED_STATUS_BIT_A:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CODED_STATUS_BIT_B:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CODED_STATUS_BIT_C:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_DETECTOR_RESET_SLOTS:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_DETECTOR_RESET:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_FAULT_MONITOR:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_FLASHING_LOGIC:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_FREE_STATUS:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_OFFSET_OUTPUT:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_TBC_AUX_OUTPUT:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_VOLTAGE_MONITOR:
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_WATCHDOG:
      default:
      {
        return 1U;
      }
  }
}

static NtcipError_t GetAscIoObject(void *groupContext,
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
  NtcipError_t error;
  const char *pinName;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case ASC_IO_TAG_MAX_MAPS:
      {
        NtcipValueSetUnsigned32(value, 1U);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_ACTIVE_MAP:
      {
        NtcipValueSetUnsigned32(value, 1U);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_ACTIVATE_REQUIREMENT:
      {
        NtcipValueSetUnsigned32(value, 0U);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_MAX_OUTPUTS:
      {
        NtcipValueSetUnsigned32(value, INTERSECTION_IO_MAP_MAX_OUTPUTS);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_MAX_OUTPUT_FUNCTIONS:
      {
        NtcipValueSetUnsigned32(
          value,
          (uint32_t) ((sizeof(kOutputFunctionNames)
                       / sizeof(kOutputFunctionNames[0]))
                      - 1U));

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_MAP_DESCRIPTION:
      {
        error = GetMapIndex(indexes, indexCount, &rowIndex);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        (void) rowIndex;
        error = GetActiveIoMapConfig(context, &ioMap);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        return NtcipValueSetOctetString(
          value,
          ioMap.description,
          DescriptionLength(ioMap.description, INTERSECTION_IO_MAP_DESCRIPTION_MAX));
      }

      case ASC_IO_TAG_OUTPUT_MAP_IO_INDEX:
      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_TYPE:
      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PNN:
      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PTYPE:
      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_ADDR:
      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PIN:
      case ASC_IO_TAG_OUTPUT_MAP_FUNC_TYPE:
      case ASC_IO_TAG_OUTPUT_MAP_FUNC_PTYPE:
      case ASC_IO_TAG_OUTPUT_MAP_FUNCTION:
      case ASC_IO_TAG_OUTPUT_MAP_FUNCTION_INDEX:
      case ASC_IO_TAG_OUTPUT_PIN_DESCRIPTION:
      case ASC_IO_TAG_OUTPUT_PIN_STATUS:
      {
        error = GetOutputRowIndex(indexes, indexCount, &rowIndex);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        error = GetActiveIoMapConfig(context, &ioMap);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        break;
      }

      case ASC_IO_TAG_OUTPUT_FUNCTION_INDEX:
      case ASC_IO_TAG_OUTPUT_FUNCTION_MAX_INDEX:
      case ASC_IO_TAG_OUTPUT_FUNCTION_NAME:
      {
        error = GetOutputFunctionIndex(indexes, indexCount, &functionIndex);

        if (error != NTCIP_ERROR_OK)
        {
          return error;
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
      case ASC_IO_TAG_OUTPUT_MAP_IO_INDEX:
      {
        NtcipValueSetUnsigned32(value, indexes[1]);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_TYPE:
      {
        NtcipValueSetUnsigned32(value, ioMap.outputs[rowIndex].deviceType);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PNN:
      {
        NtcipValueSetUnsigned32(value, ioMap.outputs[rowIndex].devicePnn);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PTYPE:
      {
        NtcipValueSetUnsigned32(value, ioMap.outputs[rowIndex].devicePtype);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_ADDR:
      {
        NtcipValueSetUnsigned32(value, ioMap.outputs[rowIndex].deviceAddr);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PIN:
      {
        NtcipValueSetUnsigned32(value, ioMap.outputs[rowIndex].devicePin);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_MAP_FUNC_TYPE:
      {
        NtcipValueSetUnsigned32(value, ioMap.outputs[rowIndex].functionType);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_MAP_FUNC_PTYPE:
      {
        NtcipValueSetUnsigned32(value, ioMap.outputs[rowIndex].functionPtype);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_MAP_FUNCTION:
      {
        NtcipValueSetUnsigned32(value, ioMap.outputs[rowIndex].function);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_MAP_FUNCTION_INDEX:
      {
        NtcipValueSetUnsigned32(value, ioMap.outputs[rowIndex].functionIndex);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_PIN_DESCRIPTION:
      {
        pinName = GetOutputPinName(ioMap.outputs[rowIndex].deviceType,
                                   ioMap.outputs[rowIndex].devicePin);

        if (pinName == NULL)
        {
          return NtcipValueSetOctetString(value, NULL, 0U);
        }

        return NtcipValueSetOctetString(value,
                                        (const uint8_t *) pinName,
                                        (uint16_t) strlen(pinName));
      }

      case ASC_IO_TAG_OUTPUT_PIN_STATUS:
      {
        NtcipValueSetUnsigned32(
          value,
          GetOutputPinStatus(context, &ioMap.outputs[rowIndex]) != 0U ? 1U : 0U);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_FUNCTION_INDEX:
      {
        NtcipValueSetUnsigned32(value, functionIndex);

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_FUNCTION_MAX_INDEX:
      {
        NtcipValueSetUnsigned32(value, GetOutputFunctionMaxIndex(functionIndex));

        return NTCIP_ERROR_OK;
      }

      case ASC_IO_TAG_OUTPUT_FUNCTION_NAME:
      {
        return NtcipValueSetOctetString(
          value,
          (const uint8_t *) kOutputFunctionNames[functionIndex],
          (uint16_t) strlen(kOutputFunctionNames[functionIndex]));
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetTestAscIoObject(void *groupContext,
                                       const NtcipObjectDescriptor_t *descriptor,
                                       const uint32_t *indexes,
                                       uint8_t indexCount,
                                       const NtcipRequestContext_t *requestContext,
                                       const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  NtcipError_t error;
  uint8_t rowIndex;

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
      case ASC_IO_TAG_ACTIVE_MAP:
      {
        return (value->type == NTCIP_VALUE_TYPE_UNSIGNED32)
               && (value->data.unsigned32 == 1U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_BAD_VALUE;
      }

      case ASC_IO_TAG_MAP_DESCRIPTION:
      {
        if (GetMapIndex(indexes, indexCount, &rowIndex) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        (void) rowIndex;

        return (value->type == NTCIP_VALUE_TYPE_OCTET_STRING)
               && (value->data.octetString.length
                   <= INTERSECTION_IO_MAP_DESCRIPTION_MAX)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_BAD_VALUE;
      }

      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_TYPE:
      {
        if (GetOutputRowIndex(indexes, indexCount, &rowIndex) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        (void) rowIndex;

        return (value->type == NTCIP_VALUE_TYPE_UNSIGNED32)
               && (value->data.unsigned32
                   != (uint32_t) INTERSECTION_IO_MAP_DEVICE_CUSTOM)
               && (value->data.unsigned32
                   >= (uint32_t) INTERSECTION_IO_MAP_DEVICE_UNUSED)
               && (value->data.unsigned32
                   <= (uint32_t) INTERSECTION_IO_MAP_DEVICE_AUX)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_BAD_VALUE;
      }

      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PNN:
      case ASC_IO_TAG_OUTPUT_MAP_FUNC_TYPE:
      {
        if (GetOutputRowIndex(indexes, indexCount, &rowIndex) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        (void) rowIndex;

        return (value->type == NTCIP_VALUE_TYPE_UNSIGNED32)
               && (value->data.unsigned32 <= 65535U)
               && ((descriptor->tag != ASC_IO_TAG_OUTPUT_MAP_FUNC_TYPE)
                   || (value->data.unsigned32 == 0U))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_BAD_VALUE;
      }

      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PTYPE:
      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_ADDR:
      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PIN:
      case ASC_IO_TAG_OUTPUT_MAP_FUNC_PTYPE:
      case ASC_IO_TAG_OUTPUT_MAP_FUNCTION_INDEX:
      {
        if (GetOutputRowIndex(indexes, indexCount, &rowIndex) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        (void) rowIndex;

        return (value->type == NTCIP_VALUE_TYPE_UNSIGNED32)
               && (value->data.unsigned32 <= 255U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_BAD_VALUE;
      }

      case ASC_IO_TAG_OUTPUT_MAP_FUNCTION:
      {
        if (GetOutputRowIndex(indexes, indexCount, &rowIndex) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        (void) rowIndex;

        if (value->type != NTCIP_VALUE_TYPE_UNSIGNED32)
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        switch ((IntersectionIoMapOutputFunction_t) value->data.unsigned32)
        {
            case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_UNUSED_OUTPUT:
            case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_IO_USED_AS_INPUT:
            case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_GREEN:
            case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_RED:
            case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_YELLOW:
            {
              return NTCIP_ERROR_OK;
            }

            default:
            {
              return NTCIP_ERROR_BAD_VALUE;
            }
        }
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }
}

static NtcipError_t SetValueAscIoObject(void *groupContext,
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

  error = SetTestAscIoObject(groupContext,
                             descriptor,
                             indexes,
                             indexCount,
                             requestContext,
                             value);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if (descriptor->tag == ASC_IO_TAG_ACTIVE_MAP)
  {
    return NTCIP_ERROR_OK;
  }

  if ((context == NULL) || (GetCandidateIoMapConfig(context, &ioMap) != NTCIP_ERROR_OK))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  switch (descriptor->tag)
  {
      case ASC_IO_TAG_MAP_DESCRIPTION:
      {
        SetDescription(ioMap.description,
                       INTERSECTION_IO_MAP_DESCRIPTION_MAX,
                       value);
        break;
      }

      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_TYPE:
      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PNN:
      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PTYPE:
      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_ADDR:
      case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PIN:
      case ASC_IO_TAG_OUTPUT_MAP_FUNC_TYPE:
      case ASC_IO_TAG_OUTPUT_MAP_FUNC_PTYPE:
      case ASC_IO_TAG_OUTPUT_MAP_FUNCTION:
      case ASC_IO_TAG_OUTPUT_MAP_FUNCTION_INDEX:
      {
        if (GetOutputRowIndex(indexes, indexCount, &rowIndex) != NTCIP_ERROR_OK)
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        switch (descriptor->tag)
        {
            case ASC_IO_TAG_OUTPUT_MAP_DEVICE_TYPE:
            {
              ioMap.outputs[rowIndex].deviceType = (uint8_t) value->data.unsigned32;
              break;
            }

            case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PNN:
            {
              ioMap.outputs[rowIndex].devicePnn = (uint16_t) value->data.unsigned32;
              break;
            }

            case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PTYPE:
            {
              ioMap.outputs[rowIndex].devicePtype = (uint8_t) value->data.unsigned32;
              break;
            }

            case ASC_IO_TAG_OUTPUT_MAP_DEVICE_ADDR:
            {
              ioMap.outputs[rowIndex].deviceAddr = (uint8_t) value->data.unsigned32;
              break;
            }

            case ASC_IO_TAG_OUTPUT_MAP_DEVICE_PIN:
            {
              ioMap.outputs[rowIndex].devicePin = (uint8_t) value->data.unsigned32;
              break;
            }

            case ASC_IO_TAG_OUTPUT_MAP_FUNC_TYPE:
            {
              ioMap.outputs[rowIndex].functionType = (uint16_t) value->data.unsigned32;
              break;
            }

            case ASC_IO_TAG_OUTPUT_MAP_FUNC_PTYPE:
            {
              ioMap.outputs[rowIndex].functionPtype = (uint8_t) value->data.unsigned32;
              break;
            }

            case ASC_IO_TAG_OUTPUT_MAP_FUNCTION:
            {
              ioMap.outputs[rowIndex].function = (uint8_t) value->data.unsigned32;
              break;
            }

            case ASC_IO_TAG_OUTPUT_MAP_FUNCTION_INDEX:
            default:
            {
              ioMap.outputs[rowIndex].functionIndex = (uint8_t) value->data.unsigned32;
              break;
            }
        }

        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }

  return SaveCandidateIoMapConfig(context, &ioMap);
}

static const NtcipObjectDescriptor_t kAscIoOutputMappingObjects[] = {
  { kAscIOmaxMapsOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32, ASC_IO_TAG_MAX_MAPS,
    GetAscIoObject, NULL, NULL },
  { kAscIOactiveMapOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_ACTIVE_MAP, GetAscIoObject, SetTestAscIoObject,
    SetValueAscIoObject },
  { kAscIOactivateRequirementOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_ACTIVATE_REQUIREMENT, GetAscIoObject, NULL, NULL },
  { kAscIOmapMaxOutputsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_MAX_OUTPUTS, GetAscIoObject, NULL, NULL },
  { kAscIOoutputMapIOindexOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_MAP_IO_INDEX, GetAscIoObject, NULL, NULL },
  { kAscIOoutputMapDeviceTypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_MAP_DEVICE_TYPE, GetAscIoObject, SetTestAscIoObject,
    SetValueAscIoObject },
  { kAscIOoutputMapDevicePNNOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_MAP_DEVICE_PNN, GetAscIoObject, SetTestAscIoObject,
    SetValueAscIoObject },
  { kAscIOoutputMapDevicePtypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_MAP_DEVICE_PTYPE, GetAscIoObject, SetTestAscIoObject,
    SetValueAscIoObject },
  { kAscIOoutputMapDeviceAddrOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_MAP_DEVICE_ADDR, GetAscIoObject, SetTestAscIoObject,
    SetValueAscIoObject },
  { kAscIOoutputMapDevicePinOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_MAP_DEVICE_PIN, GetAscIoObject, SetTestAscIoObject,
    SetValueAscIoObject },
  { kAscIOoutputMapFuncTypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_MAP_FUNC_TYPE, GetAscIoObject, SetTestAscIoObject,
    SetValueAscIoObject },
  { kAscIOoutputMapFuncPtypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_MAP_FUNC_PTYPE, GetAscIoObject, SetTestAscIoObject,
    SetValueAscIoObject },
  { kAscIOoutputMapFunctionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_MAP_FUNCTION, GetAscIoObject, SetTestAscIoObject,
    SetValueAscIoObject },
  { kAscIOoutputMapFuncIndexOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_MAP_FUNCTION_INDEX, GetAscIoObject, SetTestAscIoObject,
    SetValueAscIoObject },
  { kAscIOoutputMapDevPinDescrOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OCTET_STRING,
    ASC_IO_TAG_OUTPUT_PIN_DESCRIPTION, GetAscIoObject, NULL, NULL },
  { kAscIOoutputMapDevPinStatusOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_PIN_STATUS, GetAscIoObject, NULL, NULL },
  { kAscIOmapDescriptionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    ASC_IO_TAG_MAP_DESCRIPTION, GetAscIoObject, SetTestAscIoObject,
    SetValueAscIoObject },
  { kAscIOmapMaxOutputFunctionsOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_MAX_OUTPUT_FUNCTIONS, GetAscIoObject, NULL, NULL },
  { kAscIOoutputIndexOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_FUNCTION_INDEX, GetAscIoObject, NULL, NULL },
  { kAscIOoutputMaxFuncIndexOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    ASC_IO_TAG_OUTPUT_FUNCTION_MAX_INDEX, GetAscIoObject, NULL, NULL },
  { kAscIOoutputFunctionNameOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OCTET_STRING,
    ASC_IO_TAG_OUTPUT_FUNCTION_NAME, GetAscIoObject, NULL, NULL }
};

void AscIoOutputMappingObjectsRegister(NtcipObjectDirectory_t *directory,
                                       NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.ascIOmapping.output",
    kAscIoOutputMappingObjects,
    (uint16_t) (sizeof(kAscIoOutputMappingObjects)
                / sizeof(kAscIoOutputMappingObjects[0])),
    context);
}
