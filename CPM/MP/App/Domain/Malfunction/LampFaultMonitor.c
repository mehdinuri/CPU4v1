/* App/Domain/Malfunction/LampFaultMonitor.c */

#include "Malfunction/LampFaultMonitor.h"

#include <stddef.h>
#include <string.h>

void LampFaultMonitorInit(LampFaultMonitor_t *monitor, uint16_t threshold)
{
  if (monitor == NULL)
  {
    return;
  }

  (void) memset(monitor, 0, sizeof(*monitor));
  monitor->threshold = (threshold == 0U) ? LAMP_FAULT_DWELL_TICKS_DEFAULT
                       : threshold;
}

void LampFaultMonitorTick(LampFaultMonitor_t *monitor,
                          const FieldBusLoadSwitchImage_t *commanded,
                          const FieldBusSsmTelemetry_t *ssm,
                          uint8_t ssmCount,
                          uint32_t timestampTicks,
                          const FaultEmit_t *emit)
{
  uint32_t moduleIdx;
  uint32_t slot;

  if ((monitor == NULL) || (commanded == NULL) || (ssm == NULL))
  {
    return;
  }

  if (ssmCount > FIELD_BUS_SSM_COUNT)
  {
    return;
  }

  for (moduleIdx = 0U; moduleIdx < ssmCount; moduleIdx++)
  {
    if (ssm[moduleIdx].alive == 0U)
    {
      continue;
    }

    for (slot = 0U; slot < FIELD_BUS_SSM_OUTPUTS_PER_MODULE; slot++)
    {
      uint32_t outputIdx = (moduleIdx * FIELD_BUS_SSM_OUTPUTS_PER_MODULE)
                           + slot;

      if (outputIdx >= MP_SIGNAL_OUTPUT_COUNT_MAX)
      {
        break;
      }

      uint8_t cmd = ChannelStateResolverGetOutputBit(commanded,
                                                     (uint8_t) outputIdx);
      uint8_t meas = (uint8_t) ((ssm[moduleIdx].voltagePresenceBits >> slot)
                                & 0x01U);

      if ((cmd != 0U) && (meas == 0U))
      {
        if (monitor->openDwell[outputIdx] < 0xFFFFU)
        {
          monitor->openDwell[outputIdx]++;
        }

        if (monitor->openDwell[outputIdx] == monitor->threshold)
        {
          FaultEvent_t event;

          event.code = FAULT_CODE_LAMP_OPEN_CIRCUIT;
          event.severity = FAULT_SEVERITY_WARNING;
          event.source = (uint16_t) outputIdx;
          event.timestampTicks = timestampTicks;
          event.param = 0U;
          FaultEmitPublish(emit, &event);
        }
      }
      else
      {
        monitor->openDwell[outputIdx] = 0U;
      }

      if ((cmd == 0U) && (meas != 0U))
      {
        if (monitor->externalDwell[outputIdx] < 0xFFFFU)
        {
          monitor->externalDwell[outputIdx]++;
        }

        if (monitor->externalDwell[outputIdx] == monitor->threshold)
        {
          FaultEvent_t event;

          event.code = FAULT_CODE_LAMP_DRIVEN_EXTERNALLY;
          event.severity = FAULT_SEVERITY_ERROR;
          event.source = (uint16_t) outputIdx;
          event.timestampTicks = timestampTicks;
          event.param = 0U;
          FaultEmitPublish(emit, &event);
        }
      }
      else
      {
        monitor->externalDwell[outputIdx] = 0U;
      }
    }
  }
} /* LampFaultMonitorTick */
