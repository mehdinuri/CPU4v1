/* App/Adapters/STM32/FieldBusRxAdapter.c
 *
 * FDCAN1 field input adapter for FEIG loop detectors, legacy pedestrian
 * inputs, and the module-snapshot families that now ride on the shared
 * field bus. FEIG occupancy is consumed from TPDO1 when available and from
 * legacy 0x580+node SDO responses as a compatibility fallback. Low-rate
 * 210x.sub4/sub5 polling is retained for per-loop offline/fault detail.
 */
#include "FieldBusRxAdapter.h"

#include <string.h>

#include "cmsis_os2.h"
#include "FieldCanQueueTx.h"
#include "LegacyFieldCanIds.h"
#include "stm32h7xx_hal.h"

#define FIELD_INPUT_FEIG_NODE_COUNT 8U
#define FIELD_INPUT_FEIG_LOOPS_PER_NODE 4U
#define FIELD_INPUT_FEIG_TPDO1_BASE 0x180U
#define FIELD_INPUT_FEIG_SDO_RESPONSE_BASE 0x580U
#define FIELD_INPUT_FEIG_SDO_REQUEST_BASE LEGACY_FIELD_CAN_ID_LOOP_SDO_REQUEST_BASE
#define FIELD_INPUT_FEIG_HEARTBEAT_BASE 0x700U
#define FIELD_INPUT_FEIG_EMCY_BASE 0x300U
#define FIELD_INPUT_PED_LEGACY_BASE LEGACY_FIELD_CAN_ID_IO_INPUTS0
#define FIELD_INPUT_SSM_MEASURE_BASE 0x050U
#define FIELD_INPUT_PSM_MEASURE_BASE 0x05AU
#define FIELD_INPUT_MODULE_PREEMPT_INPUT_ID 0x512U
#define FIELD_INPUT_MODULE_PREEMPT_CONTROL_ID 0x513U
#define FIELD_INPUT_MODULE_MMU_STATUS_ID 0x514U

#define FIELD_INPUT_MODULE_TYPE_PREEMPT_INPUTS 3U
#define FIELD_INPUT_MODULE_TYPE_PREEMPT_CONTROLS 4U
#define FIELD_INPUT_MODULE_TYPE_MMU_STATUS 5U

#define FIELD_INPUT_FEIG_TIMEOUT_MS 1500U
#define FIELD_INPUT_PED_TIMEOUT_MS 500U
#define FIELD_INPUT_SSM_TIMEOUT_MS 100U
#define FIELD_INPUT_MODULE_TIMEOUT_INPUTS_MS 100U
#define FIELD_INPUT_MODULE_TIMEOUT_CRITICAL_MS 30U
#define FIELD_INPUT_FEIG_STARTUP_INTERVAL_MS 50U
#define FIELD_INPUT_FEIG_HEALTH_POLL_INTERVAL_MS 100U
#define FIELD_INPUT_RX_DEPTH 32U
#define FIELD_INPUT_RX_MAX_PAYLOAD_BYTES 64U

#define FIELD_INPUT_DETECTOR_ALARM_COMMUNICATIONS 0x08U
#define FIELD_INPUT_REPORTED_ALARM_OTHER 0x01U

#define FIELD_INPUT_DYNAMIC_SOURCE_MASK \
  (MODULE_BUS_SNAPSHOT_VALID_DETECTORS \
   | MODULE_BUS_SNAPSHOT_VALID_PEDS \
   | MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH \
   | MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS)

#define FIELD_INPUT_MODULE_SOURCE_INDEX_PREEMPT_INPUTS 0U
#define FIELD_INPUT_MODULE_SOURCE_INDEX_PREEMPT_CONTROLS 1U
#define FIELD_INPUT_MODULE_SOURCE_INDEX_MMU_STATUS 2U
#define FIELD_INPUT_MODULE_SOURCE_COUNT 3U

typedef struct
{
  uint16_t standardId;
  uint8_t length;
  uint8_t data[FIELD_INPUT_RX_MAX_PAYLOAD_BYTES];
} FieldInputQueuedFrame_t;

static FieldBusRxAdapterCtx_t *s_registeredCtx = NULL;

static uint16_t ReadLe16(const uint8_t *data)
{
  return (uint16_t) ((uint16_t) data[0] | ((uint16_t) data[1] << 8U));
}

static uint16_t ReadPsmField(uint8_t low, uint8_t high)
{
  return (uint16_t) (low + ((uint16_t) high << 8U));
}

static uint8_t DlcToLength(uint32_t dlc)
{
  switch (dlc)
  {
      case FDCAN_DLC_BYTES_0: return 0U;
      case FDCAN_DLC_BYTES_1: return 1U;
      case FDCAN_DLC_BYTES_2: return 2U;
      case FDCAN_DLC_BYTES_3: return 3U;
      case FDCAN_DLC_BYTES_4: return 4U;
      case FDCAN_DLC_BYTES_5: return 5U;
      case FDCAN_DLC_BYTES_6: return 6U;
      case FDCAN_DLC_BYTES_7: return 7U;
      case FDCAN_DLC_BYTES_8: return 8U;
      case FDCAN_DLC_BYTES_12: return 12U;
      case FDCAN_DLC_BYTES_16: return 16U;
      case FDCAN_DLC_BYTES_20: return 20U;
      case FDCAN_DLC_BYTES_24: return 24U;
      case FDCAN_DLC_BYTES_32: return 32U;
      case FDCAN_DLC_BYTES_48: return 48U;
      case FDCAN_DLC_BYTES_64: return 64U;
      default: return 0U;
  }
}

static uint32_t CurrentTickMs(void)
{
  return HAL_GetTick();
}

static uint8_t SourceStatusIsHealthy(uint8_t status)
{
  return (uint8_t) ((status & MODULE_BUS_SOURCE_STATUS_FAULT) == 0U);
}

static uint32_t ModuleSourceTimeoutMs(uint8_t sourceIndex)
{
  switch (sourceIndex)
  {
      case FIELD_INPUT_MODULE_SOURCE_INDEX_MMU_STATUS:
      {
        return FIELD_INPUT_MODULE_TIMEOUT_CRITICAL_MS;
      }

      case FIELD_INPUT_MODULE_SOURCE_INDEX_PREEMPT_INPUTS:
      case FIELD_INPUT_MODULE_SOURCE_INDEX_PREEMPT_CONTROLS:
      default:
      {
        return FIELD_INPUT_MODULE_TIMEOUT_INPUTS_MS;
      }
  }
}

