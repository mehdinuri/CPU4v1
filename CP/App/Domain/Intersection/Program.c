/*
 * App/Domain/Intersection/Program.c
 *
 * Top-level Intersection Program coordinator.
 * Orchestrates phase/Sequence state machines, transition rule evaluation,
 * Conflict detection, and signal output updates on every 100 ms tick.
 *
 * Ported from legacy Tasks/Src/Program.c.
 */
#include "Domain/Intersection/Program.h"
#include "Domain/Intersection/Phase.h"
#include "Domain/Intersection/SignalGroup.h"
#include "Domain/Intersection/Conflict.h"
#include "Domain/Intersection/TransitionRule.h"
#include "Domain/Intersection/Detector.h"
#include "Domain/Intersection/Sequence.h"
#include <string.h>

#define TICKS_PER_SECOND 10U  /* 10 × 100 ms = 1 second */

/* ---------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------------*/

static void apply_all_red(ProgramCtx_t *ctx)
{
  for (uint8_t i = 0U; i < ctx->config.signalGroupCount; i++)
  {
    SignalGroupRuntime_t *rt = &ctx->runtime.signalGroups[i];

    rt->state = SG_STATE_CLOSED;
    rt->stateElapsedSeconds = 0U;
    SignalOutput_SetLamp(ctx->signalOutput,
                         ctx->config.signalGroups[i].firstOutputIndex,
                         SIGNAL_COLOR_RED);
  }

  SignalOutput_Flush(ctx->signalOutput);
}

static void apply_all_flash(ProgramCtx_t *ctx)
{
  for (uint8_t i = 0U; i < ctx->config.signalGroupCount; i++)
  {
    ctx->runtime.signalGroups[i].state = SG_STATE_FLASH;
    SignalOutput_SetLamp(ctx->signalOutput,
                         ctx->config.signalGroups[i].firstOutputIndex,
                         SIGNAL_COLOR_FLASH);
  }

  SignalOutput_Flush(ctx->signalOutput);
}

static void apply_all_dark(ProgramCtx_t *ctx)
{
  for (uint8_t i = 0U; i < ctx->config.signalGroupCount; i++)
  {
    ctx->runtime.signalGroups[i].state = SG_STATE_NONE;
    SignalOutput_SetLamp(ctx->signalOutput,
                         ctx->config.signalGroups[i].firstOutputIndex,
                         SIGNAL_COLOR_OFF);
  }

  SignalOutput_Flush(ctx->signalOutput);
}

/* Start a phase: open all signal groups in the phase's signalGroupMask */
static void start_phase(ProgramCtx_t *ctx, uint8_t phaseIdx)
{
  const PhaseConfig_t *phaseCfg = &ctx->config.phases[phaseIdx];

  ctx->runtime.activePhase = phaseIdx;
  Phase_Reset(&ctx->runtime.phases[phaseIdx]);

  for (uint8_t sg = 0U; sg < ctx->config.signalGroupCount; sg++)
  {
    bool inPhase = (phaseCfg->signalGroupMask & (1UL << sg)) != 0U;

    if (inPhase)
    {
      SG_Open(sg,
              &ctx->runtime.signalGroups[sg],
              &ctx->config.signalGroups[sg],
              ctx->signalOutput);
    }
  }

  SignalOutput_Flush(ctx->signalOutput);
}

/* Close all signal groups that are currently open */
static void close_all_open_sgs(ProgramCtx_t *ctx)
{
  for (uint8_t sg = 0U; sg < ctx->config.signalGroupCount; sg++)
  {
    if (ctx->runtime.signalGroups[sg].state != SG_STATE_CLOSED)
    {
      SG_Close(sg,
               &ctx->runtime.signalGroups[sg],
               &ctx->config.signalGroups[sg],
               ctx->signalOutput);
    }
  }

  SignalOutput_Flush(ctx->signalOutput);
}

