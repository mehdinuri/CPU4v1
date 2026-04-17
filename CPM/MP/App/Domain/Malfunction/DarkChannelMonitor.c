/* App/Domain/Malfunction/DarkChannelMonitor.c */

#include "Malfunction/DarkChannelMonitor.h"

#include <stddef.h>
#include <string.h>

static uint8_t ChannelAnyLit(const ChannelColorState_t *state)
{
  return (uint8_t) ((state->red != 0U) || (state->yellow != 0U)
                    || (state->green != 0U));
}

void DarkChannelMonitorInit(DarkChannelMonitor_t *monitor, uint16_t threshold)
{
  if (monitor == NULL)
  {
    return;
  }

  (void) memset(monitor, 0, sizeof(*monitor));
  monitor->threshold = (threshold == 0U) ? DARK_CHANNEL_DWELL_TICKS_DEFAULT
                       : threshold;
}

void DarkChannelMonitorTick(DarkChannelMonitor_t *monitor,
                            const ChannelStateImage_t *commanded,
                            const ChannelStateImage_t *measured,
                            uint32_t timestampTicks,
                            const FaultEmit_t *emit)
{
  uint32_t i;

  if ((monitor == NULL) || (commanded == NULL) || (measured == NULL))
  {
    return;
  }

  for (i = 0U; i < MP_CHANNEL_COUNT_MAX; i++)
  {
    if ((ChannelAnyLit(&commanded->channels[i]) != 0U)
        && (ChannelAnyLit(&measured->channels[i]) == 0U))
    {
      if (monitor->dwell[i] < 0xFFFFU)
      {
        monitor->dwell[i]++;
      }

      if (monitor->dwell[i] == monitor->threshold)
      {
        FaultEvent_t event;

        event.code = FAULT_CODE_DARK_CHANNEL;
        event.severity = FAULT_SEVERITY_CRITICAL;
        event.source = (uint16_t) i;
        event.timestampTicks = timestampTicks;
        event.param = 0U;
        FaultEmitPublish(emit, &event);
      }
    }
    else
    {
      monitor->dwell[i] = 0U;
    }
  }
}
