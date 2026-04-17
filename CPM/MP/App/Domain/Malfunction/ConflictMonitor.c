/* App/Domain/Malfunction/ConflictMonitor.c */

#include "Malfunction/ConflictMonitor.h"

#include <stddef.h>
#include <string.h>

typedef enum
{
  CONFLICT_KIND_NONE = 0,
  CONFLICT_KIND_YY,
  CONFLICT_KIND_YG,
  CONFLICT_KIND_GG
} ConflictKind_t;

static ConflictKind_t ClassifyConflict(const ChannelColorState_t *a,
                                       const ChannelColorState_t *b)
{
  if ((a->green != 0U) && (b->green != 0U))
  {
    return CONFLICT_KIND_GG;
  }

  if (((a->green != 0U) && (b->yellow != 0U))
      || ((a->yellow != 0U) && (b->green != 0U)))
  {
    return CONFLICT_KIND_YG;
  }

  if ((a->yellow != 0U) && (b->yellow != 0U))
  {
    return CONFLICT_KIND_YY;
  }

  return CONFLICT_KIND_NONE;
}

static FaultCode_t ToFaultCode(ConflictKind_t kind)
{
  switch (kind)
  {
      case CONFLICT_KIND_GG:
      {
        return FAULT_CODE_CONFLICT_GREEN_GREEN;
      }

      case CONFLICT_KIND_YG:
      {
        return FAULT_CODE_CONFLICT_YELLOW_GREEN;
      }

      case CONFLICT_KIND_YY:
      {
        return FAULT_CODE_CONFLICT_YELLOW_YELLOW;
      }

      case CONFLICT_KIND_NONE:
      default:
      {
        return FAULT_CODE_NONE;
      }
  }
}

void ConflictMonitorInit(ConflictMonitor_t *monitor, uint16_t threshold)
{
  if (monitor == NULL)
  {
    return;
  }

  (void) memset(monitor, 0, sizeof(*monitor));
  monitor->threshold = (threshold == 0U) ? CONFLICT_DWELL_TICKS_DEFAULT
                       : threshold;
}

void ConflictMonitorTick(ConflictMonitor_t *monitor,
                         const ConfigurationService_t *config,
                         const ChannelStateImage_t *measured,
                         uint32_t timestampTicks,
                         const FaultEmit_t *emit)
{
  uint32_t a;
  uint32_t b;

  if ((monitor == NULL) || (config == NULL) || (measured == NULL))
  {
    return;
  }

  for (a = 0U; a < MP_CHANNEL_COUNT_MAX; a++)
  {
    for (b = a + 1U; b < MP_CHANNEL_COUNT_MAX; b++)
    {
      if (ConfigurationServiceChannelsConflict(config,
                                               (uint8_t) a,
                                               (uint8_t) b) == 0U)
      {
        continue;
      }

      ConflictKind_t kind = ClassifyConflict(&measured->channels[a],
                                             &measured->channels[b]);

      if (kind == CONFLICT_KIND_NONE)
      {
        monitor->dwell[a][b] = 0U;
        continue;
      }

      if (monitor->dwell[a][b] < 0xFFFFU)
      {
        monitor->dwell[a][b]++;
      }

      if (monitor->dwell[a][b] == monitor->threshold)
      {
        FaultEvent_t event;

        event.code = ToFaultCode(kind);
        event.severity = FAULT_SEVERITY_CRITICAL;
        event.source = (uint16_t) ((a << 8U) | b);
        event.timestampTicks = timestampTicks;
        event.param = 0U;
        FaultEmitPublish(emit, &event);
      }
    }
  }
} /* ConflictMonitorTick */
