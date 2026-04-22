/**
 ******************************************************************************
 * @file    Adapters/STM32/IwdgWatchdogAdapter.c
 * @brief   STM32 adapter for IWatchdogPort.
 ******************************************************************************
 */

#include "Adapters/STM32/IwdgWatchdogAdapter.h"
#include "iwdg.h"

static void AdapterRefresh(void *ctx)
{
  (void) ctx;
  IWDGRefresh();
}

void IwdgWatchdogAdapter_Init(IwdgWatchdogAdapterCtx_t *ctx)
{
  ctx->reserved = 0U;

  /* MX_IWDG_Init() is called from main() before the scheduler starts;
   * this adapter does not initialise the peripheral itself.
   */
}

IWatchdogPort_t IwdgWatchdogAdapter_CreatePort(IwdgWatchdogAdapterCtx_t *ctx)
{
  IWatchdogPort_t port;

  port.ctx = ctx;
  port.Refresh = AdapterRefresh;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
