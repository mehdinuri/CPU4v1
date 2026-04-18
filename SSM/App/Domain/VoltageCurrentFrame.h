/**
 ******************************************************************************
 * @file    Domain/VoltageCurrentFrame.h
 * @brief   Byte-explicit encoder for the SSM → supervisor voltage/current
 *          telemetry frame (CAN IDs 0x050..0x057). Replaces a memcpy of the
 *          legacy `__packed` tSVoltageCurrent_t struct, whose layout depended
 *          on compiler-specific handling of nested `__packed` unions.
 *
 *          The emitted bytes are bit-for-bit identical to the legacy format
 *          so CP/PSM/MP receivers need no change. The win is: the wire
 *          contract is now written down in C that any compiler produces
 *          the same bytes for, and is unit-testable.
 *
 * Wire format (8 bytes, little-endian over the air):
 *
 *   byte 0 : voltage bits 0..7   (channel N → bit N; 1 = energised)
 *   byte 1 : voltage bits 8..15  (channels 8..11 → bits 0..3; bits 4..7 = 0)
 *   byte 2 : channel 0 current, low 8 bits
 *   byte 3 : channel 1 current, low 8 bits
 *   byte 4 : channel 2 current, low 8 bits
 *   byte 5 : channel 3 current, low 8 bits
 *   byte 6 : packed 2-bit-per-channel current high bits
 *              bit 0..1 = channel 0 high bits
 *              bit 2..3 = channel 1 high bits
 *              bit 4..5 = channel 2 high bits
 *              bit 6..7 = channel 3 high bits
 *   byte 7 : reserved — always 0
 *
 * ("Low 8 bits" + "2 high bits" = 10-bit clamped current per channel,
 *  matching Domain/CurrentMeasurement's pack.)
 ******************************************************************************
 */

#ifndef DOMAIN_VOLTAGE_CURRENT_FRAME_H
#define DOMAIN_VOLTAGE_CURRENT_FRAME_H

#include <stdint.h>
#include "Ports/ISignalOutputPort.h"        /* tSSignalOutputImage */
#include "Domain/CurrentMeasurement.h"      /* tSCurrentMeasurementWire */

#define VOLTAGE_CURRENT_FRAME_BYTES 8U

typedef struct
{
  tSSignalOutputImage SVoltageImage;           /* 12 observed channel states */
  tSCurrentMeasurementWire SCurrentWire;       /* 4 × 10-bit currents */
} tSVoltageCurrentFrameInputs;

/**
 * @brief Encode the telemetry frame as VOLTAGE_CURRENT_FRAME_BYTES bytes.
 *        Output bytes match the legacy __packed struct layout byte-for-byte.
 */
void VoltageCurrentFrame_Encode(const tSVoltageCurrentFrameInputs *pIn,
                                uint8_t pOutBytes[VOLTAGE_CURRENT_FRAME_BYTES]);

#endif /* DOMAIN_VOLTAGE_CURRENT_FRAME_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
