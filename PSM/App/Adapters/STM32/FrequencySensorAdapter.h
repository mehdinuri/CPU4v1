/**
 ******************************************************************************
 * @file    Adapters/STM32/FrequencySensorAdapter.h
 * @brief   STM32 adapter for IFrequencySensorPort.
 *          The bFrequency field is written from ISR context via the inline
 *          setter below.  main_stm32.c provides the globally-named ISR
 *          wrapper that forwards to this setter.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_FREQUENCYSENSORADAPTER_H
#define ADAPTERS_STM32_FREQUENCYSENSORADAPTER_H

#include "Ports/IFrequencySensorPort.h"

/* bFrequency is written from ISR context; volatile prevents the compiler from
 * caching the value in a register across the ISR boundary. */
typedef struct
{
  volatile uint8_t bFrequency;
} FrequencySensorAdapterCtx_t;

void                    FrequencySensorAdapterInit(FrequencySensorAdapterCtx_t *ctx);
IFrequencySensorPort_t  FrequencySensorAdapterCreatePort(FrequencySensorAdapterCtx_t *ctx);

/* ---------------------------------------------------------------------------
 * ISR-safe setter — called from ISR wrapper in main_stm32.c.
 * ---------------------------------------------------------------------------*/
static inline void FrequencySensorAdapter_SetNetFrequency(
    FrequencySensorAdapterCtx_t *ctx, uint8_t bFreq)
{
  ctx->bFrequency = bFreq;
}

#endif /* ADAPTERS_STM32_FREQUENCYSENSORADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
