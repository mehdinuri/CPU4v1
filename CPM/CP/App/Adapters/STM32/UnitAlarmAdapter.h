/* App/Adapters/STM32/UnitAlarmAdapter.h
 *
 * In-memory unit alarm state adapter used by the production wiring until
 * alarm sources are bound to the new architecture.
 */
#ifndef UNIT_ALARM_ADAPTER_H
#define UNIT_ALARM_ADAPTER_H

#include "Ports/IUnitAlarmPort.h"

typedef struct
{
  uint8_t maxAlarmGroups;
  uint8_t alarmGroupState[UNIT_ALARM_GROUP_COUNT_MAX];
  uint8_t unitAlarmStatus1;
  uint8_t unitAlarmStatus2;
  uint8_t unitAlarmStatus3;
  uint8_t unitAlarmStatus4;
} UnitAlarmAdapterCtx_t;

void UnitAlarmAdapterInit(UnitAlarmAdapterCtx_t *ctx);
IUnitAlarmPort_t UnitAlarmAdapterCreatePort(UnitAlarmAdapterCtx_t *ctx);

#endif /* UNIT_ALARM_ADAPTER_H */