static uint8_t ModuleSourceMaskFromIndex(uint8_t sourceIndex)
{
  switch (sourceIndex)
  {
      case FIELD_INPUT_MODULE_SOURCE_INDEX_PREEMPT_INPUTS:
      {
        return MODULE_BUS_SNAPSHOT_VALID_PREEMPT_INPUTS;
      }

      case FIELD_INPUT_MODULE_SOURCE_INDEX_PREEMPT_CONTROLS:
      {
        return MODULE_BUS_SNAPSHOT_VALID_PREEMPT_CONTROLS;
      }

      case FIELD_INPUT_MODULE_SOURCE_INDEX_MMU_STATUS:
      {
        return MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS;
      }

      default:
      {
        return 0U;
      }
  }
}

static void UpdateSourceState(ModuleBusSnapshot_t *snapshot,
                              uint8_t sourceMask,
                              uint8_t sourceHealthy)
{
  if (snapshot == NULL)
  {
    return;
  }

  snapshot->validMask |= sourceMask;
  snapshot->staleMask &= (uint8_t) ~sourceMask;

  if (sourceHealthy != 0U)
  {
    snapshot->healthMask |= sourceMask;
  }
  else
  {
    snapshot->healthMask &= (uint8_t) ~sourceMask;
  }
}

static void UpdateSourceFaultState(ModuleBusSnapshot_t *snapshot,
                                   uint8_t sourceMask,
                                   uint8_t contextFault,
                                   uint8_t sequenceFault)
{
  if (snapshot == NULL)
  {
    return;
  }

  if (contextFault != 0U)
  {
    snapshot->contextFaultMask |= sourceMask;
  }
  else
  {
    snapshot->contextFaultMask &= (uint8_t) ~sourceMask;
  }

  if (sequenceFault != 0U)
  {
    snapshot->sequenceFaultMask |= sourceMask;
  }
  else
  {
    snapshot->sequenceFaultMask &= (uint8_t) ~sourceMask;
  }
}

static uint8_t ModulePayloadLength(uint16_t identifier)
{
  switch (identifier)
  {
      case FIELD_INPUT_MODULE_PREEMPT_INPUT_ID:
      case FIELD_INPUT_MODULE_PREEMPT_CONTROL_ID:
      case FIELD_INPUT_MODULE_MMU_STATUS_ID:
      {
        return 7U;
      }

      default:
      {
        return 0U;
      }
  }
}

static uint8_t ResolveModuleSourceDescriptor(uint16_t identifier,
                                             uint8_t *sourceIndex,
                                             uint8_t *sourceMask,
                                             uint8_t *expectedType)
{
  if ((sourceIndex == NULL) || (sourceMask == NULL) || (expectedType == NULL))
  {
    return 0U;
  }

  switch (identifier)
  {
      case FIELD_INPUT_MODULE_PREEMPT_INPUT_ID:
      {
        *sourceIndex = FIELD_INPUT_MODULE_SOURCE_INDEX_PREEMPT_INPUTS;
        *expectedType = FIELD_INPUT_MODULE_TYPE_PREEMPT_INPUTS;
        break;
      }

      case FIELD_INPUT_MODULE_PREEMPT_CONTROL_ID:
      {
        *sourceIndex = FIELD_INPUT_MODULE_SOURCE_INDEX_PREEMPT_CONTROLS;
        *expectedType = FIELD_INPUT_MODULE_TYPE_PREEMPT_CONTROLS;
        break;
      }

      case FIELD_INPUT_MODULE_MMU_STATUS_ID:
      {
        *sourceIndex = FIELD_INPUT_MODULE_SOURCE_INDEX_MMU_STATUS;
        *expectedType = FIELD_INPUT_MODULE_TYPE_MMU_STATUS;
        break;
      }

      default:
      {
        return 0U;
      }
  }

  *sourceMask = ModuleSourceMaskFromIndex(*sourceIndex);

  return (uint8_t) (*sourceMask != 0U);
}

static uint8_t FrameMatchesContext(const FieldBusRxAdapterCtx_t *ctx,
                                   const uint8_t *data,
                                   uint8_t expectedType,
                                   uint8_t payloadLength)
{
  uint16_t epoch;

  if ((ctx == NULL) || (data == NULL) || (payloadLength < 5U))
  {
    return 0U;
  }

  if ((data[0] != MODULE_BUS_PROTOCOL_VERSION) || (data[1] != expectedType))
  {
    return 0U;
  }

  epoch = ReadLe16(&data[2]);
  if ((ctx->configEpoch != 0U) && (epoch != ctx->configEpoch))
  {
    return 0U;
  }

  return 1U;
}

static uint8_t SequenceAdvanced(const FieldBusRxAdapterCtx_t *ctx,
                                uint8_t sourceIndex,
                                uint8_t sequence)
{
  uint8_t seenMask;

  if ((ctx == NULL) || (sourceIndex >= FIELD_INPUT_MODULE_SOURCE_COUNT))
  {
    return 0U;
  }

  seenMask = (uint8_t) (1U << sourceIndex);
  if ((ctx->moduleSequenceSeenMask & seenMask) == 0U)
  {
    return 1U;
  }

  return (uint8_t) (ctx->moduleLastSequence[sourceIndex] != sequence);
}

static void RememberSequence(FieldBusRxAdapterCtx_t *ctx,
                             uint8_t sourceIndex,
                             uint8_t sequence)
{
  uint8_t seenMask;

  if ((ctx == NULL) || (sourceIndex >= FIELD_INPUT_MODULE_SOURCE_COUNT))
  {
    return;
  }

  seenMask = (uint8_t) (1U << sourceIndex);
  ctx->moduleLastSequence[sourceIndex] = sequence;
  ctx->moduleSequenceSeenMask |= seenMask;
}

