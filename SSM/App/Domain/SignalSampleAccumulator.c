/**
 ******************************************************************************
 * @file    Domain/SignalSampleAccumulator.c
 ******************************************************************************
 */

#include <string.h>
#include "Domain/SignalSampleAccumulator.h"
#include "Domain/SignalOutput.h"

void SignalSampleAccumulator_Reset(tSSignalSampleAccumulator *pAcc)
{
  memset(pAcc, 0, sizeof(*pAcc));
}

void SignalSampleAccumulator_Observe(tSSignalSampleAccumulator *pAcc,
                                     const tSSignalInputSnapshot *pSnap)
{
  uint8_t i;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    if (pSnap->aChannels[i] != 0U)
    {
      if (pAcc->aOnCntr[i] < 0xFFU)
      {
        pAcc->aOnCntr[i]++;
      }
    }
    else
    {
      if (pAcc->aOffCntr[i] < 0xFFU)
      {
        pAcc->aOffCntr[i]++;
      }
    }
  }
}

void SignalSampleAccumulator_Summary(const tSSignalSampleAccumulator *pAcc,
                                     tSSignalOutputImage *pOut)
{
  uint8_t i;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    pOut->aChannels[i] = SignalOutput_IsActive(pAcc->aOnCntr[i],
                                               pAcc->aOffCntr[i]);
  }
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
