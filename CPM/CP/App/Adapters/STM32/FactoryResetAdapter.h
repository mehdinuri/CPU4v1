/* App/Adapters/STM32/FactoryResetAdapter.h */
#ifndef FACTORY_RESET_ADAPTER_H
#define FACTORY_RESET_ADAPTER_H

#include "Ports/IFactoryResetPort.h"

typedef struct
{
  uint8_t reserved;
} FactoryResetAdapterCtx_t;

void FactoryResetAdapterInit(FactoryResetAdapterCtx_t *ctx);
IFactoryResetPort_t FactoryResetAdapterCreatePort(FactoryResetAdapterCtx_t *ctx);

#endif /* FACTORY_RESET_ADAPTER_H */