static void PublishModuleSnapshotUpdate(FieldBusRxAdapterCtx_t *ctx,
                                        uint16_t identifier,
                                        const uint8_t *data,
                                        uint8_t payloadLength)
{
  ModuleBusSnapshot_t nextSnapshot;
  uint8_t nextIndex;
  uint8_t sourceHealthy = 0U;
  uint8_t sourceIndex = 0U;
  uint8_t sourceMask = 0U;
  uint8_t expectedType = 0U;
  uint8_t frameMatchesContext;
  uint8_t sequenceFresh = 0U;
  uint8_t contextFault;
  uint8_t sequenceFault = 0U;
  uint16_t frameEpoch;

  if ((ctx == NULL) || (data == NULL))
  {
    return;
  }

  if (ResolveModuleSourceDescriptor(identifier,
                                    &sourceIndex,
                                    &sourceMask,
                                    &expectedType) == 0U)
  {
    return;
  }

  nextIndex = (uint8_t) (ctx->activeSnapshotIndex ^ 1U);
  nextSnapshot = ctx->snapshots[ctx->activeSnapshotIndex];
  nextSnapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  frameEpoch = ReadLe16(&data[2]);
  nextSnapshot.configEpoch = (ctx->configEpoch != 0U) ? ctx->configEpoch
                           : frameEpoch;
  nextSnapshot.sequence = data[4];
  frameMatchesContext =
    (uint8_t) (FrameMatchesContext(ctx, data, expectedType, payloadLength) != 0U);
  frameMatchesContext = (uint8_t) (frameMatchesContext
                                   && (payloadLength
                                       >= ModulePayloadLength(identifier)));
  contextFault = (uint8_t) (frameMatchesContext == 0U);

  if (frameMatchesContext != 0U)
  {
    sequenceFresh = SequenceAdvanced(ctx, sourceIndex, data[4]);
    RememberSequence(ctx, sourceIndex, data[4]);
    sequenceFault = (uint8_t) (sequenceFresh == 0U);
  }

  switch (identifier)
  {
      case FIELD_INPUT_MODULE_PREEMPT_INPUT_ID:
      {
        if ((frameMatchesContext == 0U) || (sequenceFresh == 0U))
        {
          break;
        }

        nextSnapshot.preemptInputs = data[5];
        sourceHealthy = SourceStatusIsHealthy(data[6]);
        break;
      }

      case FIELD_INPUT_MODULE_PREEMPT_CONTROL_ID:
      {
        if ((frameMatchesContext == 0U) || (sequenceFresh == 0U))
        {
          break;
        }

        nextSnapshot.preemptControls = data[5];
        sourceHealthy = SourceStatusIsHealthy(data[6]);
        break;
      }

      case FIELD_INPUT_MODULE_MMU_STATUS_ID:
      {
        if ((frameMatchesContext == 0U) || (sequenceFresh == 0U))
        {
          break;
        }

        nextSnapshot.mmuStatus = data[5];
        sourceHealthy = SourceStatusIsHealthy(data[6]);
        break;
      }

      default:
      {
        return;
      }
  }

  UpdateSourceFaultState(&nextSnapshot, sourceMask, contextFault, sequenceFault);
  UpdateSourceState(&nextSnapshot, sourceMask, sourceHealthy);
  ctx->moduleLastRxTick[sourceIndex] = CurrentTickMs();
  ctx->snapshots[nextIndex] = nextSnapshot;
  ctx->activeSnapshotIndex = nextIndex;
  ctx->hasSnapshot = 1U;
}

static void DecodeSsmTelemetry(FieldBusRxAdapterCtx_t *ctx,
                               uint8_t ssmIndex,
                               const uint8_t *data,
                               uint8_t length)
{
  if ((ctx == NULL) || (data == NULL) || (ssmIndex >= 8U) || (length < 2U))
  {
    return;
  }

  ctx->ssmVoltagePresence[ssmIndex] =
    (uint16_t) ((uint16_t) data[0] | (((uint16_t) data[1] & 0x0FU) << 8U));
  ctx->ssmLastTick[ssmIndex] = CurrentTickMs();
}

static uint8_t OutputFunctionIsSignalColor(uint8_t function)
{
  return (uint8_t) ((function
                     == (uint8_t) INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_RED)
                    || (function
                        == (uint8_t)
                        INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_YELLOW)
                    || (function
                        == (uint8_t)
                        INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_GREEN));
}

