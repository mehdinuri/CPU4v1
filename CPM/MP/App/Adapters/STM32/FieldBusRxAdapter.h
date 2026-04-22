/* App/Adapters/STM32/FieldBusRxAdapter.h
 *
 * IFieldBusPort implementation: FDCAN1 observer for the shared field
 * bus. Decodes load-switch commands (0x040/0x041), per-SSM voltage /
 * current telemetry (0x050..0x057), and PSM telemetry (0x05A/0x05B)
 * into a double-buffered FieldBusSnapshot_t that the MalfunctionEngine
 * pulls each tick.
 *
 * RX is ISR-driven via ControlBusAdapterOnRxIsr-style entry points
 * the bootstrap registers with the FDCAN IRQ handler.
 */
#ifndef FIELD_BUS_RX_ADAPTER_H
#define FIELD_BUS_RX_ADAPTER_H

#include "cmsis_os2.h"
#include "Ports/IFieldBusPort.h"

#include "fdcan.h"

typedef struct
{
  FDCAN_HandleTypeDef *hfdcan;
  osMemoryPoolId_t rxPool;
  osMessageQueueId_t rxQueue;
  FieldBusSnapshot_t write;
  FieldBusSnapshot_t read;
  uint32_t droppedFrames;
} FieldBusRxAdapterCtx_t;

void FieldBusRxAdapterInit(FieldBusRxAdapterCtx_t *ctx,
                           FDCAN_HandleTypeDef *hfdcan);
IFieldBusPort_t FieldBusRxAdapterCreatePort(FieldBusRxAdapterCtx_t *ctx);
void FieldBusRxAdapterOnRxIsr(FieldBusRxAdapterCtx_t *ctx,
                              const FDCAN_RxHeaderTypeDef *header,
                              const uint8_t *data);
void FieldBusRxAdapterStep(FieldBusRxAdapterCtx_t *ctx);
void FieldBusRxAdapterCommit(FieldBusRxAdapterCtx_t *ctx, uint32_t nowTicks);

#endif /* FIELD_BUS_RX_ADAPTER_H */
