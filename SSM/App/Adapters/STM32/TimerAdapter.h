/**
 ******************************************************************************
 * @file    Adapters/STM32/TimerAdapter.h
 * @brief   STM32 adapter for ITimerPort. Each TimerId_e maps to one of the
 *          existing Tim*Start* wrappers from Core/Src/tim.c.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_TIMER_ADAPTER_H
#define ADAPTERS_STM32_TIMER_ADAPTER_H

#include <stdint.h>
#include "Ports/ITimerPort.h"

typedef struct
{
  uint8_t reserved;
} TimerAdapterCtx_t;

void TimerAdapter_Init(TimerAdapterCtx_t *ctx);
ITimerPort_t TimerAdapter_CreatePort(TimerAdapterCtx_t *ctx);

#endif /* ADAPTERS_STM32_TIMER_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
