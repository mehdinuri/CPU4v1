/* App/Adapters/STM32/AdminInfoAdapter.h */
#ifndef ADMIN_INFO_ADAPTER_H
#define ADMIN_INFO_ADAPTER_H

#include "Ports/IAdminInfoPort.h"

typedef struct
{
  uint8_t reserved;
} AdminInfoAdapterCtx_t;

void AdminInfoAdapterInit(AdminInfoAdapterCtx_t *ctx);
IAdminInfoPort_t AdminInfoAdapterCreatePort(AdminInfoAdapterCtx_t *ctx);

#endif /* ADMIN_INFO_ADAPTER_H */
