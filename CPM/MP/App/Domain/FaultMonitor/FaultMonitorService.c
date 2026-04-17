/* App/Domain/FaultMonitor/FaultMonitorService.c */

#include "FaultMonitor/FaultMonitorService.h"

#include <stddef.h>
#include <string.h>

static uint8_t ChannelIndexFromSource(uint16_t source)
{
  /* Conflict / clearance events pack (a<<8 | b); for per-channel
   * indicators use the higher of the two participants so the
   * surface bit is set on at least one of the involved channels. */
  uint8_t high = (uint8_t) ((source >> 8U) & 0xFFU);
  uint8_t low = (uint8_t) (source & 0xFFU);

  return (high > low) ? high : low;
}

static void SetChannelFlag(FaultMonitorChannelFlags_t *flags,
                           FaultCode_t code)
{
  switch (code)
  {
      case FAULT_CODE_CONFLICT_GREEN_GREEN:
      case FAULT_CODE_CONFLICT_YELLOW_GREEN:
      case FAULT_CODE_CONFLICT_YELLOW_YELLOW:
      {
        flags->conflict = 1U;
        break;
      }

      case FAULT_CODE_DUAL_INDICATION:
      {
        flags->dualIndication = 1U;
        break;
      }

      case FAULT_CODE_RED_FAIL:
      {
        flags->redFail = 1U;
        break;
      }

      case FAULT_CODE_DARK_CHANNEL:
      {
        flags->dark = 1U;
        break;
      }

      case FAULT_CODE_MIN_YELLOW_SHORT:
      {
        flags->minYellowShort = 1U;
        break;
      }

      case FAULT_CODE_CLEARANCE_SHORT:
      {
        flags->clearanceShort = 1U;
        break;
      }

      case FAULT_CODE_SIGNAL_SEQUENCE:
      {
        flags->signalSequence = 1U;
        break;
      }

      case FAULT_CODE_LAMP_OPEN_CIRCUIT:
      case FAULT_CODE_LAMP_SHORT_CIRCUIT:
      case FAULT_CODE_LAMP_ALL_BROKEN:
      {
        flags->lampOpen = 1U;
        break;
      }

      case FAULT_CODE_LAMP_DRIVEN_EXTERNALLY:
      {
        flags->lampExternallyDriven = 1U;
        break;
      }

      case FAULT_CODE_NONE:
      case FAULT_CODE_VOLTAGE_SENSOR_FAILURE:
      case FAULT_CODE_LAMP_WORKING_COUNT_CHANGE:
      case FAULT_CODE_PSM_LINE_VOLTAGE_LOW:
      case FAULT_CODE_PSM_LINE_VOLTAGE_HIGH:
      case FAULT_CODE_PSM_FREQUENCY_LOW:
      case FAULT_CODE_PSM_FREQUENCY_HIGH:
      case FAULT_CODE_PSM_RAIL_24V_FAIL:
      case FAULT_CODE_PSM_RAIL_5V_FAIL:
      case FAULT_CODE_MODULE_CP_MISSING:
      case FAULT_CODE_MODULE_PSM_MISSING:
      case FAULT_CODE_MODULE_SSM_MISSING:
      case FAULT_CODE_MP_WATCHDOG:
      case FAULT_CODE_MP_BATTERY_LOW:
      case FAULT_CODE_MP_TEMPERATURE_HIGH:
      case FAULT_CODE_MP_CONFIG_INVALID:
      case FAULT_CODE_MP_RELAY_FEEDBACK_MISMATCH:
      case FAULT_CODE_COUNT:
      default:
      {
        break;
      }
  } /* switch */
} /* SetChannelFlag */

