/* App/Ports/ISystemResetPort.h
 *
 * Port for requesting a controlled system reset.
 */
#ifndef ISYSTEM_RESET_PORT_H
#define ISYSTEM_RESET_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  void *ctx;

  void (*RequestReset)(void *ctx);
} ISystemResetPort_t;

static inline void SystemResetPortRequest(ISystemResetPort_t *port)
{
  if ((port != NULL) && (port->RequestReset != NULL))
  {
    port->RequestReset(port->ctx);
  }
}

#endif /* ISYSTEM_RESET_PORT_H */
