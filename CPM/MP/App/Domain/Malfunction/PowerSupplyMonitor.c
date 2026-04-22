/* App/Domain/Malfunction/PowerSupplyMonitor.c */

#include "Malfunction/PowerSupplyMonitor.h"

#include <stddef.h>
#include <string.h>

/* From the legacy MP thresholds:
 *   VOLTAGE_VALUE_LOWER_BOUND = 241 (176 V)
 *   VOLTAGE_VALUE_UPPER_BOUND = 364 (265 V)
 *   Hysteresis 14 (10 V)
 *   FREQUENCY_VALUE_LOWER_BOUND = 95  (-4 %)
 *   FREQUENCY_VALUE_UPPER_BOUND = 105 (+4 %)
 * 24 V / 5 V rails - legacy checks presence only; using nominal-minus-15%. */
const PowerSupplyMonitorLimits_t PowerSupplyMonitorDefaultLimits = {
  .lineVoltageLower = 241U,
  .lineVoltageUpper = 364U,
  .lineVoltageHysteresis = 14U,
  .lineFrequencyLower = 95U,
  .lineFrequencyUpper = 105U,
  .rail24VMin = 20U,            /* arbitrary until ADC scaling confirmed */
  .rail5VMin = 4U
};

static void Evaluate(uint8_t *latched,
                     uint16_t value,
                     uint16_t lower,
                     uint16_t upper,
                     uint16_t hysteresis,
                     FaultCode_t lowCode,
                     FaultCode_t highCode,
                     uint16_t source,
                     uint32_t timestampTicks,
                     const FaultEmit_t *emit)
{
  if (*latched == 0U)
  {
    if ((lower != 0U) && (value < lower))
    {
      FaultEvent_t event;

      event.code = lowCode;
      event.severity = FAULT_SEVERITY_ERROR;
      event.source = source;
      event.timestampTicks = timestampTicks;
      event.param = (uint32_t) value;
      FaultEmitPublish(emit, &event);
      *latched = 1U;
    }
    else if ((upper != 0U) && (value > upper))
    {
      FaultEvent_t event;

      event.code = highCode;
      event.severity = FAULT_SEVERITY_ERROR;
      event.source = source;
      event.timestampTicks = timestampTicks;
      event.param = (uint32_t) value;
      FaultEmitPublish(emit, &event);
      *latched = 1U;
    }
    else
    {
      /* nothing */
    }
  }
  else
  {
    if ((value >= (uint16_t) (lower + hysteresis))
        && (value <= (uint16_t) (upper - hysteresis)))
    {
      *latched = 0U;
    }
  }
} /* Evaluate */

void PowerSupplyMonitorInit(PowerSupplyMonitor_t *monitor,
                            const PowerSupplyMonitorLimits_t *limits)
{
  if (monitor == NULL)
  {
    return;
  }

  (void) memset(monitor, 0, sizeof(*monitor));
  monitor->limits = (limits != NULL) ? *limits
                    : PowerSupplyMonitorDefaultLimits;
}

void PowerSupplyMonitorTick(PowerSupplyMonitor_t *monitor,
                            const FieldBusSnapshot_t *snapshot,
                            uint32_t timestampTicks,
                            const FaultEmit_t *emit)
{
  uint32_t i;

  if ((monitor == NULL) || (snapshot == NULL))
  {
    return;
  }

  for (i = 0U; i < FIELD_BUS_PSM_COUNT; i++)
  {
    const FieldBusPsmTelemetry_t *psm = &snapshot->psm[i];

    if (psm->alive == 0U)
    {
      continue;
    }

    Evaluate(&monitor->lineVoltageFaultLatched[i],
             psm->lineVoltageAdcCounts,
             monitor->limits.lineVoltageLower,
             monitor->limits.lineVoltageUpper,
             monitor->limits.lineVoltageHysteresis,
             FAULT_CODE_PSM_LINE_VOLTAGE_LOW,
             FAULT_CODE_PSM_LINE_VOLTAGE_HIGH,
             (uint16_t) i,
             timestampTicks,
             emit);

    Evaluate(&monitor->frequencyFaultLatched[i],
             psm->lineFrequencyRaw,
             monitor->limits.lineFrequencyLower,
             monitor->limits.lineFrequencyUpper,
             0U,
             FAULT_CODE_PSM_FREQUENCY_LOW,
             FAULT_CODE_PSM_FREQUENCY_HIGH,
             (uint16_t) i,
             timestampTicks,
             emit);

    if ((monitor->limits.rail24VMin != 0U)
        && (monitor->rail24FaultLatched[i] == 0U)
        && ((psm->rail24V1AdcCounts < monitor->limits.rail24VMin)
            || (psm->rail24V2AdcCounts < monitor->limits.rail24VMin)))
    {
      FaultEvent_t event;

      event.code = FAULT_CODE_PSM_RAIL_24V_FAIL;
      event.severity = FAULT_SEVERITY_ERROR;
      event.source = (uint16_t) i;
      event.timestampTicks = timestampTicks;
      event.param = (uint32_t) psm->rail24V1AdcCounts
                    | ((uint32_t) psm->rail24V2AdcCounts << 16U);
      FaultEmitPublish(emit, &event);
      monitor->rail24FaultLatched[i] = 1U;
    }
    else if ((psm->rail24V1AdcCounts
              >= (uint16_t) (monitor->limits.rail24VMin + 2U))
             && (psm->rail24V2AdcCounts
                 >= (uint16_t) (monitor->limits.rail24VMin + 2U)))
    {
      monitor->rail24FaultLatched[i] = 0U;
    }
    else
    {
      /* nothing */
    }

    if ((monitor->limits.rail5VMin != 0U)
        && (monitor->rail5FaultLatched[i] == 0U)
        && ((psm->rail5V1AdcCounts < monitor->limits.rail5VMin)
            || (psm->rail5V2AdcCounts < monitor->limits.rail5VMin)))
    {
      FaultEvent_t event;

      event.code = FAULT_CODE_PSM_RAIL_5V_FAIL;
      event.severity = FAULT_SEVERITY_ERROR;
      event.source = (uint16_t) i;
      event.timestampTicks = timestampTicks;
      event.param = (uint32_t) psm->rail5V1AdcCounts
                    | ((uint32_t) psm->rail5V2AdcCounts << 16U);
      FaultEmitPublish(emit, &event);
      monitor->rail5FaultLatched[i] = 1U;
    }
    else if ((psm->rail5V1AdcCounts
              >= (uint16_t) (monitor->limits.rail5VMin + 2U))
             && (psm->rail5V2AdcCounts
                 >= (uint16_t) (monitor->limits.rail5VMin + 2U)))
    {
      monitor->rail5FaultLatched[i] = 0U;
    }
    else
    {
      /* nothing */
    }
  }
} /* PowerSupplyMonitorTick */
