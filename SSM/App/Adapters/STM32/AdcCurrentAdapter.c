/**
 ******************************************************************************
 * @file    Adapters/STM32/AdcCurrentAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/STM32/AdcCurrentAdapter.h"
#include "stm32g4xx_hal.h"    /* for __DMB() */

static void AdapterGetLatest(void *ctx, CurrentMeasurementSnapshot_t *out)
{
  AdcCurrentAdapterCtx_t *pC = (AdcCurrentAdapterCtx_t *) ctx;
  uint32_t seqBefore = 1U;      /* any odd value — forces first-pass retry
                                *  if the first read sees a busy writer */
  uint32_t seqAfter = 0U;
  uint8_t i;

  /* Seqlock read loop. If the writer interrupted us, retry. In the
   * single-writer / single-reader same-task flow this loop runs exactly
   * once; the retry path exists for the documented ISR publishing case.
   */
  do
  {
    seqBefore = pC->seq;
    if ((seqBefore & 1U) != 0U)
    {
      /* Writer is mid-update — spin until it finishes. */
      seqAfter = seqBefore + 1U;          /* force retry */
      continue;
    }

    __DMB();

    /* Field-wise copy: pC->snap is volatile, so struct assignment is
     * not portable — read each field explicitly.
     */
    for (i = 0U; i < CURRENT_CHANNEL_COUNT; i++)
    {
      out->currentsMa[i] = pC->snap.currentsMa[i];
    }

    out->status = pC->snap.status;
    out->seqNo = pC->snap.seqNo;
    __DMB();
    seqAfter = pC->seq;
  } while (seqBefore != seqAfter);
}

void AdcCurrentAdapter_Init(AdcCurrentAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

ICurrentMeasurementPort_t AdcCurrentAdapter_CreatePort(
  AdcCurrentAdapterCtx_t *ctx)
{
  ICurrentMeasurementPort_t port;

  port.ctx = ctx;
  port.GetLatest = AdapterGetLatest;

  return port;
}

void AdcCurrentAdapter_Publish(AdcCurrentAdapterCtx_t *ctx,
                               const float *currents,
                               uint8_t status)
{
  uint8_t i;

  /* Mark write-in-progress (odd seq), fence, write data, fence, mark done. */
  ctx->seq++;
  __DMB();

  for (i = 0U; i < CURRENT_CHANNEL_COUNT; i++)
  {
    float f = currents[i];

    if (f < 0.0f)
    {
      f = 0.0f;
    }

    if (f > 65535.0f)
    {
      f = 65535.0f;
    }

    ctx->snap.currentsMa[i] = (uint16_t) f;
  }

  ctx->snap.status = status;
  ctx->snap.seqNo = ctx->seq + 1U;      /* will match seq after final bump */

  __DMB();
  ctx->seq++;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
