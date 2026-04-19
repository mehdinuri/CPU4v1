/* App/Ports/IControllerModeControlPort.h */
#ifndef ICONTROLLER_MODE_CONTROL_PORT_H
#define ICONTROLLER_MODE_CONTROL_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
  CONTROLLER_MODE_REQUEST_ALL_RED = 1U,
  CONTROLLER_MODE_REQUEST_DARK = 2U,
  CONTROLLER_MODE_REQUEST_FLASH = 3U,
  CONTROLLER_MODE_REQUEST_PLAN_RETURN = 4U
} ControllerModeRequest_t;

typedef struct
{
  void *ctx;

  uint8_t (*RequestModeControl)(void *ctx, uint8_t requestedControl);
} IControllerModeControlPort_t;

static inline uint8_t ControllerModeControlPortRequest(
  IControllerModeControlPort_t *port,
  uint8_t requestedControl)
{
  if ((port == NULL) || (port->RequestModeControl == NULL))
  {
    return 0U;
  }

  return port->RequestModeControl(port->ctx, requestedControl);
}

#endif /* ICONTROLLER_MODE_CONTROL_PORT_H */
