/* App/Adapters/STM32/FieldInputCanAdapter.c
 *
 * FDCAN1 field input adapter for FEIG loop detectors and legacy pedestrian
 * inputs. FEIG occupancy is consumed from TPDO1 when available and from
 * legacy 0x580+node SDO responses as a compatibility fallback. Low-rate
 * 210x.sub4/sub5 polling is retained for per-loop offline/fault detail.
 */
#include "FieldInputCanAdapter.h"

#include <string.h>

#include "CanMsgParser.h"
#include "stm32h7xx_hal.h"

#define FIELD_INPUT_FEIG_NODE_COUNT 8U
#define FIELD_INPUT_FEIG_LOOPS_PER_NODE 4U
#define FIELD_INPUT_FEIG_TPDO1_BASE 0x180U
#define FIELD_INPUT_FEIG_SDO_RESPONSE_BASE 0x580U
#define FIELD_INPUT_FEIG_SDO_REQUEST_BASE CAN_MID_LOOP_DETECTOR_STATUS_REQUEST0
#define FIELD_INPUT_FEIG_HEARTBEAT_BASE 0x700U
#define FIELD_INPUT_FEIG_EMCY_BASE 0x300U
#define FIELD_INPUT_PED_LEGACY_BASE CAN_MID_IO_INPUTS0

#define FIELD_INPUT_FEIG_TIMEOUT_MS 1500U
#define FIELD_INPUT_PED_TIMEOUT_MS 500U
#define FIELD_INPUT_FEIG_STARTUP_INTERVAL_MS 50U
#define FIELD_INPUT_FEIG_HEALTH_POLL_INTERVAL_MS 100U

#define FIELD_INPUT_DETECTOR_ALARM_COMMUNICATIONS 0x08U
#define FIELD_INPUT_REPORTED_ALARM_OTHER 0x01U

static FieldInputCanAdapterCtx_t *s_registeredCtx = NULL;

static uint16_t ReadLe16(const uint8_t *data)
{
  return (uint16_t) ((uint16_t) data[0] | ((uint16_t) data[1] << 8U));
}

static uint32_t CurrentTickMs(void)
{
  return HAL_GetTick();
}

static void SeedSnapshot(FieldInputCanAdapterCtx_t *ctx)
{
  uint8_t index;

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
  CANTxRequest(length, CAN_ID_TYPE_STD, identifier, payload);
}

