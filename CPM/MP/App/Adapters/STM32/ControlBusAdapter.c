/* App/Adapters/STM32/ControlBusAdapter.c */

#include "ControlBusAdapter.h"

#include <stddef.h>
#include <string.h>

#include "stm32g4xx_hal.h"

static uint32_t LengthToDlc(uint8_t length)
{
  static const struct
  {
    uint8_t bytes;
    uint32_t dlc;
  } table[] = {
    { 0U,  FDCAN_DLC_BYTES_0  },
    { 1U,  FDCAN_DLC_BYTES_1  },
    { 2U,  FDCAN_DLC_BYTES_2  },
    { 3U,  FDCAN_DLC_BYTES_3  },
    { 4U,  FDCAN_DLC_BYTES_4  },
    { 5U,  FDCAN_DLC_BYTES_5  },
    { 6U,  FDCAN_DLC_BYTES_6  },
    { 7U,  FDCAN_DLC_BYTES_7  },
    { 8U,  FDCAN_DLC_BYTES_8  },
    { 12U, FDCAN_DLC_BYTES_12 },
    { 16U, FDCAN_DLC_BYTES_16 },
    { 20U, FDCAN_DLC_BYTES_20 },
    { 24U, FDCAN_DLC_BYTES_24 },
    { 32U, FDCAN_DLC_BYTES_32 },
    { 48U, FDCAN_DLC_BYTES_48 },
    { 64U, FDCAN_DLC_BYTES_64 }
  };
  uint32_t i;

  for (i = 0U; i < (sizeof(table) / sizeof(table[0])); i++)
  {
    if (table[i].bytes >= length)
    {
      return table[i].dlc;
    }
  }

  return FDCAN_DLC_BYTES_64;
}

static uint8_t AdapterSendFrame(void *ctx, const ControlBusFrame_t *frame)
{
  ControlBusAdapterCtx_t *self = (ControlBusAdapterCtx_t *) ctx;
  FDCAN_TxHeaderTypeDef header = { 0 };

  if ((self == NULL) || (frame == NULL) || (self->hfdcan == NULL))
  {
    return 0U;
  }

  if (frame->length > CONTROL_BUS_FRAME_MAX_LENGTH)
  {
    return 0U;
  }

  header.Identifier = frame->extendedId;
  header.IdType = FDCAN_EXTENDED_ID;
  header.TxFrameType = FDCAN_DATA_FRAME;
  header.DataLength = LengthToDlc(frame->length);
  header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  header.BitRateSwitch = FDCAN_BRS_OFF;
  header.FDFormat = FDCAN_FD_CAN;
  header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  header.MessageMarker = 0U;

  if (HAL_FDCAN_AddMessageToTxFifoQ(self->hfdcan,
                                    &header,
                                    (uint8_t *) frame->data) != HAL_OK)
  {
    self->txErrors++;

    return 0U;
  }

  return 1U;
}

static uint8_t AdapterRegisterRxCallback(void *ctx,
                                         ControlBusRxCallback_t cb,
                                         void *cbCtx)
{
  ControlBusAdapterCtx_t *self = (ControlBusAdapterCtx_t *) ctx;

  if (self == NULL)
  {
    return 0U;
  }

  self->rxCallback = cb;
  self->rxCallbackCtx = cbCtx;

  return 1U;
}

void ControlBusAdapterInit(ControlBusAdapterCtx_t *ctx,
                           FDCAN_HandleTypeDef *hfdcan)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
  ctx->hfdcan = hfdcan;
}

IControlBusPort_t ControlBusAdapterCreatePort(ControlBusAdapterCtx_t *ctx)
{
  IControlBusPort_t port;

  port.ctx = ctx;
  port.SendFrame = AdapterSendFrame;
  port.RegisterRxCallback = AdapterRegisterRxCallback;

  return port;
}

void ControlBusAdapterOnRxIsr(ControlBusAdapterCtx_t *ctx,
                              const FDCAN_RxHeaderTypeDef *header,
                              const uint8_t *data)
{
  ControlBusFrame_t frame;

  if ((ctx == NULL) || (header == NULL) || (data == NULL))
  {
    return;
  }

  if (header->IdType != FDCAN_EXTENDED_ID)
  {
    return;
  }

  frame.extendedId = header->Identifier;
  frame.length = (uint8_t) (header->DataLength);

  if (frame.length > CONTROL_BUS_FRAME_MAX_LENGTH)
  {
    frame.length = CONTROL_BUS_FRAME_MAX_LENGTH;
  }

  (void) memcpy(frame.data, data, frame.length);

  if (ctx->rxCallback != NULL)
  {
    ctx->rxCallback(ctx->rxCallbackCtx, &frame);
  }
}
