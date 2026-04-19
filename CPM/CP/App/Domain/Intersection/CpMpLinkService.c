/* App/Domain/Intersection/CpMpLinkService.c */
#include "CpMpLinkService.h"

#include <stddef.h>
#include <string.h>

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

uint8_t CpMpLinkServiceAuthorityReady(const CpMpLinkService_t *service)
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
                                      CpMpFaultStatusImageV1_t *faultStatus)
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
  uint8_t outputRowIndex;
  uint8_t outputMapCount = 0U;

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

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_CHANNEL_COUNT_MAX; phaseIndex++)
  {
    service->configImage.channelControlType[phaseIndex] =
      config->channels[phaseIndex].controlType;
    service->configImage.channelControlSource[phaseIndex] =
      config->channels[phaseIndex].controlSource;
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

  if (service == NULL)
  {
    return;
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
        if (frame->length >= 44U)
        {
          uint8_t channelIndex;

          service->lastFaultStatus.sequence = ReadUint32Le(&frame->data[1]);
          service->lastFaultStatus.globalFlags = ReadUint32Le(&frame->data[5]);
          for (channelIndex = 0U;
               channelIndex < (uint8_t) (sizeof(service->lastFaultStatus.channelFlags)
                                         / sizeof(service->lastFaultStatus.channelFlags[0]));
               channelIndex++)
          {
            service->lastFaultStatus.channelFlags[channelIndex] =
              ReadUint16Le(&frame->data[9U + (uint8_t) (channelIndex * 2U)]);
          }
          service->lastFaultStatus.safetyAction = frame->data[41];
          service->lastFaultStatus.safetyReasonCode = frame->data[42];
          service->lastFaultStatus.configState = frame->data[43];
          service->lastFaultStatus.reserved0 = 0U;
          service->lastFaultStatusValid = 1U;
          service->lastSafetyAction =
            (CpMpSafetyAction_t) service->lastFaultStatus.safetyAction;
          service->lastSafetyReasonCode =
            service->lastFaultStatus.safetyReasonCode;
        }
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
                         IntersectionController_t *controller)
{
  if (service == NULL)
  {
    return;
  }

  (void) memset(service, 0, sizeof(*service));
  service->controlBusPort = controlBusPort;
  service->configurationService = configurationService;
  service->controller = controller;
  service->lastSafetyAction = CPMP_SAFETY_ACTION_FLASH;
  service->lastMpConfigState = CPMP_CONFIG_STATE_EMPTY;

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
