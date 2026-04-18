/**
 ******************************************************************************
 * @file    Ports/ICANTxPort.h
 * @brief   Port interface for CAN frame transmission.
 *          STM32: wraps CANTxRequest() on FDCAN1 (classic CAN, standard ID).
 *          Host: records last frame sent for inspection.
 ******************************************************************************
 */

#ifndef PORTS_ICANTXPORT_H
#define PORTS_ICANTXPORT_H

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Port interface
 * ---------------------------------------------------------------------------*/
typedef struct
{
  void *ctx;
  void (*Send)(void *ctx,
               uint32_t lID,
               const uint8_t *baData,
               uint8_t bDataLen);
} ICANTxPort_t;

/* ---------------------------------------------------------------------------
 * Zero-cost inline dispatch helper
 * ---------------------------------------------------------------------------*/
static inline void CANTx_Send(ICANTxPort_t *p,
                               uint32_t lID,
                               const uint8_t *baData,
                               uint8_t bDataLen)
{
  p->Send(p->ctx, lID, baData, bDataLen);
}

#endif /* PORTS_ICANTXPORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
