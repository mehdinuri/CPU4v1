/**
 ******************************************************************************
 * @file    Domain/FrequencyCaptureFSM.c
 * @brief   Edge-timing, wraparound, tolerance, and sleep-strike FSM.
 ******************************************************************************
 */

#include "Domain/FrequencyCaptureFSM.h"

#include <stddef.h>

void FrequencyCaptureFSM_Init(tSFrequencyCaptureFSMState *pState,
                              uint32_t lNowMs)
{
  if (pState == NULL)
  {
    return;
  }

  pState->lPrevCaptureValue  = 0U;
  pState->lLastCaptureTimeMs = lNowMs;
  pState->sBadReadingsCount  = 0U;
  pState->bMeasuredFreqHz    = 0U;
  pState->fPrimed             = 0U;
}

uint8_t FrequencyCaptureFSM_OnEdge(tSFrequencyCaptureFSMState *pState,
                                   const tSFrequencyCaptureFSMConfig *pCfg,
                                   uint32_t lCaptureValue,
                                   uint32_t lNowMs)
{
  if ((pState == NULL) || (pCfg == NULL))
  {
    return 0U;
  }

  pState->lLastCaptureTimeMs = lNowMs;

  if (pState->fPrimed == 0U)
  {
    pState->lPrevCaptureValue = lCaptureValue;
    pState->fPrimed            = 1U;
    return 0U;
  }

  /* Compute wraparound-safe diff (counter is a uint up to lCounterMax). */
  uint32_t lDiff = 0U;

  if (lCaptureValue > pState->lPrevCaptureValue)
  {
    lDiff = lCaptureValue - pState->lPrevCaptureValue;
  }
  else if (lCaptureValue < pState->lPrevCaptureValue)
  {
    lDiff = (pCfg->lCounterMax - pState->lPrevCaptureValue)
          + lCaptureValue
          + 1U;
  }
  else
  {
    /* Same counter value twice — force re-prime on next edge. */
    lDiff = 0U;
    pState->fPrimed = 0U;
  }

  if (lDiff != 0U)
  {
    pState->bMeasuredFreqHz = (uint8_t) (pCfg->lClockFreqHz / lDiff);
  }
  else
  {
    pState->bMeasuredFreqHz = 0U;
  }

  pState->lPrevCaptureValue = lCaptureValue;
  return 1U;
}

tEFrequencyCaptureFSMVerdict FrequencyCaptureFSM_Evaluate(
    tSFrequencyCaptureFSMState *pState,
    const tSFrequencyCaptureFSMConfig *pCfg,
    uint32_t lNowMs)
{
  if ((pState == NULL) || (pCfg == NULL))
  {
    return FREQ_FSM_VERDICT_BAD;
  }

  uint8_t fTimedOut = 0U;

  if (pState->fPrimed == 0U)
  {
    fTimedOut = 1U;
  }
  else if ((lNowMs - pState->lLastCaptureTimeMs) > pCfg->lCaptureTimeoutMs)
  {
    fTimedOut = 1U;
  }
  else
  {
    fTimedOut = 0U;
  }

  if (fTimedOut != 0U)
  {
    pState->bMeasuredFreqHz = 0U;
    pState->fPrimed          = 0U;
  }

  uint8_t bFreq  = pState->bMeasuredFreqHz;
  uint8_t bLo    = pCfg->bTargetFreqHz - pCfg->bFreqToleranceHz;
  uint8_t bHi    = pCfg->bTargetFreqHz + pCfg->bFreqToleranceHz;
  uint8_t fInBand = ((fTimedOut == 0U) && (bFreq >= bLo) && (bFreq <= bHi)) ? 1U : 0U;

  if (fInBand != 0U)
  {
    pState->sBadReadingsCount = 0U;
    return FREQ_FSM_VERDICT_OK;
  }

  pState->sBadReadingsCount++;

  if (pState->sBadReadingsCount >= pCfg->sBadReadingsBeforeSleep)
  {
    return FREQ_FSM_VERDICT_ENTER_SLEEP;
  }

  return FREQ_FSM_VERDICT_BAD;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
