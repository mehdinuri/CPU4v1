/**
 ******************************************************************************
 * @file    Ports/ICanBusPort.h
 * @brief   Port: send a standard-ID CAN frame on one of the two FDCAN buses.
 *          RX path is still routed through Tasks/Src/can_msg_parser.c today;
 *          a future phase will add an Rx callback slot to this port.
 ******************************************************************************
 */

#ifndef PORTS_ICAN_BUS_PORT_H
#define PORTS_ICAN_BUS_PORT_H

#include <stdint.h>

typedef enum
{
  CAN_BUS_FDCAN1 = 0,      /* Active field bus: classic CAN, standard IDs */
  CAN_BUS_FDCAN2 = 1,      /* Reserved for future expansion */
  CAN_BUS__COUNT
} tECanBusId;

typedef struct
{
  uint16_t sStdId;
  uint8_t bLen;              /* 0..8 */
  uint8_t abData[8];
} tSCanFrame;

typedef struct ICanBusPort
{
  void *pCtx;

  /* Returns 1 if the frame was accepted for transmit, 0 otherwise. */
  uint8_t (*SendStd)(void *pCtx, tECanBusId eBus, const tSCanFrame *pFrame);
} ICanBusPort_t;

static inline uint8_t CanBus_SendStd(ICanBusPort_t *pPort,
                                     tECanBusId eBus,
                                     const tSCanFrame *pFrame)
{
  return pPort->SendStd(pPort->pCtx, eBus, pFrame);
}

#endif /* PORTS_ICAN_BUS_PORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
