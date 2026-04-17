/* App/Ports/IRelayPort.h
 *
 * Port interface for the output relay (PA6).
 * In a NEMA TS2 context this relay is used as a fault or flash
 * output.  The domain reads it back to confirm state.
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
