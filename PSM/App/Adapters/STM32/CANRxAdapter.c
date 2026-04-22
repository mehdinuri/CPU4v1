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
static void AdapterSubmitFrame(void *ctx, const CanRxFrame_t *frame)
{
  (void) ctx;

  if (frame == NULL)
  {
    return;
  }

  FdcanRxMsg_t msg;
  memset(&msg, 0, sizeof(msg));

  msg.hfdcan                = &hfdcan1;
  msg.rxHeader.IdType      = (frame->extendedId != 0U)
                                ? FDCAN_EXTENDED_ID
                                : FDCAN_STANDARD_ID;
  msg.rxHeader.Identifier  = frame->id;
  msg.rxHeader.DataLength  = frame->dataLen;

  uint8_t copy = (frame->dataLen > CANRX_MAX_DATA_LEN)
                ? CANRX_MAX_DATA_LEN
                : frame->dataLen;
  if (copy > (uint8_t) sizeof(msg.data))
  {
    copy = (uint8_t) sizeof(msg.data);
  }
  memcpy(msg.data, frame->data, copy);

  CANRxRequest(&msg);
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void CANRxAdapterInit(CANRxAdapterCtx_t *ctx)
{
  ctx->initialised = 1U;
}

ICANRxPort_t CANRxAdapterCreatePort(CANRxAdapterCtx_t *ctx)
{
  ICANRxPort_t port;
  port.ctx          = ctx;
  port.SubmitFrame  = AdapterSubmitFrame;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
