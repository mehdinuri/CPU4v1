/**
 ******************************************************************************
 * @file    Domain/VoltageCurrentFrame.h
 * @brief   Byte-explicit encoder for the SSM → supervisor voltage/current
 *          telemetry frame (CAN IDs 0x050..0x057). Replaces a memcpy of the
 *          legacy `__packed` tSVoltageCurrent_t struct, whose layout depended
 *          on compiler-specific handling of nested `__packed` unions.
 *
 *          Bytes 0..6 stay bit-for-bit compatible with the legacy format.
 *          Byte 7 now carries explicit SSM status flags instead of always
 *          being zero. The wire contract is written down in C that any
 *          compiler produces identically, and is unit-testable.
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
 *   byte 7 : status flags (0 = healthy / absent)
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
#define VOLTAGE_CURRENT_STATUS_MEASUREMENT_FAULT   0x01U
#define VOLTAGE_CURRENT_STATUS_FLASHSYNC_STALE     0x02U
#define VOLTAGE_CURRENT_STATUS_OUTPUT_VERIFY_FAULT 0x04U
#define VOLTAGE_CURRENT_STATUS_CAN_RX_FAULT        0x08U
#define VOLTAGE_CURRENT_STATUS_CAN_TX_FAULT        0x10U
#define VOLTAGE_CURRENT_STATUS_STORAGE_FAULT       0x20U
#define VOLTAGE_CURRENT_STATUS_CURRENT_SATURATED   0x40U
#define VOLTAGE_CURRENT_STATUS_INVALID_COMMAND     0x80U

typedef struct
{
  tSSignalOutputImage SVoltageImage;           /* 12 observed channel states */
  tSCurrentMeasurementWire SCurrentWire;       /* 4 × 10-bit currents */
  uint8_t bStatus;                             /* VOLTAGE_CURRENT_STATUS_* */
} tSVoltageCurrentFrameInputs;

/**
 * @brief Encode the telemetry frame as VOLTAGE_CURRENT_FRAME_BYTES bytes.
 *        Output bytes match the legacy __packed struct layout byte-for-byte.
 */
void VoltageCurrentFrame_Encode(const tSVoltageCurrentFrameInputs *pIn,
                                uint8_t pOutBytes[VOLTAGE_CURRENT_FRAME_BYTES]);

#endif /* DOMAIN_VOLTAGE_CURRENT_FRAME_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
