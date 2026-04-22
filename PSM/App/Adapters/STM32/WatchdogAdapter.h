/**
 ******************************************************************************
 * @file    Adapters/STM32/WatchdogAdapter.h
 * @brief   STM32 adapter for IWatchdogPort — wraps IWDGRefresh() from
 *          Core/Src/iwdg.c, which already handles the DEBUG-build no-op.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_WATCHDOGADAPTER_H
#define ADAPTERS_STM32_WATCHDOGADAPTER_H

#include "Ports/IWatchdogPort.h"

typedef struct
{
  uint8_t initialised;
} WatchdogAdapterCtx_t;

void            WatchdogAdapterInit(WatchdogAdapterCtx_t *ctx);
IWatchdogPort_t WatchdogAdapterCreatePort(WatchdogAdapterCtx_t *ctx);

#endif /* ADAPTERS_STM32_WATCHDOGADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
