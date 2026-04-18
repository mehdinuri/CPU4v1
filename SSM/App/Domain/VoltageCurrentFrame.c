/**
 ******************************************************************************
 * @file    Domain/VoltageCurrentFrame.c
 ******************************************************************************
 */

#include "Domain/VoltageCurrentFrame.h"

void VoltageCurrentFrame_Encode(const tSVoltageCurrentFrameInputs *pIn,
                                uint8_t pOutBytes[VOLTAGE_CURRENT_FRAME_BYTES])
{
  uint8_t i;

  /* Bytes 0..1: voltage bits. GCC's __packed bit-field layout for
   * tSBitFlags16 places fBit0 at the LSB of byte 0. Reproduce that
   * explicitly: bit i of the 16-bit word corresponds to channel i.
   */
  uint16_t sVoltageBits = 0U;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    if (pIn->SVoltageImage.aChannels[i] != 0U)
    {
      sVoltageBits = (uint16_t) (sVoltageBits | (uint16_t) (1U << i));
    }
  }

  pOutBytes[0] = (uint8_t) (sVoltageBits & 0xFFU);
  pOutBytes[1] = (uint8_t) ((sVoltageBits >> 8) & 0xFFU);

  /* Bytes 2..5: per-channel current low bytes, already packed by
   * CurrentMeasurement_Pack in the right order (channel 0..3).
   */
  pOutBytes[2] = pIn->SCurrentWire.aCurLow[0];
  pOutBytes[3] = pIn->SCurrentWire.aCurLow[1];
  pOutBytes[4] = pIn->SCurrentWire.aCurLow[2];
  pOutBytes[5] = pIn->SCurrentWire.aCurLow[3];

  /* Byte 6: packed 2-bit-per-channel high bits; same convention as the
   * legacy bit-field (channel 0 → bits 0..1, channel 3 → bits 6..7).
   */
  pOutBytes[6] = pIn->SCurrentWire.bCurHighBitsPacked;

  /* Byte 7: reserved — receivers must accept 0. */
  pOutBytes[7] = 0U;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
