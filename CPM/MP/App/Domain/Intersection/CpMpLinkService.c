/* App/Domain/Intersection/CpMpLinkService.c */

#include "Intersection/CpMpLinkService.h"

#include <stddef.h>
#include <string.h>

#include "Intersection/ChannelStateResolver.h"

static void WriteUint16Le(uint8_t *bytes, uint16_t value)
{
  if (bytes == NULL)
  {
    return;
  }

  bytes[0] = (uint8_t) (value & 0xFFU);
  bytes[1] = (uint8_t) ((value >> 8U) & 0xFFU);
}

static void WriteUint32Le(uint8_t *bytes, uint32_t value)
{
  if (bytes == NULL)
  {
    return;
  }

  bytes[0] = (uint8_t) (value & 0xFFU);
  bytes[1] = (uint8_t) ((value >> 8U) & 0xFFU);
  bytes[2] = (uint8_t) ((value >> 16U) & 0xFFU);
  bytes[3] = (uint8_t) ((value >> 24U) & 0xFFU);
}

static uint8_t BuildIntersectionConfig(const CpMpMmuConfigImageV1_t *image,
                                       IntersectionConfig_t *config,
                                       ChannelOutputMapping_t *mapping)
{
  uint8_t phaseIndex;
  uint8_t channelIndex;
  uint8_t outputIndex;

  if ((image == NULL) || (config == NULL) || (mapping == NULL))
  {
    return 0U;
  }

  if ((image->phaseCount == 0U) || (image->phaseCount > INTERSECTION_PHASE_COUNT_MAX)
      || (image->ringCount == 0U) || (image->ringCount > INTERSECTION_RING_COUNT_MAX)
      || (image->channelCount > INTERSECTION_CHANNEL_COUNT_MAX)
      || (image->outputMapCount > CPMP_CONFIG_IMAGE_MAX_OUTPUT_MAP))
  {
    return 0U;
  }

  (void) memset(config, 0, sizeof(*config));
  ChannelStateResolverInit(mapping);

  config->phaseCount = image->phaseCount;
  config->ringCount = image->ringCount;
  config->unit.startUpFlashSeconds = image->startupFlashSeconds;
  config->unit.startUpFlashMode = image->startupFlashMode;

  for (phaseIndex = 0U; phaseIndex < image->phaseCount; ++phaseIndex)
  {
    uint8_t concurrencyIndex = 0U;
    uint8_t phaseMask = image->phaseConcurrencyMask[phaseIndex];
    uint8_t concurrentPhase;

    config->phases[phaseIndex].yellowChangeDs = image->phaseYellowChangeDs[phaseIndex];
    config->phases[phaseIndex].redClearDs = image->phaseRedClearDs[phaseIndex];

    for (concurrentPhase = 0U;
         concurrentPhase < INTERSECTION_PHASE_COUNT_MAX;
         ++concurrentPhase)
    {
      if ((phaseMask & (uint8_t) (1U << concurrentPhase)) != 0U)
      {
        if (concurrencyIndex >= INTERSECTION_PHASE_COUNT_MAX)
        {
          return 0U;
        }

        config->phases[phaseIndex].concurrency.values[concurrencyIndex] =
          (uint8_t) (concurrentPhase + 1U);
        concurrencyIndex++;
      }
    }

    config->phases[phaseIndex].concurrency.length = concurrencyIndex;
  }

  for (channelIndex = 0U; channelIndex < image->channelCount; ++channelIndex)
  {
    uint8_t controlType = image->channelControlType[channelIndex];
    uint8_t controlSource = image->channelControlSource[channelIndex];

    if (((controlType == INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE)
         || (controlType == INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN))
        && ((controlSource == 0U) || (controlSource > image->phaseCount)))
    {
      return 0U;
    }

    config->channels[channelIndex].controlType = controlType;
    config->channels[channelIndex].controlSource = controlSource;
  }

  for (outputIndex = 0U; outputIndex < image->outputMapCount; ++outputIndex)
  {
    const CpMpOutputMapEntry_t *entry = &image->outputMap[outputIndex];
    OutputChannelMap_t *mapEntry;

    if ((entry->outputIndex >= MP_SIGNAL_OUTPUT_COUNT_MAX)
        || (entry->channelIndex >= MP_CHANNEL_COUNT_MAX))
    {
      return 0U;
    }

    if ((entry->color != CPMP_OUTPUT_COLOR_RED)
        && (entry->color != CPMP_OUTPUT_COLOR_YELLOW)
        && (entry->color != CPMP_OUTPUT_COLOR_GREEN))
    {
      return 0U;
    }

    mapEntry = &mapping->outputs[entry->outputIndex];
    mapEntry->channelIndex = entry->channelIndex;
    mapEntry->color = (ChannelColor_t) entry->color;
  }

  return 1U;
}

