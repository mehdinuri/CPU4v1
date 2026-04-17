/* App/Ports/IStatusLEDPort.h
 *
 * Port interface for the MP front-panel communication LED (PC0).
 * Used by domain code to signal bus health and fault status without
 * knowing about GPIO.
 */
#ifndef I_STATUS_LED_PORT_H
#define I_STATUS_LED_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
  STATUS_LED_STATE_OFF = 0,
  STATUS_LED_STATE_ON = 1,
  STATUS_LED_STATE_BLINK_SLOW = 2,
  STATUS_LED_STATE_BLINK_FAST = 3
} StatusLEDState_t;

typedef struct
{
  void *ctx;

  uint8_t (*SetState)(void *ctx, StatusLEDState_t state);
} IStatusLEDPort_t;

static inline uint8_t StatusLEDSetState(IStatusLEDPort_t *port,
                                        StatusLEDState_t state)
{
  if ((port == NULL) || (port->SetState == NULL))
  {
    return 0U;
  }

  return port->SetState(port->ctx, state);
}

#endif /* I_STATUS_LED_PORT_H */
