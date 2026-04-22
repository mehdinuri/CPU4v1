/**
 ******************************************************************************
 * @file    Adapters/STM32/IwdgWatchdogAdapter.h
 * @brief   STM32 adapter for IWatchdogPort — wraps IWDGRefresh().
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_IWDG_WATCHDOG_ADAPTER_H
#define ADAPTERS_STM32_IWDG_WATCHDOG_ADAPTER_H

#include <stdint.h>
#include "Ports/IWatchdogPort.h"

typedef struct
{
  uint8_t reserved;
} IwdgWatchdogAdapterCtx_t;

void IwdgWatchdogAdapter_Init(IwdgWatchdogAdapterCtx_t *ctx);
IWatchdogPort_t IwdgWatchdogAdapter_CreatePort(IwdgWatchdogAdapterCtx_t *ctx);

#endif /* ADAPTERS_STM32_IWDG_WATCHDOG_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