static uint8_t ApplyConfigImage(CpMpLinkService_t *service)
{
  IntersectionConfig_t config;
  ChannelOutputMapping_t mapping;

  if (service == NULL)
  {
    return 0U;
  }

  if (BuildIntersectionConfig(&service->configImage, &config, &mapping) == 0U)
  {
    service->configState = CPMP_CONFIG_STATE_INVALID;
    return 0U;
  }

  if ((ConfigurationServiceSetConfig(service->configurationService, &config) == 0U)
      || (ConfigurationServiceSetOutputMapping(service->configurationService, &mapping) == 0U)
      || (ConfigurationServiceValidate(service->configurationService) == 0U))
  {
    service->configState = CPMP_CONFIG_STATE_INVALID;
    return 0U;
  }

  service->appliedSetId = service->loadingSetId;
  service->appliedGeneration = service->loadingGeneration;
  service->configState = CPMP_CONFIG_STATE_APPLIED;

  return 1U;
}

static uint16_t CurrentReportedSetId(const CpMpLinkService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  if (service->configState == CPMP_CONFIG_STATE_APPLIED)
  {
    return service->appliedSetId;
  }

  if (service->configState == CPMP_CONFIG_STATE_LOADING)
  {
    return service->loadingSetId;
  }

  return service->expectedSetId;
}

static uint32_t CurrentReportedGeneration(const CpMpLinkService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  if (service->configState == CPMP_CONFIG_STATE_APPLIED)
  {
    return service->appliedGeneration;
  }

  if (service->configState == CPMP_CONFIG_STATE_LOADING)
  {
    return service->loadingGeneration;
  }

  return service->expectedGeneration;
}

static CpMpSafetyAction_t CurrentSafetyAction(const CpMpLinkService_t *service)
{
  SafetyAction_t latchedAction;

  if ((service == NULL) || (service->safetyDecisionService == NULL))
  {
    return CPMP_SAFETY_ACTION_FLASH;
  }

  latchedAction = SafetyDecisionServiceGetLatchedAction(service->safetyDecisionService);
  if (latchedAction == SAFETY_ACTION_DARK)
  {
    return CPMP_SAFETY_ACTION_DARK;
  }

  if (latchedAction == SAFETY_ACTION_FLASH)
  {
    return CPMP_SAFETY_ACTION_FLASH;
  }

  if (((service->tickCount - service->lastCpHeartbeatTick) > CPMP_PEER_TIMEOUT_TICKS)
      || (service->configState != CPMP_CONFIG_STATE_APPLIED)
      || (service->appliedSetId != service->expectedSetId)
      || (service->appliedGeneration != service->expectedGeneration))
  {
    return CPMP_SAFETY_ACTION_FLASH;
  }

  return CPMP_SAFETY_ACTION_NORMAL;
}

static uint8_t CurrentSafetyReasonCode(const CpMpLinkService_t *service)
{
  if (service == NULL)
  {
    return (uint8_t) FAULT_CODE_NONE;
  }

  if ((service->tickCount - service->lastCpHeartbeatTick) > CPMP_PEER_TIMEOUT_TICKS)
  {
    return (uint8_t) FAULT_CODE_MODULE_CP_MISSING;
  }

  if ((service->configState != CPMP_CONFIG_STATE_APPLIED)
      || (service->appliedSetId != service->expectedSetId)
      || (service->appliedGeneration != service->expectedGeneration))
  {
    return (uint8_t) FAULT_CODE_MP_CONFIG_INVALID;
  }

  if (service->safetyDecisionService == NULL)
  {
    return (uint8_t) FAULT_CODE_NONE;
  }

  return (uint8_t) SafetyDecisionServiceGetLastLatchedCode(
    service->safetyDecisionService);
}

