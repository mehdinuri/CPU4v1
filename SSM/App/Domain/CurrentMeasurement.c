/**
 ******************************************************************************
 * @file    Domain/CurrentMeasurement.c
 ******************************************************************************
 */

#include "Domain/CurrentMeasurement.h"

#define CURRENT_MAX_10BIT  0x03FFU

void CurrentMeasurement_Pack(const CurrentMeasurementSnapshot_t *snap,
                             CurrentMeasurementWire_t *out)
{
  uint8_t i;
  uint8_t highPacked = 0U;

  for (i = 0U; i < CURRENT_CHANNEL_COUNT; i++)
  {
    uint16_t value = snap->currentsMa[i];

    if (value > CURRENT_MAX_10BIT)
    {
      value = CURRENT_MAX_10BIT;
    }

    out->curLow[i] = (uint8_t) (value & 0xFFU);
    uint8_t high2 = (uint8_t) ((value >> 8) & 0x03U);

    highPacked = (uint8_t) (highPacked | (uint8_t) (high2 << (2U * i)));
  }

  out->curHighBitsPacked = highPacked;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
