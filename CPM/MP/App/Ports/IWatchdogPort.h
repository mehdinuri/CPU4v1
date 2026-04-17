/* App/Ports/IWatchdogPort.h
 *
 * Port interface for the STM32 independent watchdog. Abstracted so the
 * MaintenanceTask can feed it without depending on HAL, and so host
 * tests can assert on feed cadence via the mock.
 */
#ifndef I_WATCHDOG_PORT_H
#define I_WATCHDOG_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*Feed)(void *ctx);
} IWatchdogPort_t;

static inline uint8_t WatchdogFeed(IWatchdogPort_t *port)
{
  if ((port == NULL) || (port->Feed == NULL))
  {
    return 0U;
  }

  return port->Feed(port->ctx);
}

#endif /* I_WATCHDOG_PORT_H */
