/**
 ******************************************************************************
 * @file    Adapters/STM32/AdcCurrentAdapter.c
 ******************************************************************************
 */

#include <string.h>
#include "Adapters/STM32/AdcCurrentAdapter.h"
#include "stm32g4xx_hal.h"    /* for __DMB() */

static void AdapterGetLatest(void *pCtx, tSCurrentMeasurementSnapshot *pOut)
{
  tSAdcCurrentAdapterCtx *pC = (tSAdcCurrentAdapterCtx *) pCtx;
  uint32_t lSeqBefore = 1U;      /* any odd value — forces first-pass retry
                                 *  if the first read sees a busy writer */
  uint32_t lSeqAfter = 0U;

  /* Seqlock read loop. If the writer interrupted us, retry. In the
   * single-writer / single-reader same-task flow this loop runs exactly
   * once; the retry path exists for the documented ISR publishing case.
   */
  do
  {
    lSeqBefore = pC->lSeq;
    if ((lSeqBefore & 1U) != 0U)
    {
      /* Writer is mid-update — spin until it finishes. */
      lSeqAfter = lSeqBefore + 1U;          /* force retry */
      continue;
    }

    __DMB();
    *pOut = pC->SSnap;
    __DMB();
    lSeqAfter = pC->lSeq;
  } while (lSeqBefore != lSeqAfter);
}

void AdcCurrentAdapter_Init(tSAdcCurrentAdapterCtx *pCtx)
{
  memset(pCtx, 0, sizeof(*pCtx));
}

ICurrentMeasurementPort_t AdcCurrentAdapter_CreatePort(
  tSAdcCurrentAdapterCtx *pCtx)
{
  ICurrentMeasurementPort_t port;

  port.pCtx = pCtx;
  port.GetLatest = AdapterGetLatest;

  return port;
}

void AdcCurrentAdapter_Publish(tSAdcCurrentAdapterCtx *pCtx,
                               const float *pfCurrents_mA)
{
  uint8_t i;

  /* Mark write-in-progress (odd seq), fence, write data, fence, mark done. */
  pCtx->lSeq++;
  __DMB();

  for (i = 0U; i < CURRENT_CHANNEL_COUNT; i++)
  {
    float f = pfCurrents_mA[i];

    if (f < 0.0f)
    {
      f = 0.0f;
    }

    if (f > 65535.0f)
    {
      f = 65535.0f;
    }

    pCtx->SSnap.aCurrents_mA[i] = (uint16_t) f;
  }

  pCtx->SSnap.lSeqNo = pCtx->lSeq + 1U;      /* will match lSeq after final bump */

  __DMB();
  pCtx->lSeq++;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