/* Execute a transition action statement */
static void execute_statement(ProgramCtx_t *ctx, const StatementConfig_t *stmt)
{
  switch (stmt->cmd)
  {
      case CMD_PHASE_START:
      {
        if (stmt->param1 < ctx->config.phaseCount)
        {
          ctx->runtime.currentState = CTRL_STATE_PHASE;
          start_phase(ctx, stmt->param1);
        }

        break;
      }

      case CMD_PHASE_STOP:
      {
        close_all_open_sgs(ctx);
        ctx->runtime.currentState = CTRL_STATE_ALL_RED;
        apply_all_red(ctx);
        break;
      }

      case CMD_PHASE_EXTEND:
      {
        if (ctx->runtime.activePhase < ctx->config.phaseCount)
        {
          Phase_ApplyExtension(
            &ctx->runtime.phases[ctx->runtime.activePhase],
            &ctx->config.phases[ctx->runtime.activePhase],
            stmt->param3);
        }

        break;
      }

      case CMD_SEQ_START:
      {
        if (stmt->param1 < ctx->config.SequenceCount)
        {
          ctx->runtime.activeSequence = stmt->param1;
          ctx->runtime.currentState = CTRL_STATE_SEQUENCE;
          Sequence_Reset(&ctx->runtime.Sequences[stmt->param1]);
          Sequence_ApplyStep(0U,
                             &ctx->config.Sequences[stmt->param1],
                             ctx->config.signalGroups,
                             ctx->config.signalGroupCount,
                             ctx->signalOutput);
        }

        break;
      }

      case CMD_SEQ_STOP:
      {
        close_all_open_sgs(ctx);
        ctx->runtime.currentState = CTRL_STATE_ALL_RED;
        break;
      }

      case CMD_TRANSITIONS_LOCK_ADD:
      {
        ctx->runtime.transitionsLocked = true;
        ctx->runtime.transitionLockCount++;
        break;
      }

      case CMD_TRANSITIONS_LOCK_END:
      {
        if (ctx->runtime.transitionLockCount > 0U)
        {
          ctx->runtime.transitionLockCount--;
        }

        if (ctx->runtime.transitionLockCount == 0U)
        {
          ctx->runtime.transitionsLocked = false;
        }

        break;
      }

      default:
      {
        break;
      }
  } /* switch */
} /* execute_statement */

/* ---------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/

void ProgramInit(ProgramCtx_t *ctx,
                 ISignalOutputPort_t  *signalOutput,
                 IDetectorInputPort_t *DetectorInput,
                 ISystemClockPort_t   *systemClock,
                 ISnmpNotifierPort_t  *snmpNotifier)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->signalOutput = signalOutput;
  ctx->DetectorInput = DetectorInput;
  ctx->systemClock = systemClock;
  ctx->snmpNotifier = snmpNotifier;

  ctx->runtime.currentState = CTRL_STATE_DARK;
  ctx->runtime.requestedState = CTRL_STATE_DARK;

  /* Initialise all SG runtimes to CLOSED (red) */
  for (uint8_t i = 0U; i < SIGNAL_GROUPS_MAX; i++)
  {
    SG_Reset(&ctx->runtime.signalGroups[i]);
  }

  for (uint8_t i = 0U; i < PHASES_MAX; i++)
  {
    Phase_Reset(&ctx->runtime.phases[i]);
  }

  for (uint8_t i = 0U; i < SEQUENCES_MAX; i++)
  {
    Sequence_Reset(&ctx->runtime.Sequences[i]);
  }

  /* Emit startup SNMP notification */
  SnmpNotifier_SendTrap(snmpNotifier, SNMP_TRAP_CONTROLLER_STARTUP, 0U);
}

void ProgramLoadConfig(ProgramCtx_t *ctx, const ProgramConfig_t *config)
{
  memcpy(&ctx->config, config, sizeof(ctx->config));
}

void ProgramRequestState(ProgramCtx_t *ctx, ControllerState_t newState)
{
  ctx->runtime.requestedState = newState;
}

ControllerState_t ProgramGetState(const ProgramCtx_t *ctx)
{
  return ctx->runtime.currentState;
}

