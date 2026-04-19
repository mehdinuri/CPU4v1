/* App/Adapters/STM32/FieldInputCanAdapter.h
 *
 * FDCAN1-backed field input adapter for FEIG loop detector modules and
 * pedestrian input modules.
 */
#ifndef FIELD_INPUT_CAN_ADAPTER_H
#define FIELD_INPUT_CAN_ADAPTER_H

#include <stdint.h>

#include "cmsis_os2.h"
#include "Domain/Services/UiPowerService.h"
#include "fdcan.h"
#include "Ports/IModuleBusPort.h"

typedef struct
{
  ModuleBusSnapshot_t snapshots[2];
  uint32_t feigLastStatusTick[INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX / 4U];
  uint32_t feigLastHeartbeatTick[INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX / 4U];
  uint32_t feigNextStartupTick[INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX / 4U];
  uint8_t feigOccupancy[INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX / 4U];
  uint8_t feigOffline[INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX / 4U];
  uint8_t feigFault[INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX / 4U];
  uint8_t feigStartupStage[INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX / 4U];
  uint32_t pedLegacyLastTick[2];
  uint16_t pedLegacyActive[2];
  uint32_t nextHealthPollTick;
  uint8_t nextHealthPollNode;
  uint8_t nextHealthPollLoop;
  uint8_t nextHealthPollSubindex;
  uint8_t activeSnapshotIndex;
  uint8_t hasSnapshot;
  uint16_t configEpoch;
  osMemoryPoolId_t rxPool;
  osMessageQueueId_t rxQueue;
  uint32_t droppedFrames;
  UiPowerService_t *powerService;
} FieldInputCanAdapterCtx_t;

void FieldInputCanAdapterInit(FieldInputCanAdapterCtx_t *ctx,
                              uint16_t configEpoch,
                              UiPowerService_t *powerService);
void FieldInputCanAdapterSetConfigEpoch(FieldInputCanAdapterCtx_t *ctx,
                                        uint16_t configEpoch);
IModuleBusPort_t FieldInputCanAdapterCreatePort(FieldInputCanAdapterCtx_t *ctx);
void FieldInputCanAdapterOnRxIsr(const FDCAN_RxHeaderTypeDef *header,
                                 const uint8_t *data);
void FieldInputCanAdapterStep(void);

#endif /* FIELD_INPUT_CAN_ADAPTER_H */
