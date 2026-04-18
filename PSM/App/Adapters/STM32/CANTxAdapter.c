/**
 ******************************************************************************
 * @file    Adapters/STM32/CANTxAdapter.c
 * @brief   STM32 adapter for ICANTxPort — wraps CANTxRequest() on FDCAN1
 *          with fixed classic-CAN / standard-ID / no-BRS settings.
 ******************************************************************************
 */

#include "CANTxAdapter.h"
#include "fdcan.h"
#include "CanMsgSender.h"

/* ---------------------------------------------------------------------------
 * Private adapter implementation
 * ---------------------------------------------------------------------------*/
static void AdapterSend(void *ctx,
                         uint32_t lID,
                         const uint8_t *baData,
                         uint8_t bDataLen)
{
  (void) ctx;
  CANTxRequest(&hfdcan1,
               FDCAN_STANDARD_ID,
               lID,
               FDCAN_DATA_FRAME,
               FDCAN_BRS_OFF,
               FDCAN_CLASSIC_CAN,
               (uint8_t *)(uintptr_t) baData,
               bDataLen);
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void CANTxAdapterInit(CANTxAdapterCtx_t *ctx)
{
  ctx->bInitialised = 1U;
}

ICANTxPort_t CANTxAdapterCreatePort(CANTxAdapterCtx_t *ctx)
{
  ICANTxPort_t port;
  port.ctx  = ctx;
  port.Send = AdapterSend;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