static uint16_t BuildChannelFaultFlags(
  const FaultMonitorChannelFlags_t *channelStatus)
{
  uint16_t flags = 0U;

  if (channelStatus == NULL)
  {
    return 0U;
  }

  if (channelStatus->conflict != 0U)
  {
    flags |= CPMP_FAULT_CHANNEL_FLAG_CONFLICT;
  }

  if (channelStatus->dualIndication != 0U)
  {
    flags |= CPMP_FAULT_CHANNEL_FLAG_DUAL_INDICATION;
  }

  if (channelStatus->redFail != 0U)
  {
    flags |= CPMP_FAULT_CHANNEL_FLAG_RED_FAIL;
  }

  if (channelStatus->dark != 0U)
  {
    flags |= CPMP_FAULT_CHANNEL_FLAG_DARK;
  }

  if (channelStatus->minYellowShort != 0U)
  {
    flags |= CPMP_FAULT_CHANNEL_FLAG_MIN_YELLOW_SHORT;
  }

  if (channelStatus->clearanceShort != 0U)
  {
    flags |= CPMP_FAULT_CHANNEL_FLAG_CLEARANCE_SHORT;
  }

  if (channelStatus->signalSequence != 0U)
  {
    flags |= CPMP_FAULT_CHANNEL_FLAG_SIGNAL_SEQUENCE;
  }

  if (channelStatus->lampOpen != 0U)
  {
    flags |= CPMP_FAULT_CHANNEL_FLAG_LAMP_OPEN;
  }

  if (channelStatus->lampExternallyDriven != 0U)
  {
    flags |= CPMP_FAULT_CHANNEL_FLAG_LAMP_EXTERNALLY_DRIVEN;
  }

  return flags;
}

static uint32_t BuildGlobalFaultFlags(const CpMpLinkService_t *service,
                                      const FaultMonitorStatus_t *status)
{
  uint32_t flags = 0U;

  if (status != NULL)
  {
    if (status->global.acLineFault != 0U)
    {
      flags |= CPMP_FAULT_GLOBAL_FLAG_AC_LINE;
    }

    if (status->global.rail24VFault != 0U)
    {
      flags |= CPMP_FAULT_GLOBAL_FLAG_RAIL_24V;
    }

    if (status->global.rail5VFault != 0U)
    {
      flags |= CPMP_FAULT_GLOBAL_FLAG_RAIL_5V;
    }

    if (status->global.cpMissing != 0U)
    {
      flags |= CPMP_FAULT_GLOBAL_FLAG_CP_MISSING;
    }

    if (status->global.psmMissing != 0U)
    {
      flags |= CPMP_FAULT_GLOBAL_FLAG_PSM_MISSING;
    }

    if (status->global.ssmMissing != 0U)
    {
      flags |= CPMP_FAULT_GLOBAL_FLAG_SSM_MISSING;
    }

    if (status->global.watchdog != 0U)
    {
      flags |= CPMP_FAULT_GLOBAL_FLAG_WATCHDOG;
    }

    if (status->global.relayFeedbackMismatch != 0U)
    {
      flags |= CPMP_FAULT_GLOBAL_FLAG_RELAY_FEEDBACK_MISMATCH;
    }

    if (status->global.configInvalid != 0U)
    {
      flags |= CPMP_FAULT_GLOBAL_FLAG_CONFIG_INVALID;
    }

    if (status->global.localFlashActive != 0U)
    {
      flags |= CPMP_FAULT_GLOBAL_FLAG_LOCAL_FLASH_ACTIVE;
    }

    if (status->global.startupFlashActive != 0U)
    {
      flags |= CPMP_FAULT_GLOBAL_FLAG_STARTUP_FLASH_ACTIVE;
    }

    if (status->global.mmuFlashActive != 0U)
    {
      flags |= CPMP_FAULT_GLOBAL_FLAG_MMU_FLASH_ACTIVE;
    }
  }

  if ((service != NULL)
      && ((service->tickCount - service->lastCpHeartbeatTick)
          > CPMP_PEER_TIMEOUT_TICKS))
  {
    flags |= CPMP_FAULT_GLOBAL_FLAG_CP_MISSING;
  }

  if ((service != NULL)
      && ((service->configState != CPMP_CONFIG_STATE_APPLIED)
          || (service->appliedSetId != service->expectedSetId)
          || (service->appliedGeneration != service->expectedGeneration)))
  {
    flags |= CPMP_FAULT_GLOBAL_FLAG_CONFIG_INVALID;
  }

  return flags;
}

