/* App/Adapters/STM32/FieldBusRxAdapter.h
 *
 * FDCAN1-backed field-bus Rx adapter for live field-bus inputs plus the
 * module-snapshot families that have been re-homed onto that bus.
 */
#ifndef FIELD_BUS_RX_ADAPTER_H
#define FIELD_BUS_RX_ADAPTER_H

#include <stdint.h>

#include "cmsis_os2.h"
#include "Domain/Intersection/ConfigurationService.h"
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
  uint32_t ssmLastTick[8];
  uint16_t ssmVoltagePresence[8];
  uint32_t moduleLastRxTick[4];
  uint8_t moduleLastSequence[4];
  uint8_t moduleSequenceSeenMask;
  uint8_t activeSnapshotIndex;
  uint8_t hasSnapshot;
  uint16_t configEpoch;
  osMemoryPoolId_t rxPool;
  osMessageQueueId_t rxQueue;
  uint32_t droppedFrames;
  UiPowerService_t *powerService;
  ConfigurationService_t *configurationService;
} FieldBusRxAdapterCtx_t;

void FieldBusRxAdapterInit(FieldBusRxAdapterCtx_t *ctx,
                           uint16_t configEpoch,
                           UiPowerService_t *powerService,
                           ConfigurationService_t *configurationService);
void FieldBusRxAdapterSetConfigEpoch(FieldBusRxAdapterCtx_t *ctx,
                                     uint16_t configEpoch);
IModuleBusPort_t FieldBusRxAdapterCreatePort(FieldBusRxAdapterCtx_t *ctx);
uint8_t FieldBusRxAdapterOwnsStandardId(uint16_t standardId);
void FieldBusRxAdapterOnRxIsr(const FDCAN_RxHeaderTypeDef *header,
                              const uint8_t *data);
void FieldBusRxAdapterStep(void);

#endif /* FIELD_BUS_RX_ADAPTER_H */
