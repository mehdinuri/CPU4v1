/* App/Adapters/Mock/MockRTCAdapter.c */
#include "MockRTCAdapter.h"

#include <string.h>

static uint8_t AdapterReadSnapshot(void *ctx, RtcSnapshot_t *snapshot)
{
  MockRTCAdapterCtx_t *c = (MockRTCAdapterCtx_t *) ctx;

  if (snapshot == NULL)
  {
    return 0U;
  }

  *snapshot = c->snapshot;

  return 1U;
}

static uint8_t AdapterWriteSnapshot(void *ctx, const RtcSnapshot_t *snapshot)
{
  MockRTCAdapterCtx_t *c = (MockRTCAdapterCtx_t *) ctx;

  if (snapshot == NULL)
  {
    return 0U;
  }

  c->snapshot = *snapshot;
  c->centuryMetadata = snapshot->Century;

  return 1U;
}

static uint8_t AdapterReadMetadata(void *ctx, RtcMetadataId_t metadataId,
                                   uint32_t *value)
{
  MockRTCAdapterCtx_t *c = (MockRTCAdapterCtx_t *) ctx;

  if (value == NULL)
  {
    return 0U;
  }

  switch (metadataId)
  {
      case RTC_META_INIT_SIGNATURE:
      {
        *value = c->initSignature;

        return 1U;
      }

      case RTC_META_CENTURY:
      {
        *value = c->centuryMetadata;

        return 1U;
      }

      default:
      {
        return 0U;
      }
  }
}

static uint8_t AdapterWriteMetadata(void *ctx, RtcMetadataId_t metadataId,
                                    uint32_t value)
{
  MockRTCAdapterCtx_t *c = (MockRTCAdapterCtx_t *) ctx;

  switch (metadataId)
  {
      case RTC_META_INIT_SIGNATURE:
      {
        c->initSignature = value;

        return 1U;
      }

      case RTC_META_CENTURY:
      {
        c->centuryMetadata = value;
        c->snapshot.Century = (uint8_t) value;

        return 1U;
      }

      default:
      {
        return 0U;
      }
  }
}

static uint8_t AdapterApplyDstAdjustment(void *ctx,
                                         RtcDstAdjustment_t adjustment)
{
  MockRTCAdapterCtx_t *c = (MockRTCAdapterCtx_t *) ctx;

  c->lastAdjustment = adjustment;
  c->adjustmentCount++;

  switch (adjustment)
  {
      case RTC_DST_ADD_ONE_HOUR:
      {
        c->snapshot.Hours = (uint8_t) ((c->snapshot.Hours + 1U) % 24U);

        return 1U;
      }

      case RTC_DST_SUBTRACT_ONE_HOUR:
      {
        c->snapshot.Hours = (uint8_t) ((c->snapshot.Hours + 23U) % 24U);

        return 1U;
      }

      default:
      {
        return 0U;
      }
  }
}

void MockRTCAdapterInit(MockRTCAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IRealtimeClockPort_t MockRTCAdapterCreatePort(MockRTCAdapterCtx_t *ctx)
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
