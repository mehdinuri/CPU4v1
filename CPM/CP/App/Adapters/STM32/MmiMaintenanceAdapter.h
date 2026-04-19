/* App/Adapters/STM32/MmiMaintenanceAdapter.h */
#ifndef MMI_MAINTENANCE_ADAPTER_H
#define MMI_MAINTENANCE_ADAPTER_H

#include "Ports/IMmiMaintenancePort.h"

typedef struct
{
  uint8_t selectedOutputNumber;
  uint8_t outputTestActive;
} MmiMaintenanceAdapterCtx_t;

void MmiMaintenanceAdapterInit(MmiMaintenanceAdapterCtx_t *ctx);
IMmiMaintenancePort_t MmiMaintenanceAdapterCreatePort(
  MmiMaintenanceAdapterCtx_t *ctx);

#endif /* MMI_MAINTENANCE_ADAPTER_H */
