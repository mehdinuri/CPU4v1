/*
 * App/Domain/NTCIP/NTCIP1202.c
 *
 * NTCIP 1202 status and fault objects — Domain-facing implementation.
 */
#include "Domain/NTCIP/NTCIP1202.h"

/* ---------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------------*/

static bool sg_is_green(const SignalGroupRuntime_t *rt)
{
  return rt->state == SG_STATE_OPEN
         || rt->state == SG_STATE_OPENING
         || rt->state == SG_STATE_GREEN_FLASH;
}

static bool sg_is_yellow(const SignalGroupRuntime_t *rt)
{
  return rt->state == SG_STATE_CLOSING;
}

/* Minimal epoch-to-calendar conversion (no time zone). */
static Ntcip1202DateTime_t epoch_to_datetime(uint32_t epoch)
{
  /* Days since Unix epoch */
  uint32_t days = epoch / 86400U;
  uint32_t rem = epoch % 86400U;

  Ntcip1202DateTime_t dt;

  dt.hour = (uint8_t) (rem / 3600U);
  dt.minute = (uint8_t) ((rem % 3600U) / 60U);
  dt.second = (uint8_t) (rem % 60U);

  /* Gregorian calendar calculation from day count */
  uint32_t year = 1970U;

  while (true)
  {
    bool leap = ((year % 4U == 0U) && (year % 100U != 0U))
                || (year % 400U == 0U);
    uint32_t daysInYear = leap ? 366U : 365U;

    if (days < daysInYear)
    {
      break;
    }

    days -= daysInYear;
    year++;
  }

  dt.year = (uint16_t) year;

  bool leap = ((year % 4U == 0U) && (year % 100U != 0U))
              || (year % 400U == 0U);
  static const uint8_t dim[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30,
                                   31 };
  uint8_t month;

  for (month = 0U; month < 12U; month++)
  {
    uint8_t d = dim[month];

    if ((month == 1U) && leap)
    {
      d = 29U;
    }

    if (days < (uint32_t) d)
    {
      break;
    }

    days -= d;
  }

  dt.month = (uint8_t) (month + 1U);
  dt.day = (uint8_t) (days + 1U);

  return dt;
} /* epoch_to_datetime */

static uint32_t datetime_to_epoch(const Ntcip1202DateTime_t *dt)
{
  /* Simplified: compute days from 1970-01-01 */
  uint32_t days = 0U;
  uint32_t y;

  for (y = 1970U; y < dt->year; y++)
  {
    bool leap = ((y % 4U == 0U) && (y % 100U != 0U)) || (y % 400U == 0U);

    days += leap ? 366U : 365U;
  }

  static const uint8_t dim[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30,
                                   31 };
  bool leap = ((dt->year % 4U == 0U) && (dt->year % 100U != 0U))
              || (dt->year % 400U == 0U);
  uint8_t m;

  for (m = 0U; m < (uint8_t) (dt->month - 1U); m++)
  {
    uint8_t d = dim[m];

    if ((m == 1U) && leap)
    {
      d = 29U;
    }

    days += d;
  }

  days += (uint32_t) (dt->day - 1U);

  return days * 86400U
         + (uint32_t) dt->hour * 3600U
         + (uint32_t) dt->minute * 60U
         + (uint32_t) dt->second;
}

/* ---------------------------------------------------------------------------
 * Phase status
 * ---------------------------------------------------------------------------*/

