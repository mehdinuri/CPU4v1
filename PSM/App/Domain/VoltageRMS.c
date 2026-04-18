/**
 ******************************************************************************
 * @file    Domain/VoltageRMS.c
 * @brief   ADC-sample-to-voltage math (pure C11).
 ******************************************************************************
 */

#include "Domain/VoltageRMS.h"

#include <math.h>
#include <stddef.h>

float VoltageRMS_ComputeAC(const uint16_t *pSamples,
                           size_t sampleCount,
                           float fVRef,
                           float fAdcMax,
                           float fScaling)
{
  if ((pSamples == NULL) || (sampleCount == 0U) || (fAdcMax <= 0.0f))
  {
    return 0.0f;
  }

  /* First pass — integer mean (keeps full 12-bit precision). */
  uint32_t lSum = 0U;
  for (size_t i = 0U; i < sampleCount; i++)
  {
    lSum += (uint32_t) pSamples[i];
  }

  float fMean = (float) lSum / (float) sampleCount;

  /* Second pass — sum of squared AC components. */
  float fSumSquares = 0.0f;
  for (size_t i = 0U; i < sampleCount; i++)
  {
    float fAC = (float) pSamples[i] - fMean;
    fSumSquares += fAC * fAC;
  }

  float fRawRMS = sqrtf(fSumSquares / (float) sampleCount);

  return fRawRMS * (fVRef / fAdcMax) * fScaling;
}

float VoltageRMS_ConvertDC(uint16_t sSample,
                           float fVRef,
                           float fAdcMax,
                           float fScaling)
{
  if (fAdcMax <= 0.0f)
  {
    return 0.0f;
  }

  return (float) sSample * (fVRef / fAdcMax) * fScaling;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
