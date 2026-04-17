/* App/Adapters/STM32/FieldBusAdapter.c */

#include "FieldBusAdapter.h"

#include <stddef.h>
#include <string.h>

#include "stm32g4xx_hal.h"

static void DecodeLoadSwitchLow(FieldBusAdapterCtx_t *ctx,
                                const uint8_t *data,
                                uint8_t length)
{
  uint8_t i;

  if (length < 4U)
  {
    return;
  }

  /* 0x040 carries bits 0..47 in little-endian form over up to 6 bytes. */
  for (i = 0U; (i < 6U) && (i < length); i++)
  {
    ctx->write.cpu.commandedLoadSwitches.bits[i] = data[i];
  }
}

static void DecodeLoadSwitchHigh(FieldBusAdapterCtx_t *ctx,
                                 const uint8_t *data,
                                 uint8_t length)
{
  uint8_t i;

  if (length < 4U)
  {
    return;
  }

  for (i = 0U; (i < 6U) && (i < length); i++)
  {
    ctx->write.cpu.commandedLoadSwitches.bits[6U + i] = data[i];
  }

  ctx->write.cpu.cpuImageSequence++;
  ctx->write.cpu.cpAlive = 1U;
}

static void DecodeSsmTelemetry(FieldBusAdapterCtx_t *ctx,
                               uint8_t ssmIndex,
                               const uint8_t *data,
                               uint8_t length)
{
  FieldBusSsmTelemetry_t *slot;

  if ((ssmIndex >= FIELD_BUS_SSM_COUNT) || (length < 7U))
  {
    return;
  }

  slot = &ctx->write.ssm[ssmIndex];
  slot->voltagePresenceBits = (uint16_t) ((uint16_t) data[0]
                                          | (((uint16_t) data[1] & 0x0FU) <<
                                             8U));

  uint8_t highByte = data[6];
  uint8_t g;

  for (g = 0U; g < FIELD_BUS_SSM_CURRENT_GROUPS_PER_MODULE; g++)
  {
    uint16_t lo = data[2U + g];
    uint16_t hi = (uint16_t) ((highByte >> (g * 2U)) & 0x03U);

    slot->currentByGroupDeciAmps[g] = (uint16_t) ((hi << 8U) | lo);
  }

  slot->alive = 1U;
}

static void DecodePsmTelemetry(FieldBusAdapterCtx_t *ctx,
                               uint8_t psmIndex,
                               const uint8_t *data,
                               uint8_t length)
{
  FieldBusPsmTelemetry_t *slot;

  if ((psmIndex >= FIELD_BUS_PSM_COUNT) || (length < 8U))
  {
    return;
  }

  slot = &ctx->write.psm[psmIndex];

  slot->lineVoltageAdcCounts =
    (uint16_t) ((uint16_t) data[0] | ((uint16_t) (data[5] & 0x03U) << 8U));
  slot->rail24V1AdcCounts =
    (uint16_t) ((uint16_t) data[1] | ((uint16_t) ((data[5] >> 2U) & 0x03U) <<
                                      8U));
  slot->rail24V2AdcCounts =
    (uint16_t) ((uint16_t) data[2] | ((uint16_t) ((data[5] >> 4U) & 0x03U) <<
                                      8U));
  slot->rail5V1AdcCounts =
    (uint16_t) ((uint16_t) data[3] | ((uint16_t) ((data[5] >> 6U) & 0x03U) <<
                                      8U));
  slot->rail5V2AdcCounts =
    (uint16_t) ((uint16_t) data[4] | ((uint16_t) (data[6] & 0x03U) << 8U));
  slot->isolatedVoltage = (uint8_t) ((data[6] >> 2U) & 0x01U);
  slot->lineFrequencyRaw = (uint16_t) data[7];
  slot->alive = 1U;
}

static void DecodeFlashSignalsLow(FieldBusAdapterCtx_t *ctx,
                                  const uint8_t *data,
                                  uint8_t length)
{
  uint8_t i;

  if (length < 4U)
  {
    return;
  }

  for (i = 0U; (i < 6U) && (i < length); i++)
  {
    ctx->write.cpu.flashChannels.bits[i] = data[i];
  }
}

