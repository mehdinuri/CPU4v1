/**
 ******************************************************************************
 * @file    Ports/IWatchdogPort.h
 * @brief   Port interface for independent-watchdog refresh.
 *          STM32 adapter calls HAL_IWDG_Refresh() (no-op in DEBUG builds).
 *          Mock increments a refresh counter for tests.
 ******************************************************************************
 */

#ifndef PORTS_IWATCHDOGPORT_H
#define PORTS_IWATCHDOGPORT_H

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Port interface
 * ---------------------------------------------------------------------------*/
typedef struct
{
  void *ctx;
  void (*Refresh)(void *ctx);
} IWatchdogPort_t;

/* ---------------------------------------------------------------------------
 * Zero-cost inline dispatch helper
 * ---------------------------------------------------------------------------*/
static inline void Watchdog_Refresh(IWatchdogPort_t *p)
{
  p->Refresh(p->ctx);
}

#endif /* PORTS_IWATCHDOGPORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
