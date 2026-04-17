/* App/Domain/Alarm/UnitAlarmService.c */

#include "Alarm/UnitAlarmService.h"

#include <stddef.h>

static void OrStatus1(IUnitAlarmPort_t *port, uint8_t bit)
{
  uint8_t current = 0U;

  if (UnitAlarmPortGetUnitAlarmStatus1(port, &current) == 0U)
  {
    current = 0U;
  }

  (void) UnitAlarmPortSetUnitAlarmStatus1(port, (uint8_t) (current | bit));
}

static void OrStatus2(IUnitAlarmPort_t *port, uint8_t bit)
{
  uint8_t current = 0U;

  if (UnitAlarmPortGetUnitAlarmStatus2(port, &current) == 0U)
  {
    current = 0U;
  }

  (void) UnitAlarmPortSetUnitAlarmStatus2(port, (uint8_t) (current | bit));
}

static void OrStatus3(IUnitAlarmPort_t *port, uint8_t bit)
{
  uint8_t current = 0U;

  if (UnitAlarmPortGetUnitAlarmStatus3(port, &current) == 0U)
  {
    current = 0U;
  }

  (void) UnitAlarmPortSetUnitAlarmStatus3(port, (uint8_t) (current | bit));
}

static void OrStatus4(IUnitAlarmPort_t *port, uint8_t bit)
{
  uint8_t current = 0U;

  if (UnitAlarmPortGetUnitAlarmStatus4(port, &current) == 0U)
  {
    current = 0U;
  }

  (void) UnitAlarmPortSetUnitAlarmStatus4(port, (uint8_t) (current | bit));
}

void UnitAlarmServiceInit(UnitAlarmService_t *service,
                          IUnitAlarmPort_t *port)
{
  if (service == NULL)
  {
    return;
  }

  service->port = port;
}

void UnitAlarmServiceOnFault(UnitAlarmService_t *service,
                             const FaultEvent_t *event)
{
  if ((service == NULL) || (event == NULL) || (service->port == NULL))
  {
    return;
  }

  switch (event->code)
  {
      case FAULT_CODE_CONFLICT_GREEN_GREEN:
      case FAULT_CODE_CONFLICT_YELLOW_GREEN:
      case FAULT_CODE_CONFLICT_YELLOW_YELLOW:
      {
        OrStatus2(service->port, UNIT_ALARM_STATUS2_CONFLICT);
        break;
      }

      case FAULT_CODE_DUAL_INDICATION:
      {
        OrStatus2(service->port, UNIT_ALARM_STATUS2_DUAL_INDICATION);
        break;
      }

      case FAULT_CODE_RED_FAIL:
      {
        OrStatus2(service->port, UNIT_ALARM_STATUS2_RED_FAIL);
        break;
      }

      case FAULT_CODE_CLEARANCE_SHORT:
      {
        OrStatus2(service->port, UNIT_ALARM_STATUS2_CLEARANCE);
        break;
      }

      case FAULT_CODE_MIN_YELLOW_SHORT:
      {
        OrStatus2(service->port, UNIT_ALARM_STATUS2_MIN_YELLOW);
        break;
      }

      case FAULT_CODE_DARK_CHANNEL:
      {
        OrStatus2(service->port, UNIT_ALARM_STATUS2_DARK_CHANNEL);
        break;
      }

      case FAULT_CODE_SIGNAL_SEQUENCE:
      {
        OrStatus2(service->port, UNIT_ALARM_STATUS2_SIGNAL_SEQUENCE);
        break;
      }

      case FAULT_CODE_MODULE_CP_MISSING:
      case FAULT_CODE_MODULE_PSM_MISSING:
      case FAULT_CODE_MODULE_SSM_MISSING:
      {
        OrStatus1(service->port, UNIT_ALARM_STATUS1_MODULE_MISSING);
        break;
      }

      case FAULT_CODE_MP_BATTERY_LOW:
      {
        OrStatus1(service->port, UNIT_ALARM_STATUS1_BATTERY_LOW);
        break;
      }

      case FAULT_CODE_MP_TEMPERATURE_HIGH:
      {
        OrStatus1(service->port, UNIT_ALARM_STATUS1_TEMP_HIGH);
        break;
      }

      case FAULT_CODE_MP_CONFIG_INVALID:
      {
        OrStatus1(service->port, UNIT_ALARM_STATUS1_CONFIG_INVALID);
        break;
      }

      case FAULT_CODE_LAMP_OPEN_CIRCUIT:
      case FAULT_CODE_LAMP_SHORT_CIRCUIT:
      case FAULT_CODE_LAMP_DRIVEN_EXTERNALLY:
      case FAULT_CODE_LAMP_ALL_BROKEN:
      case FAULT_CODE_LAMP_WORKING_COUNT_CHANGE:
      case FAULT_CODE_VOLTAGE_SENSOR_FAILURE:
      {
        OrStatus1(service->port, UNIT_ALARM_STATUS1_LAMP_FAULT);
        break;
      }

      case FAULT_CODE_PSM_LINE_VOLTAGE_LOW:
      {
        OrStatus3(service->port, UNIT_ALARM_STATUS3_LINE_V_LOW);
        break;
      }

      case FAULT_CODE_PSM_LINE_VOLTAGE_HIGH:
      {
        OrStatus3(service->port, UNIT_ALARM_STATUS3_LINE_V_HIGH);
        break;
      }

      case FAULT_CODE_PSM_FREQUENCY_LOW:
      {
        OrStatus3(service->port, UNIT_ALARM_STATUS3_FREQ_LOW);
        break;
      }

      case FAULT_CODE_PSM_FREQUENCY_HIGH:
      {
        OrStatus3(service->port, UNIT_ALARM_STATUS3_FREQ_HIGH);
        break;
      }

      case FAULT_CODE_PSM_RAIL_24V_FAIL:
      {
        OrStatus3(service->port, UNIT_ALARM_STATUS3_RAIL_24V_FAIL);
        break;
      }

      case FAULT_CODE_PSM_RAIL_5V_FAIL:
      {
        OrStatus3(service->port, UNIT_ALARM_STATUS3_RAIL_5V_FAIL);
        break;
      }

      case FAULT_CODE_MP_WATCHDOG:
      {
        OrStatus4(service->port, UNIT_ALARM_STATUS4_WATCHDOG);
        break;
      }

      case FAULT_CODE_MP_RELAY_FEEDBACK_MISMATCH:
      {
        OrStatus4(service->port, UNIT_ALARM_STATUS4_RELAY_MISMATCH);
        break;
      }

      case FAULT_CODE_NONE:
      case FAULT_CODE_COUNT:
      default:
      {
        break;
      }
  } /* switch */
} /* UnitAlarmServiceOnFault */

void UnitAlarmServiceClear(UnitAlarmService_t *service)
{
  if ((service == NULL) || (service->port == NULL))
  {
    return;
  }

  (void) UnitAlarmPortSetUnitAlarmStatus1(service->port, 0U);
  (void) UnitAlarmPortSetUnitAlarmStatus2(service->port, 0U);
  (void) UnitAlarmPortSetUnitAlarmStatus3(service->port, 0U);
  (void) UnitAlarmPortSetUnitAlarmStatus4(service->port, 0U);
}
