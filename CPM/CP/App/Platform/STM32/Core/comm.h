#ifndef __COMM_H
#define __COMM_H

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "usart.h"
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  definitions */

/*  comm structure */
typedef struct _tSComm
{
  UART_HandleTypeDef *pSUARTHandler;
  DMA_HandleTypeDef *pSDMAHandler;

  uint8_t *pbDMARxBuffer;
  uint16_t sDMARxBufferSize;

  uint8_t *pbDMAMainBuffer;
  uint16_t sDMAMainBufferSize;

  struct
  {
    uint16_t sHead;
    uint16_t sTail;
    uint16_t sOldPosition;
    uint16_t sNewPosition;
  } SDMAMainBufferParams;

  struct
  {
    uint8_t fDataAvailable : 1;
    uint8_t fReserved : 6;
  } SFlags;
} tSComm, *tpSComm;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  public methods */
extern void CommResetDMABuffer(tpSComm pSComm);
extern void CommInitDMABuffer(tpSComm pSComm);
extern void CommUARTReceiveToIdleDMA(tpSComm pSComm);

#endif /* ifndef __COMM_H */