Ntcip1202PhaseStatus_t Ntcip1202_GetPhaseStatus(const ProgramCtx_t *ctx,
                                                uint8_t phaseIdx)
{
  if (phaseIdx >= ctx->config.phaseCount)
  {
    return 0U;
  }

  Ntcip1202PhaseStatus_t status = 0U;
  bool isActive = (ctx->runtime.currentState == CTRL_STATE_PHASE
                   && ctx->runtime.activePhase  == phaseIdx);

  if (isActive)
  {
    status |= NTCIP1202_PHASE_STATUS_ACTIVE;
  }

  /* Inspect SGs in this phase for their current signal state */
  const PhaseConfig_t *pc = &ctx->config.phases[phaseIdx];
  uint8_t sg;

  for (sg = 0U; sg < ctx->config.signalGroupCount; sg++)
  {
    if (!(pc->signalGroupMask & (1UL << sg)))
    {
      continue;
    }

    const SignalGroupRuntime_t *rt = &ctx->runtime.signalGroups[sg];
    const SignalGroupConfig_t  *c = &ctx->config.signalGroups[sg];

    if (sg_is_green(rt))
    {
      if (c->type == SG_TYPE_PEDESTRIAN)
      {
        if (rt->state == SG_STATE_GREEN_FLASH)
        {
          status |= NTCIP1202_PHASE_STATUS_PED_CLEAR;
        }
        else
        {
          status |= NTCIP1202_PHASE_STATUS_WALK;
        }
      }
    }
    else if (sg_is_yellow(rt))
    {
      /* Yellow / closing — not separately flagged in NTCIP 1202 bitmask */
    }
    else
    {
      if (c->type == SG_TYPE_PEDESTRIAN)
      {
        status |= NTCIP1202_PHASE_STATUS_DONT_WALK;
      }
    }
  }

  if (ctx->runtime.phases[phaseIdx].maxTimeElapsed)
  {
    status |= NTCIP1202_PHASE_STATUS_MAX_OUT;
  }

  return status;
} /* Ntcip1202_GetPhaseStatus */

/* ---------------------------------------------------------------------------
 * Detector table
 * ---------------------------------------------------------------------------*/

bool Ntcip1202_GetDetector(const ProgramCtx_t *ctx, uint8_t detIdx,
                           Ntcip1202DetectorEntry_t *out)
{
  if ((detIdx >= ctx->config.DetectorCount) || (out == (void *) 0))
  {
    return false;
  }

  const DetectorRuntime_t *rt = &ctx->runtime.Detectors[detIdx];

  out->DetectorCallStatus = rt->demandCountInPeriod;
  out->DetectorOccupancyMs = rt->occupancyTimeMs;
  out->DetectorVolume = rt->demandCountInPeriod;        /* NTCIP volume = count */
  out->DetectorFault = rt->isBroken;

  return true;
}

/* ---------------------------------------------------------------------------
 * Alarm table
 * ---------------------------------------------------------------------------*/

uint8_t Ntcip1202_GetActiveAlarms(const ProgramCtx_t *ctx,
                                  Ntcip1202AlarmEntry_t *out,
                                  uint8_t outCapacity)
{
  uint8_t count = 0U;

  /* Scan signal groups for lamp failures */
  uint8_t sg;

  for (sg = 0U; sg < ctx->config.signalGroupCount && count < outCapacity; sg++)
  {
    const SignalGroupRuntime_t *rt = &ctx->runtime.signalGroups[sg];

    if (rt->redLampCritical || rt->yellowLampBroken || rt->greenLampBroken)
    {
      out[count].alarmType = NTCIP1202_ALARM_LAMP_FAILURE;
      out[count].objectIndex = sg;
      out[count].active = true;
      count++;
    }
  }

  /* Scan Detectors for failures */
  uint8_t det;

  for (det = 0U; det < ctx->config.DetectorCount && count < outCapacity; det++)
  {
    if (ctx->runtime.Detectors[det].isBroken)
    {
      out[count].alarmType = NTCIP1202_ALARM_DETECTOR_FAILURE;
      out[count].objectIndex = det;
      out[count].active = true;
      count++;
    }
  }

  return count;
}

/* ---------------------------------------------------------------------------
 * System date/time
 * ---------------------------------------------------------------------------*/

Ntcip1202DateTime_t Ntcip1202_GetDateTime(const ProgramCtx_t *ctx)
{
  uint32_t epoch = SystemClock_GetEpoch(ctx->systemClock);

  return epoch_to_datetime(epoch);
}

bool Ntcip1202_SetDateTime(ProgramCtx_t *ctx, const Ntcip1202DateTime_t *dt)
{
  if (dt == (void *) 0)
  {
    return false;
  }

  if ((dt->year < 2000U) || (dt->year > 2099U) )
  {
    return false;
  }

  if ((dt->month < 1U)  || (dt->month > 12U))
  {
    return false;
  }

  if ((dt->day < 1U)    || (dt->day > 31U))
  {
    return false;
  }

  if ((dt->hour > 23U) || (dt->minute > 59U) || (dt->second > 59U) )
  {
    return false;
  }

  uint32_t epoch = datetime_to_epoch(dt);

  return SystemClock_SetEpoch(ctx->systemClock, epoch);
}
