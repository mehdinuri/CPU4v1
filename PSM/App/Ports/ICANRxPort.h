/**
 ******************************************************************************
 * @file    Ports/ICANRxPort.h
 * @brief   Port interface for inbound CAN frame delivery.
 *          Called from the FDCAN RxFifo0 ISR to queue frames for the
 *          parser task.  STM32 adapter owns the memory pool and message
 *          queue; mock records submitted frames for test inspection.
 ******************************************************************************
 */

#ifndef PORTS_ICANRXPORT_H
#define PORTS_ICANRXPORT_H

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Generic CAN frame descriptor — kept minimal so the port does not leak
 * HAL typedefs into the domain.  Adapters translate to/from their native
 * frame representation (FdcanRxMsg_t on STM32).
 * ---------------------------------------------------------------------------*/
#ifndef CANRX_MAX_DATA_LEN
#define CANRX_MAX_DATA_LEN 64U
#endif

typedef struct
{
  uint32_t id;                          /* 11-bit or 29-bit identifier      */
  uint8_t  extendedId;                  /* 0 = standard, 1 = extended       */
  uint8_t  dataLen;                     /* number of valid payload bytes    */
  uint8_t  data[CANRX_MAX_DATA_LEN];   /* payload                          */
} CanRxFrame_t;

/* ---------------------------------------------------------------------------
 * Port interface
 * ---------------------------------------------------------------------------*/
typedef struct
{
  void *ctx;
  void (*SubmitFrame)(void *ctx, const CanRxFrame_t *frame);
} ICANRxPort_t;

/* ---------------------------------------------------------------------------
 * Zero-cost inline dispatch helper
 * ---------------------------------------------------------------------------*/
static inline void CanRx_SubmitFrame(ICANRxPort_t *p,
                                      const CanRxFrame_t *frame)
{
  p->SubmitFrame(p->ctx, frame);
}

#endif /* PORTS_ICANRXPORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
