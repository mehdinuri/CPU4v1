/* App/Ports/IRelayPort.h
 *
 * Raw CP-side relay GPIO port. The MMU layer converts permit semantics into a
 * raw drive level; the adapter/hardware implement the final active-low vote.
 */
#ifndef IRELAY_PORT_H
#define IRELAY_PORT_H

#include <stdint.h>

typedef struct
{
  void    *ctx;

  void (*Set)(void *ctx, uint8_t on);
  uint8_t (*Get)(void *ctx);
} IRelayPort_t;

static inline void RelaySet(IRelayPort_t *p, uint8_t on)
{
  p->Set(p->ctx, on);
}

static inline uint8_t RelayGet(IRelayPort_t *p)
{
  return p->Get(p->ctx);
}

#endif /* IRELAY_PORT_H */
