/* App/Ports/IMmuPort.h
 *
 * Port interface for MMU filtering of the requested cabinet output image.
 */
#ifndef IMMU_PORT_H
#define IMMU_PORT_H

#include <stddef.h>
#include <stdint.h>

#include "Ports/IOutputDriverPort.h"

typedef enum
{
  MMU_CONTROL_ACTION_NORMAL = 0U,
  MMU_CONTROL_ACTION_FLASH = 1U,
  MMU_CONTROL_ACTION_DARK = 2U
} MmuControlAction_t;

typedef struct
{
  void *ctx;

  uint8_t (*SetForceAllRed)(void *ctx, uint8_t forceAllRed);
  uint8_t (*SetSafetyAction)(void *ctx, MmuControlAction_t action);
  uint8_t (*SetConfigReady)(void *ctx, uint8_t configReady);
  uint8_t (*SetRequiredSsmHealthy)(void *ctx, uint8_t requiredSsmHealthy);
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

static inline uint8_t MmuSetSafetyAction(IMmuPort_t *p,
                                         MmuControlAction_t action)
{
  if ((p == NULL) || (p->SetSafetyAction == NULL))
  {
    return 1U;
  }

  return p->SetSafetyAction(p->ctx, action);
}

static inline uint8_t MmuSetConfigReady(IMmuPort_t *p, uint8_t configReady)
{
  if ((p == NULL) || (p->SetConfigReady == NULL))
  {
    return 1U;
  }

  return p->SetConfigReady(p->ctx, configReady);
}

static inline uint8_t MmuSetRequiredSsmHealthy(IMmuPort_t *p,
                                               uint8_t requiredSsmHealthy)
{
  if ((p == NULL) || (p->SetRequiredSsmHealthy == NULL))
  {
    return 1U;
  }

  return p->SetRequiredSsmHealthy(p->ctx, requiredSsmHealthy);
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
