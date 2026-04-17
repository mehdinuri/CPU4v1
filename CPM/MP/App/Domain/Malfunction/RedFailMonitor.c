/* App/Domain/Malfunction/RedFailMonitor.c */

#include "Malfunction/RedFailMonitor.h"

#include <stddef.h>
#include <string.h>

void RedFailMonitorInit(RedFailMonitor_t *monitor, uint16_t threshold)
{
  if (monitor == NULL)
  {
    return;
  }

  (void) memset(monitor, 0, sizeof(*monitor));
  monitor->threshold = (threshold == 0U) ? RED_FAIL_DWELL_TICKS_DEFAULT
                       : threshold;
}

void RedFailMonitorTick(RedFailMonitor_t *monitor,
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
    const ChannelColorState_t *c = &commanded->channels[i];
    const ChannelColorState_t *m = &measured->channels[i];

    /* Only evaluate while the channel is commanded red alone (steady). */
    uint8_t steadyRed =
      (uint8_t) ((c->red != 0U) && (c->yellow == 0U) && (c->green == 0U));

    if ((steadyRed != 0U) && (m->red == 0U))
    {
      if (monitor->dwell[i] < 0xFFFFU)
      {
        monitor->dwell[i]++;
      }

      if (monitor->dwell[i] == monitor->threshold)
      {
        FaultEvent_t event;

        event.code = FAULT_CODE_RED_FAIL;
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
} /* RedFailMonitorTick */
