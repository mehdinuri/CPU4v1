/*
 * App/Domain/Intersection/Detector.c
 *
 * Vehicle Detector demand / occupancy / gap aggregation.
 * Ported from legacy Tasks/Src/Program.c (Detector runtime update).
 */
#include "Domain/Intersection/Detector.h"
#include <string.h>

#define TICK_MS 100U  /* One tick = 100 ms */

void Detector_ResetPeriod(DetectorRuntime_t *rt)
{
  rt->demandCountInPeriod = 0U;
  rt->demandCountInRed = 0U;
  rt->demandCountInGreen = 0U;
  rt->firstDemandTimeMs = 0U;
  rt->occupancyTimeMs = 0U;
  rt->occupancyInRedMs = 0U;
  rt->occupancyInGreenMs = 0U;
  rt->maxGapInGreenMs = 0U;
  /* Do NOT reset isBroken or brokenDurationMs — those are cross-period */
}

void Detector_Tick(uint8_t detIdx,
                   DetectorRuntime_t     *rt,
                   const DetectorConfig_t *cfg,
                   IDetectorInputPort_t  *port,
                   ISnmpNotifierPort_t   *snmpNotifier)
{
  DetectorState_t state = Detector_GetState(port, detIdx);

  bool wasBroken = rt->isBroken;

  rt->isBroken = (state == DETECTOR_STATE_BROKEN);

  if (rt->isBroken)
  {
    /* Accumulate broken duration */
    if (rt->brokenDurationMs < UINT16_MAX - TICK_MS)
    {
      rt->brokenDurationMs += TICK_MS;
    }

    if (!wasBroken)
    {
      /* New fault — emit SNMP trap */
      SnmpNotifier_SendTrap(snmpNotifier, SNMP_TRAP_DETECTOR_FAILURE,
                            (uint32_t) detIdx);
    }

    return;     /* No occupancy tracking when broken */
  }

  /* Detector healthy — reset broken timer */
  rt->brokenDurationMs = 0U;

  if (state == DETECTOR_STATE_BUSY)
  {
    /* Accumulate occupancy */
    if (rt->occupancyTimeMs < UINT16_MAX - TICK_MS)
    {
      rt->occupancyTimeMs += TICK_MS;
    }

    /* Count demand pulses (rising edge = new entry in period) */
    if (rt->demandCountInPeriod < UINT8_MAX)
    {
      rt->demandCountInPeriod++;
    }

    /* Record time-to-first-demand if this is the first hit */
    if ((rt->demandCountInPeriod == 1U) && (rt->firstDemandTimeMs == 0U) )
    {
      rt->firstDemandTimeMs = TICK_MS;       /* Will accumulate from port */
    }
  }

  (void) cfg;  /* Used by green extension logic in ProgramTick */
  (void) snmpNotifier;
} /* Detector_Tick */

bool Detector_HasDemand(const DetectorRuntime_t *rt)
{
  return rt->demandCountInPeriod > 0U;
}

uint16_t Detector_GapMs(const DetectorRuntime_t *rt)
{
  if (rt->demandCountInPeriod == 0U)
  {
    return UINT16_MAX;     /* No demand seen yet */
  }

  return rt->maxGapInGreenMs;
}
