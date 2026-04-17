/* App/Adapters/STM32/RTCAdapter.c
 *
 * IRealtimeClockPort concrete implementation using the STM32 RTC HAL.
 */
#include "RTCAdapter.h"

#include "rtc.h"

static uint8_t BackupRegisterGet(RtcMetadataId_t metadataId, uint32_t *reg)
{
  if (reg == NULL)
  {
    return 0U;
  }

  switch (metadataId)
  {
      case RTC_META_INIT_SIGNATURE:
      {
        *reg = RTC_BKP_DR0;

        return 1U;
      }

      case RTC_META_CENTURY:
      {
        *reg = RTC_BKP_DR1;

        return 1U;
      }

      default:
      {
        return 0U;
      }
  }
}

static uint8_t AdapterReadSnapshot(void *ctx, RtcSnapshot_t *snapshot)
{
  RTC_TimeTypeDef halTime = { 0 };
  RTC_DateTypeDef halDate = { 0 };

  (void) ctx;

  if (snapshot == NULL)
  {
    return 0U;
  }

  if (HAL_RTC_GetTime(&hrtc, &halTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    return 0U;
  }

  if (HAL_RTC_GetDate(&hrtc, &halDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    return 0U;
  }

  snapshot->Century = (uint8_t) HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
  snapshot->Year = halDate.Year;
  snapshot->Month = halDate.Month;
  snapshot->Date = halDate.Date;
  snapshot->WeekDay = halDate.WeekDay;
  snapshot->Hours = halTime.Hours;
  snapshot->Minutes = halTime.Minutes;
  snapshot->Seconds = halTime.Seconds;

  return 1U;
}

static uint8_t AdapterWriteSnapshot(void *ctx, const RtcSnapshot_t *snapshot)
{
  RTC_TimeTypeDef halTime = { 0 };
  RTC_DateTypeDef halDate = { 0 };

  (void) ctx;

  if (snapshot == NULL)
  {
    return 0U;
  }

  halDate.Year = snapshot->Year;
  halDate.Month = snapshot->Month;
  halDate.Date = snapshot->Date;
  halDate.WeekDay = snapshot->WeekDay;

  halTime.Hours = snapshot->Hours;
  halTime.Minutes = snapshot->Minutes;
  halTime.Seconds = snapshot->Seconds;
  halTime.TimeFormat = RTC_HOURFORMAT_24;
  halTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  halTime.StoreOperation = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetDate(&hrtc, &halDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    return 0U;
  }

  if (HAL_RTC_SetTime(&hrtc, &halTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    return 0U;
  }

  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, snapshot->Century);

  return 1U;
}

static uint8_t AdapterReadMetadata(void *ctx, RtcMetadataId_t metadataId,
                                   uint32_t *value)
{
  uint32_t reg = 0U;

  (void) ctx;

  if ((value == NULL) || (BackupRegisterGet(metadataId, &reg) == 0U))
  {
    return 0U;
  }

  *value = HAL_RTCEx_BKUPRead(&hrtc, reg);

  return 1U;
}

static uint8_t AdapterWriteMetadata(void *ctx, RtcMetadataId_t metadataId,
                                    uint32_t value)
{
  uint32_t reg = 0U;

  (void) ctx;

  if (BackupRegisterGet(metadataId, &reg) == 0U)
  {
    return 0U;
  }

  HAL_RTCEx_BKUPWrite(&hrtc, reg, value);

  return 1U;
}

static uint8_t AdapterApplyDstAdjustment(void *ctx,
                                         RtcDstAdjustment_t adjustment)
{
  (void) ctx;

  switch (adjustment)
  {
      case RTC_DST_ADD_ONE_HOUR:
      {
        __HAL_RTC_DAYLIGHT_SAVING_TIME_ADD1H(&hrtc, RTC_STOREOPERATION_SET);
        break;
      }

      case RTC_DST_SUBTRACT_ONE_HOUR:
      {
        __HAL_RTC_DAYLIGHT_SAVING_TIME_SUB1H(&hrtc, RTC_STOREOPERATION_SET);
        break;
      }

      default:
      {
        return 0U;
      }
  }

  return 1U;
}

void RTCAdapterInit(RTCAdapterCtx_t *ctx)
{
  ctx->initialised = 0U;

  /* Read calendar values directly from the RTC counter instead of the
   * shadow registers. This avoids stale shadow snapshots on STM32H7 and
   * matches the polling-heavy usage pattern in TimeTask.
   */
  if (HAL_RTCEx_EnableBypassShadow(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  ctx->initialised = 1U;
}

IRealtimeClockPort_t RTCAdapterCreatePort(RTCAdapterCtx_t *ctx)
{
  IRealtimeClockPort_t port;

  port.ctx = ctx;
  port.ReadSnapshot = AdapterReadSnapshot;
  port.WriteSnapshot = AdapterWriteSnapshot;
  port.ReadMetadata = AdapterReadMetadata;
  port.WriteMetadata = AdapterWriteMetadata;
  port.ApplyDstAdjustment = AdapterApplyDstAdjustment;

  return port;
}
