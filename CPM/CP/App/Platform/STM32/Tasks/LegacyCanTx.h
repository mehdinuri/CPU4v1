/* App/Platform/STM32/Tasks/LegacyCanTx.h
 *
 * Narrow declaration seam for the remaining legacy CAN transmit helper.
 */
#ifndef LEGACY_CAN_TX_H
#define LEGACY_CAN_TX_H

#include <stdint.h>

#define LEGACY_CAN_ID_TYPE_STD 0x01U
#define LEGACY_CAN_ID_TYPE_EXT 0x02U

void CANTxRequest(uint8_t bDLC, uint8_t bIdType, uint16_t sMID, uint8_t *baData);

#endif /* LEGACY_CAN_TX_H */
