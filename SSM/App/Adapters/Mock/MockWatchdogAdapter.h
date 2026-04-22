/**
 ******************************************************************************
 * @file    Adapters/Mock/MockWatchdogAdapter.h
 * @brief   Test double for IWatchdogPort — counts Refresh() calls.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_WATCHDOG_ADAPTER_H
#define ADAPTERS_MOCK_WATCHDOG_ADAPTER_H

#include <stdint.h>
#include "Ports/IWatchdogPort.h"

typedef struct
{
  uint32_t refreshCount;
} MockWatchdogAdapterCtx_t;

void MockWatchdogAdapter_Init(MockWatchdogAdapterCtx_t *ctx);
IWatchdogPort_t MockWatchdogAdapter_CreatePort(MockWatchdogAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_WATCHDOG_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
