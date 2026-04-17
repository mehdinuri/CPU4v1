/* App/Adapters/STM32/RelayAdapter.h
 *
 * IRelayPort concrete implementation for the output relay (PA6).
 * Drives RELAY_Pin (PA6) directly via HAL.
 * Init opens the relay (safe-state = off).
 */
#ifndef RELAY_ADAPTER_H
#define RELAY_ADAPTER_H

#include "Ports/IRelayPort.h"

typedef struct
{
  uint8_t state;
} RelayAdapterCtx_t;

/* Open the relay and initialise context. */
void RelayAdapterInit(RelayAdapterCtx_t *ctx);

/* Build an IRelayPort_t wired to ctx. */
IRelayPort_t RelayAdapterCreatePort(RelayAdapterCtx_t *ctx);

#endif /* RELAY_ADAPTER_H */
