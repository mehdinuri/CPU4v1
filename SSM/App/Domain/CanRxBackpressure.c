/**
 ******************************************************************************
 * @file    Domain/CanRxBackpressure.c
 ******************************************************************************
 */

#include <stddef.h>
#include "Domain/CanRxBackpressure.h"

void CanRxBackpressure_Reset(CanRxBackpressure_t *state)
{
  if (state == NULL)
  {
    return;
  }

  state->consecutiveDrops = 0U;
}

void CanRxBackpressure_RecordSuccess(CanRxBackpressure_t *state)
{
  if (state == NULL)
  {
    return;
  }

  state->consecutiveDrops = 0U;
}

uint8_t CanRxBackpressure_RecordDrop(CanRxBackpressure_t *state,
                                     uint32_t threshold)
{
  if (state == NULL)
  {
    return 0U;
  }

  /* Saturate at UINT32_MAX so a pathological run of drops cannot wrap
   * and briefly re-enter below-threshold territory.
   */
  if (state->consecutiveDrops < 0xFFFFFFFFU)
  {
    state->consecutiveDrops++;
  }

  return (state->consecutiveDrops >= threshold) ? 1U : 0U;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
