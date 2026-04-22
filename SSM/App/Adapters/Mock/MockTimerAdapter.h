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
  uint32_t startCount[TIMER_ID__COUNT];
} MockTimerAdapterCtx_t;

void MockTimerAdapter_Init(MockTimerAdapterCtx_t *ctx);
ITimerPort_t MockTimerAdapter_CreatePort(MockTimerAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_TIMER_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
