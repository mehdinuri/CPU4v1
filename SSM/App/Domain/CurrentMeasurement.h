/**
 ******************************************************************************
 * @file    Domain/CurrentMeasurement.h
 * @brief   Pure-computation packing of per-group RMS currents into the
 *          outgoing CAN telemetry wire format (10 bits per channel, split
 *          into a low byte + 2-bit high-nibble).
 ******************************************************************************
 */

#ifndef DOMAIN_CURRENT_MEASUREMENT_H
#define DOMAIN_CURRENT_MEASUREMENT_H

#include <stdint.h>
#include "Ports/ICurrentMeasurementPort.h"

typedef struct
{
  uint8_t curLow[CURRENT_CHANNEL_COUNT];       /* bits 0..7 of each current */
  uint8_t curHighBitsPacked;                   /* 4 × 2-bit fields: channel i → bits (2*i)..(2*i+1) */
} CurrentMeasurementWire_t;

/**
 * @brief Pack a snapshot into the 10-bit-per-channel wire format.
 *
 *  Layout of curHighBitsPacked:
 *      bit 0..1  = high 2 bits of channel 0 current
 *      bit 2..3  = high 2 bits of channel 1 current
 *      bit 4..5  = high 2 bits of channel 2 current
 *      bit 6..7  = high 2 bits of channel 3 current
 *
 *  Values above 0x3FF (1023) are saturated to 0x3FF.
 */
void CurrentMeasurement_Pack(const CurrentMeasurementSnapshot_t *snap,
                             CurrentMeasurementWire_t *out);

#endif /* DOMAIN_CURRENT_MEASUREMENT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
