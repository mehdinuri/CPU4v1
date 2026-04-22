/**
 ******************************************************************************
 * @file    Adapters/STM32/TimerAdapter.c
 ******************************************************************************
 */

#include "Adapters/STM32/TimerAdapter.h"
#include "tim.h"

static void AdapterStart(void *ctx, TimerId_e eId)
{
  (void) ctx;

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

void TimerAdapter_Init(TimerAdapterCtx_t *ctx)
{
  ctx->reserved = 0U;
}

ITimerPort_t TimerAdapter_CreatePort(TimerAdapterCtx_t *ctx)
{
  ITimerPort_t port;

  port.ctx = ctx;
  port.Start = AdapterStart;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
