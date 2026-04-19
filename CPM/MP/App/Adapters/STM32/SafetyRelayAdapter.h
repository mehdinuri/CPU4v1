/* App/Adapters/STM32/SafetyRelayAdapter.h */

#ifndef SAFETY_RELAY_ADAPTER_H
#define SAFETY_RELAY_ADAPTER_H

#include <stdint.h>

#include "Ports/ISafetyRelayPort.h"

typedef enum
{
  SAFETY_RELAY_TOPOLOGY_LEGACY_ACTIVE_HIGH_CLOSE = 0U,
  SAFETY_RELAY_TOPOLOGY_ECO_ACTIVE_HIGH_TRIP = 1U
} SafetyRelayTopology_t;

typedef struct
{
  SafetyRelayState_t commandedState;
  SafetyRelayTopology_t topology;
  uint8_t lastRelayDrive;
  uint8_t lastTriacDrive;
} SafetyRelayAdapterCtx_t;

void SafetyRelayAdapterInit(SafetyRelayAdapterCtx_t *ctx);
void SafetyRelayAdapterSetTopology(SafetyRelayAdapterCtx_t *ctx,
                                   SafetyRelayTopology_t topology);
uint8_t SafetyRelayAdapterGetLastRelayDrive(const SafetyRelayAdapterCtx_t *ctx);
uint8_t SafetyRelayAdapterGetLastTriacDrive(const SafetyRelayAdapterCtx_t *ctx);
ISafetyRelayPort_t SafetyRelayAdapterCreatePort(SafetyRelayAdapterCtx_t *ctx);

#endif /* SAFETY_RELAY_ADAPTER_H */
