/* App/Domain/Malfunction/ModuleHealthMonitor.c */

#include "Malfunction/ModuleHealthMonitor.h"

#include <stddef.h>
#include <string.h>

static void EmitMissing(const FaultEmit_t *emit,
                        FaultCode_t code,
                        uint16_t source,
                        uint32_t timestampTicks,
                        uint32_t staleTicks)
{
  FaultEvent_t event;

  event.code = code;
  event.severity = FAULT_SEVERITY_CRITICAL;
  event.source = source;
  event.timestampTicks = timestampTicks;
  event.param = staleTicks;
  FaultEmitPublish(emit, &event);
}

void ModuleHealthMonitorInit(ModuleHealthMonitor_t *monitor)
{
  if (monitor == NULL)
  {
    return;
  }

  (void) memset(monitor, 0, sizeof(*monitor));
  monitor->cpThreshold = MODULE_HEALTH_CP_THRESHOLD_DEFAULT;
  monitor->psmThreshold = MODULE_HEALTH_PSM_THRESHOLD_DEFAULT;
  monitor->ssmThreshold = MODULE_HEALTH_SSM_THRESHOLD_DEFAULT;
}

static void UpdateSingle(uint16_t *stale,
                         uint8_t *missing,
                         uint8_t alive,
                         uint16_t threshold,
                         FaultCode_t code,
                         uint16_t source,
                         uint32_t timestampTicks,
                         const FaultEmit_t *emit)
{
  if (alive != 0U)
  {
    *stale = 0U;
    *missing = 0U;

    return;
  }

  if (*stale < 0xFFFFU)
  {
    (*stale)++;
  }

  if ((*stale == threshold) && (*missing == 0U))
  {
    *missing = 1U;
    EmitMissing(emit, code, source, timestampTicks, *stale);
  }
}

void ModuleHealthMonitorTick(ModuleHealthMonitor_t *monitor,
                             const FieldBusSnapshot_t *snapshot,
                             uint32_t timestampTicks,
                             const FaultEmit_t *emit)
{
  uint32_t i;

  if ((monitor == NULL) || (snapshot == NULL))
  {
    return;
  }

  UpdateSingle(&monitor->cpStaleTicks,
               &monitor->cpMissing,
               snapshot->cpu.cpAlive,
               monitor->cpThreshold,
               FAULT_CODE_MODULE_CP_MISSING,
               0U,
               timestampTicks,
               emit);

  for (i = 0U; i < FIELD_BUS_PSM_COUNT; i++)
  {
    UpdateSingle(&monitor->psmStaleTicks[i],
                 &monitor->psmMissing[i],
                 snapshot->psm[i].alive,
                 monitor->psmThreshold,
                 FAULT_CODE_MODULE_PSM_MISSING,
                 (uint16_t) i,
                 timestampTicks,
                 emit);
  }

  for (i = 0U; i < FIELD_BUS_SSM_COUNT; i++)
  {
    UpdateSingle(&monitor->ssmStaleTicks[i],
                 &monitor->ssmMissing[i],
                 snapshot->ssm[i].alive,
                 monitor->ssmThreshold,
                 FAULT_CODE_MODULE_SSM_MISSING,
                 (uint16_t) i,
                 timestampTicks,
                 emit);
  }
} /* ModuleHealthMonitorTick */
