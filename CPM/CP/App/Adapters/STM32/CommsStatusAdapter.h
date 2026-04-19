/* App/Adapters/STM32/CommsStatusAdapter.h */
#ifndef COMMS_STATUS_ADAPTER_H
#define COMMS_STATUS_ADAPTER_H

#include "Ports/ICommsStatusPort.h"

typedef struct
{
  uint8_t reserved;
} CommsStatusAdapterCtx_t;

void CommsStatusAdapterInit(CommsStatusAdapterCtx_t *ctx);
ICommsStatusPort_t CommsStatusAdapterCreatePort(CommsStatusAdapterCtx_t *ctx);

#endif /* COMMS_STATUS_ADAPTER_H */
