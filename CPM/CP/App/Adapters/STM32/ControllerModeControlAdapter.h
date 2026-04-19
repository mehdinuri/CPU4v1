/* App/Adapters/STM32/ControllerModeControlAdapter.h */
#ifndef CONTROLLER_MODE_CONTROL_ADAPTER_H
#define CONTROLLER_MODE_CONTROL_ADAPTER_H

#include "Ports/IControllerModeControlPort.h"

typedef struct
{
  uint8_t reserved;
} ControllerModeControlAdapterCtx_t;

void ControllerModeControlAdapterInit(ControllerModeControlAdapterCtx_t *ctx);
IControllerModeControlPort_t ControllerModeControlAdapterCreatePort(
  ControllerModeControlAdapterCtx_t *ctx);

#endif /* CONTROLLER_MODE_CONTROL_ADAPTER_H */
