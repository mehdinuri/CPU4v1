/**
 ******************************************************************************
 * @file    Adapters/STM32/CanBusAdapter.c
 ******************************************************************************
 */

#include "Adapters/STM32/CanBusAdapter.h"
#include "fdcan.h"
#include "can_msg_sender.h"

static FDCAN_HandleTypeDef *HandleFor(CanBusId_e eBus)
{
  switch (eBus)
  {
      case CAN_BUS_FDCAN1:
      {
        return &hfdcan1;
      }

      case CAN_BUS_FDCAN2:
      {
        /* Reserved bus. FDCAN2 is intentionally not initialised in SSM's
         * current deployment profile, so reject attempts to use it.
         */
        return 0;
      }

      default:
      {
        return 0;
      }
  }
}

static uint8_t AdapterSendStd(void *ctx,
                              CanBusId_e eBus,
                              const CanFrame_t *frame)
{
  (void) ctx;

  FDCAN_HandleTypeDef *handle = HandleFor(eBus);

  if (handle == 0)
  {
    return 0U;
  }

  if (frame->len > 8U)
  {
    return 0U;
  }

  /* CANTxRequest's data is non-const for historical reasons; the enqueue
   * path memcpy's before returning, so casting away const is safe.
   */
  return CANTxRequest(handle,
                      FDCAN_STANDARD_ID,
                      (uint32_t) frame->stdId,
                      FDCAN_DATA_FRAME,
                      FDCAN_BRS_OFF,
                      FDCAN_CLASSIC_CAN,
                      frame->abData,
                      frame->len);
}

void CanBusAdapter_Init(CanBusAdapterCtx_t *ctx)
{
  ctx->reserved = 0U;
}

ICanBusPort_t CanBusAdapter_CreatePort(CanBusAdapterCtx_t *ctx)
{
  ICanBusPort_t port;

  port.ctx = ctx;
  port.SendStd = AdapterSendStd;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
