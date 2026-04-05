/*
 * App/Domain/NTCIP/NTCIP1201.c
 *
 * NTCIP 1201 phase timing MIB — Domain-facing implementation.
 */
#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/Intersection/Phase.h"

/* ---------------------------------------------------------------------------
 * Phase table GET
 * ---------------------------------------------------------------------------*/

bool Ntcip1201_GetPhase(const ProgramCtx_t *ctx, uint8_t phaseIdx,
                        Ntcip1201PhaseEntry_t *out)
{
  if ((phaseIdx >= ctx->config.phaseCount) || (out == (void *) 0))
  {
    return false;
  }

  const PhaseConfig_t      *pc = &ctx->config.phases[phaseIdx];
  const SignalGroupConfig_t *sg = (void *) 0;

  /* Find the first SG in this phase to get its yellow/clearance intervals */
  uint8_t i;

  for (i = 0U; i < ctx->config.signalGroupCount; i++)
  {
    if (pc->signalGroupMask & (1UL << i))
    {
      sg = &ctx->config.signalGroups[i];
      break;
    }
  }

  out->phaseNumber = (uint8_t) (phaseIdx + 1U);                /* 1-based */
  out->phaseMinGreenTime = pc->minGreenTime;
  out->phaseMaxGreenTime = pc->maxGreenTime;
  out->phaseYellowChangeInterval = sg ? sg->yellowChangeInterval : 0U;
  out->phaseRedClearanceInterval = 0U;   /* From Conflict matrix — not per-phase */
  out->phaseWalk = sg ? sg->pedestrianWalk : 0U;
  out->phasePedestrianClearance = sg ? sg->pedestrianClearance : 0U;
  out->phaseEnabled = (pc->signalGroupMask != 0U);

  return true;
}

/* ---------------------------------------------------------------------------
 * Phase table SET — validate then apply to live config
 * ---------------------------------------------------------------------------*/

bool Ntcip1201_SetPhaseMinGreen(ProgramCtx_t *ctx,
                                uint8_t phaseIdx,
                                uint8_t value)
{
  if (phaseIdx >= ctx->config.phaseCount)
  {
    return false;
  }

  if (value > ctx->config.phases[phaseIdx].maxGreenTime)
  {
    return false;     /* minGreen must not exceed maxGreen */
  }

  ctx->config.phases[phaseIdx].minGreenTime = value;

  return true;
}

bool Ntcip1201_SetPhaseMaxGreen(ProgramCtx_t *ctx,
                                uint8_t phaseIdx,
                                uint8_t value)
{
  if (phaseIdx >= ctx->config.phaseCount)
  {
    return false;
  }

  if (value < ctx->config.phases[phaseIdx].minGreenTime)
  {
    return false;     /* maxGreen must not be less than minGreen */
  }

  ctx->config.phases[phaseIdx].maxGreenTime = value;

  return true;
}

bool Ntcip1201_SetPhaseYellowChange(ProgramCtx_t *ctx,
                                    uint8_t sgIdx,
                                    uint8_t value)
{
  if (sgIdx >= ctx->config.signalGroupCount)
  {
    return false;
  }

  if (value > 99U)
  {
    return false;     /* Sanity cap */
  }

  ctx->config.signalGroups[sgIdx].yellowChangeInterval = value;

  return true;
}

bool Ntcip1201_SetPhaseRedClearance(ProgramCtx_t *ctx,
                                    uint8_t sgIdx,
                                    uint8_t sgJIdx,
                                    uint8_t value)
{
  if ((sgIdx >= ctx->config.signalGroupCount)
      || (sgJIdx >= SIGNAL_GROUPS_MAX) )
  {
    return false;
  }

  ctx->config.signalGroups[sgIdx].Conflicts[sgJIdx].redClearanceInterval =
    value;

  return true;
}

/* ---------------------------------------------------------------------------
 * Unit status
 * ---------------------------------------------------------------------------*/

Ntcip1201UnitStatus_t Ntcip1201_GetUnitStatus(const ProgramCtx_t *ctx)
{
  Ntcip1201UnitStatus_t s;

  s.maxPhases = ctx->config.phaseCount;
  s.unitControlMode = (uint8_t) ctx->config.controlMode;
  s.activeTimingPlanIndex = ctx->config.activeSignalProgram;

  return s;
}

bool Ntcip1201_SetControlMode(ProgramCtx_t *ctx, uint8_t NTCIPControlMode)
{
  /* NTCIP 1201 unitControlMode values:
   *   1=CIC, 2=ACTD, 3=PLAN, 4=FLASH, 5=DARK, 6=PREEMPT, 7=MANUAL
   * Map to our ControlMode_t. Only a subset is actionable here. */
  switch (NTCIPControlMode)
  {
      case 4U:   /* FLASH */
      {
        ProgramRequestState(ctx, CTRL_STATE_FLASH);
        ctx->config.controlMode = CONTROL_MODE_FLASH;

        return true;
      }

      case 5U:   /* DARK */
      {
        ProgramRequestState(ctx, CTRL_STATE_DARK);
        ctx->config.controlMode = CONTROL_MODE_DARK;

        return true;
      }

      case 3U:   /* PLAN (fixed plan) */
      {
        ctx->config.controlMode = CONTROL_MODE_FIXED_PLAN;
        ProgramRequestState(ctx, CTRL_STATE_ALL_RED);

        return true;
      }

      default:
      {
        return false;     /* Unsupported / read-only in this implementation */
      }
  }
}

/* ---------------------------------------------------------------------------
 * Phase status (runtime snapshot)
 * ---------------------------------------------------------------------------*/

bool Ntcip1201_GetPhaseStatus(const ProgramCtx_t *ctx, uint8_t phaseIdx,
                              Ntcip1201PhaseStatus_t *out)
{
  if ((phaseIdx >= ctx->config.phaseCount) || (out == (void *) 0))
  {
    return false;
  }

  const PhaseRuntime_t *rt = &ctx->runtime.phases[phaseIdx];
  const PhaseConfig_t  *cfg = &ctx->config.phases[phaseIdx];

  out->phaseNumber = (uint8_t) (phaseIdx + 1U);
  out->elapsedGreenSeconds = rt->elapsedSeconds;
  out->isActive = (ctx->runtime.currentState == CTRL_STATE_PHASE
                   && ctx->runtime.activePhase  == phaseIdx);
  out->minTimeElapsed = Phase_MinTimeElapsed(rt, cfg);
  out->maxTimeElapsed = Phase_MaxTimeElapsed(rt, cfg);

  return true;
}
