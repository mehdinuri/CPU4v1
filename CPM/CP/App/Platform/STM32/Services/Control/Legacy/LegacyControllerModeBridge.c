/* App/Platform/STM32/Services/Control/Legacy/LegacyControllerModeBridge.c */
#include "LegacyControllerModeBridge.h"

#include "ControllerModeControlAdapter.h"

uint8_t LegacyControllerModeBridgeRequest(uint8_t requestedControl)
{
  ControllerModeControlAdapterCtx_t ctx;
  IControllerModeControlPort_t port;

  ControllerModeControlAdapterInit(&ctx);
  port = ControllerModeControlAdapterCreatePort(&ctx);

  return ControllerModeControlPortRequest(&port, requestedControl);
}
