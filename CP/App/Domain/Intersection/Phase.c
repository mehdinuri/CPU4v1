/*
 * App/Domain/Intersection/Phase.c
 *
 * Phase timing state machine implementation.
 * Ported from legacy Tasks/Src/Program.c (phase timing logic).
 */
#include "Domain/Intersection/Phase.h"
#include <string.h>

void Phase_Reset(PhaseRuntime_t *rt)
{
  memset(rt, 0, sizeof(*rt));
}

bool Phase_Tick(PhaseRuntime_t *rt, const PhaseConfig_t *cfg)
{
  /* Advance elapsed time by 1 second (one 10-tick period at 100 ms/tick).
   * Callers accumulate ticks and call this once per second boundary. */
  rt->elapsedSeconds++;

  /* Clamp to prevent 16-bit overflow on very long phases */
  if (rt->elapsedSeconds > 3600U)
  {
    rt->elapsedSeconds = 3600U;
  }

  /* Compute effective max time (base + any extension/shortening) */
  int32_t effectiveMax = (int32_t) cfg->maxGreenTime + rt->extensionSeconds;

  if (effectiveMax < (int32_t) cfg->minGreenTime)
  {
    effectiveMax = cfg->minGreenTime;
  }

  if (rt->elapsedSeconds >= (uint16_t) effectiveMax)
  {
    rt->maxTimeElapsed = true;
  }

  return rt->maxTimeElapsed;
}

void Phase_ApplyExtension(PhaseRuntime_t *rt, const PhaseConfig_t *cfg,
                          int8_t extensionSeconds)
{
  int32_t newExt = (int32_t) rt->extensionSeconds + extensionSeconds;

  /* Clamp: effective max must not go below minGreenTime */
  int32_t effectiveMax = (int32_t) cfg->maxGreenTime + newExt;

  if (effectiveMax < (int32_t) cfg->minGreenTime)
  {
    newExt = (int32_t) cfg->minGreenTime - (int32_t) cfg->maxGreenTime;
  }

  /* Clamp: do not extend beyond a reasonable ceiling (10 minutes) */
  if (effectiveMax > 600)
  {
    newExt = 600 - (int32_t) cfg->maxGreenTime;
  }

  rt->extensionSeconds = (int8_t) newExt;

  /* Re-evaluate maxTimeElapsed with the new extension */
  int32_t effectiveFinal = (int32_t) cfg->maxGreenTime + rt->extensionSeconds;

  if (effectiveFinal < (int32_t) cfg->minGreenTime)
  {
    effectiveFinal = cfg->minGreenTime;
  }

  rt->maxTimeElapsed = (rt->elapsedSeconds >= (uint16_t) effectiveFinal);
}

bool Phase_MinTimeElapsed(const PhaseRuntime_t *rt, const PhaseConfig_t *cfg)
{
  return rt->elapsedSeconds >= cfg->minGreenTime;
}

bool Phase_MaxTimeElapsed(const PhaseRuntime_t *rt, const PhaseConfig_t *cfg)
{
  return rt->maxTimeElapsed;
}
