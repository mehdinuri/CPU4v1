/* App/Domain/Malfunction/ClearanceMonitor.c */

#include "Malfunction/ClearanceMonitor.h"

#include <stddef.h>
#include <string.h>

static ChannelColor_t DominantColor(const ChannelColorState_t *s)
{
  if (s->green != 0U)
  {
    return CHANNEL_COLOR_GREEN;
  }

  if (s->yellow != 0U)
  {
    return CHANNEL_COLOR_YELLOW;
  }

  if (s->red != 0U)
  {
    return CHANNEL_COLOR_RED;
  }

  return CHANNEL_COLOR_NONE;
}

static uint16_t ChannelRedClearTicks(const ConfigurationService_t *service,
                                     uint8_t channelIndex)
{
  uint16_t decis =
    ConfigurationServiceGetChannelRedClearDs(service, channelIndex);

  return (uint16_t) (decis * CLEARANCE_TICKS_PER_DECISECOND);
}

void ClearanceMonitorInit(ClearanceMonitor_t *monitor)
{
  if (monitor == NULL)
  {
    return;
  }

  (void) memset(monitor, 0, sizeof(*monitor));
}

void ClearanceMonitorTick(ClearanceMonitor_t *monitor,
                          const ConfigurationService_t *config,
                          const ChannelStateImage_t *measured,
                          uint32_t timestampTicks,
                          const FaultEmit_t *emit)
{
  uint32_t i;
  uint32_t j;

  if ((monitor == NULL) || (config == NULL) || (measured == NULL))
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

    if ((prev == CHANNEL_COLOR_YELLOW) && (now == CHANNEL_COLOR_RED))
    {
      monitor->clearanceRemainingTicks[i] =
        ChannelRedClearTicks(config, (uint8_t) i);
    }
    else if (monitor->clearanceRemainingTicks[i] > 0U)
    {
      monitor->clearanceRemainingTicks[i]--;
    }
    else
    {
      /* nothing */
    }

    monitor->lastColor[i] = now;
  }

  /* Evaluate violations: channel i has clearance remaining, and a
   * conflicting channel j is already lit. */
  for (i = 0U; i < MP_CHANNEL_COUNT_MAX; i++)
  {
    if (monitor->clearanceRemainingTicks[i] == 0U)
    {
      continue;
    }

    for (j = 0U; j < MP_CHANNEL_COUNT_MAX; j++)
    {
      if (i == j)
      {
        continue;
      }

      if (ConfigurationServiceChannelsConflict(config,
                                               (uint8_t) i,
                                               (uint8_t) j) == 0U)
      {
        continue;
      }

      ChannelColor_t jColor = DominantColor(&measured->channels[j]);

      if ((jColor == CHANNEL_COLOR_GREEN) || (jColor == CHANNEL_COLOR_YELLOW))
      {
        FaultEvent_t event;

        event.code = FAULT_CODE_CLEARANCE_SHORT;
        event.severity = FAULT_SEVERITY_CRITICAL;
        event.source = (uint16_t) ((i << 8U) | j);
        event.timestampTicks = timestampTicks;
        event.param = monitor->clearanceRemainingTicks[i];
        FaultEmitPublish(emit, &event);
      }
    }
  }
} /* ClearanceMonitorTick */
