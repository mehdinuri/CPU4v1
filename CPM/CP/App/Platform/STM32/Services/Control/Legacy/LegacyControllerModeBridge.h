/* App/Platform/STM32/Services/Control/Legacy/LegacyControllerModeBridge.h */
#ifndef LEGACY_CONTROLLER_MODE_BRIDGE_H
#define LEGACY_CONTROLLER_MODE_BRIDGE_H

#include <stdint.h>

#include "Ports/IControllerModeControlPort.h"

uint8_t LegacyControllerModeBridgeRequest(uint8_t requestedControl);

#endif /* LEGACY_CONTROLLER_MODE_BRIDGE_H */
