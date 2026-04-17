/* App/Ports/IStatusLEDPort.h
 *
 * Port interface for a single-pin status or activity LED (PA4).
 * Used by communication tasks to indicate bus activity and by
 * diagnostic routines to signal fault states.
 */
#ifndef ISTATUS_LED_PORT_H
#define ISTATUS_LED_PORT_H

#include <stdint.h>

typedef struct
{
  void *ctx;

  void (*Toggle)(void *ctx);
  void (*SetState)(void *ctx, uint8_t on);
} IStatusLEDPort_t;

static inline void StatusLEDToggle(IStatusLEDPort_t *p)
{
  p->Toggle(p->ctx);
}

static inline void StatusLEDSetState(IStatusLEDPort_t *p, uint8_t on)
{
  p->SetState(p->ctx, on);
}

#endif /* ISTATUS_LED_PORT_H */
