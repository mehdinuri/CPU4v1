/**
 ******************************************************************************
 * @file    Adapters/STM32/CanBusAdapter.c
 ******************************************************************************
 */

#include "Adapters/STM32/CanBusAdapter.h"
#include "fdcan.h"
#include "can_msg_sender.h"

static FDCAN_HandleTypeDef *HandleFor(tECanBusId eBus)
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

static uint8_t AdapterSendStd(void *pCtx,
                              tECanBusId eBus,
                              const tSCanFrame *pFrame)
{
  (void) pCtx;

  FDCAN_HandleTypeDef *pHandle = HandleFor(eBus);

  if (pHandle == 0)
  {
    return 0U;
  }

  if (pFrame->bLen > 8U)
  {
    return 0U;
  }

  /* CANTxRequest's pData is non-const for historical reasons; the enqueue
   * path memcpy's before returning, so casting away const is safe.
   */
  return CANTxRequest(pHandle,
                      FDCAN_STANDARD_ID,
                      (uint32_t) pFrame->sStdId,
                      FDCAN_DATA_FRAME,
                      FDCAN_BRS_OFF,
                      FDCAN_CLASSIC_CAN,
                      pFrame->abData,
                      pFrame->bLen);
}

void CanBusAdapter_Init(tSCanBusAdapterCtx *pCtx)
{
  pCtx->bReserved = 0U;
}

ICanBusPort_t CanBusAdapter_CreatePort(tSCanBusAdapterCtx *pCtx)
{
  ICanBusPort_t port;

  port.pCtx = pCtx;
  port.SendStd = AdapterSendStd;

  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
