/* App/Domain/Alarm/UnitAlarmService.h
 *
 * Domain wrapper around IUnitAlarmPort that mirrors CP's NTCIP 1202
 * unit alarm surface. Fault events update unitAlarmStatus1..4 bits
 * and the per-group alarm state array, matching the OID semantics
 * so CP can project MP's alarm state over SNMP unchanged.
 *
 * The bit layout follows NTCIP 1202 §5.11 unitAlarmStatus conventions:
 *   unitAlarmStatus1  non-critical indications (SSM/PSM missing,
 *                      battery, temperature, config invalid)
 *   unitAlarmStatus2  ack-on-read summary of emergency indications
 *                      (conflict, dual indication, red fail, clearance)
 *   unitAlarmStatus3  line-voltage / frequency / rail failures
 *   unitAlarmStatus4  MMU-local indications (watchdog, relay feedback)
 */
#ifndef UNIT_ALARM_SERVICE_H
#define UNIT_ALARM_SERVICE_H

#include <stdint.h>

#include "Malfunction/FaultCodes.h"
#include "Ports/IUnitAlarmPort.h"

/* unitAlarmStatus1 bits */
#define UNIT_ALARM_STATUS1_MODULE_MISSING  0x01U
#define UNIT_ALARM_STATUS1_BATTERY_LOW     0x02U
#define UNIT_ALARM_STATUS1_TEMP_HIGH       0x04U
#define UNIT_ALARM_STATUS1_CONFIG_INVALID  0x08U
#define UNIT_ALARM_STATUS1_LAMP_FAULT      0x10U

/* unitAlarmStatus2 bits (ack-on-read) */
#define UNIT_ALARM_STATUS2_CONFLICT        0x01U
#define UNIT_ALARM_STATUS2_DUAL_INDICATION 0x02U
#define UNIT_ALARM_STATUS2_RED_FAIL        0x04U
#define UNIT_ALARM_STATUS2_CLEARANCE       0x08U
#define UNIT_ALARM_STATUS2_MIN_YELLOW      0x10U
#define UNIT_ALARM_STATUS2_DARK_CHANNEL    0x20U
#define UNIT_ALARM_STATUS2_SIGNAL_SEQUENCE 0x40U

/* unitAlarmStatus3 bits */
#define UNIT_ALARM_STATUS3_LINE_V_LOW      0x01U
#define UNIT_ALARM_STATUS3_LINE_V_HIGH     0x02U
#define UNIT_ALARM_STATUS3_FREQ_LOW        0x04U
#define UNIT_ALARM_STATUS3_FREQ_HIGH       0x08U
#define UNIT_ALARM_STATUS3_RAIL_24V_FAIL   0x10U
#define UNIT_ALARM_STATUS3_RAIL_5V_FAIL    0x20U

/* unitAlarmStatus4 bits */
#define UNIT_ALARM_STATUS4_WATCHDOG        0x01U
#define UNIT_ALARM_STATUS4_RELAY_MISMATCH  0x02U

typedef struct
{
  IUnitAlarmPort_t *port;
} UnitAlarmService_t;

void UnitAlarmServiceInit(UnitAlarmService_t *service,
                          IUnitAlarmPort_t *port);
void UnitAlarmServiceOnFault(UnitAlarmService_t *service,
                             const FaultEvent_t *event);
void UnitAlarmServiceClear(UnitAlarmService_t *service);

#endif /* UNIT_ALARM_SERVICE_H */
