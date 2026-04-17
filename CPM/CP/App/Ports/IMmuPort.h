/* App/Ports/IMmuPort.h
 *
 * Port interface for MMU filtering of the requested cabinet output image.
 */
#ifndef IMMU_PORT_H
#define IMMU_PORT_H

#include <stddef.h>
#include <stdint.h>

#include "Ports/IOutputDriverPort.h"

typedef struct
{
  void *ctx;

  uint8_t (*SetForceAllRed)(void *ctx, uint8_t forceAllRed);
  uint8_t (*FilterOutputImage)(void *ctx,
                               const OutputDriverImage_t *requested,
                               OutputDriverImage_t *approved);
} IMmuPort_t;

static inline uint8_t MmuSetForceAllRed(IMmuPort_t *p, uint8_t forceAllRed)
{
  if ((p == NULL) || (p->SetForceAllRed == NULL))
  {
    return 1U;
  }

  return p->SetForceAllRed(p->ctx, forceAllRed);
}

static inline uint8_t MmuFilterOutputImage(IMmuPort_t *p,
                                           const OutputDriverImage_t *requested,
                                           OutputDriverImage_t *approved)
{
  if ((p == NULL) || (p->FilterOutputImage == NULL))
  {
    if ((requested == NULL) || (approved == NULL))
    {
      return 0U;
    }

    *approved = *requested;

    return 1U;
  }

  return p->FilterOutputImage(p->ctx, requested, approved);
}

#endif /* IMMU_PORT_H */
