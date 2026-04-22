/* App/Ports/ISafetyRelayPort.h
 *
 * Port interface for MP's local vote on the shared SSM power relay. CP and MP
 * each drive their own active-low permit output; hardware ANDs those votes.
 * The domain sees only open/closed contactor state through this vtable.
 */
#ifndef I_SAFETY_RELAY_PORT_H
#define I_SAFETY_RELAY_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
  SAFETY_RELAY_STATE_OPEN = 0,  /* contactor open — SSMs unpowered */
  SAFETY_RELAY_STATE_CLOSED = 1 /* contactor closed — SSMs powered */
} SafetyRelayState_t;

typedef struct
{
  void *ctx;

  uint8_t (*SetState)(void *ctx, SafetyRelayState_t state);
  uint8_t (*GetCommandedState)(void *ctx, SafetyRelayState_t *state);
  uint8_t (*GetActualState)(void *ctx, SafetyRelayState_t *state);
} ISafetyRelayPort_t;

static inline uint8_t SafetyRelaySetState(ISafetyRelayPort_t *port,
                                          SafetyRelayState_t state)
{
  if ((port == NULL) || (port->SetState == NULL))
  {
    return 0U;
  }

  return port->SetState(port->ctx, state);
}

static inline uint8_t SafetyRelayGetCommandedState(
  const ISafetyRelayPort_t *port,
  SafetyRelayState_t *state)
{
  if ((port == NULL) || (port->GetCommandedState == NULL))
  {
    return 0U;
  }

  return port->GetCommandedState(port->ctx, state);
}

static inline uint8_t SafetyRelayGetActualState(const ISafetyRelayPort_t *port,
                                                SafetyRelayState_t *state)
{
  if ((port == NULL) || (port->GetActualState == NULL))
  {
    return 0U;
  }

  return port->GetActualState(port->ctx, state);
}

#endif /* I_SAFETY_RELAY_PORT_H */
