/* App/Adapters/STM32/MmiLegacyStatusAdapter.h */
#ifndef MMI_LEGACY_STATUS_ADAPTER_H
#define MMI_LEGACY_STATUS_ADAPTER_H

#include "Ports/IMmiLegacyStatusPort.h"

typedef struct
{
  uint8_t reserved0;
} MmiLegacyStatusAdapterCtx_t;

void MmiLegacyStatusAdapterInit(MmiLegacyStatusAdapterCtx_t *ctx);
IMmiLegacyStatusPort_t MmiLegacyStatusAdapterCreatePort(
  MmiLegacyStatusAdapterCtx_t *ctx);

#endif /* MMI_LEGACY_STATUS_ADAPTER_H */
