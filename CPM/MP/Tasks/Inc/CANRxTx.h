#ifndef __CANRXTX_H__
#define __CANRXTX_H__

#include "data.h"

/*
 *  Public Data
 */

/*
 *  Public Methods
 */
extern uint8_t CANGetRxDataLength(uint32_t lCode);
extern void CanRxRequest(tpSCanRxMsg pSRxMsg);
extern void CanTxRequest(FDCAN_HandleTypeDef *hfdcan,
                         uint32_t lIDType,
                         uint32_t lID,
                         uint32_t lFrameType,
                         uint32_t lBitRateSwitch,
                         uint32_t lFDFormat,
                         uint8_t *baData,
                         uint8_t bDataLen);

#endif /* ifndef __CANRXTX_H__ */
