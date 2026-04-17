/* App/Adapters/STM32/UnitInputAdapter.h
 *
 * IUnitInputPort concrete implementation for local cabinet/control inputs.
 */
#ifndef UNIT_INPUT_ADAPTER_H
#define UNIT_INPUT_ADAPTER_H

#include "Ports/IUnitInputPort.h"

typedef struct
{
  uint8_t reserved;
} UnitInputAdapterCtx_t;

void UnitInputAdapterInit(UnitInputAdapterCtx_t *ctx);
IUnitInputPort_t UnitInputAdapterCreatePort(UnitInputAdapterCtx_t *ctx);

#endif /* UNIT_INPUT_ADAPTER_H */
