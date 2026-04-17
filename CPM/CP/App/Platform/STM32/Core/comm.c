/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "comm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  members */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  methods */
void CommResetDMABuffer(tpSComm pSComm)
{
  memset(pSComm->pbDMARxBuffer, 0, pSComm->sDMARxBufferSize);
  if (pSComm->pbDMAMainBuffer != NULL)
  {
    memset(pSComm->pbDMAMainBuffer, 0, pSComm->sDMAMainBufferSize);

    memset(&pSComm->SDMAMainBufferParams, 0,
           sizeof(pSComm->SDMAMainBufferParams));
  }

  memset(&pSComm->SFlags, 0, sizeof(pSComm->SFlags));
}

void CommUARTReceiveToIdleDMA(tpSComm pSComm)
{
  pSComm->SFlags.fDataAvailable = FALSE;
  HAL_UARTEx_ReceiveToIdle_DMA(pSComm->pSUARTHandler,
                               pSComm->pbDMARxBuffer,
                               pSComm->sDMARxBufferSize);
  __HAL_DMA_DISABLE_IT(pSComm->pSDMAHandler, DMA_IT_HT);
}

void CommInitDMABuffer(tpSComm pSComm)
{
  CommResetDMABuffer(pSComm);
  CommUARTReceiveToIdleDMA(pSComm);
}
