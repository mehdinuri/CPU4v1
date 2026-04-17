/* App/Adapters/Mock/MockRelayAdapter.h
 *
 * IRelayPort in-memory test double.
 */
#ifndef MOCK_RELAY_ADAPTER_H
#define MOCK_RELAY_ADAPTER_H

#include "Ports/IRelayPort.h"

typedef struct
{
  uint8_t relayState;
  uint32_t setCount;
} MockRelayAdapterCtx_t;

void MockRelayAdapterInit(MockRelayAdapterCtx_t *ctx);
IRelayPort_t MockRelayAdapterCreatePort(MockRelayAdapterCtx_t *ctx);

#endif /* MOCK_RELAY_ADAPTER_H */
