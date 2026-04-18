/**
 ******************************************************************************
 * @file    Ports/ISignalInputPort.h
 * @brief   Port interface for digital signal inputs (ACOK, DCOK, COMMOK pins).
 *          Returns 1 (pin high / GPIO_PIN_SET) or 0 (pin low / GPIO_PIN_RESET).
 *          Implementations: SignalInputAdapter (STM32), MockSignalInputAdapter (Host).
 ******************************************************************************
 */

#ifndef PORTS_ISIGNALINPUTPORT_H
#define PORTS_ISIGNALINPUTPORT_H

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Input identifiers
 * ---------------------------------------------------------------------------*/
typedef enum
{
  SIGNAL_IN_ACOK   = 0,
  SIGNAL_IN_DCOK   = 1,
  SIGNAL_IN_COMMOK = 2
} SignalInputId_t;

/* ---------------------------------------------------------------------------
 * Port interface
 * ---------------------------------------------------------------------------*/
typedef struct
{
  void *ctx;
  /* Returns 1 if pin is high (SET), 0 if low (RESET) */
  uint8_t (*GetState)(void *ctx, SignalInputId_t id);
} ISignalInputPort_t;

/* ---------------------------------------------------------------------------
 * Zero-cost inline dispatch helper
 * ---------------------------------------------------------------------------*/
static inline uint8_t SignalInput_GetState(ISignalInputPort_t *p,
                                            SignalInputId_t id)
{
  return p->GetState(p->ctx, id);
}

#endif /* PORTS_ISIGNALINPUTPORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
