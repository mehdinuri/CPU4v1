/* App/Adapters/STM32/ControlBusAdapter.c */
#include "ControlBusAdapter.h"

#include <string.h>

#define CONTROL_BUS_ADAPTER_RX_DEPTH 16U
#define CONTROL_BUS_ADAPTER_TX_DEPTH 16U

typedef struct
{
  ControlBusFrame_t frame;
} ControlBusQueuedFrame_t;

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
  uint32_t index;

  for (index = 0U; index < (sizeof(table) / sizeof(table[0])); index++)
  {
    if (table[index].bytes >= length)
    {
      return table[index].dlc;
    }
  }

  return FDCAN_DLC_BYTES_64;
}

static uint8_t DlcToLength(uint32_t dlc)
{
  switch (dlc)
  {
      case FDCAN_DLC_BYTES_0: return 0U;
      case FDCAN_DLC_BYTES_1: return 1U;
      case FDCAN_DLC_BYTES_2: return 2U;
      case FDCAN_DLC_BYTES_3: return 3U;
      case FDCAN_DLC_BYTES_4: return 4U;
      case FDCAN_DLC_BYTES_5: return 5U;
      case FDCAN_DLC_BYTES_6: return 6U;
      case FDCAN_DLC_BYTES_7: return 7U;
      case FDCAN_DLC_BYTES_8: return 8U;
      case FDCAN_DLC_BYTES_12: return 12U;
      case FDCAN_DLC_BYTES_16: return 16U;
      case FDCAN_DLC_BYTES_20: return 20U;
      case FDCAN_DLC_BYTES_24: return 24U;
      case FDCAN_DLC_BYTES_32: return 32U;
      case FDCAN_DLC_BYTES_48: return 48U;
      case FDCAN_DLC_BYTES_64: return 64U;
      default: return CONTROL_BUS_FRAME_MAX_LENGTH;
  }
}

static void CreateOsObjects(ControlBusAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  if (ctx->rxPool == NULL)
  {
    ctx->rxPool = osMemoryPoolNew(CONTROL_BUS_ADAPTER_RX_DEPTH,
                                  sizeof(ControlBusQueuedFrame_t),
                                  NULL);
  }

  if (ctx->txPool == NULL)
  {
    ctx->txPool = osMemoryPoolNew(CONTROL_BUS_ADAPTER_TX_DEPTH,
                                  sizeof(ControlBusQueuedFrame_t),
                                  NULL);
  }

  if (ctx->rxQueue == NULL)
  {
    ctx->rxQueue = osMessageQueueNew(CONTROL_BUS_ADAPTER_RX_DEPTH,
                                     sizeof(ControlBusQueuedFrame_t *),
                                     NULL);
  }

  if (ctx->txQueue == NULL)
  {
    ctx->txQueue = osMessageQueueNew(CONTROL_BUS_ADAPTER_TX_DEPTH,
                                     sizeof(ControlBusQueuedFrame_t *),
                                     NULL);
  }
}

static uint8_t QueueFrame(osMemoryPoolId_t pool,
                          osMessageQueueId_t queue,
                          const ControlBusFrame_t *frame)
{
  ControlBusQueuedFrame_t *queuedFrame;

  if ((pool == NULL) || (queue == NULL) || (frame == NULL))
  {
    return 0U;
  }

  queuedFrame = (ControlBusQueuedFrame_t *) osMemoryPoolAlloc(pool, 0U);
  if (queuedFrame == NULL)
  {
    return 0U;
  }

  queuedFrame->frame = *frame;
  if (osMessageQueuePut(queue, &queuedFrame, 0U, 0U) != osOK)
  {
    (void) osMemoryPoolFree(pool, queuedFrame);
    return 0U;
  }

  return 1U;
}

static uint8_t AdapterSendFrame(void *ctx, const ControlBusFrame_t *frame)
{
  ControlBusAdapterCtx_t *self = (ControlBusAdapterCtx_t *) ctx;

  if ((self == NULL) || (frame == NULL)
      || (frame->length > CONTROL_BUS_FRAME_MAX_LENGTH)
      || (frame->standardId > 0x7FFU))
  {
    return 0U;
  }

  if (QueueFrame(self->txPool, self->txQueue, frame) == 0U)
  {
    self->txDrops++;
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
  CreateOsObjects(ctx);
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

  if ((ctx == NULL) || (header == NULL) || (data == NULL)
      || (header->IdType != FDCAN_STANDARD_ID))
  {
    return;
  }

  frame.standardId = (uint16_t) header->Identifier;
  frame.length = DlcToLength(header->DataLength);
  if (frame.length > CONTROL_BUS_FRAME_MAX_LENGTH)
  {
    frame.length = CONTROL_BUS_FRAME_MAX_LENGTH;
  }

  (void) memcpy(frame.data, data, frame.length);

  if (QueueFrame(ctx->rxPool, ctx->rxQueue, &frame) == 0U)
  {
    ctx->rxDrops++;
  }
}

void ControlBusAdapterStep(ControlBusAdapterCtx_t *ctx)
{
  ControlBusQueuedFrame_t *queuedFrame;

  if (ctx == NULL)
  {
    return;
  }

  while ((ctx->rxQueue != NULL)
         && (osMessageQueueGet(ctx->rxQueue,
                               &queuedFrame,
                               NULL,
                               0U) == osOK))
  {
    if (ctx->rxCallback != NULL)
    {
      ctx->rxCallback(ctx->rxCallbackCtx, &queuedFrame->frame);
    }

    (void) osMemoryPoolFree(ctx->rxPool, queuedFrame);
  }

  while ((ctx->txQueue != NULL)
         && (osMessageQueueGet(ctx->txQueue,
                               &queuedFrame,
                               NULL,
                               0U) == osOK))
  {
    FDCAN_TxHeaderTypeDef header;

    (void) memset(&header, 0, sizeof(header));
    header.Identifier = queuedFrame->frame.standardId;
    header.IdType = FDCAN_STANDARD_ID;
    header.TxFrameType = FDCAN_DATA_FRAME;
    header.DataLength = LengthToDlc(queuedFrame->frame.length);
    header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    header.BitRateSwitch = FDCAN_BRS_ON;
    header.FDFormat = FDCAN_FD_CAN;
    header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    header.MessageMarker = 0U;

    if ((ctx->hfdcan == NULL)
        || (HAL_FDCAN_AddMessageToTxFifoQ(ctx->hfdcan,
                                          &header,
                                          queuedFrame->frame.data) != HAL_OK))
    {
      if ((ctx->txQueue == NULL)
          || (osMessageQueuePut(ctx->txQueue, &queuedFrame, 0U, 0U) != osOK))
      {
        ctx->txErrors++;
        (void) osMemoryPoolFree(ctx->txPool, queuedFrame);
      }
      break;
    }

    (void) osMemoryPoolFree(ctx->txPool, queuedFrame);
  }
}