static void BuildFaultStatusImage(const CpMpLinkService_t *service,
                                  CpMpFaultStatusImageV1_t *image)
{
  FaultMonitorStatus_t status;
  uint8_t channelIndex;

  if (image == NULL)
  {
    return;
  }

  (void) memset(image, 0, sizeof(*image));

  if ((service != NULL) && (service->faultMonitorService != NULL))
  {
    FaultMonitorServiceRead(service->faultMonitorService, &status);
    image->sequence = status.sequence;

    for (channelIndex = 0U;
         channelIndex < (uint8_t) (sizeof(image->channelFlags)
                                   / sizeof(image->channelFlags[0]));
         channelIndex++)
    {
      image->channelFlags[channelIndex] = BuildChannelFaultFlags(
        &status.channels[channelIndex]);
    }

    image->globalFlags = BuildGlobalFaultFlags(service, &status);
  }
  else
  {
    image->globalFlags = BuildGlobalFaultFlags(service, NULL);
  }

  image->safetyAction = (uint8_t) CurrentSafetyAction(service);
  image->safetyReasonCode = CurrentSafetyReasonCode(service);
  image->configState = (uint8_t) ((service != NULL) ? service->configState
                                                    : CPMP_CONFIG_STATE_INVALID);
}

static uint8_t SendFrame(CpMpLinkService_t *service,
                         uint16_t standardId,
                         const uint8_t *payload,
                         uint8_t payloadLength)
{
  ControlBusFrame_t frame;

  if ((service == NULL) || (service->controlBusPort == NULL)
      || (payloadLength > CONTROL_BUS_FRAME_MAX_LENGTH))
  {
    return 0U;
  }

  (void) memset(&frame, 0, sizeof(frame));
  frame.standardId = standardId;
  frame.length = payloadLength;
  if ((payload != NULL) && (payloadLength != 0U))
  {
    (void) memcpy(frame.data, payload, payloadLength);
  }

  return ControlBusSendFrame(service->controlBusPort, &frame);
}

static void SendHeartbeat(CpMpLinkService_t *service)
{
  uint8_t payload[10];
  uint16_t setId;
  uint32_t generation;

  if (service == NULL)
  {
    return;
  }

  setId = CurrentReportedSetId(service);
  generation = CurrentReportedGeneration(service);

  (void) memset(payload, 0, sizeof(payload));
  payload[0] = CPMP_PROTOCOL_VERSION;
  payload[1] = (uint8_t) service->configState;
  payload[2] = (uint8_t) CurrentSafetyAction(service);
  payload[4] = (uint8_t) (setId & 0xFFU);
  payload[5] = (uint8_t) ((setId >> 8U) & 0xFFU);
  payload[6] = (uint8_t) (generation & 0xFFU);
  payload[7] = (uint8_t) ((generation >> 8U) & 0xFFU);
  payload[8] = (uint8_t) ((generation >> 16U) & 0xFFU);
  payload[9] = (uint8_t) ((generation >> 24U) & 0xFFU);

  (void) SendFrame(service, CPMP_FRAME_ID_MP_HEARTBEAT, payload, sizeof(payload));
}

