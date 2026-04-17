/* App/Domain/Malfunction/SignalSequenceMonitor.c */

#include "Malfunction/SignalSequenceMonitor.h"

#include <stddef.h>
#include <string.h>

static ChannelColor_t DominantColor(const ChannelColorState_t *s)
{
  if ((s->red == 0U) && (s->yellow == 0U) && (s->green == 0U))
  {
    return CHANNEL_COLOR_NONE;
  }

  if (s->green != 0U)
  {
    return CHANNEL_COLOR_GREEN;
  }

  if (s->yellow != 0U)
  {
    return CHANNEL_COLOR_YELLOW;
  }

  return CHANNEL_COLOR_RED;
}

static uint8_t IsLegalTransition(ChannelColor_t from, ChannelColor_t to)
{
  if (from == to)
  {
    return 1U;
  }

  /* Any color can legally return to dark (flash enter / power loss). */
  if (to == CHANNEL_COLOR_NONE)
  {
    return 1U;
  }

  /* Any color can legally come from dark (startup / flash exit). */
  if (from == CHANNEL_COLOR_NONE)
  {
    return 1U;
  }

  switch (from)
  {
      case CHANNEL_COLOR_RED:
      {
        return (uint8_t) (to == CHANNEL_COLOR_GREEN);
      }

      case CHANNEL_COLOR_GREEN:
      {
        return (uint8_t) (to == CHANNEL_COLOR_YELLOW);
      }

      case CHANNEL_COLOR_YELLOW:
      {
        return (uint8_t) (to == CHANNEL_COLOR_RED);
      }

      case CHANNEL_COLOR_NONE:
      default:
      {
        return 0U;
      }
  }
} /* IsLegalTransition */

void SignalSequenceMonitorInit(SignalSequenceMonitor_t *monitor)
{
  if (monitor == NULL)
  {
    return;
  }

  (void) memset(monitor, 0, sizeof(*monitor));
}

void SignalSequenceMonitorTick(SignalSequenceMonitor_t *monitor,
                               const ChannelStateImage_t *measured,
                               uint32_t timestampTicks,
                               const FaultEmit_t *emit)
{
  uint32_t i;

  if ((monitor == NULL) || (measured == NULL))
  {
    return;
  }

  if (monitor->initialized == 0U)
  {
    for (i = 0U; i < MP_CHANNEL_COUNT_MAX; i++)
    {
      monitor->lastColor[i] = DominantColor(&measured->channels[i]);
    }

    monitor->initialized = 1U;

    return;
  }

  for (i = 0U; i < MP_CHANNEL_COUNT_MAX; i++)
  {
    ChannelColor_t now = DominantColor(&measured->channels[i]);
    ChannelColor_t prev = monitor->lastColor[i];

    if (now != prev)
    {
      if (IsLegalTransition(prev, now) == 0U)
      {
        FaultEvent_t event;

        event.code = FAULT_CODE_SIGNAL_SEQUENCE;
        event.severity = FAULT_SEVERITY_ERROR;
        event.source = (uint16_t) i;
        event.timestampTicks = timestampTicks;
        event.param = (uint32_t) ((uint32_t) prev << 8U) | (uint32_t) now;
        FaultEmitPublish(emit, &event);
      }

      monitor->lastColor[i] = now;
    }
  }
} /* SignalSequenceMonitorTick */
