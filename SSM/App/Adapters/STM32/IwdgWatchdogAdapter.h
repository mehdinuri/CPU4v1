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
  uint8_t bReserved;
} tSIwdgWatchdogAdapterCtx;

void IwdgWatchdogAdapter_Init(tSIwdgWatchdogAdapterCtx *pCtx);
IWatchdogPort_t IwdgWatchdogAdapter_CreatePort(tSIwdgWatchdogAdapterCtx *pCtx);

#endif /* ADAPTERS_STM32_IWDG_WATCHDOG_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
