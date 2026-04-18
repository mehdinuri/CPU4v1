/**
 ******************************************************************************
 * @file    Domain/SignalOutputImageBuilder.c
 * @brief   Pure-computation builder for tSSignalOutputImage.
 ******************************************************************************
 */

#include "Domain/SignalOutputImageBuilder.h"

void SignalOutputImageBuilder_Build(const tSSignalOutputBuildInputs *pInputs,
                                    tSSignalOutputImage *pImage)
{
  uint8_t i;

  if (pInputs->bFlashActive == 0U)
  {
    for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
    {
      pImage->aChannels[i] = (pInputs->aRunChannels[i] != 0U) ? 1U : 0U;
    }
  }
  else if (pInputs->bFlashSyncActive == 0U)
  {
    for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
    {
      pImage->aChannels[i] = 0U;
    }
  }
  else
  {
    for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
    {
      pImage->aChannels[i] = (pInputs->aFlashChannels[i] != 0U) ? 1U : 0U;
    }
  }
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
