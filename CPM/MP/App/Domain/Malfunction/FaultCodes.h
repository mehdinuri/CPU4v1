/* App/Domain/Malfunction/FaultCodes.h
 *
 * MP fault code enumeration. Every monitor raises one of these; the
 * SafetyDecisionService maps them to relay actions and the
 * FaultMonitorService lights the matching bit in FaultMonitorStatus_t.
 *
 * Values are stable wire constants — CP maps them into NTCIP 1202
 * unit alarm groups. Do not renumber.
 */
#ifndef FAULT_CODES_H
#define FAULT_CODES_H

#include <stdint.h>

typedef enum
{
  FAULT_CODE_NONE = 0U,

  /* Field-bus command vs. measured inconsistencies */
  FAULT_CODE_CONFLICT_GREEN_GREEN = 1U,
  FAULT_CODE_CONFLICT_YELLOW_GREEN = 2U,
  FAULT_CODE_CONFLICT_YELLOW_YELLOW = 3U,
  FAULT_CODE_DUAL_INDICATION = 4U,
  FAULT_CODE_DARK_CHANNEL = 5U,
  FAULT_CODE_SIGNAL_SEQUENCE = 6U,

  /* TS2 timing faults */
  FAULT_CODE_MIN_YELLOW_SHORT = 10U,
  FAULT_CODE_CLEARANCE_SHORT = 11U,
  FAULT_CODE_RED_FAIL = 12U,

  /* Lamp / load-switch faults */
  FAULT_CODE_LAMP_OPEN_CIRCUIT = 20U,
  FAULT_CODE_LAMP_SHORT_CIRCUIT = 21U,
  FAULT_CODE_LAMP_DRIVEN_EXTERNALLY = 22U,
  FAULT_CODE_LAMP_ALL_BROKEN = 23U,
  FAULT_CODE_LAMP_WORKING_COUNT_CHANGE = 24U,
  FAULT_CODE_VOLTAGE_SENSOR_FAILURE = 25U,

  /* Power supply */
  FAULT_CODE_PSM_LINE_VOLTAGE_LOW = 30U,
  FAULT_CODE_PSM_LINE_VOLTAGE_HIGH = 31U,
  FAULT_CODE_PSM_FREQUENCY_LOW = 32U,
  FAULT_CODE_PSM_FREQUENCY_HIGH = 33U,
  FAULT_CODE_PSM_RAIL_24V_FAIL = 34U,
  FAULT_CODE_PSM_RAIL_5V_FAIL = 35U,

  /* Module liveness */
  FAULT_CODE_MODULE_CP_MISSING = 40U,
  FAULT_CODE_MODULE_PSM_MISSING = 41U,
  FAULT_CODE_MODULE_SSM_MISSING = 42U,

  /* MP self */
  FAULT_CODE_MP_WATCHDOG = 50U,
  FAULT_CODE_MP_BATTERY_LOW = 51U,
  FAULT_CODE_MP_TEMPERATURE_HIGH = 52U,
  FAULT_CODE_MP_CONFIG_INVALID = 53U,
  FAULT_CODE_MP_RELAY_FEEDBACK_MISMATCH = 54U,

  FAULT_CODE_COUNT
} FaultCode_t;

typedef enum
{
  FAULT_SEVERITY_INFO = 0U,
  FAULT_SEVERITY_WARNING = 1U,
  FAULT_SEVERITY_ERROR = 2U,
  FAULT_SEVERITY_CRITICAL = 3U
} FaultSeverity_t;

typedef struct
{
  FaultCode_t code;
  FaultSeverity_t severity;
  uint16_t source;    /* channel or module index the fault is about */
  uint32_t timestampTicks;
  uint32_t param;     /* code-specific payload */
} FaultEvent_t;

#endif /* FAULT_CODES_H */