static void SendConfigStatus(CpMpLinkService_t *service)
{
  uint8_t payload[8];
  uint16_t setId;
  uint32_t generation;

  if (service == NULL)
  {
    return;
  }

  setId = CurrentReportedSetId(service);
  generation = CurrentReportedGeneration(service);

  (void) memset(payload, 0, sizeof(payload));
  payload[0] = CPMP_PROTOCOL_VERSION;
  payload[1] = (uint8_t) service->configState;
  payload[2] = (uint8_t) (setId & 0xFFU);
  payload[3] = (uint8_t) ((setId >> 8U) & 0xFFU);
  payload[4] = (uint8_t) (generation & 0xFFU);
  payload[5] = (uint8_t) ((generation >> 8U) & 0xFFU);
  payload[6] = (uint8_t) ((generation >> 16U) & 0xFFU);
  payload[7] = (uint8_t) ((generation >> 24U) & 0xFFU);

  (void) SendFrame(service, CPMP_FRAME_ID_MP_CFG_STATUS, payload, sizeof(payload));
}

static void SendSafetyStatus(CpMpLinkService_t *service)
{
  uint8_t payload[2];

  if (service == NULL)
  {
    return;
  }

  payload[0] = CPMP_PROTOCOL_VERSION;
  payload[1] = (uint8_t) CurrentSafetyAction(service);

  (void) SendFrame(service, CPMP_FRAME_ID_MP_SAFETY, payload, sizeof(payload));
}

static void SendFaultStatus(CpMpLinkService_t *service)
{
  CpMpFaultStatusImageV1_t image;
  uint8_t payload[44];
  uint8_t channelIndex;

  if (service == NULL)
  {
    return;
  }

  BuildFaultStatusImage(service, &image);

  (void) memset(payload, 0, sizeof(payload));
  payload[0] = CPMP_PROTOCOL_VERSION;
  WriteUint32Le(&payload[1], image.sequence);
  WriteUint32Le(&payload[5], image.globalFlags);
  for (channelIndex = 0U;
       channelIndex < (uint8_t) (sizeof(image.channelFlags)
                                 / sizeof(image.channelFlags[0]));
       channelIndex++)
  {
    WriteUint16Le(&payload[9U + (uint8_t) (channelIndex * 2U)],
                  image.channelFlags[channelIndex]);
  }
  payload[41] = image.safetyAction;
  payload[42] = image.safetyReasonCode;
  payload[43] = image.configState;

  (void) SendFrame(service, CPMP_FRAME_ID_MP_FAULTS, payload, sizeof(payload));
}

static void ResetLoadingState(CpMpLinkService_t *service)
{
  if (service == NULL)
  {
    return;
  }

  (void) memset(&service->configImage, 0, sizeof(service->configImage));
  service->expectedImageBytes = 0U;
  service->totalChunks = 0U;
  service->receivedChunkMask = 0U;
  service->loadingSetId = 0U;
  service->loadingGeneration = 0U;
}

