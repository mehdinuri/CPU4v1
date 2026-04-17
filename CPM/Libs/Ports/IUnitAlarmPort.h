/* App/Ports/IUnitAlarmPort.h
 *
 * Port interface for application-owned unit alarm state exposed through the
 * NTCIP 1202 unit alarm objects.
 */
#ifndef I_UNIT_ALARM_PORT_H
#define I_UNIT_ALARM_PORT_H

#include <stddef.h>
#include <stdint.h>

#define UNIT_ALARM_GROUP_COUNT_MAX 255U

typedef struct
{
  void *ctx;

  uint8_t (*GetMaxAlarmGroups)(void *ctx, uint8_t *maxAlarmGroups);
  uint8_t (*SetMaxAlarmGroups)(void *ctx, uint8_t maxAlarmGroups);
  uint8_t (*GetAlarmGroupState)(void *ctx,
                                uint8_t groupIndex,
                                uint8_t *alarmGroupState);
  uint8_t (*SetAlarmGroupState)(void *ctx,
                                uint8_t groupIndex,
                                uint8_t alarmGroupState);
  uint8_t (*GetUnitAlarmStatus1)(void *ctx, uint8_t *unitAlarmStatus1);
  uint8_t (*SetUnitAlarmStatus1)(void *ctx, uint8_t unitAlarmStatus1);
  uint8_t (*GetUnitAlarmStatus2)(void *ctx, uint8_t *unitAlarmStatus2);
  uint8_t (*SetUnitAlarmStatus2)(void *ctx, uint8_t unitAlarmStatus2);
  void (*AcknowledgeUnitAlarmStatus2Read)(void *ctx);
  uint8_t (*GetUnitAlarmStatus3)(void *ctx, uint8_t *unitAlarmStatus3);
  uint8_t (*SetUnitAlarmStatus3)(void *ctx, uint8_t unitAlarmStatus3);
  uint8_t (*GetUnitAlarmStatus4)(void *ctx, uint8_t *unitAlarmStatus4);
  uint8_t (*SetUnitAlarmStatus4)(void *ctx, uint8_t unitAlarmStatus4);
} IUnitAlarmPort_t;

static inline uint8_t UnitAlarmPortGetMaxAlarmGroups(
  const IUnitAlarmPort_t *port,
  uint8_t *maxAlarmGroups)
{
  if ((port == NULL) || (port->GetMaxAlarmGroups == NULL))
  {
    return 0U;
  }

  return port->GetMaxAlarmGroups(port->ctx, maxAlarmGroups);
}

static inline uint8_t UnitAlarmPortSetMaxAlarmGroups(
  IUnitAlarmPort_t *port,
  uint8_t maxAlarmGroups)
{
  if ((port == NULL) || (port->SetMaxAlarmGroups == NULL))
  {
    return 0U;
  }

  return port->SetMaxAlarmGroups(port->ctx, maxAlarmGroups);
}

static inline uint8_t UnitAlarmPortGetAlarmGroupState(
  const IUnitAlarmPort_t *port,
  uint8_t groupIndex,
  uint8_t *alarmGroupState)
{
  if ((port == NULL) || (port->GetAlarmGroupState == NULL))
  {
    return 0U;
  }

  return port->GetAlarmGroupState(port->ctx, groupIndex, alarmGroupState);
}

static inline uint8_t UnitAlarmPortSetAlarmGroupState(
  IUnitAlarmPort_t *port,
  uint8_t groupIndex,
  uint8_t alarmGroupState)
{
  if ((port == NULL) || (port->SetAlarmGroupState == NULL))
  {
    return 0U;
  }

  return port->SetAlarmGroupState(port->ctx, groupIndex, alarmGroupState);
}

static inline uint8_t UnitAlarmPortGetUnitAlarmStatus2(
  const IUnitAlarmPort_t *port,
  uint8_t *unitAlarmStatus2)
{
  if ((port == NULL) || (port->GetUnitAlarmStatus2 == NULL))
  {
    return 0U;
  }

  return port->GetUnitAlarmStatus2(port->ctx, unitAlarmStatus2);
}

static inline uint8_t UnitAlarmPortSetUnitAlarmStatus2(
  IUnitAlarmPort_t *port,
  uint8_t unitAlarmStatus2)
{
  if ((port == NULL) || (port->SetUnitAlarmStatus2 == NULL))
  {
    return 0U;
  }

  return port->SetUnitAlarmStatus2(port->ctx, unitAlarmStatus2);
}

static inline uint8_t UnitAlarmPortGetUnitAlarmStatus1(
  const IUnitAlarmPort_t *port,
  uint8_t *unitAlarmStatus1)
{
  if ((port == NULL) || (port->GetUnitAlarmStatus1 == NULL))
  {
    return 0U;
  }

  return port->GetUnitAlarmStatus1(port->ctx, unitAlarmStatus1);
}

static inline uint8_t UnitAlarmPortSetUnitAlarmStatus1(
  IUnitAlarmPort_t *port,
  uint8_t unitAlarmStatus1)
{
  if ((port == NULL) || (port->SetUnitAlarmStatus1 == NULL))
  {
    return 0U;
  }

  return port->SetUnitAlarmStatus1(port->ctx, unitAlarmStatus1);
}

static inline void UnitAlarmPortAcknowledgeUnitAlarmStatus2Read(
  const IUnitAlarmPort_t *port)
{
  if ((port != NULL) && (port->AcknowledgeUnitAlarmStatus2Read != NULL))
  {
    port->AcknowledgeUnitAlarmStatus2Read(port->ctx);
  }
}

static inline uint8_t UnitAlarmPortGetUnitAlarmStatus3(
  const IUnitAlarmPort_t *port,
  uint8_t *unitAlarmStatus3)
{
  if ((port == NULL) || (port->GetUnitAlarmStatus3 == NULL))
  {
    return 0U;
  }

  return port->GetUnitAlarmStatus3(port->ctx, unitAlarmStatus3);
}

static inline uint8_t UnitAlarmPortSetUnitAlarmStatus3(
  IUnitAlarmPort_t *port,
  uint8_t unitAlarmStatus3)
{
  if ((port == NULL) || (port->SetUnitAlarmStatus3 == NULL))
  {
    return 0U;
  }

  return port->SetUnitAlarmStatus3(port->ctx, unitAlarmStatus3);
}

static inline uint8_t UnitAlarmPortGetUnitAlarmStatus4(
  const IUnitAlarmPort_t *port,
  uint8_t *unitAlarmStatus4)
{
  if ((port == NULL) || (port->GetUnitAlarmStatus4 == NULL))
  {
    return 0U;
  }

  return port->GetUnitAlarmStatus4(port->ctx, unitAlarmStatus4);
}

static inline uint8_t UnitAlarmPortSetUnitAlarmStatus4(
  IUnitAlarmPort_t *port,
  uint8_t unitAlarmStatus4)
{
  if ((port == NULL) || (port->SetUnitAlarmStatus4 == NULL))
  {
    return 0U;
  }

  return port->SetUnitAlarmStatus4(port->ctx, unitAlarmStatus4);
}

#endif /* I_UNIT_ALARM_PORT_H */
