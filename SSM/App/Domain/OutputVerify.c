/**
 ******************************************************************************
 * @file    Domain/OutputVerify.c
 ******************************************************************************
 */

#include <string.h>
#include "Domain/OutputVerify.h"

void OutputVerify_Reset(tSOutputVerifyState *pState)
{
  memset(pState, 0, sizeof(*pState));
}

void OutputVerify_Step(tSOutputVerifyState *pState,
                       const tSSignalOutputImage *pCommanded,
                       const tSSignalOutputImage *pObserved)
{
  uint8_t i;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    uint8_t bCmd = (pCommanded->aChannels[i] != 0U) ? 1U : 0U;
    uint8_t bObs = (pObserved->aChannels[i] != 0U) ? 1U : 0U;

    if (bCmd == bObs)
    {
      pState->aMismatchCount[i] = 0U;
    }
    else
    {
      if (pState->aMismatchCount[i] < 0xFFU)
      {
        pState->aMismatchCount[i]++;
      }

      if (pState->aMismatchCount[i] >= OUTPUT_VERIFY_FAULT_THRESHOLD)
      {
        pState->bFaultActive = 1U;
      }
    }
  }
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
