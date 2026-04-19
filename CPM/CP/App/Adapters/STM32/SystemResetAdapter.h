/* App/Adapters/STM32/SystemResetAdapter.h */
#ifndef SYSTEM_RESET_ADAPTER_H
#define SYSTEM_RESET_ADAPTER_H

#include "Ports/ISystemResetPort.h"

typedef struct
{
  uint8_t reserved0;
} SystemResetAdapterCtx_t;

void SystemResetAdapterInit(SystemResetAdapterCtx_t *ctx);
ISystemResetPort_t SystemResetAdapterCreatePort(SystemResetAdapterCtx_t *ctx);

#endif /* SYSTEM_RESET_ADAPTER_H */
