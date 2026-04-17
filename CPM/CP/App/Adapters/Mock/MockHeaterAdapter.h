/* App/Adapters/Mock/MockHeaterAdapter.h
 *
 * IHeaterPort in-memory test double.
 */
#ifndef MOCK_HEATER_ADAPTER_H
#define MOCK_HEATER_ADAPTER_H

#include "Ports/IHeaterPort.h"

typedef struct
{
  uint8_t heaterOn;
  uint32_t enableCount;
  uint32_t disableCount;
} MockHeaterAdapterCtx_t;

void MockHeaterAdapterInit(MockHeaterAdapterCtx_t *ctx);
IHeaterPort_t MockHeaterAdapterCreatePort(MockHeaterAdapterCtx_t *ctx);

#endif /* MOCK_HEATER_ADAPTER_H */
