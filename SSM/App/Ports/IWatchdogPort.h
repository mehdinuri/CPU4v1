/**
 ******************************************************************************
 * @file    Ports/IWatchdogPort.h
 * @brief   Port: kick the independent watchdog. No other behaviour — kept
 *          deliberately tiny so task wrappers can substitute a mock.
 ******************************************************************************
 */

#ifndef PORTS_IWATCHDOG_PORT_H
#define PORTS_IWATCHDOG_PORT_H

typedef struct IWatchdogPort
{
  void *pCtx;

  void (*Refresh)(void *pCtx);
} IWatchdogPort_t;

static inline void Watchdog_Refresh(IWatchdogPort_t *pPort)
{
  pPort->Refresh(pPort->pCtx);
}

#endif /* PORTS_IWATCHDOG_PORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
