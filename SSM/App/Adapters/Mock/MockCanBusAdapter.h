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
  CanBusId_e eBus;
  CanFrame_t frame;
} MockCanSentEntry_t;

typedef struct
{
  MockCanSentEntry_t sent[MOCK_CAN_MAX_RECORDED];
  uint8_t sentCount;               /* saturates at MOCK_CAN_MAX_RECORDED */
  uint8_t forceSendFail;           /* 1 = SendStd returns 0 */
} MockCanBusAdapterCtx_t;

void MockCanBusAdapter_Init(MockCanBusAdapterCtx_t *ctx);
ICanBusPort_t MockCanBusAdapter_CreatePort(MockCanBusAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_CAN_BUS_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
