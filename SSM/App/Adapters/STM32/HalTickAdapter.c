/**
 ******************************************************************************
 * @file    Adapters/STM32/HalTickAdapter.c
 * @brief   STM32 adapter for ITickPort.
 ******************************************************************************
 */

#include "Adapters/STM32/HalTickAdapter.h"
#include "stm32g4xx_hal.h"

static uint32_t AdapterNow_ms(void *ctx)
{
  (void) ctx;

  return HAL_GetTick();
}

void HalTickAdapter_Init(HalTickAdapterCtx_t *ctx)
{
  ctx->reserved = 0U;
}

ITickPort_t HalTickAdapter_CreatePort(HalTickAdapterCtx_t *ctx)
{
  ITickPort_t port;

  port.ctx = ctx;
  port.Now_ms = AdapterNow_ms;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
