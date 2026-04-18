/**
 ******************************************************************************
 * @file    Adapters/STM32/HalTickAdapter.c
 * @brief   STM32 adapter for ITickPort.
 ******************************************************************************
 */

#include "Adapters/STM32/HalTickAdapter.h"
#include "stm32g4xx_hal.h"

static uint32_t AdapterNow_ms(void *pCtx)
{
  (void) pCtx;

  return HAL_GetTick();
}

void HalTickAdapter_Init(tSHalTickAdapterCtx *pCtx)
{
  pCtx->bReserved = 0U;
}

ITickPort_t HalTickAdapter_CreatePort(tSHalTickAdapterCtx *pCtx)
{
  ITickPort_t port;

  port.pCtx = pCtx;
  port.Now_ms = AdapterNow_ms;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
