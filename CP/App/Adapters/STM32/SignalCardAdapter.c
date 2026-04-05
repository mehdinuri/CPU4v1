/*
 * App/Adapters/STM32/SignalCardAdapter.c
 *
 * ISignalOutputPort implementation — STM32H743 FDCAN SSM card driver.
 *
 * Layout:
 *   setLampState()  — writes pending[outputId] and marks dirty[]
 *   flush()         — iterates cards, builds 8-byte CAN payload,
 *                     calls HAL_FDCAN_AddMessageToTxFifo (TODO stub)
 *   isReady()       — returns ctx->ackReceived
 */
#include "SignalCardAdapter.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Forward declarations of port callback functions
 * --------------------------------------------------------------------------*/
static void SignalCard_SetLampState(void *ctx, uint8_t outputId,
                                    SignalColor_t color);
static void SignalCard_Flush(void *ctx);
static bool SignalCard_IsReady(void *ctx);

/* --------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------*/

#ifdef STM32H743xx
void SignalCardAdapter_Init(SignalCardAdapterCtx_t *ctx,
                            FDCAN_HandleTypeDef     *hfdcan)
#else
void SignalCardAdapter_Init(SignalCardAdapterCtx_t *ctx, void *hfdcan)
#endif
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->hfdcan = hfdcan;
  ctx->ackReceived = false;

  /* Default all outputs to red at power-on. */
  for (uint8_t i = 0U; i < SIGNAL_OUTPUTS_MAX; i++)
  {
    ctx->pending[i] = SIGNAL_COLOR_RED;
    ctx->dirty[i] = true;         /* Force full state push on first flush. */
  }
}

ISignalOutputPort_t SignalCardAdapter_CreatePort(SignalCardAdapterCtx_t *ctx)
{
  ISignalOutputPort_t port;

  port.ctx = ctx;
  port.setLampState = SignalCard_SetLampState;
  port.flush = SignalCard_Flush;
  port.isReady = SignalCard_IsReady;

  return port;
}

void SignalCardAdapter_NotifyAck(SignalCardAdapterCtx_t *ctx, uint32_t canId)
{
  /* Any ACK in range 0x200..0x200+SIGNAL_CARD_COUNT_MAX-1 counts. */
  uint32_t base = 0x200U;

  if ((canId >= base) && (canId < (base + SIGNAL_CARD_COUNT_MAX)))
  {
    ctx->ackReceived = true;
  }
}

/* --------------------------------------------------------------------------
 * Port callbacks (static — not part of public API)
 * --------------------------------------------------------------------------*/

static void SignalCard_SetLampState(void *vctx, uint8_t outputId,
                                    SignalColor_t color)
{
  SignalCardAdapterCtx_t *ctx = (SignalCardAdapterCtx_t *) vctx;

  if (outputId >= SIGNAL_OUTPUTS_MAX)
  {
    return;
  }

  if (ctx->pending[outputId] != color)
  {
    ctx->pending[outputId] = color;
    ctx->dirty[outputId] = true;
  }
}

static void SignalCard_Flush(void *vctx)
{
  SignalCardAdapterCtx_t *ctx = (SignalCardAdapterCtx_t *) vctx;

  /* Each card covers SIGNAL_OUTPUTS_PER_CARD consecutive outputs.
   * Build one 8-byte CAN frame per card that has any dirty output.
   * Byte layout: bytes 0-7, each byte encodes one lamp output as
   * a SignalColor_t value (lower nibble = color, upper nibble = 0). */

  for (uint8_t cardIdx = 0U; cardIdx < SIGNAL_CARD_COUNT_MAX; cardIdx++)
  {
    uint8_t baseOutput = cardIdx * SIGNAL_OUTPUTS_PER_CARD;
    bool needSend = false;

    /* Check if any output on this card is dirty. */
    for (uint8_t lane = 0U; lane < SIGNAL_OUTPUTS_PER_CARD; lane++)
    {
      uint8_t idx = baseOutput + lane;

      if ((idx < SIGNAL_OUTPUTS_MAX) && ctx->dirty[idx])
      {
        needSend = true;
        break;
      }
    }

    if (!needSend)
    {
      continue;
    }

    /* Build the 8-byte payload. */
    uint8_t payload[8] = { 0 };

    for (uint8_t lane = 0U; lane < SIGNAL_OUTPUTS_PER_CARD; lane++)
    {
      uint8_t idx = baseOutput + lane;

      payload[lane] = (idx < SIGNAL_OUTPUTS_MAX)
                      ? (uint8_t) ctx->pending[idx]
                      : (uint8_t) SIGNAL_COLOR_RED;
    }

    uint32_t canId = 0x200U + cardIdx;

    (void) canId;       /* Suppress unused-variable warning in host builds. */
    (void) payload;

    #ifdef STM32H743xx

    /* TODO: HAL impl — transmit CAN frame via FDCAN_AddMessageToTxFifo.
     *
     * FDCAN_TxHeaderTypeDef txHdr = {
     *     .Identifier          = canId,
     *     .IdType              = FDCAN_STANDARD_ID,
     *     .TxFrameType         = FDCAN_DATA_FRAME,
     *     .DataLength          = FDCAN_DLC_BYTES_8,
     *     .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
     *     .BitRateSwitch       = FDCAN_BRS_OFF,
     *     .FDFormat            = FDCAN_CLASSIC_CAN,
     *     .TxEventFifoControl  = FDCAN_NO_TX_EVENTS,
     *     .MessageMarker       = 0,
     * };
     * HAL_FDCAN_AddMessageToTxFifo(ctx->hfdcan, &txHdr, payload);
     */
    #endif

    /* Clear dirty flags for this card's outputs after (attempted) send. */
    for (uint8_t lane = 0U; lane < SIGNAL_OUTPUTS_PER_CARD; lane++)
    {
      uint8_t idx = baseOutput + lane;

      if (idx < SIGNAL_OUTPUTS_MAX)
      {
        ctx->dirty[idx] = false;
      }
    }

    /* Reset ack flag — will be set again by CANRxTask. */
    ctx->ackReceived = false;
  }
} /* SignalCard_Flush */

static bool SignalCard_IsReady(void *vctx)
{
  SignalCardAdapterCtx_t *ctx = (SignalCardAdapterCtx_t *) vctx;

  return ctx->ackReceived;
}
