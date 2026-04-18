/**
 ******************************************************************************
 * @file    Adapters/STM32/IwdgWatchdogAdapter.c
 * @brief   STM32 adapter for IWatchdogPort.
 ******************************************************************************
 */

#include "Adapters/STM32/IwdgWatchdogAdapter.h"
#include "iwdg.h"

static void AdapterRefresh(void *pCtx)
{
  (void) pCtx;
  IWDGRefresh();
}

void IwdgWatchdogAdapter_Init(tSIwdgWatchdogAdapterCtx *pCtx)
{
  pCtx->bReserved = 0U;

  /* MX_IWDG_Init() is called from main() before the scheduler starts;
   * this adapter does not initialise the peripheral itself.
   */
}

IWatchdogPort_t IwdgWatchdogAdapter_CreatePort(tSIwdgWatchdogAdapterCtx *pCtx)
{
  IWatchdogPort_t port;

  port.pCtx = pCtx;
  port.Refresh = AdapterRefresh;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
