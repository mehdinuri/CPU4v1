/* App/Adapters/STM32/HeaterAdapter.h
 *
 * IHeaterPort concrete implementation.
 * Drives HEAT_Pin (PE10, active high) directly via HAL.
 * Init leaves the heater disabled for safety.
 */
#ifndef HEATER_ADAPTER_H
#define HEATER_ADAPTER_H

#include "Ports/IHeaterPort.h"

typedef struct
{
  uint8_t enabled;
} HeaterAdapterCtx_t;

/* Disable heater and initialise context. */
void HeaterAdapterInit(HeaterAdapterCtx_t *ctx);

/* Build an IHeaterPort_t wired to ctx. */
IHeaterPort_t HeaterAdapterCreatePort(HeaterAdapterCtx_t *ctx);

#endif /* HEATER_ADAPTER_H */