static void SendNmtCommand(uint8_t commandSpecifier, uint8_t nodeId)
{
  uint8_t payload[2];

  payload[0] = commandSpecifier;
  payload[1] = nodeId;
  SendStandardCanFrame(CAN_MID_LOOP_DEDECTOR_ENTER_OPERATIONAL_MODE,
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

static void RebuildSnapshot(FieldInputCanAdapterCtx_t *ctx)
{
  ModuleBusSnapshot_t nextSnapshot;
  uint8_t nodeIndex;
  uint8_t nextIndex;
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
  nextSnapshot.validMask = 0U;
  nextSnapshot.healthMask = 0U;
  nextSnapshot.staleMask = 0U;
  nextSnapshot.contextFaultMask = 0U;
  nextSnapshot.sequenceFaultMask = 0U;
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

  nextIndex = (uint8_t) (ctx->activeSnapshotIndex ^ 1U);
  ctx->snapshots[nextIndex] = nextSnapshot;
  ctx->activeSnapshotIndex = nextIndex;
  ctx->hasSnapshot = 1U;
}

static uint8_t SnapshotRead(void *ctx, ModuleBusSnapshot_t *snapshot)
{
  FieldInputCanAdapterCtx_t *adapterCtx = (FieldInputCanAdapterCtx_t *) ctx;
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
  FieldInputCanAdapterSetConfigEpoch((FieldInputCanAdapterCtx_t *) ctx,
                                     configEpoch);

  return 1U;
}

static uint8_t CommandDetectorReset(void *ctx,
                                    ModuleBusDetectorClass_t detectorClass,
                                    uint8_t detectorNumber)
{
  FieldInputCanAdapterCtx_t *adapterCtx = (FieldInputCanAdapterCtx_t *) ctx;
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

static void HandleFeigStartup(FieldInputCanAdapterCtx_t *ctx,
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

static void PollFeigDiagnostics(FieldInputCanAdapterCtx_t *ctx, uint32_t now)
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

void FieldInputCanAdapterInit(FieldInputCanAdapterCtx_t *ctx,
                              uint16_t configEpoch)
{
  if (ctx == NULL)
  {
    return;
  }

  memset(ctx, 0, sizeof(*ctx));
  ctx->configEpoch = configEpoch;
  ctx->nextHealthPollSubindex = 4U;
  SeedSnapshot(ctx);
  s_registeredCtx = ctx;
}

void FieldInputCanAdapterSetConfigEpoch(FieldInputCanAdapterCtx_t *ctx,
                                        uint16_t configEpoch)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->configEpoch = configEpoch;
  SeedSnapshot(ctx);
}

IModuleBusPort_t FieldInputCanAdapterCreatePort(FieldInputCanAdapterCtx_t *ctx)
{
  IModuleBusPort_t port;

  port.ctx = ctx;
  port.ReadSnapshot = SnapshotRead;
  port.SetConfigEpoch = SetConfigEpoch;
  port.CommandDetectorReset = CommandDetectorReset;

  return port;
}

void FieldInputCanAdapterHandleRxFrame(const FDCAN_RxHeaderTypeDef *header,
                                       const uint8_t *data)
{
  FieldInputCanAdapterCtx_t *ctx = s_registeredCtx;
  uint32_t now;

  if ((ctx == NULL) || (header == NULL) || (data == NULL)
      || (header->IdType != FDCAN_STANDARD_ID)
      || (header->RxFrameType != FDCAN_DATA_FRAME))
  {
    return;
  }

  now = CurrentTickMs();

  if ((header->Identifier == FIELD_INPUT_PED_LEGACY_BASE)
      || (header->Identifier == (FIELD_INPUT_PED_LEGACY_BASE + 1U)))
  {
    uint8_t moduleIndex = (uint8_t) (header->Identifier - FIELD_INPUT_PED_LEGACY_BASE);

    if (header->DataLength >= 2U)
    {
      ctx->pedLegacyActive[moduleIndex] = (uint16_t) (~ReadLe16(data));
      ctx->pedLegacyLastTick[moduleIndex] = now;
      RebuildSnapshot(ctx);
    }

    return;
  }

  if ((header->Identifier >= FIELD_INPUT_FEIG_TPDO1_BASE + 1U)
      && (header->Identifier <= FIELD_INPUT_FEIG_TPDO1_BASE
          + FIELD_INPUT_FEIG_NODE_COUNT))
  {
    uint8_t nodeIndex;

    if ((header->DataLength >= 1U)
        && (GetNodeIndexFromStandardId(header->Identifier,
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

  if ((header->Identifier >= FIELD_INPUT_FEIG_SDO_RESPONSE_BASE + 1U)
      && (header->Identifier <= FIELD_INPUT_FEIG_SDO_RESPONSE_BASE
          + FIELD_INPUT_FEIG_NODE_COUNT))
  {
    uint8_t nodeIndex;

    if (GetNodeIndexFromStandardId(header->Identifier,
                                   FIELD_INPUT_FEIG_SDO_RESPONSE_BASE,
                                   &nodeIndex) == 0U)
    {
      return;
    }

    if ((header->DataLength >= 5U)
        && (ReadLe16(&data[1]) == 0x6000U)
        && (data[3] == 1U))
    {
      DecodeFeigInputByte(data[4], &ctx->feigOccupancy[nodeIndex], NULL);
      ctx->feigLastStatusTick[nodeIndex] = now;
      RebuildSnapshot(ctx);
      return;
    }

    if ((header->DataLength >= 5U)
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

  if ((header->Identifier >= FIELD_INPUT_FEIG_HEARTBEAT_BASE + 1U)
      && (header->Identifier <= FIELD_INPUT_FEIG_HEARTBEAT_BASE
          + FIELD_INPUT_FEIG_NODE_COUNT))
  {
    uint8_t nodeIndex;

    if ((header->DataLength >= 1U)
        && (GetNodeIndexFromStandardId(header->Identifier,
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

  if ((header->Identifier >= FIELD_INPUT_FEIG_EMCY_BASE + 1U)
      && (header->Identifier <= FIELD_INPUT_FEIG_EMCY_BASE
          + FIELD_INPUT_FEIG_NODE_COUNT))
  {
    uint8_t nodeIndex;

    if (GetNodeIndexFromStandardId(header->Identifier,
                                   FIELD_INPUT_FEIG_EMCY_BASE,
                                   &nodeIndex) != 0U)
    {
      ctx->feigLastHeartbeatTick[nodeIndex] = now;
      RebuildSnapshot(ctx);
    }
  }
}

void FieldInputCanAdapterStep(void)
{
  FieldInputCanAdapterCtx_t *ctx = s_registeredCtx;
  uint8_t nodeIndex;
  uint32_t now;

  if (ctx == NULL)
  {
    return;
  }

  now = CurrentTickMs();

  for (nodeIndex = 0U; nodeIndex < FIELD_INPUT_FEIG_NODE_COUNT; nodeIndex++)
  {
    HandleFeigStartup(ctx, nodeIndex, now);
  }

  PollFeigDiagnostics(ctx, now);
  RebuildSnapshot(ctx);
}
