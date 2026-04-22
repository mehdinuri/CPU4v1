/**
 ******************************************************************************
 * @file    Domain/SignalOutputImageBuilder.c
 * @brief   Pure-computation builder for SignalOutputImage_t.
 ******************************************************************************
 */

#include "Domain/SignalOutputImageBuilder.h"

void SignalOutputImageBuilder_Build(const SignalOutputBuildInputs_t *inputs,
                                    SignalOutputImage_t *image)
{
  uint8_t i;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    if (inputs->flashActive != 0U)
    {
      if (inputs->flashSyncActive != 0U)
      {
        image->channels[i] = (inputs->flashChannels[i] != 0U) ? 1U : 0U;
      }
      else
      {
        image->channels[i] = 0U;
      }
    }
    else
    {
      image->channels[i] = (inputs->runChannels[i] != 0U) ? 1U : 0U;
    }
  }
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
