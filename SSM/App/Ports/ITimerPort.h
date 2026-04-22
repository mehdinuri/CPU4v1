/**
 ******************************************************************************
 * @file    Ports/ITimerPort.h
 * @brief   Port: start timers that run the measurement/sampling cadence.
 *          One call per timer; adapters resolve TimerId_e → hardware timer.
 ******************************************************************************
 */

#ifndef PORTS_ITIMER_PORT_H
#define PORTS_ITIMER_PORT_H

typedef enum
{
  TIMER_ID_GRID_FREQ_CAPTURE = 0,      /* TIM2 input capture (grid period)   */
  TIMER_ID_GRID_FREQ_COMPARE = 1,      /* TIM2 output compare (grid edge IT) */
  TIMER_ID_ADC_TRIGGER       = 2,      /* TIM3 TRGO (drives ADC DMA)         */
  TIMER_ID_STATE_MONITOR     = 3,      /* TIM4 OC (sample-interval IT)       */
  TIMER_ID__COUNT
} TimerId_e;

typedef struct ITimerPort
{
  void *ctx;

  void (*Start)(void *ctx, TimerId_e eId);
} ITimerPort_t;

static inline void Timer_Start(ITimerPort_t *port, TimerId_e eId)
{
  port->Start(port->ctx, eId);
}

#endif /* PORTS_ITIMER_PORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
