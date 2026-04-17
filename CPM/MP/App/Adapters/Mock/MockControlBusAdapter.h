/* App/Adapters/Mock/MockControlBusAdapter.h
 *
 * IControlBusPort in-memory test double. Captures outbound frames in a
 * ring buffer and lets tests inject inbound frames through the
 * registered callback.
 */
#ifndef MOCK_CONTROL_BUS_ADAPTER_H
#define MOCK_CONTROL_BUS_ADAPTER_H

#include "Ports/IControlBusPort.h"

#define MOCK_CONTROL_BUS_TX_BUFFER 32U

typedef struct
{
  ControlBusFrame_t txBuffer[MOCK_CONTROL_BUS_TX_BUFFER];
  uint32_t txCount;
  ControlBusRxCallback_t rxCallback;
  void *rxCallbackCtx;
} MockControlBusAdapterCtx_t;

void MockControlBusAdapterInit(MockControlBusAdapterCtx_t *ctx);
IControlBusPort_t MockControlBusAdapterCreatePort(
  MockControlBusAdapterCtx_t *ctx);
void MockControlBusAdapterInjectRxFrame(MockControlBusAdapterCtx_t *ctx,
                                        const ControlBusFrame_t *frame);

#endif /* MOCK_CONTROL_BUS_ADAPTER_H */