static void DecodeFlashSignalsHigh(FieldBusAdapterCtx_t *ctx,
                                   const uint8_t *data,
                                   uint8_t length)
{
  uint8_t i;

  if (length < 5U)
  {
    return;
  }

  for (i = 0U; (i < 6U) && (i < length); i++)
  {
    ctx->write.cpu.flashChannels.bits[6U + i] = data[i];
  }

  /* Flash period is the upper 6 bits of byte 5 per the legacy wire
   * format - see tSCanCpuFlashSignals. */
  if (length >= 6U)
  {
    ctx->write.cpu.flashPeriodDs = (uint8_t) (data[5] & 0x3FU);
  }
}

static uint8_t AdapterReadSnapshot(void *ctx, FieldBusSnapshot_t *snapshot)
{
  FieldBusAdapterCtx_t *self = (FieldBusAdapterCtx_t *) ctx;

  if ((self == NULL) || (snapshot == NULL))
  {
    return 0U;
  }

  *snapshot = self->read;

  return 1U;
}

void FieldBusAdapterInit(FieldBusAdapterCtx_t *ctx,
                         FDCAN_HandleTypeDef *hfdcan)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
  ctx->hfdcan = hfdcan;
}

IFieldBusPort_t FieldBusAdapterCreatePort(FieldBusAdapterCtx_t *ctx)
{
  IFieldBusPort_t port;

  port.ctx = ctx;
  port.ReadSnapshot = AdapterReadSnapshot;

  return port;
}

void FieldBusAdapterOnRxIsr(FieldBusAdapterCtx_t *ctx,
                            const FDCAN_RxHeaderTypeDef *header,
                            const uint8_t *data)
{
  uint32_t id;
  uint8_t length;

  if ((ctx == NULL) || (header == NULL) || (data == NULL))
  {
    return;
  }

  if (header->IdType != FDCAN_STANDARD_ID)
  {
    return;
  }

  id = header->Identifier;
  length = (uint8_t) header->DataLength;

  if (id == CAN_MID_CPU_SO0)
  {
    DecodeLoadSwitchLow(ctx, data, length);
  }
  else if (id == CAN_MID_CPU_SO1)
  {
    DecodeLoadSwitchHigh(ctx, data, length);
  }
  else if (id == CAN_MID_CPU_FLASH_SIGNALS0)
  {
    DecodeFlashSignalsLow(ctx, data, length);
  }
  else if (id == CAN_MID_CPU_FLASH_SIGNALS1)
  {
    DecodeFlashSignalsHigh(ctx, data, length);
  }
  else if ((id >= CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS0)
           && (id <= CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS7))
  {
    uint8_t ssmIdx =
      (uint8_t) (id - CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS0);

    DecodeSsmTelemetry(ctx, ssmIdx, data, length);
  }
  else if ((id == CAN_MID_PSM_VOLT_MEASUREMENTS0)
           || (id == CAN_MID_PSM_VOLT_MEASUREMENTS1))
  {
    uint8_t psmIdx = (uint8_t) (id - CAN_MID_PSM_VOLT_MEASUREMENTS0);

    DecodePsmTelemetry(ctx, psmIdx, data, length);
  }
  else
  {
    ctx->droppedFrames++;
  }
} /* FieldBusAdapterOnRxIsr */

void FieldBusAdapterCommit(FieldBusAdapterCtx_t *ctx, uint32_t nowTicks)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->write.snapshotTicks = nowTicks;

  __disable_irq();
  ctx->read = ctx->write;
  /* Reset per-window alive flags for next observation window. */
  ctx->write.cpu.cpAlive = 0U;
  {
    uint8_t i;

    for (i = 0U; i < FIELD_BUS_SSM_COUNT; i++)
    {
      ctx->write.ssm[i].alive = 0U;
    }

    for (i = 0U; i < FIELD_BUS_PSM_COUNT; i++)
    {
      ctx->write.psm[i].alive = 0U;
    }
  }
  __enable_irq();
}
