/* App/Adapters/STM32/ModuleBusAdapter.c
 *
 * Aggregates FDCAN2 detector/ped/preempt/MMU/load-switch frames into a stable
 * latest snapshot for the domain controller.
 */
#include "ModuleBusAdapter.h"

#include <string.h>

#include "stm32h7xx_hal.h"

#define MODULE_BUS_MESSAGE_ID_DETECTOR_SNAPSHOT         0x510U
#define MODULE_BUS_MESSAGE_ID_PED_SNAPSHOT              0x511U
#define MODULE_BUS_MESSAGE_ID_PREEMPT_INPUT_SNAPSHOT    0x512U
#define MODULE_BUS_MESSAGE_ID_PREEMPT_CONTROL_SNAPSHOT  0x513U
#define MODULE_BUS_MESSAGE_ID_MMU_STATUS                0x514U
#define MODULE_BUS_MESSAGE_ID_LOAD_SWITCH_FEEDBACK      0x515U
#define MODULE_BUS_MESSAGE_ID_DETECTOR_DIAGNOSTICS      0x516U
#define MODULE_BUS_MESSAGE_ID_DETECTOR_RESET_COMMAND    0x517U

#define MODULE_BUS_MESSAGE_TYPE_DETECTORS               1U
#define MODULE_BUS_MESSAGE_TYPE_PEDS                    2U
#define MODULE_BUS_MESSAGE_TYPE_PREEMPT_INPUTS          3U
#define MODULE_BUS_MESSAGE_TYPE_PREEMPT_CONTROLS        4U
#define MODULE_BUS_MESSAGE_TYPE_MMU_STATUS              5U
#define MODULE_BUS_MESSAGE_TYPE_LOAD_SWITCH_FEEDBACK    6U
#define MODULE_BUS_MESSAGE_TYPE_DETECTOR_DIAGNOSTICS    7U
#define MODULE_BUS_MESSAGE_TYPE_DETECTOR_RESET_COMMAND  8U

#define MODULE_BUS_SOURCE_INDEX_DETECTORS               0U
#define MODULE_BUS_SOURCE_INDEX_PEDS                    1U
#define MODULE_BUS_SOURCE_INDEX_PREEMPT_INPUTS          2U
#define MODULE_BUS_SOURCE_INDEX_PREEMPT_CONTROLS        3U
#define MODULE_BUS_SOURCE_INDEX_MMU_STATUS              4U
#define MODULE_BUS_SOURCE_INDEX_LOAD_SWITCH             5U
#define MODULE_BUS_SOURCE_INDEX_DETECTOR_DIAGNOSTICS    6U
#define MODULE_BUS_SOURCE_COUNT                         7U

#define MODULE_BUS_TIMEOUT_INPUTS_MS                    100U
#define MODULE_BUS_TIMEOUT_CRITICAL_MS                  30U
#define MODULE_BUS_RESET_COMMAND_PAYLOAD_SIZE           8U

static ModuleBusAdapterCtx_t *s_registeredCtx = NULL;

static uint16_t ReadLe16(const uint8_t *data)
{
  return (uint16_t) ((uint16_t) data[0] | ((uint16_t) data[1] << 8U));
}

static uint32_t CurrentTickMs(void)
{
  return HAL_GetTick();
}

static uint8_t SourceStatusIsHealthy(uint8_t status)
{
  return (uint8_t) ((status & MODULE_BUS_SOURCE_STATUS_FAULT) == 0U);
}

static uint32_t SourceTimeoutMs(uint8_t sourceIndex)
{
  switch (sourceIndex)
  {
      case MODULE_BUS_SOURCE_INDEX_MMU_STATUS:
      case MODULE_BUS_SOURCE_INDEX_LOAD_SWITCH:
      {
        return MODULE_BUS_TIMEOUT_CRITICAL_MS;
      }

      case MODULE_BUS_SOURCE_INDEX_DETECTORS:
      case MODULE_BUS_SOURCE_INDEX_PEDS:
      case MODULE_BUS_SOURCE_INDEX_PREEMPT_INPUTS:
      case MODULE_BUS_SOURCE_INDEX_PREEMPT_CONTROLS:
      default:
      {
        return MODULE_BUS_TIMEOUT_INPUTS_MS;
      }
  }
}

