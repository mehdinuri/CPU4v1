/* App/Adapters/STM32/FieldBusTxAdapter.c */
#include "FieldBusTxAdapter.h"

#include <string.h>

#include "FieldCanQueueTx.h"
#include "LegacyFieldCanIds.h"

#define FIELD_OUTPUT_SIGNAL_COUNT 96U
#define FIELD_OUTPUT_FLASH_PERIOD_TICKS 50U
#define FIELD_OUTPUT_FLASH_PERIOD_HALF_TICKS (FIELD_OUTPUT_FLASH_PERIOD_TICKS / 2U)
#define FIELD_OUTPUT_PERIODIC_TICKS 50U

static FieldBusTxAdapterCtx_t *s_registeredCtx = NULL;

static void SendFrame(uint16_t identifier, const uint8_t *data, uint8_t length)
{
  uint8_t payload[8];

  if ((data == NULL) && (length != 0U))
  {
    return;
  }

  (void) memset(payload, 0, sizeof(payload));
  if ((data != NULL) && (length != 0U))
  {
    if (length > sizeof(payload))
    {
      length = sizeof(payload);
    }

    (void) memcpy(payload, data, length);
  }

  (void) FieldCanQueueTxSendStandard(identifier, payload, length);
}

static void SetBit(uint8_t *bytes, uint8_t bitIndex)
{
  if ((bytes == NULL) || (bitIndex >= FIELD_OUTPUT_SIGNAL_COUNT))
  {
    return;
  }

  bytes[bitIndex >> 3U] |= (uint8_t) (1U << (bitIndex & 0x07U));
}

static uint8_t FlashPhaseOn(const FieldBusTxAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((ctx->stepCounter % FIELD_OUTPUT_FLASH_PERIOD_TICKS)
                    >= FIELD_OUTPUT_FLASH_PERIOD_HALF_TICKS);
}

static uint8_t FailureFlashPeriodDs(const FieldBusTxAdapterCtx_t *ctx)
{
  const IntersectionConfig_t *config;

  if ((ctx == NULL) || (ctx->configurationService == NULL))
  {
    return INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_500MS_DS;
  }

  config = ConfigurationServiceGetActiveConfig(ctx->configurationService);
  if (config == NULL)
  {
    return INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_500MS_DS;
  }

  return config->unit.failureFlashPeriodDs;
}

