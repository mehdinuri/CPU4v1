/* App/Adapters/STM32/UnitClockAdapter.c
 *
 * Unit clock availability and runtime state derived from the actual STM32
 * time services that are active in this product generation.
 */
#include "UnitClockAdapter.h"

#include <string.h>

#include "Platform/STM32/Core/Tim2CaptureView.h"
#include "gps.h"
#include "TimeSourceState.h"

enum
{
  UNIT_CLOCK_SOURCE_COUNT = 3,
  LINE_SYNC_TARGET_FREQ_HZ = 100U,
  LINE_SYNC_FREQ_TOLERANCE_HZ = 5U
};

static const uint8_t kSupportedSources[UNIT_CLOCK_SOURCE_COUNT] = {
  (uint8_t) UNIT_CLOCK_SOURCE_RTC_SQWR,
  (uint8_t) UNIT_CLOCK_SOURCE_GNSS,
  (uint8_t) UNIT_CLOCK_SOURCE_LINE_SYNC
};

static uint8_t LineSyncSignalValid(void)
{
  uint32_t frequency = Tim2CapturedFreqHzGet();

  return (uint8_t) ((frequency
                     >= (LINE_SYNC_TARGET_FREQ_HZ
                         - LINE_SYNC_FREQ_TOLERANCE_HZ))
                    && (frequency
                        <= (LINE_SYNC_TARGET_FREQ_HZ
                            + LINE_SYNC_FREQ_TOLERANCE_HZ)));
}

static uint8_t AdapterGetSourceCount(void *ctx)
{
  (void) ctx;

  return UNIT_CLOCK_SOURCE_COUNT;
}

static uint8_t AdapterGetSourceAvailable(void *ctx,
                                         uint8_t sourceIndex,
                                         uint8_t *sourceAvailable)
{
  (void) ctx;

  if ((sourceAvailable == NULL) || (sourceIndex >= UNIT_CLOCK_SOURCE_COUNT))
  {
    return 0U;
  }

  *sourceAvailable = kSupportedSources[sourceIndex];

  return 1U;
}

static uint8_t AdapterGetCommandedSource(void *ctx, uint8_t *commandedSource)
{
  UnitClockAdapterCtx_t *adapter = (UnitClockAdapterCtx_t *) ctx;
  IntersectionUnitConfig_t unitConfig;

  if ((adapter == NULL) || (commandedSource == NULL)
      || (adapter->configurationService == NULL))
  {
    return 0U;
  }

  if (ConfigurationServiceGetActiveUnitConfig(adapter->configurationService,
                                              &unitConfig) == 0U)
  {
    return 0U;
  }

  *commandedSource = unitConfig.timeSourceCommanded;

  return 1U;
}

static uint8_t AdapterGetCurrentSource(void *ctx, uint8_t *currentSource)
{
  uint8_t legacySource;

  (void) ctx;

  if (currentSource == NULL)
  {
    return 0U;
  }

  legacySource = TimeSourceGet();

  switch (legacySource)
  {
      case TIME_SOURCE_GPS:
      {
        *currentSource = (uint8_t) UNIT_CLOCK_SOURCE_GNSS;

        return 1U;
      }

      case TIME_SOURCE_RTC:
      {
        *currentSource = (uint8_t) UNIT_CLOCK_SOURCE_RTC_SQWR;

        return 1U;
      }

      case TIME_SOURCE_NET:
      {
        *currentSource = (uint8_t) UNIT_CLOCK_SOURCE_LINE_SYNC;

        return 1U;
      }

      default:
      {
        *currentSource = (uint8_t) UNIT_CLOCK_SOURCE_OTHER;

        return 1U;
      }
  }
}