static uint8_t ChannelNeedsLoadSwitchFeedback(const IntersectionConfig_t *config,
                                              uint8_t channelNumber)
{
  if ((config == NULL) || (channelNumber == 0U)
      || (channelNumber > INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  return (uint8_t) (config->channels[channelNumber - 1U].controlSource != 0U);
}

static void RebuildLoadSwitchFeedback(FieldBusRxAdapterCtx_t *ctx,
                                      ModuleBusSnapshot_t *snapshot,
                                      uint32_t now)
{
  const IntersectionConfig_t *config;
  uint16_t requiredSsmMask = 0U;
  uint16_t seenSsmMask = 0U;
  uint16_t freshSsmMask = 0U;
  uint8_t outputRowIndex;

  if ((ctx == NULL) || (snapshot == NULL))
  {
    return;
  }

  snapshot->loadSwitchReds = 0U;
  snapshot->loadSwitchYellows = 0U;
  snapshot->loadSwitchGreens = 0U;

  if (ctx->configurationService == NULL)
  {
    return;
  }

  config = ConfigurationServiceGetActiveConfig(ctx->configurationService);
  if (config == NULL)
  {
    return;
  }

  for (outputRowIndex = 0U;
       outputRowIndex < INTERSECTION_IO_MAP_MAX_OUTPUTS;
       outputRowIndex++)
  {
    const IntersectionIoOutputMapRowConfig_t *row =
      &config->ioMap.outputs[outputRowIndex];
    uint8_t outputIndex;
    uint8_t ssmIndex;
    uint8_t slot;
    uint32_t channelBit;
    uint8_t fresh;

    if ((row->devicePin == 0U) || (row->devicePin > 96U)
        || (row->functionIndex == 0U)
        || (row->functionIndex > INTERSECTION_CHANNEL_COUNT_MAX)
        || (OutputFunctionIsSignalColor(row->function) == 0U)
        || (ChannelNeedsLoadSwitchFeedback(config, row->functionIndex) == 0U))
    {
      continue;
    }

    outputIndex = (uint8_t) (row->devicePin - 1U);
    ssmIndex = (uint8_t) (outputIndex / 12U);
    slot = (uint8_t) (outputIndex % 12U);
    channelBit = (uint32_t) (1UL << (row->functionIndex - 1U));
    requiredSsmMask |= (uint16_t) (1U << ssmIndex);

    if (ctx->ssmLastTick[ssmIndex] == 0U)
    {
      continue;
    }

    seenSsmMask |= (uint16_t) (1U << ssmIndex);
    fresh = (uint8_t) ((now - ctx->ssmLastTick[ssmIndex]) <= FIELD_INPUT_SSM_TIMEOUT_MS);
    if (fresh == 0U)
    {
      continue;
    }

    freshSsmMask |= (uint16_t) (1U << ssmIndex);

    if (((ctx->ssmVoltagePresence[ssmIndex] >> slot) & 0x01U) == 0U)
    {
      continue;
    }

    switch ((IntersectionIoMapOutputFunction_t) row->function)
    {
        case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_RED:
        {
          snapshot->loadSwitchReds |= channelBit;
          break;
        }

        case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_YELLOW:
        {
          snapshot->loadSwitchYellows |= channelBit;
          break;
        }

        case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_GREEN:
        {
          snapshot->loadSwitchGreens |= channelBit;
          break;
        }

        default:
        {
          break;
        }
    }
  }

  if (requiredSsmMask == 0U)
  {
    return;
  }

  if (seenSsmMask == requiredSsmMask)
  {
    snapshot->validMask |= MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  }
  else
  {
    snapshot->validMask &= (uint8_t) ~MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  }

  if (freshSsmMask == requiredSsmMask)
  {
    snapshot->healthMask |= MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
    snapshot->staleMask &= (uint8_t) ~MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  }
  else
  {
    snapshot->healthMask &= (uint8_t) ~MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
    snapshot->staleMask |= MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  }
}

static void CreateOsObjects(FieldBusRxAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  if (ctx->rxPool == NULL)
  {
    ctx->rxPool = osMemoryPoolNew(FIELD_INPUT_RX_DEPTH,
                                  sizeof(FieldInputQueuedFrame_t),
                                  NULL);
  }

  if (ctx->rxQueue == NULL)
  {
    ctx->rxQueue = osMessageQueueNew(FIELD_INPUT_RX_DEPTH,
                                     sizeof(FieldInputQueuedFrame_t *),
                                     NULL);
  }
}

static void SeedSnapshot(FieldBusRxAdapterCtx_t *ctx)
{
  uint8_t index;
  uint8_t sourceIndex;

  if (ctx == NULL)
  {
    return;
  }

  for (index = 0U; index < 2U; index++)
  {
    memset(&ctx->snapshots[index], 0, sizeof(ctx->snapshots[index]));
    ctx->snapshots[index].protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
    ctx->snapshots[index].configEpoch = ctx->configEpoch;
  }

  ctx->activeSnapshotIndex = 0U;
  ctx->hasSnapshot = 0U;

  for (sourceIndex = 0U;
       sourceIndex < FIELD_INPUT_MODULE_SOURCE_COUNT;
       sourceIndex++)
  {
    ctx->moduleLastRxTick[sourceIndex] = 0U;
    ctx->moduleLastSequence[sourceIndex] = 0U;
  }

  ctx->moduleSequenceSeenMask = 0U;
}

static void SendStandardCanFrame(uint16_t identifier,
                                 const uint8_t *data,
                                 uint8_t length)
{
  uint8_t payload[8];

  if ((data == NULL) || (length > sizeof(payload)))
  {
    return;
  }

  memset(payload, 0, sizeof(payload));
  memcpy(payload, data, length);
  (void) FieldCanQueueTxSendStandard(identifier, payload, length);
}

static void SendNmtCommand(uint8_t commandSpecifier, uint8_t nodeId)
{
  uint8_t payload[2];

  payload[0] = commandSpecifier;
  payload[1] = nodeId;
  SendStandardCanFrame(LEGACY_FIELD_CAN_ID_LOOP_NMT,
                       payload,
                       sizeof(payload));
}

static void SendSdoWrite1Byte(uint8_t nodeId,
                              uint16_t index,
                              uint8_t subindex,
                              uint8_t value)
{
  uint8_t payload[8] = {
    0x2FU,
    (uint8_t) (index & 0xFFU),
    (uint8_t) ((index >> 8U) & 0xFFU),
    subindex,
    value,
    0U,
    0U,
    0U
  };

  SendStandardCanFrame((uint16_t) (FIELD_INPUT_FEIG_SDO_REQUEST_BASE + nodeId
                                   - 1U),
                       payload,
                       sizeof(payload));
}

static void SendSdoWrite2Byte(uint8_t nodeId,
                              uint16_t index,
                              uint8_t subindex,
                              uint16_t value)
{
  uint8_t payload[8] = {
    0x2BU,
    (uint8_t) (index & 0xFFU),
    (uint8_t) ((index >> 8U) & 0xFFU),
    subindex,
    (uint8_t) (value & 0xFFU),
    (uint8_t) ((value >> 8U) & 0xFFU),
    0U,
    0U
  };

  SendStandardCanFrame((uint16_t) (FIELD_INPUT_FEIG_SDO_REQUEST_BASE + nodeId
                                   - 1U),
                       payload,
                       sizeof(payload));
}

static void SendSdoWrite4Byte(uint8_t nodeId,
                              uint16_t index,
                              uint8_t subindex,
                              uint32_t value)
{
  uint8_t payload[8] = {
    0x23U,
    (uint8_t) (index & 0xFFU),
    (uint8_t) ((index >> 8U) & 0xFFU),
    subindex,
    (uint8_t) (value & 0xFFU),
    (uint8_t) ((value >> 8U) & 0xFFU),
    (uint8_t) ((value >> 16U) & 0xFFU),
    (uint8_t) ((value >> 24U) & 0xFFU)
  };

  SendStandardCanFrame((uint16_t) (FIELD_INPUT_FEIG_SDO_REQUEST_BASE + nodeId
                                   - 1U),
                       payload,
                       sizeof(payload));
}

static void SendSdoRead(uint8_t nodeId,
                        uint16_t index,
                        uint8_t subindex)
{
  uint8_t payload[8] = {
    0x40U,
    (uint8_t) (index & 0xFFU),
    (uint8_t) ((index >> 8U) & 0xFFU),
    subindex,
    0U,
    0U,
    0U,
    0U
  };

  SendStandardCanFrame((uint16_t) (FIELD_INPUT_FEIG_SDO_REQUEST_BASE + nodeId
                                   - 1U),
                       payload,
                       sizeof(payload));
}

static uint8_t NodeIndexToNodeId(uint8_t nodeIndex)
{
  return (uint8_t) (nodeIndex + 1U);
}

static uint8_t GetNodeIndexFromStandardId(uint32_t identifier,
                                          uint16_t base,
                                          uint8_t *nodeIndex)
{
  uint32_t nodeId;

  if ((nodeIndex == NULL) || (identifier < base))
  {
    return 0U;
  }

  nodeId = identifier - base;
  if ((nodeId == 0U) || (nodeId > FIELD_INPUT_FEIG_NODE_COUNT))
  {
    return 0U;
  }

  *nodeIndex = (uint8_t) (nodeId - 1U);

  return 1U;
}

static void DecodeFeigInputByte(uint8_t rawByte,
                                uint8_t *occupancyMask,
                                uint8_t *safeMask)
{
  uint8_t decoded = (uint8_t) (0xFFU - rawByte);

  if (occupancyMask != NULL)
  {
    *occupancyMask = (uint8_t) (decoded & 0x0FU);
  }

  if (safeMask != NULL)
  {
    *safeMask = (uint8_t) ((decoded >> 4U) & 0x0FU);
  }
}

static void RebuildSnapshot(FieldBusRxAdapterCtx_t *ctx)
{
  ModuleBusSnapshot_t nextSnapshot;
  uint8_t nodeIndex;
  uint8_t nextIndex;
  uint8_t moduleSourceIndex;
  uint8_t anyFeigValid = 0U;
  uint8_t anyFeigFresh = 0U;
  uint8_t anyPedValid = 0U;
  uint8_t anyPedFresh = 0U;
  uint8_t diagnosticsValid = 0U;
  uint8_t diagnosticsFresh = 0U;
  uint32_t now;

  if (ctx == NULL)
  {
    return;
  }

  nextSnapshot = ctx->snapshots[ctx->activeSnapshotIndex];
  memset(&nextSnapshot.vehicleDetectorAlarms[0],
         0,
         sizeof(nextSnapshot.vehicleDetectorAlarms));
  memset(&nextSnapshot.vehicleDetectorReportedAlarms[0],
         0,
         sizeof(nextSnapshot.vehicleDetectorReportedAlarms));
  memset(&nextSnapshot.pedestrianDetectorAlarms[0],
         0,
         sizeof(nextSnapshot.pedestrianDetectorAlarms));
  nextSnapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  nextSnapshot.configEpoch = ctx->configEpoch;
  nextSnapshot.validMask &= (uint8_t) ~FIELD_INPUT_DYNAMIC_SOURCE_MASK;
  nextSnapshot.healthMask &= (uint8_t) ~FIELD_INPUT_DYNAMIC_SOURCE_MASK;
  nextSnapshot.staleMask &= (uint8_t) ~FIELD_INPUT_DYNAMIC_SOURCE_MASK;
  nextSnapshot.contextFaultMask &= (uint8_t) ~FIELD_INPUT_DYNAMIC_SOURCE_MASK;
  nextSnapshot.sequenceFaultMask &= (uint8_t) ~FIELD_INPUT_DYNAMIC_SOURCE_MASK;
  nextSnapshot.detectorInputs = 0U;
  nextSnapshot.pedInputs = 0U;
  nextSnapshot.rawVehicleDetectorInputs = 0U;
  nextSnapshot.rawVehicleDetectorOffline = 0U;
  nextSnapshot.rawVehicleDetectorFault = 0U;
  nextSnapshot.rawPedestrianInputs = 0U;
  now = CurrentTickMs();

  for (nodeIndex = 0U; nodeIndex < FIELD_INPUT_FEIG_NODE_COUNT; nodeIndex++)
  {
    uint8_t loopIndex;
    uint8_t nodeFresh = 0U;
    uint8_t nodeSeen =
      (uint8_t) ((ctx->feigLastStatusTick[nodeIndex] != 0U)
                 || (ctx->feigLastHeartbeatTick[nodeIndex] != 0U));

    if (nodeSeen != 0U)
    {
      anyFeigValid = 1U;
      nodeFresh = (uint8_t) (((now - ctx->feigLastStatusTick[nodeIndex])
                              <= FIELD_INPUT_FEIG_TIMEOUT_MS)
                             || ((now - ctx->feigLastHeartbeatTick[nodeIndex])
                                 <= FIELD_INPUT_FEIG_TIMEOUT_MS));
      if (nodeFresh != 0U)
      {
        anyFeigFresh = 1U;
      }
    }

    diagnosticsValid = (uint8_t) (diagnosticsValid
                                  || (ctx->feigLastStatusTick[nodeIndex] != 0U)
                                  || (ctx->feigLastHeartbeatTick[nodeIndex] != 0U));
    diagnosticsFresh = (uint8_t) (diagnosticsFresh || (nodeFresh != 0U));

    for (loopIndex = 0U; loopIndex < FIELD_INPUT_FEIG_LOOPS_PER_NODE;
         loopIndex++)
    {
      uint8_t detectorNumber =
        (uint8_t) ((nodeIndex * FIELD_INPUT_FEIG_LOOPS_PER_NODE) + loopIndex
                   + 1U);
      uint32_t mask = (uint32_t) (1UL << (uint32_t) (detectorNumber - 1U));

      if (nodeFresh != 0U)
      {
        if ((ctx->feigOccupancy[nodeIndex] & (uint8_t) (1U << loopIndex)) != 0U)
        {
          nextSnapshot.rawVehicleDetectorInputs |= mask;
          nextSnapshot.detectorInputs |= mask;
        }

        if ((ctx->feigOffline[nodeIndex] & (uint8_t) (1U << loopIndex)) != 0U)
        {
          nextSnapshot.rawVehicleDetectorOffline |= mask;
          nextSnapshot.vehicleDetectorReportedAlarms[detectorNumber - 1U] |=
            FIELD_INPUT_REPORTED_ALARM_OTHER;
        }

        if ((ctx->feigFault[nodeIndex] & (uint8_t) (1U << loopIndex)) != 0U)
        {
          nextSnapshot.rawVehicleDetectorFault |= mask;
          nextSnapshot.vehicleDetectorReportedAlarms[detectorNumber - 1U] |=
            FIELD_INPUT_REPORTED_ALARM_OTHER;
        }
      }
      else if (nodeSeen != 0U)
      {
        nextSnapshot.vehicleDetectorAlarms[detectorNumber - 1U] |=
          FIELD_INPUT_DETECTOR_ALARM_COMMUNICATIONS;
      }
    }
  }

  for (nodeIndex = 0U; nodeIndex < 2U; nodeIndex++)
  {
    uint8_t pedIndex;
    uint8_t moduleSeen = (uint8_t) (ctx->pedLegacyLastTick[nodeIndex] != 0U);
    uint8_t moduleFresh = 0U;

    if (moduleSeen != 0U)
    {
      anyPedValid = 1U;
      moduleFresh = (uint8_t) ((now - ctx->pedLegacyLastTick[nodeIndex])
                               <= FIELD_INPUT_PED_TIMEOUT_MS);
      if (moduleFresh != 0U)
      {
        anyPedFresh = 1U;
      }
    }

    diagnosticsValid = (uint8_t) (diagnosticsValid || (moduleSeen != 0U));
    diagnosticsFresh = (uint8_t) (diagnosticsFresh || (moduleFresh != 0U));

    for (pedIndex = 0U; pedIndex < 16U; pedIndex++)
    {
      uint8_t detectorNumber = (uint8_t) ((nodeIndex * 16U) + pedIndex + 1U);
      uint32_t mask = (uint32_t) (1UL << (uint32_t) (detectorNumber - 1U));

      if (moduleFresh != 0U)
      {
        if ((ctx->pedLegacyActive[nodeIndex] & (uint16_t) (1U << pedIndex))
            != 0U)
        {
          nextSnapshot.rawPedestrianInputs |= mask;
          nextSnapshot.pedInputs |= mask;
        }
      }
      else if (moduleSeen != 0U)
      {
        nextSnapshot.pedestrianDetectorAlarms[detectorNumber - 1U] |=
          FIELD_INPUT_DETECTOR_ALARM_COMMUNICATIONS;
      }
    }
  }

  if (anyFeigValid != 0U)
  {
    nextSnapshot.validMask |= MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
    nextSnapshot.healthMask |= MODULE_BUS_SNAPSHOT_VALID_DETECTORS;

    if (anyFeigFresh == 0U)
    {
      nextSnapshot.staleMask |= MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
    }
  }

  if (anyPedValid != 0U)
  {
    nextSnapshot.validMask |= MODULE_BUS_SNAPSHOT_VALID_PEDS;
    nextSnapshot.healthMask |= MODULE_BUS_SNAPSHOT_VALID_PEDS;

    if (anyPedFresh == 0U)
    {
      nextSnapshot.staleMask |= MODULE_BUS_SNAPSHOT_VALID_PEDS;
    }
  }

  if (diagnosticsValid != 0U)
  {
    nextSnapshot.validMask |= MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS;
    nextSnapshot.healthMask |= MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS;

    if (diagnosticsFresh == 0U)
    {
      nextSnapshot.staleMask |= MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS;
    }
  }

  RebuildLoadSwitchFeedback(ctx, &nextSnapshot, now);

  for (moduleSourceIndex = 0U;
       moduleSourceIndex < FIELD_INPUT_MODULE_SOURCE_COUNT;
       moduleSourceIndex++)
  {
    uint8_t sourceMask = ModuleSourceMaskFromIndex(moduleSourceIndex);

    if ((sourceMask == 0U)
        || ((nextSnapshot.validMask & sourceMask) == 0U))
    {
      continue;
    }

    if ((ctx->moduleLastRxTick[moduleSourceIndex] == 0U)
        || ((now - ctx->moduleLastRxTick[moduleSourceIndex])
            > ModuleSourceTimeoutMs(moduleSourceIndex)))
    {
      nextSnapshot.staleMask |= sourceMask;
    }
    else
    {
      nextSnapshot.staleMask &= (uint8_t) ~sourceMask;
    }
  }

  nextIndex = (uint8_t) (ctx->activeSnapshotIndex ^ 1U);
  ctx->snapshots[nextIndex] = nextSnapshot;
  ctx->activeSnapshotIndex = nextIndex;
  ctx->hasSnapshot = 1U;
}

static uint8_t SnapshotRead(void *ctx, ModuleBusSnapshot_t *snapshot)
{
  FieldBusRxAdapterCtx_t *adapterCtx = (FieldBusRxAdapterCtx_t *) ctx;
  uint8_t beforeIndex;
  uint8_t afterIndex;

  if ((adapterCtx == NULL) || (snapshot == NULL)
      || (adapterCtx->hasSnapshot == 0U))
  {
    return 0U;
  }

  do
  {
    beforeIndex = adapterCtx->activeSnapshotIndex;
    *snapshot = adapterCtx->snapshots[beforeIndex];
    afterIndex = adapterCtx->activeSnapshotIndex;
  } while (beforeIndex != afterIndex);

  return 1U;
}

static uint8_t SetConfigEpoch(void *ctx, uint16_t configEpoch)
{
  FieldBusRxAdapterSetConfigEpoch((FieldBusRxAdapterCtx_t *) ctx, configEpoch);

  return 1U;
}

static uint8_t CommandDetectorReset(void *ctx,
                                    ModuleBusDetectorClass_t detectorClass,
                                    uint8_t detectorNumber)
{
  FieldBusRxAdapterCtx_t *adapterCtx = (FieldBusRxAdapterCtx_t *) ctx;
  uint8_t nodeIndex;

  if ((adapterCtx == NULL) || (detectorNumber == 0U))
  {
    return 0U;
  }

  if (detectorClass == MODULE_BUS_DETECTOR_CLASS_VEHICLE)
  {
    if (detectorNumber > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
    {
      return 0U;
    }

    nodeIndex = (uint8_t) ((detectorNumber - 1U) / FIELD_INPUT_FEIG_LOOPS_PER_NODE);
    SendNmtCommand(0x81U, NodeIndexToNodeId(nodeIndex));
    adapterCtx->feigStartupStage[nodeIndex] = 0U;
    adapterCtx->feigNextStartupTick[nodeIndex] =
      CurrentTickMs() + FIELD_INPUT_FEIG_STARTUP_INTERVAL_MS;

    return 1U;
  }

  return 0U;
}

static void HandleFeigStartup(FieldBusRxAdapterCtx_t *ctx,
                              uint8_t nodeIndex,
                              uint32_t now)
{
  uint8_t nodeId;

  if ((ctx == NULL) || (nodeIndex >= FIELD_INPUT_FEIG_NODE_COUNT)
      || (now < ctx->feigNextStartupTick[nodeIndex]))
  {
    return;
  }

  nodeId = NodeIndexToNodeId(nodeIndex);

  switch (ctx->feigStartupStage[nodeIndex])
  {
      case 0U:
      {
        SendSdoWrite4Byte(nodeId, 0x1014U, 0U, (uint32_t) (0x300U + nodeId));
        break;
      }

      case 1U:
      {
        SendSdoWrite2Byte(nodeId, 0x1017U, 0U, 1000U);
        break;
      }

      case 2U:
      {
        SendSdoWrite1Byte(nodeId, 0x1800U, 2U, 255U);
        break;
      }

      case 3U:
      {
        SendSdoWrite2Byte(nodeId, 0x1800U, 3U, 0U);
        break;
      }

      case 4U:
      {
        SendSdoWrite2Byte(nodeId, 0x1800U, 5U, 0U);
        break;
      }

      case 5U:
      {
        SendNmtCommand(0x01U, nodeId);
        break;
      }

      default:
      {
        return;
      }
  }

  ctx->feigStartupStage[nodeIndex]++;
  ctx->feigNextStartupTick[nodeIndex] = now + FIELD_INPUT_FEIG_STARTUP_INTERVAL_MS;
}

static void PollFeigDiagnostics(FieldBusRxAdapterCtx_t *ctx, uint32_t now)
{
  uint8_t nodeId;
  uint16_t objectIndex;

  if ((ctx == NULL) || (now < ctx->nextHealthPollTick))
  {
    return;
  }

  nodeId = NodeIndexToNodeId(ctx->nextHealthPollNode);
  objectIndex = (uint16_t) (0x2101U + ctx->nextHealthPollLoop);
  SendSdoRead(nodeId, objectIndex, ctx->nextHealthPollSubindex);

  if (ctx->nextHealthPollSubindex == 4U)
  {
    ctx->nextHealthPollSubindex = 5U;
  }
  else
  {
    ctx->nextHealthPollSubindex = 4U;
    ctx->nextHealthPollLoop++;
    if (ctx->nextHealthPollLoop >= FIELD_INPUT_FEIG_LOOPS_PER_NODE)
    {
      ctx->nextHealthPollLoop = 0U;
      ctx->nextHealthPollNode++;
      if (ctx->nextHealthPollNode >= FIELD_INPUT_FEIG_NODE_COUNT)
      {
        ctx->nextHealthPollNode = 0U;
      }
    }
  }

  ctx->nextHealthPollTick = now + FIELD_INPUT_FEIG_HEALTH_POLL_INTERVAL_MS;
}

void FieldBusRxAdapterOnRxIsr(const FDCAN_RxHeaderTypeDef *header,
                              const uint8_t *data)
{
  FieldBusRxAdapterCtx_t *ctx = s_registeredCtx;
  FieldInputQueuedFrame_t *queuedFrame;

  if ((ctx == NULL) || (header == NULL) || (data == NULL)
      || (header->IdType != FDCAN_STANDARD_ID)
      || (header->RxFrameType != FDCAN_DATA_FRAME)
      || (FieldBusRxAdapterOwnsStandardId((uint16_t) header->Identifier) == 0U))
  {
    return;
  }

  if ((ctx->rxPool == NULL) || (ctx->rxQueue == NULL))
  {
    ctx->droppedFrames++;
    return;
  }

  queuedFrame = (FieldInputQueuedFrame_t *) osMemoryPoolAlloc(ctx->rxPool, 0U);
  if (queuedFrame == NULL)
  {
    ctx->droppedFrames++;
    return;
  }

  queuedFrame->standardId = (uint16_t) header->Identifier;
  queuedFrame->length = DlcToLength(header->DataLength);
  if (queuedFrame->length > sizeof(queuedFrame->data))
  {
    queuedFrame->length = sizeof(queuedFrame->data);
  }

  (void) memcpy(&queuedFrame->data[0], data, queuedFrame->length);

  if (osMessageQueuePut(ctx->rxQueue, &queuedFrame, 0U, 0U) != osOK)
  {
    (void) osMemoryPoolFree(ctx->rxPool, queuedFrame);
    ctx->droppedFrames++;
  }
}

void FieldBusRxAdapterInit(FieldBusRxAdapterCtx_t *ctx,
                           uint16_t configEpoch,
                           UiPowerService_t *powerService,
                           ConfigurationService_t *configurationService)
{
  if (ctx == NULL)
  {
    return;
  }

  memset(ctx, 0, sizeof(*ctx));
  CreateOsObjects(ctx);
  ctx->configEpoch = configEpoch;
  ctx->powerService = powerService;
  ctx->configurationService = configurationService;
  ctx->nextHealthPollSubindex = 4U;
  SeedSnapshot(ctx);
  s_registeredCtx = ctx;
}

void FieldBusRxAdapterSetConfigEpoch(FieldBusRxAdapterCtx_t *ctx,
                                     uint16_t configEpoch)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->configEpoch = configEpoch;
  SeedSnapshot(ctx);
}

IModuleBusPort_t FieldBusRxAdapterCreatePort(FieldBusRxAdapterCtx_t *ctx)
{
  IModuleBusPort_t port;

  port.ctx = ctx;
  port.ReadSnapshot = SnapshotRead;
  port.SetConfigEpoch = SetConfigEpoch;
  port.CommandDetectorReset = CommandDetectorReset;

  return port;
}

uint8_t FieldBusRxAdapterOwnsStandardId(uint16_t standardId)
{
  if (((standardId >= FIELD_INPUT_SSM_MEASURE_BASE)
       && (standardId < (uint16_t) (FIELD_INPUT_SSM_MEASURE_BASE + 8U)))
      || ((standardId >= FIELD_INPUT_PSM_MEASURE_BASE)
       && (standardId < (uint16_t) (FIELD_INPUT_PSM_MEASURE_BASE + 2U)))
      || (standardId == FIELD_INPUT_PED_LEGACY_BASE)
      || (standardId == (uint16_t) (FIELD_INPUT_PED_LEGACY_BASE + 1U))
      || ((standardId >= (uint16_t) (FIELD_INPUT_FEIG_TPDO1_BASE + 1U))
          && (standardId <= (uint16_t) (FIELD_INPUT_FEIG_TPDO1_BASE
                                        + FIELD_INPUT_FEIG_NODE_COUNT)))
      || ((standardId >= (uint16_t) (FIELD_INPUT_FEIG_SDO_RESPONSE_BASE + 1U))
          && (standardId <= (uint16_t) (FIELD_INPUT_FEIG_SDO_RESPONSE_BASE
                                        + FIELD_INPUT_FEIG_NODE_COUNT)))
      || ((standardId >= (uint16_t) (FIELD_INPUT_FEIG_HEARTBEAT_BASE + 1U))
          && (standardId <= (uint16_t) (FIELD_INPUT_FEIG_HEARTBEAT_BASE
                                        + FIELD_INPUT_FEIG_NODE_COUNT)))
      || ((standardId >= (uint16_t) (FIELD_INPUT_FEIG_EMCY_BASE + 1U))
          && (standardId <= (uint16_t) (FIELD_INPUT_FEIG_EMCY_BASE
                                        + FIELD_INPUT_FEIG_NODE_COUNT)))
      || (standardId == FIELD_INPUT_MODULE_PREEMPT_INPUT_ID)
      || (standardId == FIELD_INPUT_MODULE_PREEMPT_CONTROL_ID)
      || (standardId == FIELD_INPUT_MODULE_MMU_STATUS_ID))
  {
    return 1U;
  }

  return 0U;
}

static void ProcessRxFrame(FieldBusRxAdapterCtx_t *ctx,
                           uint16_t standardId,
                           uint8_t length,
                           const uint8_t *data)
{
  uint32_t now;

  if ((ctx == NULL) || (data == NULL))
  {
    return;
  }

  now = CurrentTickMs();

  if ((standardId >= FIELD_INPUT_SSM_MEASURE_BASE)
      && (standardId < (uint16_t) (FIELD_INPUT_SSM_MEASURE_BASE + 8U)))
  {
    DecodeSsmTelemetry(ctx,
                       (uint8_t) (standardId - FIELD_INPUT_SSM_MEASURE_BASE),
                       data,
                       length);
    RebuildSnapshot(ctx);
    return;
  }

  if ((standardId == FIELD_INPUT_MODULE_PREEMPT_INPUT_ID)
      || (standardId == FIELD_INPUT_MODULE_PREEMPT_CONTROL_ID)
      || (standardId == FIELD_INPUT_MODULE_MMU_STATUS_ID))
  {
    PublishModuleSnapshotUpdate(ctx, standardId, data, length);
    return;
  }

  if ((standardId >= FIELD_INPUT_PSM_MEASURE_BASE)
      && (standardId < (uint16_t) (FIELD_INPUT_PSM_MEASURE_BASE + 2U))
      && (length >= 8U) && (ctx->powerService != NULL))
  {
    UiPowerMeasurement_t measurement;
    uint8_t psmNumber = (uint8_t) ((standardId - FIELD_INPUT_PSM_MEASURE_BASE)
                                   + 1U);

    (void) memset(&measurement, 0, sizeof(measurement));
    measurement.netVoltageRaw =
      ReadPsmField(data[0], (uint8_t) (data[5] & 0x03U));
    measurement.voltage24v1Raw =
      ReadPsmField(data[1], (uint8_t) ((data[5] >> 2U) & 0x03U));
    measurement.voltage24v2Raw =
      ReadPsmField(data[2], (uint8_t) ((data[5] >> 4U) & 0x03U));
    measurement.voltage5v1Raw =
      ReadPsmField(data[3], (uint8_t) ((data[5] >> 6U) & 0x03U));
    measurement.voltage5v2Raw =
      ReadPsmField(data[4], (uint8_t) (data[6] & 0x03U));
    measurement.isolatedVoltagePresent =
      (uint8_t) ((data[6] & 0x04U) != 0U);
    measurement.netFrequencyRaw = data[7];
    measurement.valid = 1U;
    UiPowerServiceUpdateMeasurement(ctx->powerService, psmNumber, &measurement);
    return;
  }

  if ((standardId == FIELD_INPUT_PED_LEGACY_BASE)
      || (standardId == (FIELD_INPUT_PED_LEGACY_BASE + 1U)))
  {
    uint8_t moduleIndex = (uint8_t) (standardId - FIELD_INPUT_PED_LEGACY_BASE);

    if (length >= 2U)
    {
      ctx->pedLegacyActive[moduleIndex] = (uint16_t) (~ReadLe16(data));
      ctx->pedLegacyLastTick[moduleIndex] = now;
      RebuildSnapshot(ctx);
    }

    return;
  }

  if ((standardId >= FIELD_INPUT_FEIG_TPDO1_BASE + 1U)
      && (standardId <= FIELD_INPUT_FEIG_TPDO1_BASE
          + FIELD_INPUT_FEIG_NODE_COUNT))
  {
    uint8_t nodeIndex;

    if ((length >= 1U)
        && (GetNodeIndexFromStandardId(standardId,
                                       FIELD_INPUT_FEIG_TPDO1_BASE,
                                       &nodeIndex) != 0U))
    {
      DecodeFeigInputByte(data[0],
                          &ctx->feigOccupancy[nodeIndex],
                          NULL);
      ctx->feigLastStatusTick[nodeIndex] = now;
      RebuildSnapshot(ctx);
    }

    return;
  }

  if ((standardId >= FIELD_INPUT_FEIG_SDO_RESPONSE_BASE + 1U)
      && (standardId <= FIELD_INPUT_FEIG_SDO_RESPONSE_BASE
          + FIELD_INPUT_FEIG_NODE_COUNT))
  {
    uint8_t nodeIndex;

    if (GetNodeIndexFromStandardId(standardId,
                                   FIELD_INPUT_FEIG_SDO_RESPONSE_BASE,
                                   &nodeIndex) == 0U)
    {
      return;
    }

    if ((length >= 5U)
        && (ReadLe16(&data[1]) == 0x6000U)
        && (data[3] == 1U))
    {
      DecodeFeigInputByte(data[4], &ctx->feigOccupancy[nodeIndex], NULL);
      ctx->feigLastStatusTick[nodeIndex] = now;
      RebuildSnapshot(ctx);
      return;
    }

    if ((length >= 5U)
        && (ReadLe16(&data[1]) >= 0x2101U)
        && (ReadLe16(&data[1]) <= 0x2104U)
        && ((data[3] == 4U) || (data[3] == 5U)))
    {
      uint8_t loopIndex = (uint8_t) (ReadLe16(&data[1]) - 0x2101U);
      uint8_t loopMask = (uint8_t) (1U << loopIndex);

      if (data[3] == 4U)
      {
        if (data[4] != 0U)
        {
          ctx->feigOffline[nodeIndex] |= loopMask;
        }
        else
        {
          ctx->feigOffline[nodeIndex] &= (uint8_t) ~loopMask;
        }
      }
      else
      {
        if (data[4] != 0U)
        {
          ctx->feigFault[nodeIndex] |= loopMask;
        }
        else
        {
          ctx->feigFault[nodeIndex] &= (uint8_t) ~loopMask;
        }
      }

      ctx->feigLastHeartbeatTick[nodeIndex] = now;
      RebuildSnapshot(ctx);
    }

    return;
  }

  if ((standardId >= FIELD_INPUT_FEIG_HEARTBEAT_BASE + 1U)
      && (standardId <= FIELD_INPUT_FEIG_HEARTBEAT_BASE
          + FIELD_INPUT_FEIG_NODE_COUNT))
  {
    uint8_t nodeIndex;

    if ((length >= 1U)
        && (GetNodeIndexFromStandardId(standardId,
                                       FIELD_INPUT_FEIG_HEARTBEAT_BASE,
                                       &nodeIndex) != 0U))
    {
      ctx->feigLastHeartbeatTick[nodeIndex] = now;
      if (data[0] == 0x00U)
      {
        ctx->feigStartupStage[nodeIndex] = 0U;
        ctx->feigNextStartupTick[nodeIndex] = now + 10U;
      }
      RebuildSnapshot(ctx);
    }

    return;
  }

  if ((standardId >= FIELD_INPUT_FEIG_EMCY_BASE + 1U)
      && (standardId <= FIELD_INPUT_FEIG_EMCY_BASE
          + FIELD_INPUT_FEIG_NODE_COUNT))
  {
    uint8_t nodeIndex;

    if (GetNodeIndexFromStandardId(standardId,
                                   FIELD_INPUT_FEIG_EMCY_BASE,
                                   &nodeIndex) != 0U)
    {
      ctx->feigLastHeartbeatTick[nodeIndex] = now;
      RebuildSnapshot(ctx);
    }
  }
}

void FieldBusRxAdapterStep(void)
{
  FieldBusRxAdapterCtx_t *ctx = s_registeredCtx;
  FieldInputQueuedFrame_t *queuedFrame = NULL;
  uint8_t nodeIndex;
  uint32_t now;

  if (ctx == NULL)
  {
    return;
  }

  while ((ctx->rxQueue != NULL)
         && (osMessageQueueGet(ctx->rxQueue, &queuedFrame, NULL, 0U) == osOK))
  {
    if (queuedFrame != NULL)
    {
      ProcessRxFrame(ctx,
                     queuedFrame->standardId,
                     queuedFrame->length,
                     &queuedFrame->data[0]);
      (void) osMemoryPoolFree(ctx->rxPool, queuedFrame);
    }
  }

  now = CurrentTickMs();

  for (nodeIndex = 0U; nodeIndex < FIELD_INPUT_FEIG_NODE_COUNT; nodeIndex++)
  {
    HandleFeigStartup(ctx, nodeIndex, now);
  }

  PollFeigDiagnostics(ctx, now);
  RebuildSnapshot(ctx);
}
