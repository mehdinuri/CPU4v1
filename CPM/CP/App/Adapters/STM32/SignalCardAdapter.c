/* App/Adapters/STM32/SignalCardAdapter.c
 *
 * Publishes the new controller output image on FDCAN2 for signal-switching
 * modules. The adapter intentionally owns only transport concerns; the
 * domain still owns aspect derivation and safety policy.
 */
#include "SignalCardAdapter.h"

#include <string.h>

#include "stm32h7xx_hal.h"

#define SIGNAL_CARD_ADAPTER_PROTOCOL_VERSION            1U
#define SIGNAL_CARD_ADAPTER_OUTPUT_IMAGE_MESSAGE_TYPE   1U
#define SIGNAL_CARD_ADAPTER_OUTPUT_IMAGE_MESSAGE_ID     0x500U
#define SIGNAL_CARD_ADAPTER_OUTPUT_IMAGE_PAYLOAD_SIZE   32U

static void SetAllRed(OutputDriverImage_t *image)
{
  uint8_t channelIndex;

  if (image == NULL)
  {
    return;
  }

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       ++channelIndex)
  {
    image->channels[channelIndex] = OUTPUT_DRIVER_ASPECT_RED;
    image->channelDimmed[channelIndex] = 0U;
    image->channelDimAlternateHalfCycle[channelIndex] = 0U;
  }
}

static uint8_t EnsureStarted(SignalCardAdapterCtx_t *ctx)
{
  HAL_FDCAN_StateTypeDef state;

  if ((ctx == NULL) || (ctx->hfdcan == NULL))
  {
    return 0U;
  }

  if (ctx->started != 0U)
  {
    return 1U;
  }

  state = HAL_FDCAN_GetState(ctx->hfdcan);
  if (state == HAL_FDCAN_STATE_READY)
  {
    if (HAL_FDCAN_Start(ctx->hfdcan) != HAL_OK)
    {
      return 0U;
    }
  }
  else if (state != HAL_FDCAN_STATE_BUSY)
  {
    return 0U;
  }

  ctx->started = 1U;

  return 1U;
}

static void BuildOutputImagePayload(const SignalCardAdapterCtx_t *ctx,
                                    const OutputDriverImage_t *image,
                                    uint8_t payload[
                                      SIGNAL_CARD_ADAPTER_OUTPUT_IMAGE_PAYLOAD_SIZE])
{
  uint8_t channelIndex;
  uint16_t dimmedMask = 0U;
  uint16_t dimAlternateMask = 0U;

  payload[0] = SIGNAL_CARD_ADAPTER_PROTOCOL_VERSION;
  payload[1] = SIGNAL_CARD_ADAPTER_OUTPUT_IMAGE_MESSAGE_TYPE;
  payload[2] = INTERSECTION_CHANNEL_COUNT_MAX;
  payload[3] = 0U;
  payload[4] = (uint8_t) (ctx->configEpoch & 0xFFU);
  payload[5] = (uint8_t) ((ctx->configEpoch >> 8U) & 0xFFU);
  payload[6] = (uint8_t) (ctx->sequenceCounter & 0xFFU);
  payload[7] = (uint8_t) ((ctx->sequenceCounter >> 8U) & 0xFFU);

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       ++channelIndex)
  {
    payload[8U + channelIndex] = (uint8_t) image->channels[channelIndex];

    if (image->channelDimmed[channelIndex] != 0U)
    {
      dimmedMask |= (uint16_t) (1U << channelIndex);
    }

    if (image->channelDimAlternateHalfCycle[channelIndex] != 0U)
    {
      dimAlternateMask |= (uint16_t) (1U << channelIndex);
    }
  }

  payload[24] = (uint8_t) (dimmedMask & 0xFFU);
  payload[25] = (uint8_t) ((dimmedMask >> 8U) & 0xFFU);
  payload[26] = (uint8_t) (dimAlternateMask & 0xFFU);
  payload[27] = (uint8_t) ((dimAlternateMask >> 8U) & 0xFFU);
  payload[28] = 0U;
  payload[29] = 0U;
  payload[30] = 0U;
  payload[31] = 0U;
}

static uint8_t AdapterApply(void *ctx, const OutputDriverImage_t *image)
{
  SignalCardAdapterCtx_t *adapterCtx = (SignalCardAdapterCtx_t *) ctx;
  FDCAN_TxHeaderTypeDef txHeader;
  uint8_t payload[SIGNAL_CARD_ADAPTER_OUTPUT_IMAGE_PAYLOAD_SIZE];

  if ((adapterCtx == NULL) || (image == NULL))
  {
    return 0U;
  }

  if (EnsureStarted(adapterCtx) == 0U)
  {
    return 0U;
  }

  if (HAL_FDCAN_GetTxFifoFreeLevel(adapterCtx->hfdcan) == 0U)
  {
    return 0U;
  }

  BuildOutputImagePayload(adapterCtx, image, payload);

  memset(&txHeader, 0, sizeof(txHeader));
  txHeader.Identifier = SIGNAL_CARD_ADAPTER_OUTPUT_IMAGE_MESSAGE_ID;
  txHeader.IdType = FDCAN_STANDARD_ID;
  txHeader.TxFrameType = FDCAN_DATA_FRAME;
  txHeader.DataLength = FDCAN_DLC_BYTES_32;
  txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  txHeader.BitRateSwitch = FDCAN_BRS_ON;
  txHeader.FDFormat = FDCAN_FD_CAN;
  txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  txHeader.MessageMarker = (uint8_t) (adapterCtx->sequenceCounter & 0xFFU);

  if (HAL_FDCAN_AddMessageToTxFifoQ(adapterCtx->hfdcan, &txHeader, payload)
      != HAL_OK)
  {
    return 0U;
  }

  adapterCtx->lastAppliedImage = *image;
  adapterCtx->sequenceCounter++;

  return 1U;
} /* AdapterApply */

static uint8_t AdapterSetConfigEpoch(void *ctx, uint16_t configEpoch)
{
  SignalCardAdapterSetConfigEpoch((SignalCardAdapterCtx_t *) ctx, configEpoch);

  return 1U;
}

void SignalCardAdapterInit(SignalCardAdapterCtx_t *ctx,
                           FDCAN_HandleTypeDef *hfdcan,
                           uint16_t configEpoch)
{
  if (ctx == NULL)
  {
    return;
  }

  memset(ctx, 0, sizeof(*ctx));
  ctx->hfdcan = hfdcan;
  ctx->configEpoch = configEpoch;
  SetAllRed(&ctx->lastAppliedImage);

  (void) EnsureStarted(ctx);
}

void SignalCardAdapterSetConfigEpoch(SignalCardAdapterCtx_t *ctx,
                                     uint16_t configEpoch)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->configEpoch = configEpoch;
}

IOutputDriverPort_t SignalCardAdapterCreatePort(SignalCardAdapterCtx_t *ctx)
{
  IOutputDriverPort_t port;

  port.ctx = ctx;
  port.Apply = AdapterApply;
  port.SetConfigEpoch = AdapterSetConfigEpoch;

  return port;
}
