/**
 ******************************************************************************
 * @file    Adapters/STM32/CANRxAdapter.c
 * @brief   STM32 adapter for ICANRxPort — forwards neutral frames into the
 *          existing CANRxRequest() memory-pool + queue path.
 ******************************************************************************
 */

#include "CANRxAdapter.h"
#include "CanMsgParser.h"
#include "fdcan.h"

#include <string.h>

/* ---------------------------------------------------------------------------
 * Private adapter implementation
 * ---------------------------------------------------------------------------*/
static void AdapterSubmitFrame(void *ctx, const tSCANRxFrame *pFrame)
{
  (void) ctx;

  if (pFrame == NULL)
  {
    return;
  }

  tSFDCANRxMsg SMsg;
  memset(&SMsg, 0, sizeof(SMsg));

  SMsg.hfdcan                = &hfdcan1;
  SMsg.SRxHeader.IdType      = (pFrame->fExtendedId != 0U)
                                ? FDCAN_EXTENDED_ID
                                : FDCAN_STANDARD_ID;
  SMsg.SRxHeader.Identifier  = pFrame->lId;
  SMsg.SRxHeader.DataLength  = pFrame->bDataLen;

  uint8_t bCopy = (pFrame->bDataLen > CANRX_MAX_DATA_LEN)
                ? CANRX_MAX_DATA_LEN
                : pFrame->bDataLen;
  if (bCopy > (uint8_t) sizeof(SMsg.baData))
  {
    bCopy = (uint8_t) sizeof(SMsg.baData);
  }
  memcpy(SMsg.baData, pFrame->baData, bCopy);

  CANRxRequest(&SMsg);
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void CANRxAdapterInit(CANRxAdapterCtx_t *ctx)
{
  ctx->bInitialised = 1U;
}

ICANRxPort_t CANRxAdapterCreatePort(CANRxAdapterCtx_t *ctx)
{
  ICANRxPort_t port;
  port.ctx          = ctx;
  port.SubmitFrame  = AdapterSubmitFrame;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
