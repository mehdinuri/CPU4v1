/**
 ******************************************************************************
 * @file    Adapters/Mock/MockTimerAdapter.h
 * @brief   Test double for ITimerPort. Records every Start() call.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_TIMER_ADAPTER_H
#define ADAPTERS_MOCK_TIMER_ADAPTER_H

#include <stdint.h>
#include "Ports/ITimerPort.h"

typedef struct
{
  uint32_t aStartCount[TIMER_ID__COUNT];
} tSMockTimerAdapterCtx;

void MockTimerAdapter_Init(tSMockTimerAdapterCtx *pCtx);
ITimerPort_t MockTimerAdapter_CreatePort(tSMockTimerAdapterCtx *pCtx);

#endif /* ADAPTERS_MOCK_TIMER_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
