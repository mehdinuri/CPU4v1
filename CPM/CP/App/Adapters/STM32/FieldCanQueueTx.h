/* App/Adapters/STM32/FieldCanQueueTx.h
 *
 * Queue-backed FDCAN1 transmit helper for the active field-bus adapters.
 * This keeps legacy wire bytes unchanged while removing dependencies on the
 * legacy parser/task helpers.
 */
#ifndef FIELD_CAN_QUEUE_TX_H
#define FIELD_CAN_QUEUE_TX_H

#include <stdint.h>

uint8_t FieldCanQueueTxSendStandard(uint16_t identifier,
                                    const uint8_t *data,
                                    uint8_t length);

#endif /* FIELD_CAN_QUEUE_TX_H */
