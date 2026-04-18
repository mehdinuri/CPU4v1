/**
 ******************************************************************************
 * @file    Domain/SignalOutputImageBuilder.c
 * @brief   Pure-computation builder for tSSignalOutputImage.
 ******************************************************************************
 */

#include "Domain/SignalOutputImageBuilder.h"

#define SIGNAL_OUTPUT_GROUP_COUNT             4U
#define SIGNAL_OUTPUT_CHANNELS_PER_GROUP      3U

static uint8_t SignalGroupMaskNormalise(uint8_t bRawMask, uint8_t *pbFaulted)
{
  switch (bRawMask & 0x07U)
  {
      case 0x00U:   /* dark */
      case 0x01U:   /* red */
      case 0x02U:   /* yellow */
      case 0x03U:   /* red+yellow */
      case 0x04U:   /* green */
      {
        return (uint8_t) (bRawMask & 0x07U);
      }

      default:
      {
        *pbFaulted = 1U;
        return 0U;
      }
  }
}

uint8_t SignalOutputImageBuilder_BuildSafe(
  const tSSignalOutputBuildInputs *pInputs,
  tSSignalOutputImage *pImage)
{
  uint8_t bFaulted = 0U;
  uint8_t bGroupIdx;
  const uint8_t *paSelected =
    (pInputs->bFlashActive != 0U) ? pInputs->aFlashChannels : pInputs->aRunChannels;

  for (bGroupIdx = 0U; bGroupIdx < SIGNAL_OUTPUT_GROUP_COUNT; bGroupIdx++)
  {
    uint8_t bGroupMask = 0U;
    uint8_t bChannelIdx;

    for (bChannelIdx = 0U;
         bChannelIdx < SIGNAL_OUTPUT_CHANNELS_PER_GROUP;
         bChannelIdx++)
    {
      uint8_t bIdx = (uint8_t) ((bGroupIdx * SIGNAL_OUTPUT_CHANNELS_PER_GROUP)
                                + bChannelIdx);

      if (paSelected[bIdx] != 0U)
      {
        bGroupMask = (uint8_t) (bGroupMask | (uint8_t) (1U << bChannelIdx));
      }
    }

    bGroupMask = SignalGroupMaskNormalise(bGroupMask, &bFaulted);

    for (bChannelIdx = 0U;
         bChannelIdx < SIGNAL_OUTPUT_CHANNELS_PER_GROUP;
         bChannelIdx++)
    {
      uint8_t bIdx = (uint8_t) ((bGroupIdx * SIGNAL_OUTPUT_CHANNELS_PER_GROUP)
                                + bChannelIdx);
      uint8_t bAllowed = (uint8_t) ((bGroupMask >> bChannelIdx) & 0x01U);

      if ((pInputs->bFlashActive != 0U) && (pInputs->bFlashSyncActive == 0U))
      {
        pImage->aChannels[bIdx] = 0U;
      }
      else
      {
        pImage->aChannels[bIdx] = bAllowed;
      }
    }
  }

  return bFaulted;
}

void SignalOutputImageBuilder_Build(const tSSignalOutputBuildInputs *pInputs,
                                    tSSignalOutputImage *pImage)
{
  (void) SignalOutputImageBuilder_BuildSafe(pInputs, pImage);
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
