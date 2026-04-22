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
  uint32_t nowMs;
} MockTickAdapterCtx_t;

void MockTickAdapter_Init(MockTickAdapterCtx_t *ctx);
void MockTickAdapter_SetNow(MockTickAdapterCtx_t *ctx, uint32_t nowMs);
ITickPort_t MockTickAdapter_CreatePort(MockTickAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_TICK_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
