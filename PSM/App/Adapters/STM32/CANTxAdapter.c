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

extern volatile uint32_t g_canTxOverflowCount;

/* ---------------------------------------------------------------------------
 * Private adapter implementation
 * ---------------------------------------------------------------------------*/
static void AdapterSend(void *ctx,
                         uint32_t id,
                         const uint8_t *data,
                         uint8_t dataLen)
{
  (void) ctx;
  CANTxRequest(&hfdcan1,
               FDCAN_STANDARD_ID,
               id,
               FDCAN_DATA_FRAME,
               FDCAN_BRS_OFF,
               FDCAN_CLASSIC_CAN,
               (uint8_t *)(uintptr_t) data,
               dataLen);
}

static uint8_t AdapterGetOverflowCount(void *ctx)
{
  (void) ctx;
  /* Port contract is "non-zero means overflow has happened" — saturate the
   * widened 32-bit counter into the uint8 the measurement frame expects. */
  return (g_canTxOverflowCount > 0U) ? 0xFFU : 0U;
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void CANTxAdapterInit(CANTxAdapterCtx_t *ctx)
{
  ctx->initialised = 1U;
}

ICANTxPort_t CANTxAdapterCreatePort(CANTxAdapterCtx_t *ctx)
{
  ICANTxPort_t port;
  port.ctx              = ctx;
  port.Send             = AdapterSend;
  port.GetOverflowCount = AdapterGetOverflowCount;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
