/* App/Adapters/STM32/ModuleBusAdapter.h
 *
 * FDCAN2-backed adapter that aggregates detector, ped, preempt, and MMU
 * input frames into a latest-snapshot port for the domain controller.
 */
#ifndef MODULE_BUS_ADAPTER_H
#define MODULE_BUS_ADAPTER_H

#include <stdint.h>

#include "fdcan.h"
#include "Ports/IModuleBusPort.h"

typedef struct
{
  FDCAN_HandleTypeDef *hfdcan;
  ModuleBusSnapshot_t snapshots[2];
  uint32_t lastRxTick[7];
  uint8_t lastSequence[7];
  uint8_t sequenceSeenMask;
  uint8_t activeSnapshotIndex;
  uint8_t started;
  uint8_t hasSnapshot;
  uint16_t configEpoch;
} ModuleBusAdapterCtx_t;

void ModuleBusAdapterInit(ModuleBusAdapterCtx_t *ctx,
                          FDCAN_HandleTypeDef *hfdcan,
                          uint16_t configEpoch);
void ModuleBusAdapterSetConfigEpoch(ModuleBusAdapterCtx_t *ctx,
                                    uint16_t configEpoch);
IModuleBusPort_t ModuleBusAdapterCreatePort(ModuleBusAdapterCtx_t *ctx);
void ModuleBusAdapterHandleRxFifo0Interrupt(FDCAN_HandleTypeDef *hfdcan);

#endif /* MODULE_BUS_ADAPTER_H */
