/**
 ******************************************************************************
 * @file    Domain/VoltageRMS.c
 * @brief   ADC-sample-to-voltage math (pure C11).
 ******************************************************************************
 */

#include "Domain/VoltageRMS.h"

#include <math.h>
#include <stddef.h>

float VoltageRMS_ComputeAC(const uint16_t *samples,
                           size_t sampleCount,
                           float vRef,
                           float adcMax,
                           float scaling)
{
  if ((samples == NULL) || (sampleCount == 0U) || (adcMax <= 0.0f))
  {
    return 0.0f;
  }

  /* First pass — integer mean (keeps full 12-bit precision). */
  uint32_t sum = 0U;

  for (size_t i = 0U; i < sampleCount; i++)
  {
    sum += (uint32_t) samples[i];
  }

  float mean = (float) sum / (float) sampleCount;

  /* Second pass — sum of squared AC components. */
  float sumSquares = 0.0f;

  for (size_t i = 0U; i < sampleCount; i++)
  {
    float ac = (float) samples[i] - mean;

    sumSquares += ac * ac;
  }

  float rawRms = sqrtf(sumSquares / (float) sampleCount);

  return rawRms * (vRef / adcMax) * scaling;
}

float VoltageRMS_ConvertDC(uint16_t sample,
                           float vRef,
                           float adcMax,
                           float scaling)
{
  if (adcMax <= 0.0f)
  {
    return 0.0f;
  }

  return (float) sample * (vRef / adcMax) * scaling;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
