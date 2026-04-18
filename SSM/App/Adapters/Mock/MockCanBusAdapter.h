/**
 ******************************************************************************
 * @file    Adapters/Mock/MockCanBusAdapter.h
 * @brief   Test double for ICanBusPort. Records every SendStd in an array
 *          so tests can assert on the sequence/content of sent frames.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_CAN_BUS_ADAPTER_H
#define ADAPTERS_MOCK_CAN_BUS_ADAPTER_H

#include <stdint.h>
#include "Ports/ICanBusPort.h"

#define MOCK_CAN_MAX_RECORDED 16U

typedef struct
{
  tECanBusId eBus;
  tSCanFrame SFrame;
} tSMockCanSentEntry;

typedef struct
{
  tSMockCanSentEntry aSent[MOCK_CAN_MAX_RECORDED];
  uint8_t bSentCount;               /* saturates at MOCK_CAN_MAX_RECORDED */
  uint8_t bForceSendFail;           /* 1 = SendStd returns 0 */
} tSMockCanBusAdapterCtx;

void MockCanBusAdapter_Init(tSMockCanBusAdapterCtx *pCtx);
ICanBusPort_t MockCanBusAdapter_CreatePort(tSMockCanBusAdapterCtx *pCtx);

#endif /* ADAPTERS_MOCK_CAN_BUS_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