static uint8_t AdapterGetCurrentStatus(void *ctx, uint8_t *currentStatus)
{
  uint8_t currentSource = (uint8_t) UNIT_CLOCK_SOURCE_OTHER;

  if ((currentStatus == NULL)
      || (AdapterGetCurrentSource(ctx, &currentSource) == 0U))
  {
    return 0U;
  }

  if (currentSource == (uint8_t) UNIT_CLOCK_SOURCE_GNSS)
  {
    if (GpsModemAliveGet() == 0U)
    {
      *currentStatus = (uint8_t) UNIT_CLOCK_SOURCE_STATUS_DATA_TIMEOUT_ERROR;
    }
    else if (GpsRTCInitialUpdateDoneGet() == 0U)
    {
      *currentStatus = (uint8_t) UNIT_CLOCK_SOURCE_STATUS_PENDING_UPDATE;
    }
    else
    {
      *currentStatus = (uint8_t) UNIT_CLOCK_SOURCE_STATUS_ACTIVE;
    }

    return 1U;
  }

  if (currentSource == (uint8_t) UNIT_CLOCK_SOURCE_RTC_SQWR)
  {
    *currentStatus = (uint8_t) UNIT_CLOCK_SOURCE_STATUS_ACTIVE;

    return 1U;
  }

  if (currentSource == (uint8_t) UNIT_CLOCK_SOURCE_LINE_SYNC)
  {
    uint32_t frequency = Tim2CapturedFreqHzGet();

    if (LineSyncSignalValid() != 0U)
    {
      *currentStatus = (uint8_t) UNIT_CLOCK_SOURCE_STATUS_ACTIVE;
    }
    else if (frequency == 0U)
    {
      *currentStatus = (uint8_t) UNIT_CLOCK_SOURCE_STATUS_DATA_TIMEOUT_ERROR;
    }
    else
    {
      *currentStatus = (uint8_t) UNIT_CLOCK_SOURCE_STATUS_DATA_ERROR;
    }

    return 1U;
  }

  *currentStatus = (uint8_t) UNIT_CLOCK_SOURCE_STATUS_NOT_ACTIVE;

  return 1U;
}

static uint8_t AdapterGetNonSequentialSource(void *ctx,
                                             uint8_t *nonSequentialSource)
{
  (void) ctx;

  if (nonSequentialSource == NULL)
  {
    return 0U;
  }

  *nonSequentialSource = (uint8_t) UNIT_CLOCK_NON_SEQUENTIAL_SOURCE_UNKNOWN;

  return 1U;
}

static uint8_t AdapterGetNonSequentialChange(void *ctx,
                                             uint32_t *nonSequentialChangeSeconds)
{
  (void) ctx;

  if (nonSequentialChangeSeconds == NULL)
  {
    return 0U;
  }

  *nonSequentialChangeSeconds = 0U;

  return 1U;
}

static uint8_t AdapterGetNonSequentialDelta(void *ctx,
                                            UnitClockNonSequentialDelta_t *delta)
{
  (void) ctx;

  if (delta == NULL)
  {
    return 0U;
  }

  memset(delta, 0, sizeof(*delta));

  return 1U;
}

static void AdapterAcknowledgeCurrentStatusRead(void *ctx)
{
  (void) ctx;
}

void UnitClockAdapterInit(UnitClockAdapterCtx_t *ctx,
                          ConfigurationService_t *configurationService)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->configurationService = configurationService;
}

IUnitClockPort_t UnitClockAdapterCreatePort(UnitClockAdapterCtx_t *ctx)
{
  IUnitClockPort_t port;

  port.ctx = ctx;
  port.GetSourceCount = AdapterGetSourceCount;
  port.GetSourceAvailable = AdapterGetSourceAvailable;
  port.GetCommandedSource = AdapterGetCommandedSource;
  port.GetCurrentSource = AdapterGetCurrentSource;
  port.GetCurrentStatus = AdapterGetCurrentStatus;
  port.GetNonSequentialSource = AdapterGetNonSequentialSource;
  port.GetNonSequentialChange = AdapterGetNonSequentialChange;
  port.GetNonSequentialDelta = AdapterGetNonSequentialDelta;
  port.AcknowledgeCurrentStatusRead = AdapterAcknowledgeCurrentStatusRead;

  return port;
}