static void SetGlobalFlag(FaultMonitorGlobalFlags_t *flags, FaultCode_t code)
{
  switch (code)
  {
      case FAULT_CODE_PSM_LINE_VOLTAGE_LOW:
      case FAULT_CODE_PSM_LINE_VOLTAGE_HIGH:
      case FAULT_CODE_PSM_FREQUENCY_LOW:
      case FAULT_CODE_PSM_FREQUENCY_HIGH:
      {
        flags->acLineFault = 1U;
        break;
      }

      case FAULT_CODE_PSM_RAIL_24V_FAIL:
      {
        flags->rail24VFault = 1U;
        break;
      }

      case FAULT_CODE_PSM_RAIL_5V_FAIL:
      {
        flags->rail5VFault = 1U;
        break;
      }

      case FAULT_CODE_MODULE_CP_MISSING:
      {
        flags->cpMissing = 1U;
        break;
      }

      case FAULT_CODE_MODULE_PSM_MISSING:
      {
        flags->psmMissing = 1U;
        break;
      }

      case FAULT_CODE_MODULE_SSM_MISSING:
      {
        flags->ssmMissing = 1U;
        break;
      }

      case FAULT_CODE_MP_WATCHDOG:
      {
        flags->watchdog = 1U;
        break;
      }

      case FAULT_CODE_MP_RELAY_FEEDBACK_MISMATCH:
      {
        flags->relayFeedbackMismatch = 1U;
        break;
      }

      case FAULT_CODE_MP_CONFIG_INVALID:
      {
        flags->configInvalid = 1U;
        break;
      }

      case FAULT_CODE_NONE:
      case FAULT_CODE_CONFLICT_GREEN_GREEN:
      case FAULT_CODE_CONFLICT_YELLOW_GREEN:
      case FAULT_CODE_CONFLICT_YELLOW_YELLOW:
      case FAULT_CODE_DUAL_INDICATION:
      case FAULT_CODE_DARK_CHANNEL:
      case FAULT_CODE_SIGNAL_SEQUENCE:
      case FAULT_CODE_MIN_YELLOW_SHORT:
      case FAULT_CODE_CLEARANCE_SHORT:
      case FAULT_CODE_RED_FAIL:
      case FAULT_CODE_LAMP_OPEN_CIRCUIT:
      case FAULT_CODE_LAMP_SHORT_CIRCUIT:
      case FAULT_CODE_LAMP_DRIVEN_EXTERNALLY:
      case FAULT_CODE_LAMP_ALL_BROKEN:
      case FAULT_CODE_LAMP_WORKING_COUNT_CHANGE:
      case FAULT_CODE_VOLTAGE_SENSOR_FAILURE:
      case FAULT_CODE_MP_BATTERY_LOW:
      case FAULT_CODE_MP_TEMPERATURE_HIGH:
      case FAULT_CODE_COUNT:
      default:
      {
        break;
      }
  } /* switch */
} /* SetGlobalFlag */

void FaultMonitorServiceInit(FaultMonitorService_t *service)
{
  if (service == NULL)
  {
    return;
  }

  (void) memset(service, 0, sizeof(*service));
  FaultMonitorTraceInit(&service->trace);
}

void FaultMonitorServiceOnFault(FaultMonitorService_t *service,
                                const FaultEvent_t *event)
{
  if ((service == NULL) || (event == NULL))
  {
    return;
  }

  service->totalFaults++;

  uint8_t channelIdx = ChannelIndexFromSource(event->source);

  if (channelIdx < MP_CHANNEL_COUNT_MAX)
  {
    SetChannelFlag(&service->status.channels[channelIdx], event->code);
    SetChannelFlag(&service->statusSinceLastRead.channels[channelIdx],
                   event->code);
  }

  SetGlobalFlag(&service->status.global, event->code);
  SetGlobalFlag(&service->statusSinceLastRead.global, event->code);

  service->status.lastUpdateTicks = event->timestampTicks;
  service->statusSinceLastRead.lastUpdateTicks = event->timestampTicks;
  service->status.sequence++;
  service->statusSinceLastRead.sequence++;

  FaultMonitorTraceAppend(&service->trace, event);
}

void FaultMonitorServiceRead(FaultMonitorService_t *service,
                             FaultMonitorStatus_t *outStatus)
{
  if ((service == NULL) || (outStatus == NULL))
  {
    return;
  }

  *outStatus = service->status;
}

void FaultMonitorServiceReadAndAck(FaultMonitorService_t *service,
                                   FaultMonitorStatus_t *outStatus)
{
  if ((service == NULL) || (outStatus == NULL))
  {
    return;
  }

  *outStatus = service->statusSinceLastRead;
  FaultMonitorStatusClear(&service->statusSinceLastRead);
}

const FaultMonitorTrace_t *FaultMonitorServiceGetTrace(
  const FaultMonitorService_t *service)
{
  if (service == NULL)
  {
    return NULL;
  }

  return &service->trace;
}
