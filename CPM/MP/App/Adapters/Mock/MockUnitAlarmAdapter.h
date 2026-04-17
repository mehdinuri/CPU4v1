/* App/Adapters/Mock/MockUnitAlarmAdapter.h
 *
 * IUnitAlarmPort in-memory test double.
 */
#ifndef MOCK_UNIT_ALARM_ADAPTER_H
#define MOCK_UNIT_ALARM_ADAPTER_H

#include "Ports/IUnitAlarmPort.h"

typedef struct
{
  uint8_t maxAlarmGroups;
  uint8_t alarmGroupState[UNIT_ALARM_GROUP_COUNT_MAX];
  uint8_t unitAlarmStatus1;
  uint8_t unitAlarmStatus2;
  uint8_t unitAlarmStatus3;
  uint8_t unitAlarmStatus4;
} MockUnitAlarmAdapterCtx_t;

void MockUnitAlarmAdapterInit(MockUnitAlarmAdapterCtx_t *ctx);
IUnitAlarmPort_t MockUnitAlarmAdapterCreatePort(MockUnitAlarmAdapterCtx_t *ctx);

#endif /* MOCK_UNIT_ALARM_ADAPTER_H */