static uint8_t SourceMaskFromIndex(uint8_t sourceIndex)
{
  switch (sourceIndex)
  {
      case MODULE_BUS_SOURCE_INDEX_DETECTORS:
      {
        return MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
      }

      case MODULE_BUS_SOURCE_INDEX_PEDS:
      {
        return MODULE_BUS_SNAPSHOT_VALID_PEDS;
      }

      case MODULE_BUS_SOURCE_INDEX_PREEMPT_INPUTS:
      {
        return MODULE_BUS_SNAPSHOT_VALID_PREEMPT_INPUTS;
      }

      case MODULE_BUS_SOURCE_INDEX_PREEMPT_CONTROLS:
      {
        return MODULE_BUS_SNAPSHOT_VALID_PREEMPT_CONTROLS;
      }

      case MODULE_BUS_SOURCE_INDEX_MMU_STATUS:
      {
        return MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS;
      }

      case MODULE_BUS_SOURCE_INDEX_LOAD_SWITCH:
      {
        return MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
      }

      case MODULE_BUS_SOURCE_INDEX_DETECTOR_DIAGNOSTICS:
      {
        return MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS;
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

static uint8_t ResolveSourceDescriptor(uint32_t identifier,
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
      case MODULE_BUS_MESSAGE_ID_DETECTOR_SNAPSHOT:
      {
        *sourceIndex = MODULE_BUS_SOURCE_INDEX_DETECTORS;
        *expectedType = MODULE_BUS_MESSAGE_TYPE_DETECTORS;
        break;
      }

      case MODULE_BUS_MESSAGE_ID_PED_SNAPSHOT:
      {
        *sourceIndex = MODULE_BUS_SOURCE_INDEX_PEDS;
        *expectedType = MODULE_BUS_MESSAGE_TYPE_PEDS;
        break;
      }

      case MODULE_BUS_MESSAGE_ID_PREEMPT_INPUT_SNAPSHOT:
      {
        *sourceIndex = MODULE_BUS_SOURCE_INDEX_PREEMPT_INPUTS;
        *expectedType = MODULE_BUS_MESSAGE_TYPE_PREEMPT_INPUTS;
        break;
      }

      case MODULE_BUS_MESSAGE_ID_PREEMPT_CONTROL_SNAPSHOT:
      {
        *sourceIndex = MODULE_BUS_SOURCE_INDEX_PREEMPT_CONTROLS;
        *expectedType = MODULE_BUS_MESSAGE_TYPE_PREEMPT_CONTROLS;
        break;
      }

      case MODULE_BUS_MESSAGE_ID_MMU_STATUS:
      {
        *sourceIndex = MODULE_BUS_SOURCE_INDEX_MMU_STATUS;
        *expectedType = MODULE_BUS_MESSAGE_TYPE_MMU_STATUS;
        break;
      }

      case MODULE_BUS_MESSAGE_ID_LOAD_SWITCH_FEEDBACK:
      {
        *sourceIndex = MODULE_BUS_SOURCE_INDEX_LOAD_SWITCH;
        *expectedType = MODULE_BUS_MESSAGE_TYPE_LOAD_SWITCH_FEEDBACK;
        break;
      }

      case MODULE_BUS_MESSAGE_ID_DETECTOR_DIAGNOSTICS:
      {
        *sourceIndex = MODULE_BUS_SOURCE_INDEX_DETECTOR_DIAGNOSTICS;
        *expectedType = MODULE_BUS_MESSAGE_TYPE_DETECTOR_DIAGNOSTICS;
        break;
      }

      default:
      {
        return 0U;
      }
  }

  *sourceMask = SourceMaskFromIndex(*sourceIndex);

  return (uint8_t) (*sourceMask != 0U);
}

static uint8_t EnsureStarted(ModuleBusAdapterCtx_t *ctx)
{
  HAL_FDCAN_StateTypeDef state;

  if ((ctx == NULL) || (ctx->hfdcan == NULL))
  {
    return 0U;
  }

  if (ctx->started != 0U)
  {
    return 1U;
  }

  state = HAL_FDCAN_GetState(ctx->hfdcan);
  if (state == HAL_FDCAN_STATE_READY)
  {
    if (HAL_FDCAN_Start(ctx->hfdcan) != HAL_OK)
    {
      return 0U;
    }
  }
  else if (state != HAL_FDCAN_STATE_BUSY)
  {
    return 0U;
  }

  if (HAL_FDCAN_ActivateNotification(ctx->hfdcan,
                                     FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                     0U)
      != HAL_OK)
  {
    return 0U;
  }

  ctx->started = 1U;

  return 1U;
}

static void SeedSnapshot(ModuleBusAdapterCtx_t *ctx)
{
  uint8_t index;
  uint8_t sourceIndex;

  if (ctx == NULL)
  {
    return;
  }

  for (index = 0U; index < 2U; ++index)
  {
    memset(&ctx->snapshots[index], 0, sizeof(ctx->snapshots[index]));
    ctx->snapshots[index].protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
    ctx->snapshots[index].configEpoch = ctx->configEpoch;
  }

  ctx->activeSnapshotIndex = 0U;
  ctx->hasSnapshot = 0U;

  for (sourceIndex = 0U; sourceIndex < MODULE_BUS_SOURCE_COUNT; ++sourceIndex)
  {
    ctx->lastRxTick[sourceIndex] = 0U;
    ctx->lastSequence[sourceIndex] = 0U;
  }

  ctx->sequenceSeenMask = 0U;
}

static uint8_t SnapshotRead(void *ctx, ModuleBusSnapshot_t *snapshot)
{
  ModuleBusAdapterCtx_t *adapterCtx = (ModuleBusAdapterCtx_t *) ctx;
  uint8_t beforeIndex;
  uint8_t afterIndex;
  uint8_t sourceIndex;
  static const uint8_t sourceMasks[MODULE_BUS_SOURCE_COUNT] = {
    MODULE_BUS_SNAPSHOT_VALID_DETECTORS,
    MODULE_BUS_SNAPSHOT_VALID_PEDS,
    MODULE_BUS_SNAPSHOT_VALID_PREEMPT_INPUTS,
    MODULE_BUS_SNAPSHOT_VALID_PREEMPT_CONTROLS,
    MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS,
    MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH,
    MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS
  };
  uint32_t currentTick;

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

  currentTick = CurrentTickMs();

  for (sourceIndex = 0U; sourceIndex < MODULE_BUS_SOURCE_COUNT; ++sourceIndex)
  {
    if (((snapshot->validMask & sourceMasks[sourceIndex]) != 0U)
        && ((currentTick - adapterCtx->lastRxTick[sourceIndex])
            > SourceTimeoutMs(sourceIndex)))
    {
      snapshot->staleMask |= sourceMasks[sourceIndex];
    }
  }

  return 1U;
} /* SnapshotRead */

static uint8_t CommandDetectorReset(void *ctx,
                                    ModuleBusDetectorClass_t detectorClass,
                                    uint8_t detectorNumber)
{
  ModuleBusAdapterCtx_t *adapterCtx = (ModuleBusAdapterCtx_t *) ctx;
  FDCAN_TxHeaderTypeDef txHeader;
  uint8_t payload[MODULE_BUS_RESET_COMMAND_PAYLOAD_SIZE];

  if ((adapterCtx == NULL) || (detectorNumber == 0U))
  {
    return 0U;
  }

  if ((detectorClass == MODULE_BUS_DETECTOR_CLASS_VEHICLE)
      && (detectorNumber > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  if ((detectorClass == MODULE_BUS_DETECTOR_CLASS_PEDESTRIAN)
      && (detectorNumber > INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  if ((detectorClass != MODULE_BUS_DETECTOR_CLASS_VEHICLE)
      && (detectorClass != MODULE_BUS_DETECTOR_CLASS_PEDESTRIAN))
  {
    return 0U;
  }

  if ((EnsureStarted(adapterCtx) == 0U)
      || (HAL_FDCAN_GetTxFifoFreeLevel(adapterCtx->hfdcan) == 0U))
  {
    return 0U;
  }

  memset(&payload[0], 0, sizeof(payload));
  payload[0] = MODULE_BUS_PROTOCOL_VERSION;
  payload[1] = MODULE_BUS_MESSAGE_TYPE_DETECTOR_RESET_COMMAND;
  payload[2] = (uint8_t) (adapterCtx->configEpoch & 0xFFU);
  payload[3] = (uint8_t) ((adapterCtx->configEpoch >> 8U) & 0xFFU);
  payload[4] = (uint8_t) detectorClass;
  payload[5] = detectorNumber;
  payload[6] = 1U;
  payload[7] = 0U;

  memset(&txHeader, 0, sizeof(txHeader));
  txHeader.Identifier = MODULE_BUS_MESSAGE_ID_DETECTOR_RESET_COMMAND;
  txHeader.IdType = FDCAN_STANDARD_ID;
  txHeader.TxFrameType = FDCAN_DATA_FRAME;
  txHeader.DataLength = FDCAN_DLC_BYTES_8;
  txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  txHeader.BitRateSwitch = FDCAN_BRS_ON;
  txHeader.FDFormat = FDCAN_FD_CAN;
  txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  txHeader.MessageMarker = detectorNumber;

  return (HAL_FDCAN_AddMessageToTxFifoQ(adapterCtx->hfdcan,
                                        &txHeader,
                                        payload) == HAL_OK)
         ? 1U
         : 0U;
}

static uint8_t FrameMatchesContext(const ModuleBusAdapterCtx_t *ctx,
                                   const uint8_t *data,
                                   uint8_t expectedType)
{
  uint16_t epoch;

  if ((ctx == NULL) || (data == NULL))
  {
    return 0U;
  }

  if (data[0] != MODULE_BUS_PROTOCOL_VERSION)
  {
    return 0U;
  }

  if (data[1] != expectedType)
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

static uint8_t SequenceAdvanced(const ModuleBusAdapterCtx_t *ctx,
                                uint8_t sourceIndex,
                                uint8_t sequence)
{
  uint8_t seenMask;

  if (ctx == NULL)
  {
    return 0U;
  }

  seenMask = (uint8_t) (1U << sourceIndex);
  if ((ctx->sequenceSeenMask & seenMask) == 0U)
  {
    return 1U;
  }

  return (uint8_t) (ctx->lastSequence[sourceIndex] != sequence);
}

static void RememberSequence(ModuleBusAdapterCtx_t *ctx,
                             uint8_t sourceIndex,
                             uint8_t sequence)
{
  uint8_t seenMask;

  if (ctx == NULL)
  {
    return;
  }

  seenMask = (uint8_t) (1U << sourceIndex);
  ctx->lastSequence[sourceIndex] = sequence;
  ctx->sequenceSeenMask |= seenMask;
}

static void PublishUpdatedSnapshot(ModuleBusAdapterCtx_t *ctx,
                                   uint32_t identifier,
                                   const uint8_t *data)
{
  ModuleBusSnapshot_t nextSnapshot;
  uint8_t nextIndex;
  uint8_t frameMatchesContext;
  uint8_t sequenceFresh;
  uint8_t contextFault;
  uint8_t sequenceFault;
  uint8_t sourceHealthy = 0U;
  uint8_t sourceIndex = 0U;
  uint8_t sourceMask = 0U;
  uint8_t expectedType = 0U;
  uint16_t frameEpoch;

  if ((ctx == NULL) || (data == NULL))
  {
    return;
  }

  if (ResolveSourceDescriptor(identifier,
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
  frameMatchesContext = FrameMatchesContext(ctx, data, expectedType);
  sequenceFresh = 0U;
  contextFault = (uint8_t) (frameMatchesContext == 0U);
  sequenceFault = 0U;

  if (frameMatchesContext != 0U)
  {
    sequenceFresh = SequenceAdvanced(ctx, sourceIndex, data[4]);
    RememberSequence(ctx, sourceIndex, data[4]);
    sequenceFault = (uint8_t) (sequenceFresh == 0U);
  }

  switch (identifier)
  {
      case MODULE_BUS_MESSAGE_ID_DETECTOR_SNAPSHOT:
      {
        if ((frameMatchesContext == 0U) || (sequenceFresh == 0U))
        {
          break;
        }

        nextSnapshot.detectorInputs = ReadLe16(&data[5]);
        sourceHealthy = SourceStatusIsHealthy(data[7]);
        break;
      }

      case MODULE_BUS_MESSAGE_ID_PED_SNAPSHOT:
      {
        if ((frameMatchesContext == 0U) || (sequenceFresh == 0U))
        {
          break;
        }

        nextSnapshot.pedInputs = data[5];
        sourceHealthy = SourceStatusIsHealthy(data[6]);
        break;
      }

      case MODULE_BUS_MESSAGE_ID_PREEMPT_INPUT_SNAPSHOT:
      {
        if ((frameMatchesContext == 0U) || (sequenceFresh == 0U))
        {
          break;
        }

        nextSnapshot.preemptInputs = data[5];
        sourceHealthy = SourceStatusIsHealthy(data[6]);
        break;
      }

      case MODULE_BUS_MESSAGE_ID_PREEMPT_CONTROL_SNAPSHOT:
      {
        if ((frameMatchesContext == 0U) || (sequenceFresh == 0U))
        {
          break;
        }

        nextSnapshot.preemptControls = data[5];
        sourceHealthy = SourceStatusIsHealthy(data[6]);
        break;
      }

      case MODULE_BUS_MESSAGE_ID_MMU_STATUS:
      {
        if ((frameMatchesContext == 0U) || (sequenceFresh == 0U))
        {
          break;
        }

        nextSnapshot.mmuStatus = data[5];
        sourceHealthy = SourceStatusIsHealthy(data[6]);
        break;
      }

      case MODULE_BUS_MESSAGE_ID_LOAD_SWITCH_FEEDBACK:
      {
        if ((frameMatchesContext == 0U) || (sequenceFresh == 0U))
        {
          break;
        }

        nextSnapshot.loadSwitchReds = ReadLe16(&data[6]);
        nextSnapshot.loadSwitchYellows = ReadLe16(&data[8]);
        nextSnapshot.loadSwitchGreens = ReadLe16(&data[10]);
        sourceHealthy = SourceStatusIsHealthy(data[5]);
        break;
      }

      case MODULE_BUS_MESSAGE_ID_DETECTOR_DIAGNOSTICS:
      {
        if ((frameMatchesContext == 0U) || (sequenceFresh == 0U))
        {
          break;
        }

        memcpy(&nextSnapshot.vehicleDetectorAlarms[0],
               &data[5],
               INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX);
        memcpy(&nextSnapshot.vehicleDetectorReportedAlarms[0],
               &data[5U + INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX],
               INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX);
        memcpy(&nextSnapshot.pedestrianDetectorAlarms[0],
               &data[5U + (2U * INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)],
               INTERSECTION_PED_INPUT_COUNT_MAX);
        sourceHealthy = SourceStatusIsHealthy(
          data[5U + (2U * INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
               + INTERSECTION_PED_INPUT_COUNT_MAX]);
        break;
      }

      default:
      {
        return;
      }
  } /* switch */

  UpdateSourceFaultState(&nextSnapshot, sourceMask, contextFault, sequenceFault);
  UpdateSourceState(&nextSnapshot, sourceMask, sourceHealthy);
  ctx->lastRxTick[sourceIndex] = CurrentTickMs();
  ctx->snapshots[nextIndex] = nextSnapshot;
  ctx->activeSnapshotIndex = nextIndex;
  ctx->hasSnapshot = 1U;
} /* PublishUpdatedSnapshot */

void ModuleBusAdapterInit(ModuleBusAdapterCtx_t *ctx,
                          FDCAN_HandleTypeDef *hfdcan,
                          uint16_t configEpoch)
{
  if (ctx == NULL)
  {
    return;
  }

  memset(ctx, 0, sizeof(*ctx));
  ctx->hfdcan = hfdcan;
  ctx->configEpoch = configEpoch;
  SeedSnapshot(ctx);
  s_registeredCtx = ctx;

  (void) EnsureStarted(ctx);
}

void ModuleBusAdapterSetConfigEpoch(ModuleBusAdapterCtx_t *ctx,
                                    uint16_t configEpoch)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->configEpoch = configEpoch;
  SeedSnapshot(ctx);
}

static uint8_t SetConfigEpoch(void *ctx, uint16_t configEpoch)
{
  ModuleBusAdapterSetConfigEpoch((ModuleBusAdapterCtx_t *) ctx, configEpoch);

  return 1U;
}

IModuleBusPort_t ModuleBusAdapterCreatePort(ModuleBusAdapterCtx_t *ctx)
{
  IModuleBusPort_t port;

  port.ctx = ctx;
  port.ReadSnapshot = SnapshotRead;
  port.SetConfigEpoch = SetConfigEpoch;
  port.CommandDetectorReset = CommandDetectorReset;

  return port;
}

void ModuleBusAdapterHandleRxFifo0Interrupt(FDCAN_HandleTypeDef *hfdcan)
{
  FDCAN_RxHeaderTypeDef header;
  uint8_t data[64];

  if ((s_registeredCtx == NULL) || (hfdcan == NULL)
      || (s_registeredCtx->hfdcan != hfdcan))
  {
    return;
  }

  memset(&header, 0, sizeof(header));
  memset(data, 0, sizeof(data));

  if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &header, data) != HAL_OK)
  {
    return;
  }

  if ((header.IdType != FDCAN_STANDARD_ID)
      || (header.RxFrameType != FDCAN_DATA_FRAME))
  {
    return;
  }

  PublishUpdatedSnapshot(s_registeredCtx, header.Identifier, data);
}
