/* App/Domain/Malfunction/DualIndicationMonitor.c */

#include "Malfunction/DualIndicationMonitor.h"

#include <stddef.h>
#include <string.h>

void DualIndicationMonitorInit(DualIndicationMonitor_t *monitor,
                               uint16_t threshold)
{
  if (monitor == NULL)
  {
    return;
  }

  (void) memset(monitor, 0, sizeof(*monitor));
  monitor->threshold = (threshold == 0U) ? DUAL_INDICATION_DWELL_TICKS_DEFAULT
                       : threshold;
}

static uint8_t LitCount(const ChannelColorState_t *state)
{
  uint8_t count = 0U;

  if (state->red != 0U)
  {
    count++;
  }

  if (state->yellow != 0U)
  {
    count++;
  }

  if (state->green != 0U)
  {
    count++;
  }

  return count;
}

void DualIndicationMonitorTick(DualIndicationMonitor_t *monitor,
                               const ChannelStateImage_t *measured,
                               uint32_t timestampTicks,
                               const FaultEmit_t *emit)
{
  uint32_t i;

  if ((monitor == NULL) || (measured == NULL))
  {
    return;
  }

  for (i = 0U; i < MP_CHANNEL_COUNT_MAX; i++)
  {
    if (LitCount(&measured->channels[i]) >= 2U)
    {
      if (monitor->dwell[i] < 0xFFFFU)
      {
        monitor->dwell[i]++;
      }

      if (monitor->dwell[i] == monitor->threshold)
      {
        FaultEvent_t event;

        event.code = FAULT_CODE_DUAL_INDICATION;
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