static void OnRxFrame(void *cbCtx, const ControlBusFrame_t *frame)
{
  CpMpLinkService_t *service = (CpMpLinkService_t *) cbCtx;

  if ((service == NULL) || (frame == NULL) || (frame->length == 0U)
      || (frame->data[0] != CPMP_PROTOCOL_VERSION))
  {
    return;
  }

  switch (frame->standardId)
  {
      case CPMP_FRAME_ID_CP_HEARTBEAT:
      {
        if (frame->length >= 12U)
        {
          service->expectedSetId =
            (uint16_t) frame->data[2] | ((uint16_t) frame->data[3] << 8U);
          service->expectedGeneration =
            (uint32_t) frame->data[8]
            | ((uint32_t) frame->data[9] << 8U)
            | ((uint32_t) frame->data[10] << 16U)
            | ((uint32_t) frame->data[11] << 24U);
          service->lastCpHeartbeatTick = service->tickCount;
        }
        break;
      }

      case CPMP_FRAME_ID_CP_CFG_BEGIN:
      {
        if ((frame->length < 10U) || (frame->data[1] == 0U) || (frame->data[1] > 8U))
        {
          service->configState = CPMP_CONFIG_STATE_INVALID;
          ResetLoadingState(service);
          break;
        }

        ResetLoadingState(service);
        service->configState = CPMP_CONFIG_STATE_LOADING;
        service->totalChunks = frame->data[1];
        service->loadingSetId =
          (uint16_t) frame->data[2] | ((uint16_t) frame->data[3] << 8U);
        service->loadingGeneration =
          (uint32_t) frame->data[4]
          | ((uint32_t) frame->data[5] << 8U)
          | ((uint32_t) frame->data[6] << 16U)
          | ((uint32_t) frame->data[7] << 24U);
        service->expectedImageBytes =
          (uint16_t) frame->data[8] | ((uint16_t) frame->data[9] << 8U);

        if (service->expectedImageBytes != sizeof(service->configImage))
        {
          service->configState = CPMP_CONFIG_STATE_INVALID;
          ResetLoadingState(service);
        }
        break;
      }

      case CPMP_FRAME_ID_CP_CFG_CHUNK:
      {
        uint8_t chunkIndex;
        uint8_t chunkLength;
        uint16_t offset;

        if ((service->configState != CPMP_CONFIG_STATE_LOADING)
            || (frame->length < 4U))
        {
          break;
        }

        chunkIndex = frame->data[1];
        chunkLength = frame->data[2];
        offset = (uint16_t) chunkIndex * CPMP_CONFIG_CHUNK_PAYLOAD_BYTES;

        if ((chunkIndex >= service->totalChunks)
            || (chunkLength > CPMP_CONFIG_CHUNK_PAYLOAD_BYTES)
            || ((uint16_t) (offset + chunkLength) > sizeof(service->configImage))
            || ((uint8_t) (4U + chunkLength) > frame->length))
        {
          service->configState = CPMP_CONFIG_STATE_INVALID;
          ResetLoadingState(service);
          break;
        }

        (void) memcpy(&((uint8_t *) &service->configImage)[offset],
                      &frame->data[4],
                      chunkLength);
        service->receivedChunkMask |= (uint8_t) (1U << chunkIndex);
        break;
      }

      case CPMP_FRAME_ID_CP_CFG_COMMIT:
      {
        uint8_t requiredMask;

        if ((service->configState != CPMP_CONFIG_STATE_LOADING)
            || (frame->length < 8U))
        {
          break;
        }

        if (((uint16_t) frame->data[2] | ((uint16_t) frame->data[3] << 8U))
            != service->loadingSetId)
        {
          service->configState = CPMP_CONFIG_STATE_INVALID;
          ResetLoadingState(service);
          break;
        }

        if (((uint32_t) frame->data[4]
             | ((uint32_t) frame->data[5] << 8U)
             | ((uint32_t) frame->data[6] << 16U)
             | ((uint32_t) frame->data[7] << 24U)) != service->loadingGeneration)
        {
          service->configState = CPMP_CONFIG_STATE_INVALID;
          ResetLoadingState(service);
          break;
        }

        requiredMask = (service->totalChunks >= 8U)
                       ? 0xFFU
                       : (uint8_t) ((1U << service->totalChunks) - 1U);
        if (service->receivedChunkMask != requiredMask)
        {
          service->configState = CPMP_CONFIG_STATE_INVALID;
          ResetLoadingState(service);
          break;
        }

        (void) ApplyConfigImage(service);
        ResetLoadingState(service);
        break;
      }

      default:
      {
        break;
      }
  }
}

void CpMpLinkServiceInit(CpMpLinkService_t *service,
                         IControlBusPort_t *controlBusPort,
                         ConfigurationService_t *configurationService,
                         SafetyDecisionService_t *safetyDecisionService,
                         FaultMonitorService_t *faultMonitorService)
{
  if (service == NULL)
  {
    return;
  }

  (void) memset(service, 0, sizeof(*service));
  service->controlBusPort = controlBusPort;
  service->configurationService = configurationService;
  service->safetyDecisionService = safetyDecisionService;
  service->faultMonitorService = faultMonitorService;
  service->configState = CPMP_CONFIG_STATE_EMPTY;

  if ((controlBusPort != NULL)
      && (ControlBusRegisterRxCallback(controlBusPort, OnRxFrame, service) != 0U))
  {
    service->registeredRxCallback = 1U;
  }
}

void CpMpLinkServiceStep(CpMpLinkService_t *service)
{
  if (service == NULL)
  {
    return;
  }

  service->tickCount++;
  SendHeartbeat(service);
  SendConfigStatus(service);
  SendSafetyStatus(service);
  SendFaultStatus(service);
}
