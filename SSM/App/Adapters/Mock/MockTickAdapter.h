/**
 ******************************************************************************
 * @file    Adapters/Mock/MockTickAdapter.h
 * @brief   Test double for ITickPort — controllable "now" for deterministic tests.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_TICK_ADAPTER_H
#define ADAPTERS_MOCK_TICK_ADAPTER_H

#include <stdint.h>
#include "Ports/ITickPort.h"

typedef struct
{
  uint32_t lNow_ms;
} tSMockTickAdapterCtx;

void MockTickAdapter_Init(tSMockTickAdapterCtx *pCtx);
void MockTickAdapter_SetNow(tSMockTickAdapterCtx *pCtx, uint32_t lNow_ms);
ITickPort_t MockTickAdapter_CreatePort(tSMockTickAdapterCtx *pCtx);

#endif /* ADAPTERS_MOCK_TICK_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
