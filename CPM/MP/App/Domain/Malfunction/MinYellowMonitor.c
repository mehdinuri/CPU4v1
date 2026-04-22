/* App/Domain/Malfunction/MinYellowMonitor.c */

#include "Malfunction/MinYellowMonitor.h"

#include <stddef.h>
#include <string.h>

static uint16_t ChannelYellowMinTicks(const ConfigurationService_t *service,
                                      uint8_t channelIndex)
{
  uint16_t decis =
    ConfigurationServiceGetChannelMinYellowDs(service, channelIndex);

  return (uint16_t) (decis * MIN_YELLOW_TICKS_PER_DECISECOND);
}

void MinYellowMonitorInit(MinYellowMonitor_t *monitor)
{
  if (monitor == NULL)
  {
    return;
  }

  (void) memset(monitor, 0, sizeof(*monitor));
}

void MinYellowMonitorTick(MinYellowMonitor_t *monitor,
                          const ConfigurationService_t *config,
                          const ChannelStateImage_t *measured,
                          uint32_t timestampTicks,
                          const FaultEmit_t *emit)
{
  uint32_t i;

  if ((monitor == NULL) || (config == NULL) || (measured == NULL))
  {
    return;
  }

  for (i = 0U; i < MP_CHANNEL_COUNT_MAX; i++)
  {
    const ChannelColorState_t *m = &measured->channels[i];
    uint8_t yellowNow = (uint8_t) ((m->yellow != 0U) && (m->red == 0U)
                                   && (m->green == 0U));

    if (yellowNow != 0U)
    {
      monitor->inYellow[i] = 1U;

      if (monitor->yellowTicks[i] < 0xFFFFU)
      {
        monitor->yellowTicks[i]++;
      }
    }
    else if (monitor->inYellow[i] != 0U)
    {
      uint16_t minTicks = ChannelYellowMinTicks(config, (uint8_t) i);

      if ((minTicks != 0U) && (monitor->yellowTicks[i] < minTicks))
      {
        FaultEvent_t event;

        event.code = FAULT_CODE_MIN_YELLOW_SHORT;
        event.severity = FAULT_SEVERITY_CRITICAL;
        event.source = (uint16_t) i;
        event.timestampTicks = timestampTicks;
        event.param = (uint32_t) monitor->yellowTicks[i];
        FaultEmitPublish(emit, &event);
      }

      monitor->yellowTicks[i] = 0U;
      monitor->inYellow[i] = 0U;
    }
    else
    {
      monitor->yellowTicks[i] = 0U;
    }
  }
} /* MinYellowMonitorTick */
