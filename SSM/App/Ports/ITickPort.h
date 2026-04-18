/**
 ******************************************************************************
 * @file    Ports/ITickPort.h
 * @brief   Port: free-running millisecond tick (monotonic, may wrap).
 *          Use sparingly from domain — prefer taking a tick parameter.
 ******************************************************************************
 */

#ifndef PORTS_ITICK_PORT_H
#define PORTS_ITICK_PORT_H

#include <stdint.h>

typedef struct ITickPort
{
  void *pCtx;

  uint32_t (*Now_ms)(void *pCtx);
} ITickPort_t;

static inline uint32_t Tick_Now_ms(ITickPort_t *pPort)
{
  return pPort->Now_ms(pPort->pCtx);
}

#endif /* PORTS_ITICK_PORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
