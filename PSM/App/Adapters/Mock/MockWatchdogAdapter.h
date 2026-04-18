/**
 ******************************************************************************
 * @file    Adapters/Mock/MockWatchdogAdapter.h
 * @brief   Mock adapter for IWatchdogPort — records Refresh() calls.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_MOCKWATCHDOGADAPTER_H
#define ADAPTERS_MOCK_MOCKWATCHDOGADAPTER_H

#include "Ports/IWatchdogPort.h"

typedef struct
{
  uint32_t lRefreshCount;
} MockWatchdogAdapterCtx_t;

void            MockWatchdogAdapterInit(MockWatchdogAdapterCtx_t *ctx);
IWatchdogPort_t MockWatchdogAdapterCreatePort(MockWatchdogAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_MOCKWATCHDOGADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
