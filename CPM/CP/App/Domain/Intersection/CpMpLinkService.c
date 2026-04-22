/* App/Domain/Intersection/CpMpLinkService.c */
#include "CpMpLinkService.h"

#include <stddef.h>
#include <string.h>

#define CPMP_LOG_EVENT_MP_MMU_FAULT 120U

static uint16_t ReadUint16Le(const uint8_t *bytes)
{
  if (bytes == NULL)
  {
    return 0U;
  }

  return (uint16_t) bytes[0] | ((uint16_t) bytes[1] << 8U);
}

static uint32_t ReadUint32Le(const uint8_t *bytes)
{
  if (bytes == NULL)
  {
    return 0U;
  }

  return (uint32_t) bytes[0]
         | ((uint32_t) bytes[1] << 8U)
         | ((uint32_t) bytes[2] << 16U)
         | ((uint32_t) bytes[3] << 24U);
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

static MmuControlAction_t ToControllerSafetyAction(CpMpSafetyAction_t action)
{
  switch (action)
  {
      case CPMP_SAFETY_ACTION_FLASH:
      {
        return MMU_CONTROL_ACTION_FLASH;
      }

      case CPMP_SAFETY_ACTION_DARK:
      {
        return MMU_CONTROL_ACTION_DARK;
      }

      case CPMP_SAFETY_ACTION_NORMAL:
      default:
      {
        return MMU_CONTROL_ACTION_NORMAL;
      }
  }
}

static uint8_t PhaseListContains(const IntersectionPhaseReferenceList_t *list,
                                 uint8_t phaseNumber)
{
  uint8_t index;

  if (list == NULL)
  {
    return 0U;
  }

  for (index = 0U; index < list->length; index++)
  {
    if (list->values[index] == phaseNumber)
    {
      return 1U;
    }
  }

  return 0U;
}

static uint8_t PhasesAreConcurrent(const IntersectionConfig_t *config,
                                   uint8_t phaseA,
                                   uint8_t phaseB)
{
  const IntersectionPhaseConfig_t *a;
  const IntersectionPhaseConfig_t *b;

  if ((config == NULL) || (phaseA == 0U) || (phaseB == 0U))
  {
    return 0U;
  }

  if (phaseA == phaseB)
  {
    return 1U;
  }

  if ((phaseA > config->phaseCount) || (phaseB > config->phaseCount))
  {
    return 0U;
  }

  a = &config->phases[phaseA - 1U];
  b = &config->phases[phaseB - 1U];

  return (uint8_t) (PhaseListContains(&a->concurrency, phaseB)
                    || PhaseListContains(&b->concurrency, phaseA));
}

static uint16_t PhaseMaskFromList(const IntersectionPhaseReferenceList_t *list,
                                  uint8_t phaseCount)
{
  uint16_t mask = 0U;
  uint8_t index;

  if (list == NULL)
  {
    return 0U;
  }

  for (index = 0U; index < list->length; index++)
  {
    uint8_t phaseNumber = list->values[index];

    if ((phaseNumber == 0U) || (phaseNumber > phaseCount)
        || (phaseNumber > INTERSECTION_PHASE_COUNT_MAX))
    {
      continue;
    }

    mask |= (uint16_t) (1U << (phaseNumber - 1U));
  }

  return mask;
}

static uint16_t ChannelPhaseMask(const IntersectionConfig_t *config,
                                 uint8_t channelIndex)
{
  const IntersectionChannelConfig_t *channel;

  if ((config == NULL) || (channelIndex >= INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  channel = &config->channels[channelIndex];

  switch ((IntersectionChannelControlType_t) channel->controlType)
  {
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN:
      {
        if ((channel->controlSource == 0U)
            || (channel->controlSource > INTERSECTION_PHASE_COUNT_MAX)
            || (channel->controlSource > config->phaseCount))
        {
          return 0U;
        }

        return (uint16_t) (1U << (channel->controlSource - 1U));
      }

      case INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PED_OVERLAP:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_QUEUE_JUMP:
      {
        if ((channel->controlSource == 0U)
            || (channel->controlSource > INTERSECTION_OVERLAP_COUNT_MAX))
        {
          return 0U;
        }

        return PhaseMaskFromList(
          &config->overlaps[channel->controlSource - 1U].includedPhases,
          config->phaseCount);
      }

      default:
      {
        return 0U;
      }
  }
}

static uint8_t PhaseMasksCanRunTogether(const IntersectionConfig_t *config,
                                        uint16_t phaseMaskA,
                                        uint16_t phaseMaskB)
{
  uint8_t phaseA;
  uint8_t phaseB;

  if ((config == NULL) || (phaseMaskA == 0U) || (phaseMaskB == 0U))
  {
    return 0U;
  }

  for (phaseA = 0U; phaseA < INTERSECTION_PHASE_COUNT_MAX; phaseA++)
  {
    if ((phaseMaskA & (uint16_t) (1U << phaseA)) == 0U)
    {
      continue;
    }

    for (phaseB = 0U; phaseB < INTERSECTION_PHASE_COUNT_MAX; phaseB++)
    {
      if ((phaseMaskB & (uint16_t) (1U << phaseB)) == 0U)
      {
        continue;
      }

      if (PhasesAreConcurrent(config, (uint8_t) (phaseA + 1U),
                              (uint8_t) (phaseB + 1U)) != 0U)
      {
        return 1U;
      }
    }
  }

  return 0U;
}

static uint32_t ChannelConflictMask(const IntersectionConfig_t *config,
                                    const uint16_t *channelPhaseMasks,
                                    uint8_t channelIndex)
{
  uint32_t mask = 0U;
  uint8_t otherIndex;

  if ((config == NULL) || (channelPhaseMasks == NULL)
      || (channelIndex >= INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  if (channelPhaseMasks[channelIndex] == 0U)
  {
    return 0U;
  }

  for (otherIndex = 0U; otherIndex < INTERSECTION_CHANNEL_COUNT_MAX; otherIndex++)
  {
    if ((otherIndex == channelIndex) || (channelPhaseMasks[otherIndex] == 0U))
    {
      continue;
    }

    if (PhaseMasksCanRunTogether(config,
                                 channelPhaseMasks[channelIndex],
                                 channelPhaseMasks[otherIndex]) == 0U)
    {
      mask |= (uint32_t) (1UL << otherIndex);
    }
  }

  return mask;
}

static uint8_t ClampDsToByte(uint16_t value)
{
  return (value > 0xFFU) ? 0xFFU : (uint8_t) value;
}

static uint8_t ChannelMinYellowDs(const IntersectionConfig_t *config,
                                  uint8_t channelIndex)
{
  const IntersectionChannelConfig_t *channel;

  if ((config == NULL) || (channelIndex >= INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  channel = &config->channels[channelIndex];

  switch ((IntersectionChannelControlType_t) channel->controlType)
  {
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN:
      {
        if ((channel->controlSource == 0U)
            || (channel->controlSource > config->phaseCount))
        {
          return 0U;
        }

        return ClampDsToByte(
          config->phases[channel->controlSource - 1U].yellowChangeDs);
      }

      case INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PED_OVERLAP:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_QUEUE_JUMP:
      {
        if ((channel->controlSource == 0U)
            || (channel->controlSource > INTERSECTION_OVERLAP_COUNT_MAX))
        {
          return 0U;
        }

        return ClampDsToByte(
          config->overlaps[channel->controlSource - 1U].trailYellowDs);
      }

      default:
      {
        return 0U;
      }
  }
}

static uint8_t ChannelRedClearDs(const IntersectionConfig_t *config,
                                 uint8_t channelIndex)
{
  const IntersectionChannelConfig_t *channel;

  if ((config == NULL) || (channelIndex >= INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  channel = &config->channels[channelIndex];

  switch ((IntersectionChannelControlType_t) channel->controlType)
  {
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN:
      {
        if ((channel->controlSource == 0U)
            || (channel->controlSource > config->phaseCount))
        {
          return 0U;
        }

        return ClampDsToByte(
          config->phases[channel->controlSource - 1U].redClearDs);
      }

      case INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PED_OVERLAP:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_QUEUE_JUMP:
      {
        if ((channel->controlSource == 0U)
            || (channel->controlSource > INTERSECTION_OVERLAP_COUNT_MAX))
        {
          return 0U;
        }

        return ClampDsToByte(
          config->overlaps[channel->controlSource - 1U].trailRedDs);
      }

      default:
      {
        return 0U;
      }
  }
}

uint8_t CpMpLinkServicePeerHealthy(const CpMpLinkService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((service->lastMpHeartbeatSeen != 0U)
                    && ((service->tickCount - service->lastMpHeartbeatTick)
                        <= CPMP_PEER_TIMEOUT_TICKS));
}

static uint8_t PeerConfigMatchesActiveConfig(const CpMpLinkService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((CpMpLinkServicePeerHealthy(service) != 0U)
                    && (service->lastMpConfigState == CPMP_CONFIG_STATE_APPLIED)
                    && (service->lastMpConfigSetId == service->configSetId)
                    && (service->lastMpConfigGeneration
                        == service->configGeneration));
}

uint8_t CpMpLinkServiceAuthorityReady(const CpMpLinkService_t *service)
{
  return PeerConfigMatchesActiveConfig(service);
}

CpMpSafetyAction_t CpMpLinkServiceGetEffectiveSafetyAction(
  const CpMpLinkService_t *service)
{
  if (service == NULL)
  {
    return CPMP_SAFETY_ACTION_FLASH;
  }

  if (CpMpLinkServiceAuthorityReady(service) == 0U)
  {
    return CPMP_SAFETY_ACTION_FLASH;
  }

  return service->lastSafetyAction;
}

uint8_t CpMpLinkServiceGetLastSafetyReasonCode(
  const CpMpLinkService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  return service->lastSafetyReasonCode;
}

uint8_t CpMpLinkServiceGetFaultStatus(const CpMpLinkService_t *service,
                                      CpMpFaultStatusImage_t *faultStatus)
{
  if ((service == NULL) || (faultStatus == NULL)
      || (service->lastFaultStatusValid == 0U))
  {
    return 0U;
  }

  *faultStatus = service->lastFaultStatus;
  return 1U;
}

static void ApplyEffectiveSafetyAction(CpMpLinkService_t *service)
{
  MmuControlAction_t action = MMU_CONTROL_ACTION_FLASH;

  if ((service == NULL) || (service->controller == NULL))
  {
    return;
  }

  action = ToControllerSafetyAction(CpMpLinkServiceGetEffectiveSafetyAction(
    service));

  (void) IntersectionControllerSetMmuSafetyAction(service->controller, action);
}

static uint8_t BuildConfigImage(CpMpLinkService_t *service)
{
  const IntersectionConfig_t *config;
  uint8_t phaseIndex;
  uint8_t channelIndex;
  uint8_t outputRowIndex;
  uint8_t outputMapCount = 0U;
  uint16_t channelPhaseMasks[INTERSECTION_CHANNEL_COUNT_MAX];

  if ((service == NULL) || (service->configurationService == NULL))
  {
    return 0U;
  }

  config = ConfigurationServiceGetActiveConfig(service->configurationService);
  if (config == NULL)
  {
    return 0U;
  }

  (void) memset(&service->configImage, 0, sizeof(service->configImage));
  service->configImage.phaseCount = config->phaseCount;
  service->configImage.ringCount = config->ringCount;
  service->configImage.channelCount = INTERSECTION_CHANNEL_COUNT_MAX;
  service->configImage.startupFlashSeconds = config->unit.startUpFlashSeconds;
  service->configImage.startupFlashMode = config->unit.startUpFlashMode;
  service->configImage.failureFlashPeriodDs = config->unit.failureFlashPeriodDs;

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    uint8_t concurrencyIndex;
    uint8_t mask = 0U;

    service->configImage.phaseYellowChangeDs[phaseIndex] =
      (uint8_t) config->phases[phaseIndex].yellowChangeDs;
    service->configImage.phaseRedClearDs[phaseIndex] =
      (uint8_t) config->phases[phaseIndex].redClearDs;

    for (concurrencyIndex = 0U;
         concurrencyIndex < config->phases[phaseIndex].concurrency.length;
         concurrencyIndex++)
    {
      uint8_t phaseNumber =
        config->phases[phaseIndex].concurrency.values[concurrencyIndex];

      if ((phaseNumber != 0U) && (phaseNumber <= INTERSECTION_PHASE_COUNT_MAX))
      {
        mask |= (uint8_t) (1U << (phaseNumber - 1U));
      }
    }

    service->configImage.phaseConcurrencyMask[phaseIndex] = mask;
  }

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    service->configImage.channelControlType[channelIndex] =
      config->channels[channelIndex].controlType;
    service->configImage.channelControlSource[channelIndex] =
      config->channels[channelIndex].controlSource;
    service->configImage.channelFlashMask[channelIndex] =
      config->channels[channelIndex].flashMask;
    channelPhaseMasks[channelIndex] = ChannelPhaseMask(config, channelIndex);
  }

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    service->configImage.channelConflictMask[channelIndex] =
      ChannelConflictMask(config, channelPhaseMasks, channelIndex);
    service->configImage.channelMinYellowDs[channelIndex] =
      ChannelMinYellowDs(config, channelIndex);
    service->configImage.channelRedClearDs[channelIndex] =
      ChannelRedClearDs(config, channelIndex);
  }

  for (outputRowIndex = 0U;
       outputRowIndex < INTERSECTION_IO_MAP_MAX_OUTPUTS;
       outputRowIndex++)
  {
    const IntersectionIoOutputMapRowConfig_t *row =
      &config->ioMap.outputs[outputRowIndex];
    CpMpOutputColor_t color = CPMP_OUTPUT_COLOR_NONE;

    if ((row->devicePin == 0U) || (row->devicePin > 96U)
        || (row->functionIndex == 0U)
        || (row->functionIndex > INTERSECTION_CHANNEL_COUNT_MAX))
    {
      continue;
    }

    switch ((IntersectionIoMapOutputFunction_t) row->function)
    {
        case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_RED:
        {
          color = CPMP_OUTPUT_COLOR_RED;
          break;
        }

        case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_YELLOW:
        {
          color = CPMP_OUTPUT_COLOR_YELLOW;
          break;
        }

        case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_GREEN:
        {
          color = CPMP_OUTPUT_COLOR_GREEN;
          break;
        }

        default:
        {
          break;
        }
    }

    if ((color == CPMP_OUTPUT_COLOR_NONE)
        || (outputMapCount >= CPMP_CONFIG_IMAGE_MAX_OUTPUT_MAP))
    {
      continue;
    }

    service->configImage.outputMap[outputMapCount].outputIndex =
      (uint8_t) (row->devicePin - 1U);
    service->configImage.outputMap[outputMapCount].channelIndex =
      (uint8_t) (row->functionIndex - 1U);
    service->configImage.outputMap[outputMapCount].color = (uint8_t) color;
    outputMapCount++;
  }

  service->configImage.outputMapCount = outputMapCount;
  service->configGeneration = ConfigurationServiceGetActiveGeneration(
    service->configurationService);
  service->configSetId = ConfigurationServiceGetActiveSetId(
    service->configurationService);

  return 1U;
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

static void ApplyFaultMask(CpMpFaultStatusImage_t *faultStatus,
                           uint16_t flag,
                           uint32_t mask)
{
  uint8_t channelIndex;

  if (faultStatus == NULL)
  {
    return;
  }

  for (channelIndex = 0U; channelIndex < CPMP_CONFIG_CHANNEL_COUNT;
       channelIndex++)
  {
    if ((mask & (uint32_t) (1UL << channelIndex)) != 0U)
    {
      faultStatus->channelFlags[channelIndex] |= flag;
    }
  }
}

static void StoreReceivedEvent(CpMpLinkService_t *service,
                               const CpMpFaultEventV1_t *event)
{
  uint8_t slot;

  if ((service == NULL) || (event == NULL))
  {
    return;
  }

  slot = service->receivedEventHead;
  service->receivedEvents[slot] = *event;
  service->receivedEventHead = (uint8_t) ((slot + 1U)
    % CPMP_RECEIVED_EVENT_HISTORY_CAPACITY);
  if (service->receivedEventCount < CPMP_RECEIVED_EVENT_HISTORY_CAPACITY)
  {
    service->receivedEventCount++;
  }
}

static void LogReceivedEvent(CpMpLinkService_t *service,
                             const CpMpFaultEventV1_t *event)
{
  uint8_t packedParam;

  if ((service == NULL) || (event == NULL) || (service->logEventPort == NULL))
  {
    return;
  }

  packedParam = (uint8_t) (((event->severity & 0x03U) << 6U)
                           | (event->code & 0x3FU));
  (void) LogEventAppend(service->logEventPort,
                        CPMP_LOG_EVENT_MP_MMU_FAULT,
                        packedParam,
                        event->source,
                        event->param);
}

static void SendEventAck(CpMpLinkService_t *service, uint32_t sequence)
{
  uint8_t payload[5];

  if (service == NULL)
  {
    return;
  }

  (void) memset(payload, 0, sizeof(payload));
  payload[0] = CPMP_PROTOCOL_VERSION;
  WriteUint32Le(&payload[1], sequence);
  (void) SendFrame(service, CPMP_FRAME_ID_CP_EVENT_ACK, payload, sizeof(payload));
}

static void QueueConfigTransfer(CpMpLinkService_t *service)
{
  uint16_t imageBytes;

  if ((service == NULL) || (BuildConfigImage(service) == 0U))
  {
    return;
  }

  imageBytes = (uint16_t) sizeof(service->configImage);
  service->totalChunks =
    (uint8_t) ((imageBytes + CPMP_CONFIG_CHUNK_PAYLOAD_BYTES - 1U)
               / CPMP_CONFIG_CHUNK_PAYLOAD_BYTES);
  service->nextChunkIndex = 0U;
  service->txState = CPMP_TX_STATE_BEGIN;
}

static void SendHeartbeat(CpMpLinkService_t *service)
{
  uint8_t payload[16];
  uint8_t localPermitOutputPower = 0U;

  if (service == NULL)
  {
    return;
  }

  if (service->relayControlService != NULL)
  {
    localPermitOutputPower = RelayControlServiceGetLocalPermitOutputPower(
      service->relayControlService);
  }

  (void) memset(payload, 0, sizeof(payload));
  payload[0] = CPMP_PROTOCOL_VERSION;
  payload[1] = (uint8_t) service->lastSafetyAction;
  payload[2] = (uint8_t) (service->configSetId & 0xFFU);
  payload[3] = (uint8_t) ((service->configSetId >> 8U) & 0xFFU);
  payload[4] = (uint8_t) (service->tickCount & 0xFFU);
  payload[5] = (uint8_t) ((service->tickCount >> 8U) & 0xFFU);
  payload[6] = (uint8_t) ((service->tickCount >> 16U) & 0xFFU);
  payload[7] = (uint8_t) ((service->tickCount >> 24U) & 0xFFU);
  payload[8] = (uint8_t) (service->configGeneration & 0xFFU);
  payload[9] = (uint8_t) ((service->configGeneration >> 8U) & 0xFFU);
  payload[10] = (uint8_t) ((service->configGeneration >> 16U) & 0xFFU);
  payload[11] = (uint8_t) ((service->configGeneration >> 24U) & 0xFFU);
  payload[12] = localPermitOutputPower;

  (void) SendFrame(service, CPMP_FRAME_ID_CP_HEARTBEAT, payload, sizeof(payload));
}

static void SendConfigBegin(CpMpLinkService_t *service)
{
  uint8_t payload[12];
  uint16_t imageBytes;

  if (service == NULL)
  {
    return;
  }

  imageBytes = (uint16_t) sizeof(service->configImage);
  (void) memset(payload, 0, sizeof(payload));
  payload[0] = CPMP_PROTOCOL_VERSION;
  payload[1] = service->totalChunks;
  payload[2] = (uint8_t) (service->configSetId & 0xFFU);
  payload[3] = (uint8_t) ((service->configSetId >> 8U) & 0xFFU);
  payload[4] = (uint8_t) (service->configGeneration & 0xFFU);
  payload[5] = (uint8_t) ((service->configGeneration >> 8U) & 0xFFU);
  payload[6] = (uint8_t) ((service->configGeneration >> 16U) & 0xFFU);
  payload[7] = (uint8_t) ((service->configGeneration >> 24U) & 0xFFU);
  payload[8] = (uint8_t) (imageBytes & 0xFFU);
  payload[9] = (uint8_t) ((imageBytes >> 8U) & 0xFFU);

  if (SendFrame(service, CPMP_FRAME_ID_CP_CFG_BEGIN, payload, sizeof(payload))
      != 0U)
  {
    service->txState = CPMP_TX_STATE_CHUNKS;
  }
}

static void SendConfigChunk(CpMpLinkService_t *service)
{
  uint8_t payload[64];
  uint16_t imageBytes;
  uint16_t offset;
  uint16_t remaining;
  uint8_t length;

  if ((service == NULL) || (service->nextChunkIndex >= service->totalChunks))
  {
    return;
  }

  imageBytes = (uint16_t) sizeof(service->configImage);
  offset = (uint16_t) ((uint16_t) service->nextChunkIndex
                       * CPMP_CONFIG_CHUNK_PAYLOAD_BYTES);
  remaining = (uint16_t) (imageBytes - offset);
  if (remaining > CPMP_CONFIG_CHUNK_PAYLOAD_BYTES)
  {
    length = CPMP_CONFIG_CHUNK_PAYLOAD_BYTES;
  }
  else
  {
    length = (uint8_t) remaining;
  }

  (void) memset(payload, 0, sizeof(payload));
  payload[0] = CPMP_PROTOCOL_VERSION;
  payload[1] = service->nextChunkIndex;
  payload[2] = length;
  (void) memcpy(&payload[4],
                &((const uint8_t *) &service->configImage)[offset],
                length);

  if (SendFrame(service, CPMP_FRAME_ID_CP_CFG_CHUNK, payload, sizeof(payload))
      != 0U)
  {
    service->nextChunkIndex++;
    service->txState = (service->nextChunkIndex >= service->totalChunks)
                       ? CPMP_TX_STATE_COMMIT
                       : CPMP_TX_STATE_CHUNKS;
  }
}

static void SendConfigCommit(CpMpLinkService_t *service)
{
  uint8_t payload[8];

  if (service == NULL)
  {
    return;
  }

  (void) memset(payload, 0, sizeof(payload));
  payload[0] = CPMP_PROTOCOL_VERSION;
  payload[2] = (uint8_t) (service->configSetId & 0xFFU);
  payload[3] = (uint8_t) ((service->configSetId >> 8U) & 0xFFU);
  payload[4] = (uint8_t) (service->configGeneration & 0xFFU);
  payload[5] = (uint8_t) ((service->configGeneration >> 8U) & 0xFFU);
  payload[6] = (uint8_t) ((service->configGeneration >> 16U) & 0xFFU);
  payload[7] = (uint8_t) ((service->configGeneration >> 24U) & 0xFFU);

  if (SendFrame(service, CPMP_FRAME_ID_CP_CFG_COMMIT, payload, sizeof(payload))
      != 0U)
  {
    service->txState = CPMP_TX_STATE_IDLE;
  }
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
      case CPMP_FRAME_ID_MP_HEARTBEAT:
      {
        if (frame->length >= 10U)
        {
          service->lastMpConfigState = (CpMpConfigState_t) frame->data[1];
          service->lastSafetyAction = (CpMpSafetyAction_t) frame->data[2];
          service->lastMpPermitOutputPower = (uint8_t) (frame->data[3] != 0U);
          service->lastMpConfigSetId = ReadUint16Le(&frame->data[4]);
          service->lastMpConfigGeneration = ReadUint32Le(&frame->data[6]);
        }
        service->lastMpHeartbeatTick = service->tickCount;
        service->lastMpHeartbeatSeen = 1U;
        break;
      }

      case CPMP_FRAME_ID_MP_CFG_STATUS:
      {
        if (frame->length >= 8U)
        {
          service->lastMpConfigState = (CpMpConfigState_t) frame->data[1];
          service->lastMpConfigSetId = ReadUint16Le(&frame->data[2]);
          service->lastMpConfigGeneration = ReadUint32Le(&frame->data[4]);
        }
        break;
      }

      case CPMP_FRAME_ID_MP_SAFETY:
      {
        CpMpSafetyAction_t action;

        if (frame->length < 2U)
        {
          break;
        }

        action = (CpMpSafetyAction_t) frame->data[1];
        service->lastSafetyAction = action;
        break;
      }

      case CPMP_FRAME_ID_MP_FAULTS:
      {
        if (frame->length >= 48U)
        {
          (void) memset(&service->lastFaultStatus, 0, sizeof(service->lastFaultStatus));
          service->lastFaultStatus.sequence = ReadUint32Le(&frame->data[1]);
          service->lastFaultStatus.globalFlags = ReadUint32Le(&frame->data[5]);
          ApplyFaultMask(&service->lastFaultStatus,
                         CPMP_FAULT_CHANNEL_FLAG_CONFLICT,
                         ReadUint32Le(&frame->data[9]));
          ApplyFaultMask(&service->lastFaultStatus,
                         CPMP_FAULT_CHANNEL_FLAG_DUAL_INDICATION,
                         ReadUint32Le(&frame->data[13]));
          ApplyFaultMask(&service->lastFaultStatus,
                         CPMP_FAULT_CHANNEL_FLAG_RED_FAIL,
                         ReadUint32Le(&frame->data[17]));
          ApplyFaultMask(&service->lastFaultStatus,
                         CPMP_FAULT_CHANNEL_FLAG_DARK,
                         ReadUint32Le(&frame->data[21]));
          ApplyFaultMask(&service->lastFaultStatus,
                         CPMP_FAULT_CHANNEL_FLAG_MIN_YELLOW_SHORT,
                         ReadUint32Le(&frame->data[25]));
          ApplyFaultMask(&service->lastFaultStatus,
                         CPMP_FAULT_CHANNEL_FLAG_CLEARANCE_SHORT,
                         ReadUint32Le(&frame->data[29]));
          ApplyFaultMask(&service->lastFaultStatus,
                         CPMP_FAULT_CHANNEL_FLAG_SIGNAL_SEQUENCE,
                         ReadUint32Le(&frame->data[33]));
          ApplyFaultMask(&service->lastFaultStatus,
                         CPMP_FAULT_CHANNEL_FLAG_LAMP_OPEN,
                         ReadUint32Le(&frame->data[37]));
          ApplyFaultMask(&service->lastFaultStatus,
                         CPMP_FAULT_CHANNEL_FLAG_LAMP_EXTERNALLY_DRIVEN,
                         ReadUint32Le(&frame->data[41]));
          service->lastFaultStatus.safetyAction = frame->data[45];
          service->lastFaultStatus.safetyReasonCode = frame->data[46];
          service->lastFaultStatus.configState = frame->data[47];
          service->lastFaultStatus.reserved0 = 0U;
          service->lastFaultStatusValid = 1U;
          service->lastSafetyAction =
            (CpMpSafetyAction_t) service->lastFaultStatus.safetyAction;
          service->lastSafetyReasonCode =
            service->lastFaultStatus.safetyReasonCode;
        }
        break;
      }

      case CPMP_FRAME_ID_MP_EVENT:
      {
        CpMpFaultEventV1_t event;

        if (frame->length < 17U)
        {
          break;
        }

        event.sequence = ReadUint32Le(&frame->data[1]);
        event.timestampTicks = ReadUint32Le(&frame->data[5]);
        event.param = ReadUint32Le(&frame->data[9]);
        event.source = ReadUint16Le(&frame->data[13]);
        event.code = frame->data[15];
        event.severity = frame->data[16];

        if (event.sequence != service->lastReceivedEventSequence)
        {
          service->lastReceivedEventSequence = event.sequence;
          StoreReceivedEvent(service, &event);
          LogReceivedEvent(service, &event);
        }

        SendEventAck(service, event.sequence);
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
                         ILogEventPort_t *logEventPort,
                         ConfigurationService_t *configurationService,
                         IntersectionController_t *controller,
                         RelayControlService_t *relayControlService)
{
  if (service == NULL)
  {
    return;
  }

  (void) memset(service, 0, sizeof(*service));
  service->controlBusPort = controlBusPort;
  service->logEventPort = logEventPort;
  service->configurationService = configurationService;
  service->controller = controller;
  service->relayControlService = relayControlService;
  service->lastSafetyAction = CPMP_SAFETY_ACTION_FLASH;
  service->lastMpConfigState = CPMP_CONFIG_STATE_EMPTY;
  RelayControlServiceSetPeerState(relayControlService, 0U, 0U);

  QueueConfigTransfer(service);
  ApplyEffectiveSafetyAction(service);

  if ((controlBusPort != NULL)
      && (ControlBusRegisterRxCallback(controlBusPort, OnRxFrame, service) != 0U))
  {
    service->registeredRxCallback = 1U;
  }
}

void CpMpLinkServiceStep(CpMpLinkService_t *service)
{
  uint32_t activeGeneration;
  uint16_t activeSetId;

  if (service == NULL)
  {
    return;
  }

  service->tickCount++;

  if (service->relayControlService != NULL)
  {
    RelayControlServiceSetPeerState(service->relayControlService,
                                    CpMpLinkServicePeerHealthy(service),
                                    service->lastMpPermitOutputPower);
  }

  if (service->configurationService != NULL)
  {
    activeGeneration = ConfigurationServiceGetActiveGeneration(
      service->configurationService);
    activeSetId = ConfigurationServiceGetActiveSetId(
      service->configurationService);

    if ((activeGeneration != service->configGeneration)
        || (activeSetId != service->configSetId))
    {
      QueueConfigTransfer(service);
    }

    if ((service->txState == CPMP_TX_STATE_IDLE)
        && (PeerConfigMatchesActiveConfig(service) == 0U))
    {
      QueueConfigTransfer(service);
    }
  }

  ApplyEffectiveSafetyAction(service);
  SendHeartbeat(service);

  switch (service->txState)
  {
      case CPMP_TX_STATE_BEGIN:
      {
        SendConfigBegin(service);
        break;
      }

      case CPMP_TX_STATE_CHUNKS:
      {
        SendConfigChunk(service);
        break;
      }

      case CPMP_TX_STATE_COMMIT:
      {
        SendConfigCommit(service);
        break;
      }

      case CPMP_TX_STATE_IDLE:
      default:
      {
        break;
      }
  }
}
