/* App/Adapters/STM32/ControlBusAdapter.h
 *
 * IControlBusPort driving FDCAN2, the dedicated CP <-> MP back-
 * channel. Transport-level only: outgoing standard-ID CAN FD frames via
 * HAL_FDCAN_AddMessageToTxFifoQ, incoming frames fanned out through
 * the registered RxCallback.
 */
#ifndef CONTROL_BUS_ADAPTER_H
#define CONTROL_BUS_ADAPTER_H

#include "cmsis_os2.h"
#include "Ports/IControlBusPort.h"

#include "fdcan.h"

typedef struct
{
  FDCAN_HandleTypeDef *hfdcan;
  ControlBusRxCallback_t rxCallback;
  void *rxCallbackCtx;
  osMemoryPoolId_t rxPool;
  osMemoryPoolId_t txPool;
  osMessageQueueId_t rxQueue;
  osMessageQueueId_t txQueue;
  uint32_t txErrors;
  uint32_t rxDrops;
  uint32_t txDrops;
} ControlBusAdapterCtx_t;

void ControlBusAdapterInit(ControlBusAdapterCtx_t *ctx,
                           FDCAN_HandleTypeDef *hfdcan);
IControlBusPort_t ControlBusAdapterCreatePort(ControlBusAdapterCtx_t *ctx);
void ControlBusAdapterOnRxIsr(ControlBusAdapterCtx_t *ctx,
                              const FDCAN_RxHeaderTypeDef *header,
                              const uint8_t *data);
void ControlBusAdapterStep(ControlBusAdapterCtx_t *ctx);

#endif /* CONTROL_BUS_ADAPTER_H */