void ProgramTick(ProgramCtx_t *ctx)
{
  ctx->runtime.tickCount++;

  /* 1. Handle explicit state change requests (from SNMP SET or user input) */
  if (ctx->runtime.requestedState != ctx->runtime.currentState)
  {
    ControllerState_t req = ctx->runtime.requestedState;

    switch (req)
    {
        case CTRL_STATE_FLASH:
        {
          ctx->runtime.currentState = CTRL_STATE_FLASH;
          ctx->runtime.requestedState = CTRL_STATE_FLASH;       /* Matches new state */
          apply_all_flash(ctx);

          return;
        }

        case CTRL_STATE_DARK:
        {
          ctx->runtime.currentState = CTRL_STATE_DARK;
          ctx->runtime.requestedState = CTRL_STATE_DARK;
          apply_all_dark(ctx);

          return;
        }

        case CTRL_STATE_ALL_RED:
        {
          ctx->runtime.currentState = CTRL_STATE_ALL_RED;
          ctx->runtime.requestedState = CTRL_STATE_ALL_RED;
          apply_all_red(ctx);

          return;
        }

        default:
        {
          ctx->runtime.requestedState = ctx->runtime.currentState;       /* Ignore unknown */
          break;
        }
    }
  }

  /* 2. Update Detector accumulators */
  for (uint8_t i = 0U; i < ctx->config.DetectorCount; i++)
  {
    Detector_Tick(i,
                  &ctx->runtime.Detectors[i],
                  &ctx->config.Detectors[i],
                  ctx->DetectorInput,
                  ctx->snmpNotifier);
  }

  /* 3. Advance per-SG state machines (handle timed transitions) */
  for (uint8_t i = 0U; i < ctx->config.signalGroupCount; i++)
  {
    SG_Tick(i,
            &ctx->runtime.signalGroups[i],
            &ctx->config.signalGroups[i],
            ctx->signalOutput);
  }

  /* 4. Advance phase timers (once per second) */
  if ((ctx->runtime.tickCount % TICKS_PER_SECOND) == 0U)
  {
    if ((ctx->runtime.currentState == CTRL_STATE_PHASE)
        && (ctx->runtime.activePhase < ctx->config.phaseCount) )
    {
      Phase_Tick(&ctx->runtime.phases[ctx->runtime.activePhase],
                 &ctx->config.phases[ctx->runtime.activePhase]);
    }
  }

  /* 5. Advance Sequence (if active) */
  if (ctx->runtime.currentState == CTRL_STATE_SEQUENCE)
  {
    uint8_t seqIdx = ctx->runtime.activeSequence;

    if (seqIdx < ctx->config.SequenceCount)
    {
      bool done = Sequence_Tick(seqIdx,
                                &ctx->runtime.Sequences[seqIdx],
                                &ctx->config.Sequences[seqIdx],
                                ctx->config.signalGroups,
                                ctx->config.signalGroupCount,
                                ctx->signalOutput);

      if (done)
      {
        /* Sequence completed — return to ALL_RED pending next command */
        ctx->runtime.currentState = CTRL_STATE_ALL_RED;
        apply_all_red(ctx);
      }
    }
  }

  /* 6. Conflict detection — check every tick for safety */
  if (ctx->runtime.currentState == CTRL_STATE_PHASE)
  {
    ConflictType_t Conflict = Conflict_Check(
      ctx->config.signalGroups,
      ctx->runtime.signalGroups,
      ctx->config.signalGroupCount,
      ctx->snmpNotifier);

    if ((Conflict == CONFLICT_GREEN_GREEN)
        || (Conflict == CONFLICT_YELLOW_GREEN) )
    {
      /* Safety fallback — force ALL_RED immediately */
      ctx->runtime.currentState = CTRL_STATE_ALL_RED;
      apply_all_red(ctx);

      return;
    }
  }

  /* 7. Transition rule evaluation (skip if transitions locked) */
  if (!ctx->runtime.transitionsLocked
      && (ctx->config.activeSignalProgram < SIGNAL_PROGRAMS_MAX) )
  {
    uint8_t prog = ctx->config.activeSignalProgram;
    TransitionConfig_t selectedTransition;

    bool fired = TransitionRule_SelectBest(
      ctx->runtime.currentState,
      ctx->config.transitions[prog],
      ctx->config.transitionCounts[prog],
      ctx->config.rules[prog],
      ctx->config.operations[prog],
      256U,
      ctx->runtime.phases,
      ctx->runtime.Detectors,
      ctx->runtime.counters,
      &selectedTransition);

    if (fired)
    {
      /* Execute the winning transition's statements */
      const StatementConfig_t *stmts =
        ctx->config.statements[prog];

      for (uint8_t s = selectedTransition.param1;
           s <= selectedTransition.param2; s++)
      {
        execute_statement(ctx, &stmts[s]);
      }

      /* Sync requestedState so stale requests don't re-trigger next tick */
      ctx->runtime.requestedState = ctx->runtime.currentState;
    }
  }

  /* 8. Flush any pending lamp state changes */
  SignalOutput_Flush(ctx->signalOutput);
} /* ProgramTick */
