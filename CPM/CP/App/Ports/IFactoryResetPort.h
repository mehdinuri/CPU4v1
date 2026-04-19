/* App/Ports/IFactoryResetPort.h */
#ifndef IFACTORY_RESET_PORT_H
#define IFACTORY_RESET_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*RequestFactoryReset)(void *ctx);
} IFactoryResetPort_t;

static inline uint8_t FactoryResetPortRequest(IFactoryResetPort_t *port)
{
  if ((port == NULL) || (port->RequestFactoryReset == NULL))
  {
    return 0U;
  }

  return port->RequestFactoryReset(port->ctx);
}

#endif /* IFACTORY_RESET_PORT_H */
