/* App/Adapters/STM32/SafetyRelayAdapter.h
 *
 * ISafetyRelayPort implementation driving the SSM power relay on PA10
 * and the AC triac latch on PC13. The adapter maintains a shadow of
 * the last commanded state so the domain can detect feedback mismatch
 * (future: add a sense pin).
 */
#ifndef SAFETY_RELAY_ADAPTER_H
#define SAFETY_RELAY_ADAPTER_H

#include "Ports/ISafetyRelayPort.h"

typedef struct
{
  SafetyRelayState_t commandedState;
} SafetyRelayAdapterCtx_t;

void SafetyRelayAdapterInit(SafetyRelayAdapterCtx_t *ctx);
ISafetyRelayPort_t SafetyRelayAdapterCreatePort(SafetyRelayAdapterCtx_t *ctx);

#endif /* SAFETY_RELAY_ADAPTER_H */
