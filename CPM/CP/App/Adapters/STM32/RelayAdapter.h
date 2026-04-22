/* App/Adapters/STM32/RelayAdapter.h
 *
 * Raw CP-side relay GPIO adapter. The domain maps permit semantics to a raw
 * drive level upstream; this adapter just writes and reads RELAY_Pin.
 */
#ifndef RELAY_ADAPTER_H
#define RELAY_ADAPTER_H

#include "Ports/IRelayPort.h"

typedef struct
{
  uint8_t state;
} RelayAdapterCtx_t;

/* Initialise the raw relay GPIO to the physical safe state. */
void RelayAdapterInit(RelayAdapterCtx_t *ctx);

/* Build an IRelayPort_t wired to ctx. */
IRelayPort_t RelayAdapterCreatePort(RelayAdapterCtx_t *ctx);

#endif /* RELAY_ADAPTER_H */
