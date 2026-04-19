/* App/Ports/IControllerModeControlPort.h */
#ifndef ICONTROLLER_MODE_CONTROL_PORT_H
#define ICONTROLLER_MODE_CONTROL_PORT_H

#include <stddef.h>
#include <stdint.h>

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
