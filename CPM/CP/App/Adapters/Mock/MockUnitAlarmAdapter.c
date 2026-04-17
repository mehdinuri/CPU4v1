/* App/Adapters/Mock/MockUnitAlarmAdapter.c */
#include "MockUnitAlarmAdapter.h"

#include <string.h>

static uint8_t AdapterGetMaxAlarmGroups(void *ctx, uint8_t *maxAlarmGroups)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (maxAlarmGroups == NULL))
  {
    return 0U;
  }

  *maxAlarmGroups = adapter->maxAlarmGroups;

  return 1U;
}

static uint8_t AdapterSetMaxAlarmGroups(void *ctx, uint8_t maxAlarmGroups)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (maxAlarmGroups == 0U))
  {
    return 0U;
  }

  adapter->maxAlarmGroups = maxAlarmGroups;

  return 1U;
}

static uint8_t AdapterGetAlarmGroupState(void *ctx,
                                         uint8_t groupIndex,
                                         uint8_t *alarmGroupState)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (alarmGroupState == NULL)
      || (groupIndex >= adapter->maxAlarmGroups))
  {
    return 0U;
  }

  *alarmGroupState = adapter->alarmGroupState[groupIndex];

  return 1U;
}

static uint8_t AdapterSetAlarmGroupState(void *ctx,
                                         uint8_t groupIndex,
                                         uint8_t alarmGroupState)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (groupIndex >= UNIT_ALARM_GROUP_COUNT_MAX)
      || (groupIndex >= adapter->maxAlarmGroups))
  {
    return 0U;
  }

  adapter->alarmGroupState[groupIndex] = alarmGroupState;

  return 1U;
}

static uint8_t AdapterGetUnitAlarmStatus2(void *ctx, uint8_t *unitAlarmStatus2)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (unitAlarmStatus2 == NULL))
  {
    return 0U;
  }

  *unitAlarmStatus2 = adapter->unitAlarmStatus2;

  return 1U;
}

static uint8_t AdapterGetUnitAlarmStatus1(void *ctx, uint8_t *unitAlarmStatus1)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (unitAlarmStatus1 == NULL))
  {
    return 0U;
  }

  *unitAlarmStatus1 = adapter->unitAlarmStatus1;

  return 1U;
}

static uint8_t AdapterSetUnitAlarmStatus1(void *ctx, uint8_t unitAlarmStatus1)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if (adapter == NULL)
  {
    return 0U;
  }

  adapter->unitAlarmStatus1 = unitAlarmStatus1;

  return 1U;
}

static uint8_t AdapterSetUnitAlarmStatus2(void *ctx, uint8_t unitAlarmStatus2)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if (adapter == NULL)
  {
    return 0U;
  }

  adapter->unitAlarmStatus2 = unitAlarmStatus2;

  return 1U;
}

static void AdapterAcknowledgeUnitAlarmStatus2Read(void *ctx)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if (adapter == NULL)
  {
    return;
  }

  adapter->unitAlarmStatus2 &= (uint8_t) ~0x01U;
}

static uint8_t AdapterGetUnitAlarmStatus3(void *ctx, uint8_t *unitAlarmStatus3)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (unitAlarmStatus3 == NULL))
  {
    return 0U;
  }

  *unitAlarmStatus3 = adapter->unitAlarmStatus3;

  return 1U;
}

static uint8_t AdapterSetUnitAlarmStatus3(void *ctx, uint8_t unitAlarmStatus3)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if (adapter == NULL)
  {
    return 0U;
  }

  adapter->unitAlarmStatus3 = unitAlarmStatus3;

  return 1U;
}

static uint8_t AdapterGetUnitAlarmStatus4(void *ctx, uint8_t *unitAlarmStatus4)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (unitAlarmStatus4 == NULL))
  {
    return 0U;
  }

  *unitAlarmStatus4 = adapter->unitAlarmStatus4;

  return 1U;
}

static uint8_t AdapterSetUnitAlarmStatus4(void *ctx, uint8_t unitAlarmStatus4)
{
  MockUnitAlarmAdapterCtx_t *adapter = (MockUnitAlarmAdapterCtx_t *) ctx;

  if (adapter == NULL)
  {
    return 0U;
  }

  adapter->unitAlarmStatus4 = unitAlarmStatus4;

  return 1U;
}

void MockUnitAlarmAdapterInit(MockUnitAlarmAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  memset(ctx->alarmGroupState, 0, sizeof(ctx->alarmGroupState));
  ctx->maxAlarmGroups = 1U;
  ctx->unitAlarmStatus1 = 0U;
  ctx->unitAlarmStatus2 = 0U;
  ctx->unitAlarmStatus3 = 0U;
  ctx->unitAlarmStatus4 = 0U;
}

IUnitAlarmPort_t MockUnitAlarmAdapterCreatePort(MockUnitAlarmAdapterCtx_t *ctx)
{
  IUnitAlarmPort_t port;

  port.ctx = ctx;
  port.GetMaxAlarmGroups = AdapterGetMaxAlarmGroups;
  port.SetMaxAlarmGroups = AdapterSetMaxAlarmGroups;
  port.GetAlarmGroupState = AdapterGetAlarmGroupState;
  port.SetAlarmGroupState = AdapterSetAlarmGroupState;
  port.GetUnitAlarmStatus1 = AdapterGetUnitAlarmStatus1;
  port.SetUnitAlarmStatus1 = AdapterSetUnitAlarmStatus1;
  port.GetUnitAlarmStatus2 = AdapterGetUnitAlarmStatus2;
  port.SetUnitAlarmStatus2 = AdapterSetUnitAlarmStatus2;
  port.AcknowledgeUnitAlarmStatus2Read = AdapterAcknowledgeUnitAlarmStatus2Read;
  port.GetUnitAlarmStatus3 = AdapterGetUnitAlarmStatus3;
  port.SetUnitAlarmStatus3 = AdapterSetUnitAlarmStatus3;
  port.GetUnitAlarmStatus4 = AdapterGetUnitAlarmStatus4;
  port.SetUnitAlarmStatus4 = AdapterSetUnitAlarmStatus4;

  return port;
}
