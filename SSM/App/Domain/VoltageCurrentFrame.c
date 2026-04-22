/**
 ******************************************************************************
 * @file    Domain/VoltageCurrentFrame.c
 ******************************************************************************
 */

#include "Domain/VoltageCurrentFrame.h"

void VoltageCurrentFrame_Encode(const VoltageCurrentFrameInputs_t *in,
                                uint8_t outBytes[VOLTAGE_CURRENT_FRAME_BYTES])
{
  uint8_t i;

  /* Bytes 0..1: voltage bits. GCC's __packed bit-field layout for
   * BitFlags16_t places bit0 at the LSB of byte 0. Reproduce that
   * explicitly: bit i of the 16-bit word corresponds to channel i.
   */
  uint16_t voltageBits = 0U;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    if (in->voltageImage.channels[i] != 0U)
    {
      voltageBits = (uint16_t) (voltageBits | (uint16_t) (1U << i));
    }
  }

  outBytes[0] = (uint8_t) (voltageBits & 0xFFU);
  outBytes[1] = (uint8_t) ((voltageBits >> 8) & 0xFFU);

  /* Bytes 2..5: per-channel current low bytes, already packed by
   * CurrentMeasurement_Pack in the right order (channel 0..3).
   */
  outBytes[2] = in->currentWire.curLow[0];
  outBytes[3] = in->currentWire.curLow[1];
  outBytes[4] = in->currentWire.curLow[2];
  outBytes[5] = in->currentWire.curLow[3];

  /* Byte 6: packed 2-bit-per-channel high bits; same convention as the
   * legacy bit-field (channel 0 → bits 0..1, channel 3 → bits 6..7).
   */
  outBytes[6] = in->currentWire.curHighBitsPacked;

  /* Byte 7: status flags. */
  outBytes[7] = in->status;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