static uint8_t RowToOutputIndexColor(
  const IntersectionIoOutputMapRowConfig_t *row,
  uint8_t *outputIndex,
  uint8_t *channelIndex,
  uint8_t *colorMask)
{
  if ((row == NULL) || (outputIndex == NULL) || (channelIndex == NULL)
      || (colorMask == NULL) || (row->devicePin == 0U)
      || (row->devicePin > FIELD_OUTPUT_SIGNAL_COUNT)
      || (row->functionIndex == 0U)
      || (row->functionIndex > INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  switch ((IntersectionIoMapOutputFunction_t) row->function)
  {
      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_RED:
      {
        *colorMask = 0x04U;
        break;
      }

      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_YELLOW:
      {
        *colorMask = 0x02U;
        break;
      }

      case INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_GREEN:
      {
        *colorMask = 0x01U;
        break;
      }

      default:
      {
        return 0U;
      }
  }

  *outputIndex = (uint8_t) (row->devicePin - 1U);
  *channelIndex = (uint8_t) (row->functionIndex - 1U);

  return 1U;
}

static uint8_t AspectEnergizesColor(OutputDriverAspect_t aspect,
                                    uint8_t colorMask,
                                    uint8_t flashOn)
{
  switch (aspect)
  {
      case OUTPUT_DRIVER_ASPECT_RED:
      {
        return (uint8_t) (colorMask == 0x04U);
      }

      case OUTPUT_DRIVER_ASPECT_YELLOW:
      {
        return (uint8_t) (colorMask == 0x02U);
      }

      case OUTPUT_DRIVER_ASPECT_GREEN:
      {
        return (uint8_t) (colorMask == 0x01U);
      }

      case OUTPUT_DRIVER_ASPECT_FLASH_RED:
      {
        return (uint8_t) ((colorMask == 0x04U) && (flashOn != 0U));
      }

      case OUTPUT_DRIVER_ASPECT_FLASH_YELLOW:
      {
        return (uint8_t) ((colorMask == 0x02U) && (flashOn != 0U));
      }

      case OUTPUT_DRIVER_ASPECT_FLASH_GREEN:
      {
        return (uint8_t) ((colorMask == 0x01U) && (flashOn != 0U));
      }

      case OUTPUT_DRIVER_ASPECT_DARK:
      default:
      {
        return 0U;
      }
  }
}

static void BuildSignalImages(const FieldBusTxAdapterCtx_t *ctx,
                              uint8_t outputBits[12],
                              uint8_t flashBits[12])
{
  const IntersectionConfig_t *config;
  uint8_t outputRowIndex;
  uint8_t flashOn;

  if ((ctx == NULL) || (outputBits == NULL) || (flashBits == NULL))
  {
    return;
  }

  (void) memset(outputBits, 0, 12U);
  (void) memset(flashBits, 0, 12U);

  if (ctx->configurationService == NULL)
  {
    return;
  }

  config = ConfigurationServiceGetActiveConfig(ctx->configurationService);
  if (config == NULL)
  {
    return;
  }

  flashOn = FlashPhaseOn(ctx);

  for (outputRowIndex = 0U;
       outputRowIndex < INTERSECTION_IO_MAP_MAX_OUTPUTS;
       ++outputRowIndex)
  {
    const IntersectionIoOutputMapRowConfig_t *row =
      &config->ioMap.outputs[outputRowIndex];
    uint8_t outputIndex;
    uint8_t channelIndex;
    uint8_t colorMask;
    OutputDriverAspect_t aspect;

    if (RowToOutputIndexColor(row,
                              &outputIndex,
                              &channelIndex,
                              &colorMask) == 0U)
    {
      continue;
    }

    aspect = ctx->lastImage.channels[channelIndex];
    if (AspectEnergizesColor(aspect, colorMask, flashOn) != 0U)
    {
      SetBit(outputBits, outputIndex);
    }

    if ((config->channels[channelIndex].flashMask & colorMask) != 0U)
    {
      SetBit(flashBits, outputIndex);
    }
  }
}

static void SendSignalStateFrames(const uint8_t outputBits[12])
{
  uint8_t payload[8];

  (void) memset(payload, 0, sizeof(payload));
  (void) memcpy(&payload[0], &outputBits[0], 6U);
  SendFrame(LEGACY_FIELD_CAN_ID_CPU_SO0, payload, sizeof(payload));

  (void) memset(payload, 0, sizeof(payload));
  (void) memcpy(&payload[0], &outputBits[6], 6U);
  SendFrame(LEGACY_FIELD_CAN_ID_CPU_SO1, payload, sizeof(payload));
}

static void SendFlashConfigFrames(const uint8_t flashBits[12],
                                  uint8_t flashPeriodDs)
{
  uint8_t payload[8];

  (void) memcpy(&payload[0], &flashBits[0], 6U);
  payload[6] = flashPeriodDs;
  payload[7] = 0U;
  SendFrame(LEGACY_FIELD_CAN_ID_CPU_FLASH_SIGNALS0, payload, sizeof(payload));

  (void) memcpy(&payload[0], &flashBits[6], 6U);
  payload[6] = flashPeriodDs;
  payload[7] = 0U;
  SendFrame(LEGACY_FIELD_CAN_ID_CPU_FLASH_SIGNALS1, payload, sizeof(payload));
}

static void SendDateTimeFrame(FieldBusTxAdapterCtx_t *ctx)
{
  uint8_t payload[8];
  RtcSnapshot_t rtcSnapshot;

  (void) memset(payload, 0, sizeof(payload));
  (void) memset(&rtcSnapshot, 0, sizeof(rtcSnapshot));

  if ((ctx != NULL) && (ctx->rtcPort != NULL))
  {
    (void) RealtimeClockReadSnapshot(ctx->rtcPort, &rtcSnapshot);
  }

  payload[0] = (uint8_t) (rtcSnapshot.Seconds & 0x3FU);
  payload[1] = (uint8_t) (rtcSnapshot.Minutes & 0x3FU);
  payload[2] = (uint8_t) (rtcSnapshot.Hours & 0x1FU);
  payload[3] = (uint8_t) (rtcSnapshot.Date & 0x1FU);
  payload[4] = (uint8_t) (rtcSnapshot.Month & 0x0FU);
  payload[5] = rtcSnapshot.Year;

  SendFrame(LEGACY_FIELD_CAN_ID_CPU_DATE_TIME, payload, sizeof(payload));
}

static uint8_t AdapterApply(void *ctx, const OutputDriverImage_t *image)
{
  FieldBusTxAdapterCtx_t *adapter = (FieldBusTxAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (image == NULL))
  {
    return 0U;
  }

  adapter->lastImage = *image;
  adapter->hasImage = 1U;

  return 1U;
}

static uint8_t AdapterSetConfigEpoch(void *ctx, uint16_t configEpoch)
{
  FieldBusTxAdapterSetConfigEpoch((FieldBusTxAdapterCtx_t *) ctx, configEpoch);

  return 1U;
}

void FieldBusTxAdapterInit(FieldBusTxAdapterCtx_t *ctx,
                           ConfigurationService_t *configurationService,
                           IRealtimeClockPort_t *rtcPort,
                           uint16_t configEpoch)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
  ctx->configurationService = configurationService;
  ctx->rtcPort = rtcPort;
  ctx->configEpoch = configEpoch;
  ctx->periodicCounter = FIELD_OUTPUT_PERIODIC_TICKS;
  s_registeredCtx = ctx;
}

void FieldBusTxAdapterSetConfigEpoch(FieldBusTxAdapterCtx_t *ctx,
                                     uint16_t configEpoch)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->configEpoch = configEpoch;
  ctx->periodicCounter = FIELD_OUTPUT_PERIODIC_TICKS;
}

IOutputDriverPort_t FieldBusTxAdapterCreatePort(FieldBusTxAdapterCtx_t *ctx)
{
  IOutputDriverPort_t port;

  port.ctx = ctx;
  port.Apply = AdapterApply;
  port.SetConfigEpoch = AdapterSetConfigEpoch;

  return port;
}

void FieldBusTxAdapterStep(void)
{
  FieldBusTxAdapterCtx_t *ctx = s_registeredCtx;
  uint8_t outputBits[12];
  uint8_t flashBits[12];

  if (ctx == NULL)
  {
    return;
  }

  ctx->stepCounter++;
  BuildSignalImages(ctx, outputBits, flashBits);

  SendFrame(LEGACY_FIELD_CAN_ID_VERSION, NULL, 0U);
  SendSignalStateFrames(outputBits);

  ctx->periodicCounter++;
  if (ctx->periodicCounter >= FIELD_OUTPUT_PERIODIC_TICKS)
  {
    SendDateTimeFrame(ctx);
    SendFlashConfigFrames(flashBits, FailureFlashPeriodDs(ctx));
    ctx->periodicCounter = 0U;
  }
}
