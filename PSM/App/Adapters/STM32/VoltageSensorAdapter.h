/**
 ******************************************************************************
 * @file    Adapters/STM32/VoltageSensorAdapter.h
 * @brief   STM32 adapter for IVoltageSensorPort.
 *          The three volatile float fields are written from ISR context via
 *          the inline setters below.  main_stm32.c provides the globally-named
 *          ISR wrapper functions that forward to these setters.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_VOLTAGESENSORADAPTER_H
#define ADAPTERS_STM32_VOLTAGESENSORADAPTER_H

#include "Ports/IVoltageSensorPort.h"

/* Context fields are written from ISR context and read by MeasurementTask.
 * Each float is 32-bit word-aligned on Cortex-M4 so individual reads/writes
 * are atomic, but volatile is required to prevent the compiler from caching
 * values in registers across the ISR boundary. */
typedef struct
{
  volatile float fNetVoltage;
  volatile float fRegVIn;
  volatile float fRegVOut;
} VoltageSensorAdapterCtx_t;

void                   VoltageSensorAdapterInit(VoltageSensorAdapterCtx_t *ctx);
IVoltageSensorPort_t   VoltageSensorAdapterCreatePort(VoltageSensorAdapterCtx_t *ctx);

/* ---------------------------------------------------------------------------
 * ISR-safe setters — the adapter owns its fields, so setters live here.
 * Called from ISR wrappers in main_stm32.c (which owns the adapter context).
 * ---------------------------------------------------------------------------*/
static inline void VoltageSensorAdapter_SetNetVoltage(
    VoltageSensorAdapterCtx_t *ctx, float fVolt)
{
  ctx->fNetVoltage = fVolt;
}

static inline void VoltageSensorAdapter_SetRegVIn(
    VoltageSensorAdapterCtx_t *ctx, float fVIn)
{
  ctx->fRegVIn = fVIn;
}

static inline void VoltageSensorAdapter_SetRegVOut(
    VoltageSensorAdapterCtx_t *ctx, float fVOut)
{
  ctx->fRegVOut = fVOut;
}

#endif /* ADAPTERS_STM32_VOLTAGESENSORADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
