/* App/Adapters/STM32/ControlBusAdapter.h
 *
 * IControlBusPort driving FDCAN1, the dedicated CP <-> MP back-
 * channel. Transport-level only: outgoing extended-ID frames via
 * HAL_FDCAN_AddMessageToTxFifoQ, incoming frames fanned out through
 * the registered RxCallback.
 *
 * Packet reassembly, state machine, and CP ack semantics are
 * implemented by a higher-level service (future: CpMpTransport) on
 * top of this port, mirroring CP's cpmpcomm.c packet state machine.
 */
#ifndef CONTROL_BUS_ADAPTER_H
#define CONTROL_BUS_ADAPTER_H

#include "Ports/IControlBusPort.h"

#include "fdcan.h"

typedef struct
{
  FDCAN_HandleTypeDef *hfdcan;
  ControlBusRxCallback_t rxCallback;
  void *rxCallbackCtx;
  uint32_t txErrors;
} ControlBusAdapterCtx_t;

void ControlBusAdapterInit(ControlBusAdapterCtx_t *ctx,
                           FDCAN_HandleTypeDef *hfdcan);
IControlBusPort_t ControlBusAdapterCreatePort(ControlBusAdapterCtx_t *ctx);
void ControlBusAdapterOnRxIsr(ControlBusAdapterCtx_t *ctx,
                              const FDCAN_RxHeaderTypeDef *header,
                              const uint8_t *data);

#endif /* CONTROL_BUS_ADAPTER_H */
