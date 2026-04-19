/* App/Adapters/Mock/MockSafetyRelayAdapter.h
 *
 * In-memory safety relay adapter for host unit tests. Records commanded
 * state and the number of state transitions so tests can assert on
 * whether SafetyDecisionService drove the relay as expected.
 */
#ifndef MOCK_SAFETY_RELAY_ADAPTER_H
#define MOCK_SAFETY_RELAY_ADAPTER_H

#include <stdint.h>

#include "Adapters/STM32/SafetyRelayAdapter.h"
#include "Ports/ISafetyRelayPort.h"

typedef struct
{
  SafetyRelayState_t commandedState;
  SafetyRelayState_t actualState;
  SafetyRelayTopology_t topology;
  uint8_t lastRelayDrive;
  uint8_t lastTriacDrive;
  uint32_t transitionCount;
} MockSafetyRelayAdapterCtx_t;

void MockSafetyRelayAdapterInit(MockSafetyRelayAdapterCtx_t *ctx);
void MockSafetyRelayAdapterSetTopology(MockSafetyRelayAdapterCtx_t *ctx,
                                       SafetyRelayTopology_t topology);
ISafetyRelayPort_t MockSafetyRelayAdapterCreatePort(
  MockSafetyRelayAdapterCtx_t *ctx);

#endif /* MOCK_SAFETY_RELAY_ADAPTER_H */
