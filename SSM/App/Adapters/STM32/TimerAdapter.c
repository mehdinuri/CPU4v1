/**
 ******************************************************************************
 * @file    Adapters/STM32/TimerAdapter.c
 ******************************************************************************
 */

#include "Adapters/STM32/TimerAdapter.h"
#include "tim.h"

static void AdapterStart(void *pCtx, tETimerId eId)
{
  (void) pCtx;

  switch (eId)
  {
      case TIMER_ID_GRID_FREQ_CAPTURE:
      {
        Tim2ICStartIT();
        break;
      }

      case TIMER_ID_GRID_FREQ_COMPARE:
      {
        Tim2OCStartIT();
        break;
      }

      case TIMER_ID_ADC_TRIGGER:
      {
        Tim3Start();
        break;
      }

      case TIMER_ID_STATE_MONITOR:
      {
        Tim4OCStartIT();
        break;
      }

      default:
      {
        break;
      }
  }
}

void TimerAdapter_Init(tSTimerAdapterCtx *pCtx)
{
  pCtx->bReserved = 0U;
}

ITimerPort_t TimerAdapter_CreatePort(tSTimerAdapterCtx *pCtx)
{
  ITimerPort_t port;

  port.pCtx = pCtx;
  port.Start = AdapterStart;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
