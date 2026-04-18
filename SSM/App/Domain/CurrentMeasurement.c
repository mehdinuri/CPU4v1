/**
 ******************************************************************************
 * @file    Domain/CurrentMeasurement.c
 ******************************************************************************
 */

#include "Domain/CurrentMeasurement.h"

#define CURRENT_MAX_10BIT  0x03FFU

void CurrentMeasurement_Pack(const tSCurrentMeasurementSnapshot *pSnap,
                             tSCurrentMeasurementWire *pOut)
{
  uint8_t i;
  uint8_t bHighPacked = 0U;

  for (i = 0U; i < CURRENT_CHANNEL_COUNT; i++)
  {
    uint16_t sValue = pSnap->aCurrents_mA[i];

    if (sValue > CURRENT_MAX_10BIT)
    {
      sValue = CURRENT_MAX_10BIT;
    }

    pOut->aCurLow[i] = (uint8_t) (sValue & 0xFFU);
    uint8_t bHigh2 = (uint8_t) ((sValue >> 8) & 0x03U);

    bHighPacked = (uint8_t) (bHighPacked | (uint8_t) (bHigh2 << (2U * i)));
  }

  pOut->bCurHighBitsPacked = bHighPacked;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
