/* App/Domain/Intersection/IntersectionEngine.c
 *
 * Deterministic dual-ring controller-core engine with minimum green,
 * actuated gap-out, yellow/red-clear, simultaneous barrier crossing,
 * derived overlap outputs, and runtime status projection.
 */
#include "IntersectionEngine.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

static uint8_t FindNextDemandPosition(const IntersectionEngine_t *engine,
                                      uint8_t ringIndex,
                                      uint8_t currentPosition);
static uint8_t BarrierGroupForPosition(const IntersectionRingPlan_t *ringPlan,
                                       uint8_t position);
static uint8_t IsBarrierCrossing(const IntersectionEngine_t *engine,
                                 uint8_t ringIndex,
                                 uint8_t fromPosition,
                                 uint8_t toPosition);
static void RefreshDetectorDerivedInputs(IntersectionEngine_t *engine);
static void UpdateCoordinationRuntime(IntersectionEngine_t *engine);
static uint8_t StartUpFlashActive(const IntersectionEngine_t *engine);
static uint8_t StartUpFlashUsesAutoFlashMode(
  const IntersectionEngine_t *engine);
static uint8_t RingSystemOmitRedClearActive(const IntersectionEngine_t *engine,
                                            uint8_t ringIndex);
static uint8_t RemoteManualControlActive(const IntersectionEngine_t *engine);
static uint8_t PreemptModeActive(const IntersectionEngine_t *engine);
static uint8_t PhaseDontWalkRevertActive(const IntersectionEngine_t *engine,
                                         uint8_t phaseIndex);
static uint8_t PhaseSystemPedOmitActive(const IntersectionEngine_t *engine,
                                        uint8_t phaseIndex);
static void TickPhaseRedRevertTimers(IntersectionEngine_t *engine);
static void TickPhaseDontWalkRevertTimers(IntersectionEngine_t *engine);
static void ResetCoordinationCycleFaultDiagnostics(IntersectionEngine_t *engine);
static void UpdateCoordinationCycleFaultDiagnostics(
  IntersectionEngine_t *engine,
  uint8_t coordinatedModeActive);
static void ResetCoordinationAlarmDiagnostics(IntersectionEngine_t *engine);
static void UpdateCoordinationAlarmDiagnostics(IntersectionEngine_t *engine,
                                               uint8_t calledPattern,
                                               uint8_t runningCalledPattern);
static uint16_t PhaseCurrentWalkTicks(const IntersectionEngine_t *engine,
                                      uint8_t phaseIndex);
static uint16_t PhaseCurrentPedClearTicks(const IntersectionEngine_t *engine,
                                          uint8_t phaseIndex);
static uint16_t PhasePedAdvanceTicks(const IntersectionEngine_t *engine,
                                     uint8_t phaseIndex);
static uint16_t PhasePedStartDelayTicks(const IntersectionEngine_t *engine,
                                        uint8_t phaseIndex);
static void StartRingGreenStage(IntersectionEngine_t *engine,
                                uint8_t ringIndex,
                                uint8_t nextPosition,
                                uint8_t startPedWalk);
static void EnterBarrierWait(IntersectionEngine_t *engine,
                             uint8_t ringIndex,
                             uint8_t pendingPosition);
static void TickInactivePedStates(IntersectionEngine_t *engine);
static void TryStartAdvancePedWalks(IntersectionEngine_t *engine);
static void TickControllerRings(IntersectionEngine_t *engine);
static uint8_t PreemptCyclingPhaseAllowed(const IntersectionEngine_t *engine,
                                          uint8_t phaseIndex);
static uint8_t PreemptCyclingPedAllowed(const IntersectionEngine_t *engine,
                                        uint8_t phaseIndex);

typedef enum
{
  INTERSECTION_AUTOMATIC_FLASH_STATE_IDLE = 0,
  INTERSECTION_AUTOMATIC_FLASH_STATE_ENTRY,
  INTERSECTION_AUTOMATIC_FLASH_STATE_FLASHING
} IntersectionAutomaticFlashState_t;

typedef struct
{
  uint8_t valid;
  uint8_t position;
  IntersectionRingStage_t stage;
  uint32_t elapsedTicks;
} IntersectionPreemptExitRecoveryTarget_t;

static uint16_t PhaseWalkTicks(const IntersectionEngine_t *engine,
                               uint8_t phaseIndex)
{
  return (uint16_t) (engine->config.phases[phaseIndex].walkSeconds * 100U);
}

static uint16_t PhaseCurrentWalkTicks(const IntersectionEngine_t *engine,
                                      uint8_t phaseIndex)
{
  const IntersectionPhaseRuntime_t *phaseRuntime =
    &engine->runtime.phases[phaseIndex];
  uint16_t alternateSeconds =
    engine->config.phases[phaseIndex].pedAlternateWalkSeconds;

  if ((phaseRuntime->pedAlternateTimingActive != 0U)
      && (alternateSeconds != 0U))
  {
    return (uint16_t) (alternateSeconds * 100U);
  }

  return PhaseWalkTicks(engine, phaseIndex);
}

static uint16_t PhasePedClearTicks(const IntersectionEngine_t *engine,
                                   uint8_t phaseIndex)
{
  return (uint16_t) (engine->config.phases[phaseIndex].pedClearSeconds * 100U);
}

static uint16_t PhaseCurrentPedClearTicks(const IntersectionEngine_t *engine,
                                          uint8_t phaseIndex)
{
  const IntersectionPhaseRuntime_t *phaseRuntime =
    &engine->runtime.phases[phaseIndex];
  uint16_t alternateSeconds =
    engine->config.phases[phaseIndex].pedAlternateClearSeconds;

  if ((phaseRuntime->pedAlternateTimingActive != 0U)
      && (alternateSeconds != 0U))
  {
    return (uint16_t) (alternateSeconds * 100U);
  }

  return PhasePedClearTicks(engine, phaseIndex);
}

static uint16_t PhasePedDelayTicks(const IntersectionEngine_t *engine,
                                   uint8_t phaseIndex)
{
  return (uint16_t) (engine->config.phases[phaseIndex].pedDelayDs * 10U);
}

static uint16_t PhasePedAdvanceTicks(const IntersectionEngine_t *engine,
                                     uint8_t phaseIndex)
{
  return (uint16_t) (engine->config.phases[phaseIndex].pedAdvanceWalkDs * 10U);
}

static uint16_t PhasePedStartDelayTicks(const IntersectionEngine_t *engine,
                                        uint8_t phaseIndex)
{
  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  if (PhasePedAdvanceTicks(engine, phaseIndex) != 0U)
  {
    return 0U;
  }

  return PhasePedDelayTicks(engine, phaseIndex);
}

static void ResetPedWalkServiceCycleCounts(IntersectionEngine_t *engine)
{
  if (engine == NULL)
  {
    return;
  }

  memset(engine->pedWalkServicesThisCycle,
         0,
         sizeof(engine->pedWalkServicesThisCycle));
}

static uint8_t PhasePedConfigured(const IntersectionEngine_t *engine,
                                  uint8_t phaseIndex)
{
  const IntersectionPhaseConfig_t *phaseConfig =
    &engine->config.phases[phaseIndex];

  return (uint8_t) ((phaseConfig->walkSeconds != 0U)
                    || (phaseConfig->pedClearSeconds != 0U));
}

static uint8_t PhasePedWalkServiceAvailable(const IntersectionEngine_t *engine,
                                            uint8_t phaseIndex)
{
  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  if (engine->runtime.mode != INTERSECTION_CONTROL_MODE_COORDINATED)
  {
    return 1U;
  }

  return (uint8_t) (
    engine->pedWalkServicesThisCycle[phaseIndex]
    < engine->config.phases[phaseIndex].pedWalkService);
}

static uint8_t GetRingDeterministicTicksToPendingGreen(
  const IntersectionEngine_t *engine,
  uint8_t ringIndex,
  uint8_t *pendingPosition,
  uint32_t *ticksToGreen)
{
  const IntersectionRingRuntime_t *ringRuntime;
  const IntersectionRingPlan_t *ringPlan;
  uint8_t activePhaseIndex;
  uint32_t ticks = 0U;

  if ((engine == NULL) || (pendingPosition == NULL) || (ticksToGreen == NULL)
      || (ringIndex >= engine->config.ringCount))
  {
    return 0U;
  }

  ringRuntime = &engine->runtime.rings[ringIndex];
  ringPlan = &engine->config.rings[ringIndex];

  if ((ringRuntime->pendingPosition >= ringPlan->phaseCount)
      || (ringRuntime->pendingPosition == ringRuntime->activePosition))
  {
    return 0U;
  }

  *pendingPosition = ringRuntime->pendingPosition;
  activePhaseIndex = ringRuntime->activePhaseIndex;

  switch (ringRuntime->stage)
  {
      case INTERSECTION_RING_STAGE_WAIT_BARRIER:
      {
        ticks = 0U;
        break;
      }

      case INTERSECTION_RING_STAGE_YELLOW:
      {
        if (engine->yellowTicks[activePhaseIndex] > ringRuntime->stageElapsedTicks)
        {
          ticks = (uint32_t) (engine->yellowTicks[activePhaseIndex]
                              - ringRuntime->stageElapsedTicks);
        }

        if (RingSystemOmitRedClearActive(engine, ringIndex) == 0U)
        {
          ticks += engine->redClearTicks[activePhaseIndex];
        }

        break;
      }

      case INTERSECTION_RING_STAGE_RED_CLEAR:
      {
        if (engine->redClearTicks[activePhaseIndex]
            > ringRuntime->stageElapsedTicks)
        {
          ticks = (uint32_t) (engine->redClearTicks[activePhaseIndex]
                              - ringRuntime->stageElapsedTicks);
        }

        break;
      }

      default:
      {
        return 0U;
      }
  }

  *ticksToGreen = ticks;

  return 1U;
}

static uint8_t PhaseAdvanceWalkReady(const IntersectionEngine_t *engine,
                                     uint8_t phaseIndex)
{
  const IntersectionPhaseRuntime_t *phaseRuntime;

  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  phaseRuntime = &engine->runtime.phases[phaseIndex];

  return (uint8_t) ((PhasePedConfigured(engine, phaseIndex) != 0U)
                    && (PhasePedAdvanceTicks(engine, phaseIndex) != 0U)
                    && (phaseRuntime->interval
                        == INTERSECTION_PHASE_INTERVAL_RED)
                    && (phaseRuntime->pedInterval
                        == INTERSECTION_PED_INTERVAL_DONT_WALK)
                    && (phaseRuntime->pedServicePending != 0U)
                    && (PhaseSystemPedOmitActive(engine, phaseIndex) == 0U)
                    && (PhaseDontWalkRevertActive(engine, phaseIndex) == 0U)
                    && (PhasePedWalkServiceAvailable(engine, phaseIndex) != 0U));
}

static uint8_t AutomaticFlashRingPositionConfigured(
  const IntersectionEngine_t *engine,
  const uint8_t *positions,
  uint8_t ringIndex)
{
  const IntersectionRingPlan_t *ringPlan;

  if ((engine == NULL) || (positions == NULL)
      || (ringIndex >= engine->config.ringCount))
  {
    return 0U;
  }

  ringPlan = &engine->config.rings[ringIndex];

  return (uint8_t) (positions[ringIndex] < ringPlan->phaseCount);
}

static uint8_t AutomaticFlashTransitionConfigured(
  const IntersectionEngine_t *engine)
{
  uint8_t ringIndex;

  if (engine == NULL)
  {
    return 0U;
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    if ((AutomaticFlashRingPositionConfigured(
           engine,
           engine->automaticFlashEntryPositions,
           ringIndex) == 0U)
        || (AutomaticFlashRingPositionConfigured(
              engine,
              engine->automaticFlashExitPositions,
              ringIndex) == 0U))
    {
      return 0U;
    }
  }

  return 1U;
}

static uint8_t AutomaticFlashOutputsActive(const IntersectionEngine_t *engine)
{
  if (engine == NULL)
  {
    return 0U;
  }

  if (StartUpFlashActive(engine) != 0U)
  {
    return 0U;
  }

  if (engine->mmuFlashActive != 0U)
  {
    return 1U;
  }

  if (engine->runtime.mode != INTERSECTION_CONTROL_MODE_FLASH)
  {
    return 0U;
  }

  if (AutomaticFlashTransitionConfigured(engine) == 0U)
  {
    return 1U;
  }

  return (uint8_t) (engine->automaticFlashState
                    == (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_FLASHING);
}

static uint8_t StartUpFlashActive(const IntersectionEngine_t *engine)
{
  return (uint8_t) ((engine != NULL)
                    && (engine->startUpFlashTicksRemaining != 0UL));
}

static uint8_t StartUpFlashUsesAutoFlashMode(
  const IntersectionEngine_t *engine)
{
  return (uint8_t) ((engine != NULL)
                    && (engine->config.unit.startUpFlashMode
                        == (uint8_t)
                        INTERSECTION_UNIT_STARTUP_FLASH_MODE_AUTO_FLASH));
}

static uint16_t PhaseRedRevertTicks(const IntersectionEngine_t *engine,
                                    uint8_t phaseIndex)
{
  uint8_t effectiveDs;

  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  effectiveDs = engine->config.unit.redRevertDs;

  if (engine->config.phases[phaseIndex].redRevertDs > effectiveDs)
  {
    effectiveDs = engine->config.phases[phaseIndex].redRevertDs;
  }

  return (uint16_t) ((uint16_t) effectiveDs * 10U);
}

static void StartPhaseRedRevertTimer(IntersectionEngine_t *engine,
                                     uint8_t phaseIndex)
{
  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return;
  }

  engine->redRevertTicks[phaseIndex] = PhaseRedRevertTicks(engine, phaseIndex);
}

static uint8_t PhaseRedRevertActive(const IntersectionEngine_t *engine,
                                    uint8_t phaseIndex)
{
  return (uint8_t) ((engine != NULL)
                    && (phaseIndex < engine->config.phaseCount)
                    && (engine->redRevertTicks[phaseIndex] != 0U));
}

static void TickPhaseRedRevertTimers(IntersectionEngine_t *engine)
{
  uint8_t phaseIndex;

  if (engine == NULL)
  {
    return;
  }

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    IntersectionPhaseInterval_t interval =
      engine->runtime.phases[phaseIndex].interval;

    if ((engine->redRevertTicks[phaseIndex] == 0U)
        || (interval == INTERSECTION_PHASE_INTERVAL_GREEN)
        || (interval == INTERSECTION_PHASE_INTERVAL_YELLOW))
    {
      continue;
    }

    engine->redRevertTicks[phaseIndex]--;
  }
}

static uint16_t PhaseDontWalkRevertTicks(const IntersectionEngine_t *engine,
                                         uint8_t phaseIndex)
{
  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  return (uint16_t) ((uint16_t) engine->config.phases[phaseIndex]
                       .dontWalkRevertDs
                     * 10U);
}

static uint16_t PhasePedClearVehicleClearanceTicks(
  const IntersectionEngine_t *engine,
  uint8_t phaseIndex)
{
  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  return (uint16_t) ((uint16_t) engine->config.phases[phaseIndex]
                       .yellowRedBeforeEndPedClearDs
                     * 10U);
}

static void StartPhaseDontWalkRevertTimer(IntersectionEngine_t *engine,
                                          uint8_t phaseIndex)
{
  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return;
  }

  engine->dontWalkRevertTicks[phaseIndex] = PhaseDontWalkRevertTicks(engine,
                                                                     phaseIndex);
}

static uint8_t PhaseDontWalkRevertActive(const IntersectionEngine_t *engine,
                                         uint8_t phaseIndex)
{
  return (uint8_t) ((engine != NULL)
                    && (phaseIndex < engine->config.phaseCount)
                    && (engine->dontWalkRevertTicks[phaseIndex] != 0U));
}

static void TickPhaseDontWalkRevertTimers(IntersectionEngine_t *engine)
{
  uint8_t phaseIndex;

  if (engine == NULL)
  {
    return;
  }

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    if ((engine->dontWalkRevertTicks[phaseIndex] == 0U)
        || (engine->runtime.phases[phaseIndex].pedInterval
            != INTERSECTION_PED_INTERVAL_DONT_WALK))
    {
      continue;
    }

    engine->dontWalkRevertTicks[phaseIndex]--;
  }
}

static uint8_t PhaseSystemPhaseOmitActive(const IntersectionEngine_t *engine,
                                          uint8_t phaseIndex)
{
  return (uint8_t) ((engine != NULL)
                    && (phaseIndex < engine->config.phaseCount)
                    && (engine->systemPhaseOmit[phaseIndex] != 0U));
}

static uint8_t PhaseSystemPedOmitActive(const IntersectionEngine_t *engine,
                                        uint8_t phaseIndex)
{
  return (uint8_t) ((engine != NULL)
                    && (phaseIndex < engine->config.phaseCount)
                    && (engine->systemPedOmit[phaseIndex] != 0U));
}

static uint8_t PhaseSystemHoldActive(const IntersectionEngine_t *engine,
                                     uint8_t phaseIndex)
{
  return (uint8_t) ((engine != NULL)
                    && (phaseIndex < engine->config.phaseCount)
                    && (engine->systemPhaseHold[phaseIndex] != 0U));
}

static uint8_t PhaseSystemForceOffActive(const IntersectionEngine_t *engine,
                                         uint8_t phaseIndex)
{
  return (uint8_t) ((engine != NULL)
                    && (phaseIndex < engine->config.phaseCount)
                    && (engine->systemPhaseForceOff[phaseIndex] != 0U));
}

static uint8_t PhaseSystemVehCallActive(const IntersectionEngine_t *engine,
                                        uint8_t phaseIndex)
{
  return (uint8_t) ((engine != NULL)
                    && (phaseIndex < engine->config.phaseCount)
                    && (engine->systemVehCalls[phaseIndex] != 0U));
}

static uint8_t PhaseSystemPedCallActive(const IntersectionEngine_t *engine,
                                        uint8_t phaseIndex)
{
  return (uint8_t) ((engine != NULL)
                    && (phaseIndex < engine->config.phaseCount)
                    && (engine->systemPedCalls[phaseIndex] != 0U));
}

static uint8_t PhaseRingIndex(const IntersectionEngine_t *engine,
                              uint8_t phaseIndex,
                              uint8_t *ringIndex)
{
  uint8_t internalRingIndex;

  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount)
      || (ringIndex == NULL))
  {
    return 0U;
  }

  internalRingIndex = engine->config.phases[phaseIndex].ring;

  if (internalRingIndex >= engine->config.ringCount)
  {
    return 0U;
  }

  *ringIndex = internalRingIndex;

  return 1U;
}

static uint8_t RingControlActive(const IntersectionEngine_t *engine,
                                 const uint8_t *controls,
                                 uint8_t ringIndex)
{
  return (uint8_t) ((engine != NULL) && (controls != NULL)
                    && (ringIndex < engine->config.ringCount)
                    && (controls[ringIndex] != 0U));
}

static uint8_t RingSystemStopTimeActive(const IntersectionEngine_t *engine,
                                        uint8_t ringIndex)
{
  return RingControlActive(engine, engine->systemRingStopTime, ringIndex);
}

static uint8_t RingSystemForceOffActive(const IntersectionEngine_t *engine,
                                        uint8_t ringIndex)
{
  return RingControlActive(engine, engine->systemRingForceOff, ringIndex);
}

static uint8_t RingSystemMaximum2Active(const IntersectionEngine_t *engine,
                                        uint8_t ringIndex)
{
  return RingControlActive(engine, engine->systemRingMax2, ringIndex);
}

static uint8_t RingSystemMaximumInhibitActive(
  const IntersectionEngine_t *engine,
  uint8_t ringIndex)
{
  return RingControlActive(engine, engine->systemRingMaxInhibit, ringIndex);
}

static uint8_t RingSystemPedRecycleActive(const IntersectionEngine_t *engine,
                                          uint8_t ringIndex)
{
  return RingControlActive(engine, engine->systemRingPedRecycle, ringIndex);
}

static uint8_t RingSystemRedRestActive(const IntersectionEngine_t *engine,
                                       uint8_t ringIndex)
{
  return RingControlActive(engine, engine->systemRingRedRest, ringIndex);
}

static uint8_t RingSystemOmitRedClearActive(const IntersectionEngine_t *engine,
                                            uint8_t ringIndex)
{
  return RingControlActive(engine, engine->systemRingOmitRedClear, ringIndex);
}

static uint8_t RingSystemMaximum3Active(const IntersectionEngine_t *engine,
                                        uint8_t ringIndex)
{
  return RingControlActive(engine, engine->systemRingMax3, ringIndex);
}

static uint8_t UnitControlExternalMinRecallActive(
  const IntersectionEngine_t *engine)
{
  return (uint8_t) ((engine != NULL) && ((engine->unitControl & 0x04U) != 0U));
}

static uint8_t UnitControlWalkRestModifierActive(
  const IntersectionEngine_t *engine)
{
  return (uint8_t) ((engine != NULL) && ((engine->unitControl & 0x20U) != 0U));
}

static uint8_t UnitControlDimmingEnableActive(
  const IntersectionEngine_t *engine)
{
  return (uint8_t) ((engine != NULL) && ((engine->unitControl & 0x80U) != 0U));
}

static uint8_t UnitControlInterconnectPriorityActive(
  const IntersectionEngine_t *engine)
{
  return (uint8_t) ((engine != NULL) && ((engine->unitControl & 0x40U) != 0U));
}

static uint8_t RemoteManualControlActive(const IntersectionEngine_t *engine)
{
  return (uint8_t) ((engine != NULL)
                    && (engine->remoteManualControlTimeout != 0U));
}

static uint8_t RemoteManualVehicleCallActive(const IntersectionEngine_t *engine,
                                             uint8_t phaseIndex)
{
  return (uint8_t) ((RemoteManualControlActive(engine) != 0U)
                    && (engine != NULL)
                    && (phaseIndex < engine->config.phaseCount));
}

static uint8_t RemoteManualPedCallActive(const IntersectionEngine_t *engine,
                                         uint8_t phaseIndex)
{
  return (uint8_t) ((RemoteManualVehicleCallActive(engine, phaseIndex) != 0U)
                    && (PhasePedConfigured(engine, phaseIndex) != 0U));
}

static uint8_t PatternCommandValid(uint8_t command)
{
  return (uint8_t) ((command == 0U) || (command == 254U) || (command == 255U)
                    || ((command >= 1U)
                        && (command <= INTERSECTION_PATTERN_COUNT_MAX)));
}

static uint8_t InterconnectCommandAvailable(const IntersectionEngine_t *engine)
{
  if ((engine == NULL) || (engine->localInterconnectInputsValid == 0U))
  {
    return 0U;
  }

  if (PatternCommandValid(engine->localInterconnectCommand) == 0U)
  {
    return 0U;
  }

  return (uint8_t) (engine->localInterconnectCommand != 0U);
}

static const IntersectionTimebaseActionConfig_t *GetSelectedTimebaseAction(
  const IntersectionEngine_t *engine)
{
  if ((engine == NULL) || (engine->actionPlanControl == 0U)
      || (engine->actionPlanControl > INTERSECTION_TIMEBASE_ACTION_COUNT_MAX))
  {
    return NULL;
  }

  return &engine->config.timebase.actions[engine->actionPlanControl - 1U];
}

static uint8_t TimebaseActionDimmingRequested(const IntersectionEngine_t *engine)
{
  const IntersectionTimebaseActionConfig_t *action =
    GetSelectedTimebaseAction(engine);

  if (action == NULL)
  {
    return 0U;
  }

  if ((action->auxiliaryFunction & INTERSECTION_TIMEBASE_AUX_FUNCTION_DIMMING)
      == 0U)
  {
    return 0U;
  }

  return (uint8_t) ((UnitControlDimmingEnableActive(engine) != 0U)
                    || (engine->localDimmingInputActive != 0U));
}

static uint8_t TimebaseActionControlsPattern(const IntersectionEngine_t *engine,
                                             uint8_t *command)
{
  const IntersectionTimebaseActionConfig_t *action =
    GetSelectedTimebaseAction(engine);

  if ((action == NULL) || (command == NULL) || (engine == NULL))
  {
    return 0U;
  }

  if ((engine->config.coordination.operationalMode != 0U)
      || (engine->systemPatternControl != 0U))
  {
    return 0U;
  }

  *command = action->pattern;

  return 1U;
}

static uint8_t TimebaseActionSpecialFunctionMask(
  const IntersectionEngine_t *engine)
{
  const IntersectionTimebaseActionConfig_t *action =
    GetSelectedTimebaseAction(engine);

  return (action != NULL) ? action->specialFunction : 0U;
}

static uint8_t BackupTimerConfigured(const IntersectionEngine_t *engine)
{
  return (uint8_t) ((engine != NULL)
                    && (engine->config.unit.backupTimeSeconds != 0U)
                    && (engine->config.unit.userDefinedBackupTimeSeconds
                        == 0UL));
}

static uint8_t UserDefinedBackupTimerConfigured(
  const IntersectionEngine_t *engine)
{
  return (uint8_t) ((engine != NULL)
                    && (engine->config.unit.userDefinedBackupTimeSeconds
                        != 0UL));
}

static void ResetBackupTimer(IntersectionEngine_t *engine)
{
  if ((engine == NULL) || (BackupTimerConfigured(engine) == 0U))
  {
    return;
  }

  engine->backupTimerArmed = 1U;
  engine->backupModeActive = 0U;
  engine->backupTimerTicksRemaining =
    (uint32_t) engine->config.unit.backupTimeSeconds * 100U;
  engine->runtime.backupModeActive = 0U;
}

static void TickStartUpFlashTimer(IntersectionEngine_t *engine)
{
  if ((engine == NULL) || (engine->startUpFlashTicksRemaining == 0UL))
  {
    return;
  }

  engine->startUpFlashTicksRemaining--;
}

static void ResetUserDefinedBackupTimer(IntersectionEngine_t *engine)
{
  if ((engine == NULL) || (UserDefinedBackupTimerConfigured(engine) == 0U))
  {
    return;
  }

  engine->userDefinedBackupTimerArmed = 1U;
  engine->backupModeActive = 0U;
  engine->userDefinedBackupTimerTicksRemaining =
    engine->config.unit.userDefinedBackupTimeSeconds * 100UL;
  engine->runtime.backupModeActive = 0U;
}

static void ClearRemoteManualControl(IntersectionEngine_t *engine)
{
  if (engine == NULL)
  {
    return;
  }

  engine->remoteManualControlTimeout = 0U;
  engine->remoteManualIntervalAdvance = 0U;
  engine->remoteManualTickAccumulator = 0U;
  memset(engine->remoteManualPedAutoAdvance,
         0,
         sizeof(engine->remoteManualPedAutoAdvance));
  engine->runtime.remoteManualControlTimeout = 0U;
  engine->runtime.remoteManualIntervalAdvance = 0U;
}

static void ClearRuntimeSystemControls(IntersectionEngine_t *engine)
{
  uint8_t detectorIndex;
  uint8_t pedDetectorIndex;
  uint8_t preemptIndex;

  if (engine == NULL)
  {
    return;
  }

  engine->systemPatternControl = 0U;
  engine->systemSyncControlSeconds = 0U;
  engine->actionPlanControl = 0U;
  engine->unitControl = 0U;
  ClearRemoteManualControl(engine);
  engine->specialFunctionControl = 0U;
  memset(engine->systemPhaseOmit, 0, sizeof(engine->systemPhaseOmit));
  memset(engine->systemPedOmit, 0, sizeof(engine->systemPedOmit));
  memset(engine->systemPhaseHold, 0, sizeof(engine->systemPhaseHold));
  memset(engine->systemPhaseForceOff, 0, sizeof(engine->systemPhaseForceOff));
  memset(engine->systemVehCalls, 0, sizeof(engine->systemVehCalls));
  memset(engine->systemPedCalls, 0, sizeof(engine->systemPedCalls));
  memset(engine->systemRingStopTime, 0, sizeof(engine->systemRingStopTime));
  memset(engine->systemRingForceOff, 0, sizeof(engine->systemRingForceOff));
  memset(engine->systemRingMax2, 0, sizeof(engine->systemRingMax2));
  memset(engine->systemRingMaxInhibit, 0, sizeof(engine->systemRingMaxInhibit));
  memset(engine->systemRingPedRecycle, 0, sizeof(engine->systemRingPedRecycle));
  memset(engine->systemRingRedRest, 0, sizeof(engine->systemRingRedRest));
  memset(engine->systemRingOmitRedClear,
         0,
         sizeof(engine->systemRingOmitRedClear));
  memset(engine->systemRingMax3, 0, sizeof(engine->systemRingMax3));

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    engine->runtime.vehicleDetectors[detectorIndex].remoteActuation = 0U;
  }

  for (pedDetectorIndex = 0U;
       pedDetectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       pedDetectorIndex++)
  {
    engine->runtime.pedestrianDetectors[pedDetectorIndex].remoteActuation = 0U;
  }

  for (preemptIndex = 0U;
       preemptIndex < INTERSECTION_PREEMPT_COUNT_MAX;
       preemptIndex++)
  {
    engine->runtime.preemptControlState[preemptIndex] = 0U;
  }

  engine->runtime.systemPatternControl = 0U;
  engine->runtime.systemSyncControlSeconds = 0U;
  engine->runtime.actionPlanControl = 0U;
  engine->runtime.unitControl = 0U;
  engine->runtime.remoteManualControlTimeout = 0U;
  engine->runtime.remoteManualIntervalAdvance = 0U;
  engine->runtime.specialFunctionControl = 0U;
  engine->runtime.specialFunctionStatus = 0U;
}

static void EnterBackupMode(IntersectionEngine_t *engine)
{
  if (engine == NULL)
  {
    return;
  }

  ClearRuntimeSystemControls(engine);
  engine->backupTimerArmed = 0U;
  engine->userDefinedBackupTimerArmed = 0U;
  engine->backupModeActive = 1U;
  engine->backupTimerTicksRemaining = 0U;
  engine->userDefinedBackupTimerTicksRemaining = 0U;
  engine->runtime.backupModeActive = 1U;
  UpdateCoordinationRuntime(engine);
}

static void TickBackupTimer(IntersectionEngine_t *engine)
{
  if ((engine == NULL) || (engine->backupTimerArmed == 0U)
      || (engine->backupModeActive != 0U))
  {
    return;
  }

  if (engine->backupTimerTicksRemaining > 0U)
  {
    engine->backupTimerTicksRemaining--;
  }

  if (engine->backupTimerTicksRemaining == 0U)
  {
    EnterBackupMode(engine);
  }
}

static void TickUserDefinedBackupTimer(IntersectionEngine_t *engine)
{
  if ((engine == NULL) || (engine->userDefinedBackupTimerArmed == 0U)
      || (engine->backupModeActive != 0U))
  {
    return;
  }

  if (engine->userDefinedBackupTimerTicksRemaining > 0U)
  {
    engine->userDefinedBackupTimerTicksRemaining--;
  }

  if (engine->userDefinedBackupTimerTicksRemaining == 0U)
  {
    EnterBackupMode(engine);
  }
}

static void TickRemoteManualControlTimer(IntersectionEngine_t *engine)
{
  if ((engine == NULL) || (RemoteManualControlActive(engine) == 0U))
  {
    return;
  }

  if (engine->remoteManualTickAccumulator == 0U)
  {
    engine->remoteManualTickAccumulator = 100U;
  }

  engine->remoteManualTickAccumulator--;

  if (engine->remoteManualTickAccumulator != 0U)
  {
    return;
  }

  if (engine->remoteManualControlTimeout > 0U)
  {
    engine->remoteManualControlTimeout--;
  }

  engine->runtime.remoteManualControlTimeout =
    engine->remoteManualControlTimeout;

  if (engine->remoteManualControlTimeout == 0U)
  {
    ClearRemoteManualControl(engine);
  }
  else
  {
    engine->remoteManualTickAccumulator = 100U;
  }
}

static void FinalizeRemoteManualAdvanceCommand(IntersectionEngine_t *engine)
{
  if ((engine == NULL) || (RemoteManualControlActive(engine) == 0U)
      || (engine->remoteManualIntervalAdvance == 0U))
  {
    return;
  }

  engine->remoteManualIntervalAdvance = 0U;
  engine->runtime.remoteManualIntervalAdvance = 0U;
}

static uint8_t PhaseUnitControlledNonActuatedActive(
  const IntersectionEngine_t *engine,
  uint8_t phaseIndex)
{
  const IntersectionPhaseConfig_t *phaseConfig;

  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  if (PhasePedConfigured(engine, phaseIndex) == 0U)
  {
    return 0U;
  }

  phaseConfig = &engine->config.phases[phaseIndex];

  if (((engine->unitControl & 0x08U) != 0U)
      && ((phaseConfig->phaseOptions & PHASE_OPTIONS_NON_ACTUATED_1) != 0U))
  {
    return 1U;
  }

  if (((engine->unitControl & 0x10U) != 0U)
      && ((phaseConfig->phaseOptions & PHASE_OPTIONS_NON_ACTUATED_2) != 0U))
  {
    return 1U;
  }

  return 0U;
}

static uint8_t PhaseHasVehicleRecall(const IntersectionPhaseConfig_t *phaseConfig)
{
  return (uint8_t) ((phaseConfig != NULL)
                    && (IntersectionPhaseOptionsHasVehicleRecall(
                          phaseConfig->phaseOptions)
                        != 0U));
}

static uint8_t PhaseHasMaxRecall(const IntersectionPhaseConfig_t *phaseConfig)
{
  return (uint8_t) ((phaseConfig != NULL)
                    && ((phaseConfig->phaseOptions & PHASE_OPTIONS_MAX_RECALL)
                        != 0U));
}

static uint8_t PhaseUsesDetectorOptionBasedLocking(
  const IntersectionEngine_t *engine,
  uint8_t phaseIndex)
{
  return (uint8_t) ((engine != NULL)
                    && (phaseIndex < engine->config.phaseCount)
                    && ((engine->config.phases[phaseIndex].phaseOptions
                         & PHASE_OPTIONS_NON_LOCK_DET_MEM) != 0U));
}

static uint8_t CoordinationPatternIsActive(const IntersectionEngine_t *engine)
{
  return (uint8_t) ((engine->runtime.coordPatternStatus >= 1U)
                    && (engine->runtime.coordPatternStatus
                        <= INTERSECTION_PATTERN_COUNT_MAX)
                    && (engine->runtime.mode
                        == INTERSECTION_CONTROL_MODE_COORDINATED));
}

static const IntersectionPatternConfig_t *GetActivePattern(
  const IntersectionEngine_t *engine)
{
  uint8_t patternStatus;

  if (engine == NULL)
  {
    return NULL;
  }

  patternStatus = engine->runtime.coordPatternStatus;

  if ((engine->runtime.mode != INTERSECTION_CONTROL_MODE_COORDINATED)
      || (patternStatus == 0U)
      || (patternStatus > INTERSECTION_PATTERN_COUNT_MAX))
  {
    return NULL;
  }

  return &engine->config.coordination.patterns[patternStatus - 1U];
}

static uint8_t PreemptModeActive(const IntersectionEngine_t *engine)
{
  return (uint8_t) ((engine != NULL)
                    && (engine->runtime.mode
                        == INTERSECTION_CONTROL_MODE_PREEMPT)
                    && (engine->activePreemptIndex
                        < INTERSECTION_PREEMPT_COUNT_MAX));
}

static uint8_t PreemptBaseDemandIsPresent(const IntersectionEngine_t *engine,
                                          uint8_t preemptIndex)
{
  return (uint8_t) ((engine->runtime.preemptInputStatus[preemptIndex] != 0U)
                    || (engine->runtime.preemptControlState[preemptIndex]
                        != 0U));
}

static void ClearLinkedPreemptCall(IntersectionEngine_t *engine)
{
  if (engine == NULL)
  {
    return;
  }

  engine->linkedPreemptSourceIndex = 0xFFU;
  engine->linkedPreemptTargetIndex = 0xFFU;
}

static uint8_t LinkedPreemptCallIsPresent(const IntersectionEngine_t *engine,
                                          uint8_t preemptIndex)
{
  if ((engine == NULL)
      || (engine->linkedPreemptSourceIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (engine->linkedPreemptTargetIndex != preemptIndex))
  {
    return 0U;
  }

  return PreemptBaseDemandIsPresent(engine, engine->linkedPreemptSourceIndex);
}

static uint8_t PreemptInputIsPresent(const IntersectionEngine_t *engine,
                                     uint8_t preemptIndex)
{
  return (uint8_t) ((PreemptBaseDemandIsPresent(engine, preemptIndex) != 0U)
                    || (LinkedPreemptCallIsPresent(engine, preemptIndex) != 0U));
}

static uint8_t PreemptIsEnabled(const IntersectionEngine_t *engine,
                                uint8_t preemptIndex)
{
  return (uint8_t) ((engine->config.preempts[preemptIndex].control & 0x10U)
                    != 0U);
}

static uint8_t PreemptNonLocking(const IntersectionEngine_t *engine,
                                 uint8_t preemptIndex)
{
  return (uint8_t) ((engine->config.preempts[preemptIndex].control & 0x01U)
                    != 0U);
}

static uint8_t PreemptOverridesFlash(const IntersectionEngine_t *engine,
                                     uint8_t preemptIndex)
{
  return (uint8_t) ((engine->config.preempts[preemptIndex].control & 0x02U)
                    == 0U);
}

static uint8_t PreemptFlashDwell(const IntersectionEngine_t *engine,
                                 uint8_t preemptIndex)
{
  return (uint8_t) ((engine->config.preempts[preemptIndex].control & 0x08U)
                    != 0U);
}

static uint8_t PreemptAllRedFlashOnMaxPresence(
  const IntersectionEngine_t *engine,
  uint8_t preemptIndex)
{
  return (uint8_t) ((engine->config.preempts[preemptIndex].control & 0x20U)
                    != 0U);
}

static uint8_t PreemptStateIsServicing(IntersectionPreemptState_t state)
{
  return (uint8_t) ((state == INTERSECTION_PREEMPT_STATE_ENTRY_STARTED)
                    || (state == INTERSECTION_PREEMPT_STATE_TRACK_SERVICE)
                    || (state == INTERSECTION_PREEMPT_STATE_DWELL)
                    || (state == INTERSECTION_PREEMPT_STATE_LINK_ACTIVE)
                    || (state == INTERSECTION_PREEMPT_STATE_EXIT_STARTED)
                    || (state == INTERSECTION_PREEMPT_STATE_MAX_PRESENCE)
                    || (state == INTERSECTION_PREEMPT_STATE_ADVANCED_PREEMPT));
}

static uint8_t PreemptPriorityGroup(const IntersectionEngine_t *engine,
                                    uint8_t preemptIndex)
{
  uint8_t group = 0U;
  uint8_t index;

  for (index = 0U; index < preemptIndex; index++)
  {
    if ((engine->config.preempts[index].control & 0x04U) == 0U)
    {
      group++;
    }
  }

  return group;
}

static uint8_t ResolveLinkedPreemptTarget(const IntersectionEngine_t *engine,
                                          uint8_t sourcePreemptIndex,
                                          uint8_t *targetPreemptIndex)
{
  uint8_t configuredLink;
  uint8_t resolvedTarget;

  if ((engine == NULL) || (targetPreemptIndex == NULL)
      || (sourcePreemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  configuredLink = engine->config.preempts[sourcePreemptIndex].link;

  if ((configuredLink == 0U)
      || (configuredLink > INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  resolvedTarget = (uint8_t) (configuredLink - 1U);

  if ((resolvedTarget == sourcePreemptIndex)
      || (PreemptIsEnabled(engine, resolvedTarget) == 0U)
      || (PreemptPriorityGroup(engine, resolvedTarget)
          >= PreemptPriorityGroup(engine, sourcePreemptIndex)))
  {
    return 0U;
  }

  *targetPreemptIndex = resolvedTarget;

  return 1U;
}

static void RefreshLinkedPreemptCall(IntersectionEngine_t *engine)
{
  uint8_t expectedTarget = 0xFFU;

  if ((engine == NULL)
      || (engine->linkedPreemptSourceIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (engine->linkedPreemptTargetIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return;
  }

  if ((PreemptBaseDemandIsPresent(engine, engine->linkedPreemptSourceIndex)
       == 0U)
      || (ResolveLinkedPreemptTarget(engine,
                                     engine->linkedPreemptSourceIndex,
                                     &expectedTarget)
          == 0U)
      || (expectedTarget != engine->linkedPreemptTargetIndex))
  {
    ClearLinkedPreemptCall(engine);

    return;
  }

  engine->runtime.preemptStates[engine->linkedPreemptSourceIndex] =
    INTERSECTION_PREEMPT_STATE_LINK_ACTIVE;
}

static int16_t SelectPreemptCandidate(const IntersectionEngine_t *engine)
{
  int16_t selected = -1;
  uint8_t selectedGroup = 0xFFU;
  uint8_t preemptIndex;

  for (preemptIndex = 0U;
       preemptIndex < INTERSECTION_PREEMPT_COUNT_MAX;
       preemptIndex++)
  {
    uint8_t group;

    if ((PreemptIsEnabled(engine, preemptIndex) == 0U)
        || (PreemptInputIsPresent(engine, preemptIndex) == 0U))
    {
      continue;
    }

    group = PreemptPriorityGroup(engine, preemptIndex);

    if ((selected < 0) || (group < selectedGroup))
    {
      selected = (int16_t) preemptIndex;
      selectedGroup = group;
    }
  }

  if ((engine != NULL)
      && (engine->activePreemptIndex < INTERSECTION_PREEMPT_COUNT_MAX)
      && (PreemptStateIsServicing(
            engine->runtime.preemptStates[engine->activePreemptIndex]) != 0U))
  {
    uint8_t activeGroup = PreemptPriorityGroup(engine, engine->activePreemptIndex);

    if ((selected < 0) || (selectedGroup >= activeGroup))
    {
      return (int16_t) engine->activePreemptIndex;
    }
  }

  return selected;
}

static void ClearPreemptPhaseOutputs(IntersectionEngine_t *engine)
{
  uint8_t phaseIndex;

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    engine->runtime.phases[phaseIndex].interval =
      INTERSECTION_PHASE_INTERVAL_RED;
    engine->runtime.phases[phaseIndex].intervalElapsedTicks = 0U;
    engine->runtime.phases[phaseIndex].next = 0U;
    engine->runtime.phases[phaseIndex].pedInterval =
      INTERSECTION_PED_INTERVAL_DONT_WALK;
    engine->runtime.phases[phaseIndex].pedIntervalElapsedTicks = 0U;
    engine->runtime.phases[phaseIndex].pedServicePending = 0U;
    engine->runtime.phases[phaseIndex].pedServiceActive = 0U;
  }
}

static void ClearPreemptOverlapOutputs(IntersectionEngine_t *engine)
{
  uint8_t overlapIndex;

  for (overlapIndex = 0U;
       overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX;
       overlapIndex++)
  {
    engine->runtime.overlaps[overlapIndex].aspect =
      INTERSECTION_OUTPUT_ASPECT_RED;
  }
}

static void ApplyPhaseListInterval(IntersectionEngine_t *engine,
                                   const IntersectionPhaseReferenceList_t *
                                   phases,
                                   IntersectionPhaseInterval_t interval)
{
  uint8_t index;

  if ((engine == NULL) || (phases == NULL))
  {
    return;
  }

  for (index = 0U; index < phases->length; index++)
  {
    uint8_t phaseNumber = phases->values[index];
    uint8_t phaseIndex = (uint8_t) (phaseNumber - 1U);

    if ((phaseNumber != 0U) && (phaseIndex < engine->config.phaseCount))
    {
      engine->runtime.phases[phaseIndex].interval = interval;
    }
  }
}

static void ApplyPedListInterval(IntersectionEngine_t *engine,
                                 const IntersectionPhaseReferenceList_t *phases,
                                 IntersectionPedInterval_t interval)
{
  uint8_t index;

  if ((engine == NULL) || (phases == NULL))
  {
    return;
  }

  for (index = 0U; index < phases->length; index++)
  {
    uint8_t phaseNumber = phases->values[index];
    uint8_t phaseIndex = (uint8_t) (phaseNumber - 1U);

    if ((phaseNumber != 0U) && (phaseIndex < engine->config.phaseCount))
    {
      engine->runtime.phases[phaseIndex].pedInterval = interval;
    }
  }
}

static void ApplyOverlapListAspect(IntersectionEngine_t *engine,
                                   const IntersectionOverlapReferenceList_t *
                                   overlaps,
                                   IntersectionOutputAspect_t aspect)
{
  uint8_t index;

  if ((engine == NULL) || (overlaps == NULL))
  {
    return;
  }

  for (index = 0U; index < overlaps->length; index++)
  {
    uint8_t overlapNumber = overlaps->values[index];
    uint8_t overlapIndex = (uint8_t) (overlapNumber - 1U);

    if ((overlapNumber != 0U)
        && (overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX))
    {
      engine->runtime.overlaps[overlapIndex].aspect = aspect;
    }
  }
}

static const IntersectionSplitPhaseConfig_t *GetActiveSplit(
  const IntersectionEngine_t *engine,
  uint8_t phaseIndex)
{
  const IntersectionPatternConfig_t *pattern = GetActivePattern(engine);
  uint8_t splitIndex;

  if ((pattern == NULL) || (pattern->splitNumber == 0U)
      || (pattern->splitNumber > INTERSECTION_SPLIT_COUNT_MAX)
      || (phaseIndex >= engine->config.phaseCount))
  {
    return NULL;
  }

  splitIndex = (uint8_t) (pattern->splitNumber - 1U);

  return &engine->config.coordination.splits[splitIndex][phaseIndex];
}

static uint8_t PhaseSplitModeIsRecall(
  const IntersectionSplitPhaseConfig_t *split)
{
  if (split == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((split->mode
                     == (uint8_t) INTERSECTION_SPLIT_MODE_MINIMUM_VEHICLE_RECALL)
                    || (split->mode
                        == (uint8_t)
                        INTERSECTION_SPLIT_MODE_MAXIMUM_VEHICLE_RECALL)
                    || (split->mode
                        == (uint8_t)
                        INTERSECTION_SPLIT_MODE_MAXIMUM_VEHICLE_AND_PEDESTRIAN_RECALL)
                    || (split->mode
                        == (uint8_t) INTERSECTION_SPLIT_MODE_NON_ACTUATED));
}

static uint8_t PhaseSplitModeHasPedRecall(
  const IntersectionSplitPhaseConfig_t *split)
{
  if (split == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((split->mode
                     == (uint8_t) INTERSECTION_SPLIT_MODE_PEDESTRIAN_RECALL)
                    || (split->mode
                        == (uint8_t)
                        INTERSECTION_SPLIT_MODE_MAXIMUM_VEHICLE_AND_PEDESTRIAN_RECALL));
}

static uint8_t PhaseSplitModeIsOmitted(
  const IntersectionSplitPhaseConfig_t *split)
{
  return (uint8_t) ((split != NULL)
                    && (split->mode
                        == (uint8_t) INTERSECTION_SPLIT_MODE_PHASE_OMITTED));
}

static uint8_t PhaseSplitModeIsMaxRecall(
  const IntersectionSplitPhaseConfig_t *split)
{
  if (split == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((split->mode
                     == (uint8_t) INTERSECTION_SPLIT_MODE_MAXIMUM_VEHICLE_RECALL)
                    || (split->mode
                        == (uint8_t)
                        INTERSECTION_SPLIT_MODE_MAXIMUM_VEHICLE_AND_PEDESTRIAN_RECALL));
}

static uint8_t PhasesMayRunConcurrently(const IntersectionEngine_t *engine,
                                        uint8_t subjectPhaseIndex,
                                        uint8_t otherPhaseIndex)
{
  const IntersectionPhaseReferenceList_t *subjectList;
  const IntersectionPhaseReferenceList_t *otherList;
  uint8_t referenceIndex;
  uint8_t otherPhaseNumber;
  uint8_t subjectPhaseNumber;

  if ((engine == NULL) || (subjectPhaseIndex >= engine->config.phaseCount)
      || (otherPhaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  if (subjectPhaseIndex == otherPhaseIndex)
  {
    return 1U;
  }

  if (engine->config.phases[subjectPhaseIndex].ring
      == engine->config.phases[otherPhaseIndex].ring)
  {
    return 0U;
  }

  otherPhaseNumber = (uint8_t) (otherPhaseIndex + 1U);
  subjectPhaseNumber = (uint8_t) (subjectPhaseIndex + 1U);
  subjectList = &engine->config.phases[subjectPhaseIndex].concurrency;
  otherList = &engine->config.phases[otherPhaseIndex].concurrency;

  for (referenceIndex = 0U; referenceIndex < subjectList->length;
       referenceIndex++)
  {
    if (subjectList->values[referenceIndex] == otherPhaseNumber)
    {
      return 1U;
    }
  }

  for (referenceIndex = 0U; referenceIndex < otherList->length;
       referenceIndex++)
  {
    if (otherList->values[referenceIndex] == subjectPhaseNumber)
    {
      return 1U;
    }
  }

  return 0U;
}

static uint8_t SelectedCoordMaximumMode(const IntersectionEngine_t *engine)
{
  const IntersectionPatternConfig_t *pattern;

  if ((engine == NULL) || (CoordinationPatternIsActive(engine) == 0U))
  {
    return (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM1;
  }

  pattern = GetActivePattern(engine);

  if (pattern == NULL)
  {
    return (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM1;
  }

  switch ((IntersectionPatternOptions_t) pattern->options)
  {
      case INTERSECTION_PATTERN_OPTIONS_COORD_MAXIMUM_MODE:
      {
        return engine->config.coordination.maximumMode;
      }

      case INTERSECTION_PATTERN_OPTIONS_MAX_INHIBIT:
      {
        return (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAX_INHIBIT;
      }

      case INTERSECTION_PATTERN_OPTIONS_MAXIMUM2:
      {
        return (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM2;
      }

      case INTERSECTION_PATTERN_OPTIONS_MAXIMUM3:
      {
        return (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM3;
      }

      case INTERSECTION_PATTERN_OPTIONS_MAXIMUM1:
      case INTERSECTION_PATTERN_OPTIONS_OTHER:
      default:
      {
        return (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM1;
      }
  }
}

static uint8_t SelectedPhaseMaximumMode(const IntersectionEngine_t *engine,
                                        uint8_t phaseIndex)
{
  uint8_t ringIndex;

  if (PhaseRingIndex(engine, phaseIndex, &ringIndex) != 0U)
  {
    if (RingSystemMaximumInhibitActive(engine, ringIndex) != 0U)
    {
      return (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAX_INHIBIT;
    }

    if (RingSystemMaximum3Active(engine, ringIndex) != 0U)
    {
      return (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM3;
    }

    if (RingSystemMaximum2Active(engine, ringIndex) != 0U)
    {
      return (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM2;
    }
  }

  return SelectedCoordMaximumMode(engine);
}

static uint32_t BaseMaximumGreenTicks(const IntersectionEngine_t *engine,
                                      uint8_t phaseIndex)
{
  const IntersectionPhaseConfig_t *phaseConfig =
    &engine->config.phases[phaseIndex];

  switch ((IntersectionCoordMaximumMode_t) SelectedPhaseMaximumMode(engine,
                                                                    phaseIndex))
  {
      case INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM2:
      {
        return (uint32_t) phaseConfig->phaseMaximum2Ds * 10U;
      }

      case INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM3:
      {
        return (uint32_t) phaseConfig->phaseMaximum3Ds * 10U;
      }

      case INTERSECTION_COORD_MAXIMUM_MODE_OTHER:
      case INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM1:
      case INTERSECTION_COORD_MAXIMUM_MODE_MAX_INHIBIT:
      default:
      {
        return engine->maxGreenTicks[phaseIndex];
      }
  }
}

static uint8_t PhaseDynamicMaxEnabled(const IntersectionEngine_t *engine,
                                      uint8_t phaseIndex)
{
  const IntersectionPhaseConfig_t *phaseConfig =
    &engine->config.phases[phaseIndex];
  const IntersectionSplitPhaseConfig_t *split = GetActiveSplit(engine,
                                                               phaseIndex);

  return (uint8_t) ((phaseConfig->dynamicMaxLimitSeconds != 0U)
                    && (phaseConfig->dynamicMaxStepDs != 0U)
                    && (PhaseHasMaxRecall(phaseConfig) == 0U)
                    && (PhaseSplitModeIsMaxRecall(split) == 0U)
                    && (SelectedPhaseMaximumMode(engine, phaseIndex)
                        != (uint8_t)
                        INTERSECTION_COORD_MAXIMUM_MODE_MAX_INHIBIT));
}

static void RefreshPhaseRunningMax(IntersectionEngine_t *engine,
                                   uint8_t phaseIndex)
{
  const IntersectionPhaseConfig_t *phaseConfig =
    &engine->config.phases[phaseIndex];
  uint32_t baseMaxTicks = BaseMaximumGreenTicks(engine, phaseIndex);
  uint32_t limitTicks = (uint32_t) phaseConfig->dynamicMaxLimitSeconds * 100U;
  uint32_t lowerLimit;
  uint32_t upperLimit;

  if ((engine->selectedNormalMaxTicks[phaseIndex] != baseMaxTicks)
      || (engine->runningMaxTicks[phaseIndex] == 0U))
  {
    engine->selectedNormalMaxTicks[phaseIndex] = baseMaxTicks;
    engine->runningMaxTicks[phaseIndex] = baseMaxTicks;
    engine->dynamicMaxGapOutCount[phaseIndex] = 0U;
    engine->dynamicMaxMaxOutCount[phaseIndex] = 0U;
  }

  if (PhaseDynamicMaxEnabled(engine, phaseIndex) == 0U)
  {
    engine->runningMaxTicks[phaseIndex] = baseMaxTicks;

    return;
  }

  lowerLimit = (limitTicks < baseMaxTicks) ? limitTicks : baseMaxTicks;
  upperLimit = (limitTicks > baseMaxTicks) ? limitTicks : baseMaxTicks;

  if (engine->runningMaxTicks[phaseIndex] < lowerLimit)
  {
    engine->runningMaxTicks[phaseIndex] = lowerLimit;
  }
  else if (engine->runningMaxTicks[phaseIndex] > upperLimit)
  {
    engine->runningMaxTicks[phaseIndex] = upperLimit;
  }
}

static void ApplyDynamicMaxTermination(IntersectionEngine_t *engine,
                                       uint8_t phaseIndex,
                                       uint8_t terminationReasonBits)
{
  const IntersectionPhaseConfig_t *phaseConfig =
    &engine->config.phases[phaseIndex];
  uint32_t baseMaxTicks;
  uint32_t limitTicks;
  uint32_t lowerLimit;
  uint32_t upperLimit;
  uint32_t stepTicks;

  RefreshPhaseRunningMax(engine, phaseIndex);

  if (PhaseDynamicMaxEnabled(engine, phaseIndex) == 0U)
  {
    return;
  }

  baseMaxTicks = engine->selectedNormalMaxTicks[phaseIndex];
  limitTicks = (uint32_t) phaseConfig->dynamicMaxLimitSeconds * 100U;
  lowerLimit = (limitTicks < baseMaxTicks) ? limitTicks : baseMaxTicks;
  upperLimit = (limitTicks > baseMaxTicks) ? limitTicks : baseMaxTicks;
  stepTicks = (uint32_t) phaseConfig->dynamicMaxStepDs * 10U;

  if ((stepTicks == 0U) || (upperLimit <= lowerLimit))
  {
    return;
  }

  if ((terminationReasonBits & INTERSECTION_RING_TERMINATION_MAX_OUT) != 0U)
  {
    engine->dynamicMaxGapOutCount[phaseIndex] = 0U;

    if (engine->dynamicMaxMaxOutCount[phaseIndex] < 0xFFU)
    {
      engine->dynamicMaxMaxOutCount[phaseIndex]++;
    }

    if (engine->dynamicMaxMaxOutCount[phaseIndex] >= 2U)
    {
      uint32_t updatedTicks = engine->runningMaxTicks[phaseIndex] + stepTicks;

      engine->runningMaxTicks[phaseIndex] = (updatedTicks > upperLimit)
                                            ? upperLimit
                                            : updatedTicks;
    }
  }
  else if ((terminationReasonBits & INTERSECTION_RING_TERMINATION_GAP_OUT)
           != 0U)
  {
    engine->dynamicMaxMaxOutCount[phaseIndex] = 0U;

    if (engine->dynamicMaxGapOutCount[phaseIndex] < 0xFFU)
    {
      engine->dynamicMaxGapOutCount[phaseIndex]++;
    }

    if (engine->dynamicMaxGapOutCount[phaseIndex] >= 2U)
    {
      engine->runningMaxTicks[phaseIndex] =
        (engine->runningMaxTicks[phaseIndex] > (lowerLimit + stepTicks))
        ? (engine->runningMaxTicks[phaseIndex] - stepTicks)
        : lowerLimit;
    }
  }
}

static uint16_t PhaseMinimumServiceSeconds(const IntersectionEngine_t *engine,
                                           uint8_t phaseIndex)
{
  const IntersectionPhaseConfig_t *phaseConfig =
    &engine->config.phases[phaseIndex];
  uint16_t minimumSeconds = (uint16_t) (phaseConfig->minGreenDs / 10U);

  minimumSeconds = (uint16_t) (minimumSeconds
                               + (phaseConfig->yellowChangeDs / 10U)
                               + (phaseConfig->redClearDs / 10U));

  if (PhasePedConfigured(engine, phaseIndex) != 0U)
  {
    minimumSeconds = (uint16_t) (minimumSeconds
                                 + phaseConfig->walkSeconds
                                 + phaseConfig->pedClearSeconds);
  }

  return minimumSeconds;
}

static uint16_t CoordinationCriticalPathSeconds(
  const IntersectionEngine_t *engine)
{
  uint16_t ringTotals[INTERSECTION_RING_COUNT_MAX] = { 0U };
  uint8_t ringIndex;

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];
    uint8_t position;

    for (position = 0U; position < ringPlan->phaseCount; position++)
    {
      uint8_t phaseIndex = ringPlan->phaseOrder[position];

      ringTotals[ringIndex] = (uint16_t) (ringTotals[ringIndex]
                                          + PhaseMinimumServiceSeconds(engine,
                                                                       phaseIndex));
    }
  }

  return (ringTotals[0] >= ringTotals[1]) ? ringTotals[0] : ringTotals[1];
}

static uint16_t CoordinationSplitOverrunSeconds(
  const IntersectionEngine_t *engine,
  uint8_t splitIndex)
{
  uint16_t ringTotals[INTERSECTION_RING_COUNT_MAX] = { 0U };
  uint8_t ringIndex;

  if (splitIndex >= INTERSECTION_SPLIT_COUNT_MAX)
  {
    return 0U;
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];
    uint8_t position;

    for (position = 0U; position < ringPlan->phaseCount; position++)
    {
      uint8_t phaseIndex = ringPlan->phaseOrder[position];

      ringTotals[ringIndex] = (uint16_t) (ringTotals[ringIndex]
                                          + engine->config.coordination
                                          .splits[splitIndex][phaseIndex]
                                          .timeSeconds);
    }
  }

  return (ringTotals[0] >= ringTotals[1]) ? ringTotals[0] : ringTotals[1];
}

static uint32_t EffectiveMaxGreenTicks(IntersectionEngine_t *engine,
                                       uint8_t phaseIndex)
{
  uint32_t maxGreenTicks = UINT32_MAX;
  const IntersectionSplitPhaseConfig_t *split = GetActiveSplit(engine,
                                                               phaseIndex);

  RefreshPhaseRunningMax(engine, phaseIndex);

  if ((SelectedPhaseMaximumMode(engine, phaseIndex)
       != (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAX_INHIBIT)
      && (engine->runningMaxTicks[phaseIndex] != 0U))
  {
    maxGreenTicks = engine->runningMaxTicks[phaseIndex];
  }

  if ((split != NULL) && (split->timeSeconds != 0U))
  {
    uint32_t splitTicks = (uint32_t) split->timeSeconds * 100U;
    uint32_t clearTicks = (uint32_t) engine->yellowTicks[phaseIndex]
                          + (uint32_t) engine->redClearTicks[phaseIndex];

    if (splitTicks > clearTicks)
    {
      uint32_t splitGreenTicks = splitTicks - clearTicks;

      if (splitGreenTicks < maxGreenTicks)
      {
        maxGreenTicks = splitGreenTicks;
      }
    }
  }

  return maxGreenTicks;
}

static uint8_t PhaseHasPedDemand(const IntersectionEngine_t *engine,
                                 uint8_t phaseIndex)
{
  const IntersectionPhaseRuntime_t *phaseRuntime =
    &engine->runtime.phases[phaseIndex];
  const IntersectionPhaseConfig_t *phaseConfig =
    &engine->config.phases[phaseIndex];
  const IntersectionSplitPhaseConfig_t *split = GetActiveSplit(engine,
                                                               phaseIndex);
  uint8_t ringIndex = 0U;
  uint8_t pedRecycleActive = PhaseRingIndex(engine, phaseIndex, &ringIndex)
                             != 0U
                             ? RingSystemPedRecycleActive(engine, ringIndex)
                             : 0U;

  if (PreemptCyclingPedAllowed(engine, phaseIndex) == 0U)
  {
    return 0U;
  }

  if ((PhaseSystemPedOmitActive(engine, phaseIndex) != 0U)
      && (phaseRuntime->pedServiceActive == 0U)
      && (phaseRuntime->pedInterval == INTERSECTION_PED_INTERVAL_DONT_WALK))
  {
    return 0U;
  }

  return (uint8_t) ((PhasePedConfigured(engine, phaseIndex) != 0U)
                    && (((phaseConfig->phaseOptions & PHASE_OPTIONS_PED_RECALL)
                         != 0U)
                        || (PhaseSplitModeHasPedRecall(split) != 0U)
                        || (RemoteManualPedCallActive(engine, phaseIndex) != 0U)
                        || (PhaseSystemPedCallActive(engine, phaseIndex) != 0U)
                        || (pedRecycleActive != 0U)
                        || (phaseRuntime->pedInputActive != 0U)
                        || (phaseRuntime->pedCallLatched != 0U)
                        || (phaseRuntime->pedServicePending != 0U)
                        || (phaseRuntime->pedServiceActive != 0U)));
}

static uint8_t PhaseHasBaseDemand(const IntersectionEngine_t *engine,
                                  uint8_t phaseIndex)
{
  const IntersectionPhaseRuntime_t *phaseRuntime;
  const IntersectionPhaseConfig_t *phaseConfig;
  const IntersectionSplitPhaseConfig_t *split;

  phaseRuntime = &engine->runtime.phases[phaseIndex];
  phaseConfig = &engine->config.phases[phaseIndex];
  split = GetActiveSplit(engine, phaseIndex);

  if (PreemptCyclingPhaseAllowed(engine, phaseIndex) == 0U)
  {
    return 0U;
  }

  if ((PhaseSplitModeIsOmitted(split) != 0U)
      || (PhaseSystemPhaseOmitActive(engine, phaseIndex) != 0U))
  {
    return 0U;
  }

  return (uint8_t) ((phaseRuntime->detectorActive != 0U)
                    || (PhaseSystemVehCallActive(engine, phaseIndex) != 0U)
                    || (RemoteManualVehicleCallActive(engine, phaseIndex) != 0U)
                    || (UnitControlExternalMinRecallActive(engine) != 0U)
                    || (PhaseUnitControlledNonActuatedActive(engine,
                                                             phaseIndex) != 0U)
                    || ((phaseRuntime->callLatched != 0U)
                        && ((phaseRuntime->interval
                             != INTERSECTION_PHASE_INTERVAL_GREEN)
                            || (engine->passageTicks[phaseIndex] == 0U)
                            || (engine->gapTimerTicks[phaseIndex] > 0U)))
                    || (PhaseHasVehicleRecall(phaseConfig) != 0U)
                    || (PhaseSplitModeIsRecall(split) != 0U)
                    || (PhaseHasPedDemand(engine, phaseIndex) != 0U));
}

static uint8_t PhaseHasVehicleServiceDemand(const IntersectionEngine_t *engine,
                                            uint8_t phaseIndex)
{
  const IntersectionPhaseRuntime_t *phaseRuntime;
  const IntersectionPhaseConfig_t *phaseConfig;
  const IntersectionSplitPhaseConfig_t *split;

  phaseRuntime = &engine->runtime.phases[phaseIndex];
  phaseConfig = &engine->config.phases[phaseIndex];
  split = GetActiveSplit(engine, phaseIndex);

  if (PreemptCyclingPhaseAllowed(engine, phaseIndex) == 0U)
  {
    return 0U;
  }

  if ((PhaseSplitModeIsOmitted(split) != 0U)
      || (PhaseSystemPhaseOmitActive(engine, phaseIndex) != 0U))
  {
    return 0U;
  }

  return (uint8_t) ((phaseRuntime->detectorActive != 0U)
                    || (PhaseSystemVehCallActive(engine, phaseIndex) != 0U)
                    || (RemoteManualVehicleCallActive(engine, phaseIndex) != 0U)
                    || (UnitControlExternalMinRecallActive(engine) != 0U)
                    || (PhaseUnitControlledNonActuatedActive(engine,
                                                             phaseIndex) != 0U)
                    || ((phaseRuntime->callLatched != 0U)
                        && ((phaseRuntime->interval
                             != INTERSECTION_PHASE_INTERVAL_GREEN)
                            || (engine->passageTicks[phaseIndex] == 0U)
                            || (engine->gapTimerTicks[phaseIndex] > 0U)))
                    || (PhaseHasVehicleRecall(phaseConfig) != 0U)
                    || (PhaseSplitModeIsRecall(split) != 0U));
}

static uint8_t PhaseHasDiagnosticServiceableCall(
  const IntersectionEngine_t *engine,
  uint8_t phaseIndex)
{
  const IntersectionPhaseRuntime_t *phaseRuntime;
  const IntersectionSplitPhaseConfig_t *split;
  uint8_t vehicleCallPending;
  uint8_t pedCallPending;

  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  phaseRuntime = &engine->runtime.phases[phaseIndex];
  split = GetActiveSplit(engine, phaseIndex);

  if ((PhaseSplitModeIsOmitted(split) != 0U)
      || (PhaseSystemPhaseOmitActive(engine, phaseIndex) != 0U))
  {
    return 0U;
  }

  vehicleCallPending = (uint8_t) ((phaseRuntime->interval
                                   != INTERSECTION_PHASE_INTERVAL_GREEN)
                                  && ((phaseRuntime->detectorActive != 0U)
                                      || (PhaseSystemVehCallActive(engine,
                                                                   phaseIndex)
                                          != 0U)
                                      || (RemoteManualVehicleCallActive(
                                            engine,
                                            phaseIndex)
                                          != 0U)
                                      || ((phaseRuntime->callLatched != 0U)
                                          && ((phaseRuntime->interval
                                               != INTERSECTION_PHASE_INTERVAL_GREEN)
                                              || (engine->passageTicks[phaseIndex]
                                                  == 0U)
                                              || (engine->gapTimerTicks[phaseIndex]
                                                  > 0U)))));

  pedCallPending = (uint8_t) ((PhasePedConfigured(engine, phaseIndex) != 0U)
                              && (PhaseSystemPedOmitActive(engine,
                                                           phaseIndex)
                                  == 0U)
                              && (phaseRuntime->pedInterval
                                  == INTERSECTION_PED_INTERVAL_DONT_WALK)
                              && ((phaseRuntime->pedInputActive != 0U)
                                  || (phaseRuntime->pedCallLatched != 0U)
                                  || (phaseRuntime->pedServicePending != 0U)
                                  || (PhaseSystemPedCallActive(engine,
                                                               phaseIndex)
                                      != 0U)
                                  || (RemoteManualPedCallActive(engine,
                                                                phaseIndex)
                                      != 0U)));

  return (uint8_t) ((vehicleCallPending != 0U) || (pedCallPending != 0U));
}

static uint8_t PhaseHasConflictingDemand(const IntersectionEngine_t *engine,
                                         uint8_t phaseIndex)
{
  uint8_t otherPhaseIndex;

  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  for (otherPhaseIndex = 0U; otherPhaseIndex < engine->config.phaseCount;
       otherPhaseIndex++)
  {
    if ((otherPhaseIndex == phaseIndex)
        || (IntersectionPhaseOptionsEnabled(
              engine->config.phases[otherPhaseIndex].phaseOptions) == 0U)
        || (PhasesMayRunConcurrently(engine, phaseIndex, otherPhaseIndex)
            != 0U))
    {
      continue;
    }

    if (PhaseHasBaseDemand(engine, otherPhaseIndex) != 0U)
    {
      return 1U;
    }
  }

  return 0U;
}

static uint8_t ConflictingRingsAreResting(const IntersectionEngine_t *engine,
                                          uint8_t phaseIndex)
{
  uint8_t ringIndex;

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    const IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[
      ringIndex];
    uint8_t activePhaseIndex = ringRuntime->activePhaseIndex;

    if ((activePhaseIndex >= engine->config.phaseCount)
        || (PhasesMayRunConcurrently(engine, phaseIndex, activePhaseIndex)
            != 0U))
    {
      continue;
    }

    if ((ringRuntime->statusCode != INTERSECTION_RING_STATUS_GREEN_REST)
        && (ringRuntime->statusCode != INTERSECTION_RING_STATUS_RED_REST))
    {
      return 0U;
    }
  }

  return 1U;
}

static uint8_t PhaseHasSoftRecallDemand(const IntersectionEngine_t *engine,
                                        uint8_t phaseIndex)
{
  const IntersectionPhaseConfig_t *phaseConfig =
    &engine->config.phases[phaseIndex];

  return (uint8_t) ((PhaseSystemPhaseOmitActive(engine, phaseIndex) == 0U)
                    && ((phaseConfig->phaseOptions & PHASE_OPTIONS_SOFT_RECALL)
                        != 0U)
                    && (PhaseHasConflictingDemand(engine, phaseIndex) == 0U)
                    && (ConflictingRingsAreResting(engine, phaseIndex) != 0U));
}

static uint8_t PhaseHasDemand(const IntersectionEngine_t *engine,
                              uint8_t phaseIndex)
{
  return (uint8_t) ((PhaseHasBaseDemand(engine, phaseIndex) != 0U)
                    || (PhaseHasSoftRecallDemand(engine, phaseIndex) != 0U));
}

static void RefreshPhaseDemandWaitTimes(IntersectionEngine_t *engine)
{
  uint8_t phaseIndex;

  if (engine == NULL)
  {
    return;
  }

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    if ((PhaseHasDemand(engine, phaseIndex) != 0U)
        && (engine->runtime.phases[phaseIndex].interval
            == INTERSECTION_PHASE_INTERVAL_RED))
    {
      if (engine->phaseDemandWaitTicks[phaseIndex] < UINT32_MAX)
      {
        engine->phaseDemandWaitTicks[phaseIndex]++;
      }
    }
    else
    {
      engine->phaseDemandWaitTicks[phaseIndex] = 0U;
    }
  }
}

static void ResetCoordinationCycleFaultDiagnostics(IntersectionEngine_t *engine)
{
  if (engine == NULL)
  {
    return;
  }

  memset(engine->coordCycleFaultAges, 0, sizeof(engine->coordCycleFaultAges));
  memset(engine->coordCycleServedThisCycle,
         0,
         sizeof(engine->coordCycleServedThisCycle));
  engine->coordDiagnosticFaultPhaseMask = 0U;
  engine->coordDiagnosticRecoveryCyclesRemaining = 0U;
  engine->coordDiagnosticRetryCyclesRemaining = 0U;
  engine->runtime.coordCycleFaultActive = 0U;
  engine->runtime.coordFaultActive = 0U;
  engine->runtime.coordFailActive = 0U;
  engine->runtime.cycleFailActive = 0U;
}

static void ResetCoordinationAlarmDiagnostics(IntersectionEngine_t *engine)
{
  if (engine == NULL)
  {
    return;
  }

  engine->coordinationAlarmPattern = 0U;
  engine->coordinationAlarmMissedCycles = 0U;
  engine->runtime.coordinationAlarmActive = 0U;
}

static void UpdateCoordinationAlarmDiagnostics(IntersectionEngine_t *engine,
                                               uint8_t calledPattern,
                                               uint8_t runningCalledPattern)
{
  if (engine == NULL)
  {
    return;
  }

  if (calledPattern == 0U)
  {
    ResetCoordinationAlarmDiagnostics(engine);

    return;
  }

  if (engine->coordinationAlarmPattern != calledPattern)
  {
    engine->coordinationAlarmPattern = calledPattern;
    engine->coordinationAlarmMissedCycles = 0U;
    engine->runtime.coordinationAlarmActive = 0U;
  }

  if (runningCalledPattern != 0U)
  {
    engine->coordinationAlarmMissedCycles = 0U;
    engine->runtime.coordinationAlarmActive = 0U;
  }
  else
  {
    if (engine->coordinationAlarmMissedCycles < UINT8_MAX)
    {
      engine->coordinationAlarmMissedCycles++;
    }

    if (engine->coordinationAlarmMissedCycles >= 3U)
    {
      engine->runtime.coordinationAlarmActive = 1U;
    }
  }
}

static void UpdateCoordinationCycleFaultDiagnostics(
  IntersectionEngine_t *engine,
  uint8_t coordinatedModeActive)
{
  uint8_t phaseIndex;
  uint8_t faultPhaseMask = 0U;
  uint8_t serviceableCallPresent = 0U;

  if (engine == NULL)
  {
    return;
  }

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    uint8_t phaseHasServiceableCall =
      PhaseHasDiagnosticServiceableCall(engine, phaseIndex);

    if (phaseHasServiceableCall != 0U)
    {
      serviceableCallPresent = 1U;
    }

    if ((phaseHasServiceableCall != 0U)
        && (engine->coordCycleServedThisCycle[phaseIndex] == 0U))
    {
      if (engine->coordCycleFaultAges[phaseIndex] < UINT8_MAX)
      {
        engine->coordCycleFaultAges[phaseIndex]++;
      }
    }
    else
    {
      engine->coordCycleFaultAges[phaseIndex] = 0U;
    }

    if ((engine->coordCycleFaultAges[phaseIndex] >= 2U)
        && (phaseIndex < 8U))
    {
      faultPhaseMask = (uint8_t) (faultPhaseMask | (uint8_t) (1U << phaseIndex));
    }

    engine->coordCycleServedThisCycle[phaseIndex] = 0U;
  }

  if (coordinatedModeActive != 0U)
  {
    if (faultPhaseMask != 0U)
    {
      if ((engine->runtime.coordFaultActive != 0U)
          && (engine->coordDiagnosticRetryCyclesRemaining > 0U))
      {
        engine->runtime.coordFailActive = 1U;
        engine->runtime.coordFaultActive = 0U;
      }
      else
      {
        engine->runtime.coordFailActive = 0U;
      }

      engine->runtime.coordCycleFaultActive = 1U;
      engine->runtime.cycleFailActive = 0U;
      engine->coordDiagnosticFaultPhaseMask = faultPhaseMask;
      engine->coordDiagnosticRecoveryCyclesRemaining = 2U;
      engine->coordDiagnosticRetryCyclesRemaining = 0U;
    }
    else if (engine->runtime.coordFaultActive != 0U)
    {
      if ((serviceableCallPresent == 0U)
          && (engine->coordDiagnosticRetryCyclesRemaining > 0U))
      {
        engine->coordDiagnosticRetryCyclesRemaining--;
      }

      if (engine->coordDiagnosticRetryCyclesRemaining == 0U)
      {
        engine->runtime.coordFaultActive = 0U;
      }
    }
    else if (engine->runtime.coordCycleFaultActive == 0U)
    {
      engine->runtime.coordFailActive = 0U;
    }
  }
  else
  {
    if (faultPhaseMask != 0U)
    {
      engine->coordDiagnosticFaultPhaseMask = (uint8_t) (
        engine->coordDiagnosticFaultPhaseMask | faultPhaseMask);

      if (engine->runtime.coordCycleFaultActive != 0U)
      {
        if (engine->coordDiagnosticRecoveryCyclesRemaining > 0U)
        {
          engine->coordDiagnosticRecoveryCyclesRemaining--;
        }

        if (engine->coordDiagnosticRecoveryCyclesRemaining == 0U)
        {
          engine->runtime.cycleFailActive = 1U;
        }
      }
      else
      {
        engine->runtime.cycleFailActive = 1U;
      }
    }
    else if ((engine->runtime.coordCycleFaultActive == 0U)
             && (engine->runtime.coordFailActive == 0U))
    {
      engine->runtime.cycleFailActive = 0U;
    }
  }
}

static uint8_t PhaseMaxTimingActive(const IntersectionEngine_t *engine,
                                    uint8_t phaseIndex)
{
  const IntersectionPhaseConfig_t *phaseConfig =
    &engine->config.phases[phaseIndex];
  const IntersectionSplitPhaseConfig_t *split = GetActiveSplit(engine,
                                                               phaseIndex);

  return (uint8_t) ((PhaseHasConflictingDemand(engine, phaseIndex) != 0U)
                    || (PhaseUnitControlledNonActuatedActive(engine,
                                                             phaseIndex) != 0U)
                    || (PhaseHasMaxRecall(phaseConfig) != 0U)
                    || (PhaseSplitModeIsMaxRecall(split) != 0U));
}

static uint8_t PhaseRestInWalkActive(const IntersectionEngine_t *engine,
                                     uint8_t phaseIndex)
{
  const IntersectionPhaseConfig_t *phaseConfig =
    &engine->config.phases[phaseIndex];

  return (uint8_t) ((((phaseConfig->phaseOptions & PHASE_OPTIONS_REST_IN_WALK)
                      != 0U)
                     || ((UnitControlWalkRestModifierActive(engine) != 0U)
                         && (PhaseUnitControlledNonActuatedActive(engine,
                                                                  phaseIndex)
                             != 0U)))
                    && (PhaseHasConflictingDemand(engine, phaseIndex) == 0U));
}

static uint8_t PhaseGuaranteedPassageActive(const IntersectionEngine_t *engine,
                                            uint8_t phaseIndex)
{
  const IntersectionPhaseConfig_t *phaseConfig =
    &engine->config.phases[phaseIndex];

  return (uint8_t) (((phaseConfig->phaseOptions & PHASE_OPTIONS_GUARANTEED_PASS)
                     != 0U)
                    && (engine->currentGapTicks[phaseIndex]
                        < engine->passageTicks[phaseIndex])
                    && (engine->gapTimerTicks[phaseIndex] == 0U)
                    && (engine->passageTimerTicks[phaseIndex] > 0U));
}

static uint16_t PhaseInitialGreenTicks(const IntersectionEngine_t *engine,
                                       uint8_t phaseIndex)
{
  uint32_t detectorCount = 0U;
  uint8_t detectorIndex;

  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    const IntersectionVehicleDetectorConfig_t *detector =
      &engine->config.vehicleDetectors[detectorIndex];
    uint16_t addedCount = engine->runtime.vehicleDetectors[detectorIndex].
                          addedInitialCount;

    if ((detector->callPhase != (uint8_t) (phaseIndex + 1U))
        || ((detector->options & VEHICLE_DETECTOR_OPTIONS_ADDED_INITIAL) == 0U))
    {
      continue;
    }

    if ((engine->config.phases[phaseIndex].phaseOptions
         & PHASE_OPTIONS_ADDED_INIT_CALC) != 0U)
    {
      if (addedCount > detectorCount)
      {
        detectorCount = addedCount;
      }
    }
    else
    {
      detectorCount += addedCount;
    }
  }

  if ((detectorCount == 0U) && (engine->initialActuationCount[phaseIndex] != 0U))
  {
    detectorCount = engine->initialActuationCount[phaseIndex];
  }

  {
    uint32_t initialTicks = (uint32_t) engine->config.phases[phaseIndex].
                            addedInitialDs
                            * detectorCount * 10U;

    if (initialTicks > engine->maxInitialTicks[phaseIndex])
    {
      initialTicks = engine->maxInitialTicks[phaseIndex];
    }

    if (initialTicks < engine->minGreenTicks[phaseIndex])
    {
      initialTicks = engine->minGreenTicks[phaseIndex];
    }

    return (uint16_t) initialTicks;
  }
}

static uint32_t PhaseMinimumServiceTicks(const IntersectionEngine_t *engine,
                                         uint8_t phaseIndex)
{
  uint32_t serviceTicks;

  serviceTicks = (uint32_t) PhaseInitialGreenTicks(engine, phaseIndex)
                 + (uint32_t) engine->yellowTicks[phaseIndex]
                 + (uint32_t) engine->redClearTicks[phaseIndex];

  if (PhasePedConfigured(engine, phaseIndex) != 0U)
  {
    serviceTicks += (uint32_t) PhaseWalkTicks(engine, phaseIndex)
                    + (uint32_t) PhasePedClearTicks(engine, phaseIndex);
  }

  return serviceTicks;
}

static uint8_t PhaseShowsGreen(const IntersectionEngine_t *engine,
                               uint8_t phaseIndex)
{
  return (uint8_t) ((engine != NULL)
                    && (phaseIndex < engine->config.phaseCount)
                    && (engine->runtime.phases[phaseIndex].interval
                        == INTERSECTION_PHASE_INTERVAL_GREEN));
}

static uint8_t PhaseShowsYellow(const IntersectionEngine_t *engine,
                                uint8_t phaseIndex)
{
  return (uint8_t) ((engine != NULL)
                    && (phaseIndex < engine->config.phaseCount)
                    && (engine->runtime.phases[phaseIndex].interval
                        == INTERSECTION_PHASE_INTERVAL_YELLOW));
}

static uint8_t VehicleDetectorQueueWindowOpen(const IntersectionEngine_t *engine,
                                              uint8_t phaseIndex,
                                              const IntersectionVehicleDetectorConfig_t *
                                              detectorConfig)
{
  const IntersectionRingRuntime_t *ringRuntime;
  uint8_t ringIndex;

  if ((engine == NULL) || (detectorConfig == NULL)
      || ((detectorConfig->options & VEHICLE_DETECTOR_OPTIONS_QUEUE) == 0U))
  {
    return 0U;
  }

  if (detectorConfig->queueLimitSeconds == 0U)
  {
    return 0U;
  }

  ringIndex = engine->config.phases[phaseIndex].ring;
  ringRuntime = &engine->runtime.rings[ringIndex];

  return (uint8_t) ((ringRuntime->activePhaseIndex == phaseIndex)
                    && (ringRuntime->stage == INTERSECTION_RING_STAGE_GREEN)
                    && (ringRuntime->stageElapsedTicks
                        < ((uint32_t) detectorConfig->queueLimitSeconds
                           * 100U)));
}

static uint8_t VehicleDetectorCallTargetPhase(
  const IntersectionEngine_t *engine,
  const IntersectionVehicleDetectorConfig_t *detectorConfig,
  uint8_t *phaseIndex)
{
  uint8_t callPhaseIndex;

  if ((engine == NULL) || (detectorConfig == NULL) || (phaseIndex == NULL)
      || (detectorConfig->callPhase == 0U)
      || (detectorConfig->callPhase > engine->config.phaseCount))
  {
    return 0U;
  }

  callPhaseIndex = (uint8_t) (detectorConfig->callPhase - 1U);
  *phaseIndex = callPhaseIndex;

  if ((detectorConfig->switchPhase != 0U)
      && (detectorConfig->switchPhase <= engine->config.phaseCount)
      && (PhaseShowsGreen(engine, callPhaseIndex) == 0U)
      && (PhaseShowsGreen(engine,
                          (uint8_t) (detectorConfig->switchPhase - 1U)) != 0U))
  {
    *phaseIndex = (uint8_t) (detectorConfig->switchPhase - 1U);
  }

  return 1U;
}

static void RefreshVehicleDetectorDerivedInputs(IntersectionEngine_t *engine)
{
  uint8_t detectorIndex;

  if (engine == NULL)
  {
    return;
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    IntersectionVehicleDetectorRuntime_t *detectorRuntime =
      &engine->runtime.vehicleDetectors[detectorIndex];
    const IntersectionVehicleDetectorConfig_t *detectorConfig =
      &engine->config.vehicleDetectors[detectorIndex];
    uint8_t rawActive = (uint8_t) ((detectorRuntime->inputActive != 0U)
                                   || (detectorRuntime->remoteActuation != 0U));
    uint8_t targetPhaseIndex = 0xFFU;
    uint8_t callPhaseIndex = 0xFFU;
    uint8_t recognizedActive = 0U;
    uint8_t activeGreenTarget = 0U;

    if ((detectorConfig->callPhase != 0U)
        && (detectorConfig->callPhase <= engine->config.phaseCount))
    {
      callPhaseIndex = (uint8_t) (detectorConfig->callPhase - 1U);
      (void) VehicleDetectorCallTargetPhase(engine,
                                            detectorConfig,
                                            &targetPhaseIndex);
      activeGreenTarget = (uint8_t) ((targetPhaseIndex < engine->config.phaseCount)
                                     && (PhaseShowsGreen(engine,
                                                         targetPhaseIndex)
                                         != 0U));
    }

    if (rawActive != 0U)
    {
      if ((callPhaseIndex < engine->config.phaseCount)
          && (PhaseShowsGreen(engine, callPhaseIndex) == 0U)
          && (detectorConfig->delayDs != 0U))
      {
        uint16_t requiredTicks = (uint16_t) ((uint32_t) detectorConfig->delayDs
                                             * 10U);

        if (detectorRuntime->delayTimerTicks < requiredTicks)
        {
          detectorRuntime->delayTimerTicks++;
        }

        recognizedActive = (uint8_t) (detectorRuntime->delayTimerTicks
                                      >= requiredTicks);
      }
      else
      {
        detectorRuntime->delayTimerTicks = 0U;
        recognizedActive = 1U;
      }
    }
    else
    {
      detectorRuntime->delayTimerTicks = 0U;
    }

    if ((rawActive != 0U)
        && (detectorRuntime->previousPresenceActive == 0U)
        && (callPhaseIndex < engine->config.phaseCount)
        && (PhaseShowsGreen(engine, callPhaseIndex) == 0U)
        && ((detectorConfig->options & VEHICLE_DETECTOR_OPTIONS_ADDED_INITIAL)
            != 0U)
        && (detectorRuntime->addedInitialCount < 0xFFFFU))
    {
      detectorRuntime->addedInitialCount++;
    }

    if ((activeGreenTarget != 0U) && (rawActive == 0U)
        && ((detectorConfig->options & VEHICLE_DETECTOR_OPTIONS_PASSAGE) != 0U)
        && (detectorRuntime->previousPresenceActive != 0U)
        && (detectorConfig->extendDs != 0U))
    {
      detectorRuntime->extendTimerTicks =
        (uint16_t) ((uint32_t) detectorConfig->extendDs * 10U);
    }

    if ((activeGreenTarget != 0U) && (targetPhaseIndex < engine->config.phaseCount))
    {
      uint8_t queueOpen = VehicleDetectorQueueWindowOpen(engine,
                                                         targetPhaseIndex,
                                                         detectorConfig);

      if ((rawActive != 0U)
          && (((detectorConfig->options & VEHICLE_DETECTOR_OPTIONS_PASSAGE)
               != 0U)
              || queueOpen != 0U))
      {
        engine->runtime.phases[targetPhaseIndex].detectorActive = 1U;
      }
      else if ((detectorRuntime->extendTimerTicks > 0U)
               && ((detectorConfig->options & VEHICLE_DETECTOR_OPTIONS_PASSAGE)
                   != 0U)
               && (targetPhaseIndex == callPhaseIndex))
      {
        detectorRuntime->extendTimerTicks--;
        engine->runtime.phases[targetPhaseIndex].detectorActive = 1U;
      }
      else
      {
        detectorRuntime->extendTimerTicks = 0U;
      }
    }
    else
    {
      detectorRuntime->extendTimerTicks = 0U;
    }

    if ((recognizedActive != 0U) && (callPhaseIndex < engine->config.phaseCount)
        && ((detectorConfig->options & VEHICLE_DETECTOR_OPTIONS_CALL) != 0U)
        && ((targetPhaseIndex >= engine->config.phaseCount)
            || (PhaseShowsGreen(engine, targetPhaseIndex) == 0U)))
    {
      const IntersectionPhaseConfig_t *phaseConfig =
        &engine->config.phases[callPhaseIndex];
      IntersectionPhaseRuntime_t *phaseRuntime =
        &engine->runtime.phases[callPhaseIndex];

      phaseRuntime->detectorActive = 1U;

      if ((phaseConfig->phaseOptions & PHASE_OPTIONS_NON_LOCK_DET_MEM) == 0U)
      {
        phaseRuntime->callLatched = 1U;
      }
      else if (((detectorConfig->options & VEHICLE_DETECTOR_OPTIONS_YELLOW_LOCK)
                != 0U)
               || (((detectorConfig->options
                     & VEHICLE_DETECTOR_OPTIONS_RED_LOCK) != 0U)
                   && (PhaseShowsYellow(engine, callPhaseIndex) == 0U)))
      {
        phaseRuntime->callLatched = 1U;
      }
    }

    detectorRuntime->recognitionActive = recognizedActive;
    detectorRuntime->previousPresenceActive = rawActive;
  }
}

static void RefreshPedestrianDetectorDerivedInputs(IntersectionEngine_t *engine)
{
  uint8_t detectorIndex;

  if (engine == NULL)
  {
    return;
  }

  for (detectorIndex = 0U; detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       detectorIndex++)
  {
    IntersectionPedestrianDetectorRuntime_t *detectorRuntime =
      &engine->runtime.pedestrianDetectors[detectorIndex];
    const IntersectionPedestrianDetectorConfig_t *detectorConfig =
      &engine->config.pedestrianDetectors[detectorIndex];
    uint8_t active = (uint8_t) ((detectorRuntime->inputActive != 0U)
                                || (detectorRuntime->remoteActuation != 0U));
    uint8_t phaseIndex;
    uint8_t alternateRequested = 0U;

    if ((detectorConfig->callPhase == 0U)
        || (detectorConfig->callPhase > engine->config.phaseCount))
    {
      detectorRuntime->apsElapsedTicks = 0U;
      detectorRuntime->alternateTimingRequest = 0U;
      continue;
    }

    phaseIndex = (uint8_t) (detectorConfig->callPhase - 1U);

    if (active != 0U)
    {
      if ((detectorConfig->apsMinimumActuationDs != 0U)
          && (detectorRuntime->apsElapsedTicks
              < (uint16_t) ((uint32_t) detectorConfig->apsMinimumActuationDs
                            * 10U)))
      {
        detectorRuntime->apsElapsedTicks++;
      }

      if (((detectorConfig->options & PED_DETECTOR_OPTIONS_ALT_TIMING) != 0U)
          && (detectorConfig->apsMinimumActuationDs != 0U)
          && (detectorRuntime->apsElapsedTicks
              >= (uint16_t) ((uint32_t) detectorConfig->apsMinimumActuationDs
                             * 10U)))
      {
        alternateRequested = 1U;
      }
    }
    else
    {
      detectorRuntime->apsElapsedTicks = 0U;
    }

    detectorRuntime->alternateTimingRequest = alternateRequested;

    if ((detectorConfig->options & PED_DETECTOR_OPTIONS_PRESENCE) != 0U)
    {
      continue;
    }

    engine->runtime.phases[phaseIndex].pedInputActive |= active;

    if (alternateRequested != 0U)
    {
      engine->runtime.phases[phaseIndex].pedAlternateTimingPending = 1U;
    }

    if ((detectorConfig->options & PED_DETECTOR_OPTIONS_NON_LOCKING) == 0U)
    {
      if (active != 0U)
      {
        engine->runtime.phases[phaseIndex].pedCallLatched = 1U;
        engine->runtime.phases[phaseIndex].pedServicePending = 1U;
      }
    }
  }
}

static void RefreshDetectorDerivedInputs(IntersectionEngine_t *engine)
{
  uint8_t phaseIndex;

  if (engine == NULL)
  {
    return;
  }

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    engine->runtime.phases[phaseIndex].detectorActive =
      engine->directVehicleInputs[phaseIndex];
    engine->runtime.phases[phaseIndex].pedInputActive =
      engine->directPedInputs[phaseIndex];
  }

  RefreshVehicleDetectorDerivedInputs(engine);
  RefreshPedestrianDetectorDerivedInputs(engine);
}

static uint8_t RingHasDemandInBarrierGroup(const IntersectionEngine_t *engine,
                                           uint8_t ringIndex,
                                           uint8_t barrierGroup)
{
  const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];
  uint8_t position;

  for (position = 0U; position < ringPlan->phaseCount; position++)
  {
    if (BarrierGroupForPosition(ringPlan, position) != barrierGroup)
    {
      continue;
    }

    if (PhaseHasDemand(engine, ringPlan->phaseOrder[position]) != 0U)
    {
      return 1U;
    }
  }

  return 0U;
}

static uint8_t FindBarrierCrossRequestForRing(const IntersectionEngine_t *engine,
                                              uint8_t ringIndex,
                                              uint8_t *targetBarrierGroup,
                                              uint8_t *partnerPhaseIndex)
{
  uint8_t otherRingIndex;

  for (otherRingIndex = 0U; otherRingIndex < engine->config.ringCount;
       otherRingIndex++)
  {
    const IntersectionRingPlan_t *otherPlan;
    const IntersectionRingRuntime_t *otherRuntime;
    uint8_t pendingPosition;

    if (otherRingIndex == ringIndex)
    {
      continue;
    }

    otherPlan = &engine->config.rings[otherRingIndex];
    otherRuntime = &engine->runtime.rings[otherRingIndex];
    pendingPosition = otherRuntime->pendingPosition;

    if (otherRuntime->stage != INTERSECTION_RING_STAGE_WAIT_BARRIER)
    {
      pendingPosition = FindNextDemandPosition(engine,
                                               otherRingIndex,
                                               otherRuntime->activePosition);
    }

    if ((pendingPosition >= otherPlan->phaseCount)
        || (pendingPosition == otherRuntime->activePosition)
        || (IsBarrierCrossing(engine,
                              otherRingIndex,
                              otherRuntime->activePosition,
                              pendingPosition) == 0U))
    {
      continue;
    }

    if (targetBarrierGroup != NULL)
    {
      *targetBarrierGroup = BarrierGroupForPosition(otherPlan, pendingPosition);
    }

    if (partnerPhaseIndex != NULL)
    {
      *partnerPhaseIndex = otherPlan->phaseOrder[pendingPosition];
    }

    return 1U;
  }

  return 0U;
}

static uint8_t FindDualEntryPositionForBarrierGroup(
  const IntersectionEngine_t *engine,
  uint8_t ringIndex,
  uint8_t barrierGroup,
  uint8_t partnerPhaseIndex,
  uint8_t *dualEntryPosition)
{
  const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];
  uint8_t position;

  for (position = 0U; position < ringPlan->phaseCount; position++)
  {
    uint8_t phaseIndex = ringPlan->phaseOrder[position];
    const IntersectionPhaseConfig_t *phaseConfig = &engine->config.phases[
      phaseIndex];

    if ((BarrierGroupForPosition(ringPlan, position) != barrierGroup)
        || ((phaseConfig->phaseOptions & PHASE_OPTIONS_DUAL_ENTRY) == 0U)
        || (PhasesMayRunConcurrently(engine,
                                     phaseIndex,
                                     partnerPhaseIndex) == 0U))
    {
      continue;
    }

    if (dualEntryPosition != NULL)
    {
      *dualEntryPosition = position;
    }

    return 1U;
  }

  return 0U;
}

static uint8_t FindAutomaticFlashPhasePosition(
  const IntersectionEngine_t *engine,
  uint8_t ringIndex,
  uint16_t optionBit,
  uint8_t *position)
{
  const IntersectionRingPlan_t *ringPlan;
  uint8_t ringPosition;

  if ((engine == NULL) || (ringIndex >= engine->config.ringCount))
  {
    return 0U;
  }

  ringPlan = &engine->config.rings[ringIndex];

  for (ringPosition = 0U; ringPosition < ringPlan->phaseCount; ringPosition++)
  {
    uint8_t phaseIndex = ringPlan->phaseOrder[ringPosition];

    if ((engine->config.phases[phaseIndex].phaseOptions & optionBit) == 0U)
    {
      continue;
    }

    if (position != NULL)
    {
      *position = ringPosition;
    }

    return 1U;
  }

  return 0U;
}

static void RefreshAutomaticFlashPhasePositions(IntersectionEngine_t *engine)
{
  uint8_t ringIndex;

  if (engine == NULL)
  {
    return;
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
  {
    engine->automaticFlashEntryPositions[ringIndex] = 0xFFU;
    engine->automaticFlashExitPositions[ringIndex] = 0xFFU;
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    (void) FindAutomaticFlashPhasePosition(engine,
                                           ringIndex,
                                           PHASE_OPTIONS_AUTO_FLASH_ENTRY,
                                           &engine->automaticFlashEntryPositions
                                           [ringIndex]);
    (void) FindAutomaticFlashPhasePosition(engine,
                                           ringIndex,
                                           PHASE_OPTIONS_AUTO_FLASH_EXIT,
                                           &engine->automaticFlashExitPositions
                                           [ringIndex]);
  }
}

static uint8_t RingAutomaticFlashEntryPosition(const IntersectionEngine_t *engine,
                                               uint8_t ringIndex,
                                               uint8_t *position)
{
  if ((AutomaticFlashRingPositionConfigured(engine,
                                            engine->automaticFlashEntryPositions,
                                            ringIndex) == 0U))
  {
    return 0U;
  }

  if (position != NULL)
  {
    *position = engine->automaticFlashEntryPositions[ringIndex];
  }

  return 1U;
}

static uint8_t ResolveRequestedPosition(const IntersectionEngine_t *engine,
                                        uint8_t ringIndex,
                                        uint8_t currentPosition)
{
  uint8_t automaticFlashPosition;

  if ((engine != NULL)
      && (engine->automaticFlashState
          == (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_ENTRY)
      && (RingAutomaticFlashEntryPosition(engine,
                                          ringIndex,
                                          &automaticFlashPosition) != 0U))
  {
    if (currentPosition == automaticFlashPosition)
    {
      return currentPosition;
    }

    return automaticFlashPosition;
  }

  uint8_t requestedPosition = FindNextDemandPosition(engine,
                                                     ringIndex,
                                                     currentPosition);

  if (requestedPosition != currentPosition)
  {
    return requestedPosition;
  }

  {
    uint8_t targetBarrierGroup;
    uint8_t partnerPhaseIndex;
    uint8_t dualEntryPosition;

    if ((FindBarrierCrossRequestForRing(engine,
                                        ringIndex,
                                        &targetBarrierGroup,
                                        &partnerPhaseIndex) != 0U)
        && (RingHasDemandInBarrierGroup(engine,
                                        ringIndex,
                                        targetBarrierGroup) == 0U)
        && (FindDualEntryPositionForBarrierGroup(engine,
                                                 ringIndex,
                                                 targetBarrierGroup,
                                                 partnerPhaseIndex,
                                                 &dualEntryPosition) != 0U))
    {
      return dualEntryPosition;
    }
  }

  return currentPosition;
}

static uint8_t FindFirstDemandPosition(const IntersectionEngine_t *engine,
                                       uint8_t ringIndex)
{
  const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];
  uint8_t position;

  for (position = 0U; position < ringPlan->phaseCount; position++)
  {
    uint8_t phaseIndex = ringPlan->phaseOrder[position];

    if (PhaseHasDemand(engine, phaseIndex) != 0U)
    {
      return position;
    }
  }

  return 0U;
}

static uint8_t RingHasDemand(const IntersectionEngine_t *engine,
                             uint8_t ringIndex)
{
  const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];
  uint8_t position;

  for (position = 0U; position < ringPlan->phaseCount; position++)
  {
    uint8_t phaseIndex = ringPlan->phaseOrder[position];

    if (PhaseHasDemand(engine, phaseIndex) != 0U)
    {
      return 1U;
    }
  }

  return 0U;
}

static uint8_t FindNextDemandPosition(const IntersectionEngine_t *engine,
                                      uint8_t ringIndex,
                                      uint8_t currentPosition)
{
  const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];
  uint8_t offset;

  for (offset = 1U; offset <= ringPlan->phaseCount; offset++)
  {
    uint8_t position = (uint8_t) ((currentPosition + offset)
                                  % ringPlan->phaseCount);
    uint8_t phaseIndex = ringPlan->phaseOrder[position];

    if (PhaseHasDemand(engine, phaseIndex) != 0U)
    {
      return position;
    }
  }

  return currentPosition;
}

static uint8_t BarrierGroupForPosition(const IntersectionRingPlan_t *ringPlan,
                                       uint8_t position)
{
  return (uint8_t) ((position < ringPlan->barrierPhaseCount) ? 0U : 1U);
}

static uint8_t IsBarrierCrossing(const IntersectionEngine_t *engine,
                                 uint8_t ringIndex,
                                 uint8_t fromPosition,
                                 uint8_t toPosition)
{
  const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];

  return (uint8_t) (BarrierGroupForPosition(ringPlan, fromPosition)
                    != BarrierGroupForPosition(ringPlan, toPosition));
}

static uint8_t AspectIsRed(IntersectionOutputAspect_t aspect)
{
  return (uint8_t) ((aspect == INTERSECTION_OUTPUT_ASPECT_RED)
                    || (aspect == INTERSECTION_OUTPUT_ASPECT_FLASH_RED));
}

static uint8_t AspectIsYellow(IntersectionOutputAspect_t aspect)
{
  return (uint8_t) ((aspect == INTERSECTION_OUTPUT_ASPECT_YELLOW)
                    || (aspect == INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW));
}

static uint8_t AspectIsGreen(IntersectionOutputAspect_t aspect)
{
  return (uint8_t) (aspect == INTERSECTION_OUTPUT_ASPECT_GREEN);
}

static uint8_t GroupBitMask(uint8_t zeroBasedIndex)
{
  return (uint8_t) (1U << (zeroBasedIndex % 8U));
}

static IntersectionOutputAspect_t PhaseIntervalToAspect(
  IntersectionPhaseInterval_t interval)
{
  switch (interval)
  {
      case INTERSECTION_PHASE_INTERVAL_GREEN:
      {
        return INTERSECTION_OUTPUT_ASPECT_GREEN;
      }

      case INTERSECTION_PHASE_INTERVAL_YELLOW:
      {
        return INTERSECTION_OUTPUT_ASPECT_YELLOW;
      }

      case INTERSECTION_PHASE_INTERVAL_RED:
      case INTERSECTION_PHASE_INTERVAL_RED_CLEAR:
      default:
      {
        return INTERSECTION_OUTPUT_ASPECT_RED;
      }
  }
}

static IntersectionOutputAspect_t PedIntervalToAspect(
  IntersectionPedInterval_t pedInterval)
{
  switch (pedInterval)
  {
      case INTERSECTION_PED_INTERVAL_WALK:
      {
        return INTERSECTION_OUTPUT_ASPECT_GREEN;
      }

      case INTERSECTION_PED_INTERVAL_CLEAR:
      {
        return INTERSECTION_OUTPUT_ASPECT_YELLOW;
      }

      case INTERSECTION_PED_INTERVAL_DONT_WALK:
      default:
      {
        return INTERSECTION_OUTPUT_ASPECT_RED;
      }
  }
}

static void ResetPhasePedState(IntersectionEngine_t *engine, uint8_t phaseIndex)
{
  engine->runtime.phases[phaseIndex].pedInterval =
    INTERSECTION_PED_INTERVAL_DONT_WALK;
  engine->runtime.phases[phaseIndex].pedIntervalElapsedTicks = 0U;
  engine->runtime.phases[phaseIndex].pedServicePending = 0U;
  engine->runtime.phases[phaseIndex].pedServiceActive = 0U;
  engine->runtime.phases[phaseIndex].pedAlternateTimingPending = 0U;
  engine->runtime.phases[phaseIndex].pedAlternateTimingActive = 0U;
  engine->pedClearClearanceTicks[phaseIndex] = 0U;
}

static void EndPhasePedService(IntersectionEngine_t *engine, uint8_t phaseIndex)
{
  if ((engine != NULL)
      && (engine->runtime.phases[phaseIndex].pedInterval
          != INTERSECTION_PED_INTERVAL_DONT_WALK))
  {
    StartPhaseDontWalkRevertTimer(engine, phaseIndex);
  }

  ResetPhasePedState(engine, phaseIndex);
}

static void StartPhasePedWalk(IntersectionEngine_t *engine, uint8_t phaseIndex)
{
  IntersectionPhaseRuntime_t *phaseRuntime =
    &engine->runtime.phases[phaseIndex];

  phaseRuntime->pedInterval = INTERSECTION_PED_INTERVAL_WALK;
  phaseRuntime->pedIntervalElapsedTicks = 0U;
  phaseRuntime->pedServicePending = 0U;
  phaseRuntime->pedServiceActive = 1U;
  phaseRuntime->pedCallLatched = 0U;
  phaseRuntime->pedAlternateTimingActive =
    phaseRuntime->pedAlternateTimingPending;
  phaseRuntime->pedAlternateTimingPending = 0U;

  if ((engine->runtime.mode == INTERSECTION_CONTROL_MODE_COORDINATED)
      && (engine->pedWalkServicesThisCycle[phaseIndex] < UINT8_MAX))
  {
    engine->pedWalkServicesThisCycle[phaseIndex]++;
  }
}

static void StartPhasePedClear(IntersectionEngine_t *engine, uint8_t phaseIndex)
{
  IntersectionPhaseRuntime_t *phaseRuntime =
    &engine->runtime.phases[phaseIndex];

  phaseRuntime->pedInterval = INTERSECTION_PED_INTERVAL_CLEAR;
  phaseRuntime->pedIntervalElapsedTicks = 0U;
  phaseRuntime->pedServicePending = 0U;
  phaseRuntime->pedServiceActive = 1U;
  engine->pedClearClearanceTicks[phaseIndex] = 0U;
}

static void TickPhasePedState(IntersectionEngine_t *engine, uint8_t phaseIndex)
{
  IntersectionPhaseRuntime_t *phaseRuntime =
    &engine->runtime.phases[phaseIndex];

  if (PhasePedConfigured(engine, phaseIndex) == 0U)
  {
    ResetPhasePedState(engine, phaseIndex);

    return;
  }

  if ((PhaseSystemPedOmitActive(engine, phaseIndex) != 0U)
      && (phaseRuntime->pedInterval == INTERSECTION_PED_INTERVAL_DONT_WALK))
  {
    phaseRuntime->pedServicePending = 0U;
  }

  if (((phaseRuntime->pedInputActive != 0U)
       || (phaseRuntime->pedCallLatched != 0U)
       || (PhaseSystemPedCallActive(engine, phaseIndex) != 0U)
       || (RemoteManualPedCallActive(engine, phaseIndex) != 0U))
      && (PhaseSystemPedOmitActive(engine, phaseIndex) == 0U)
      && (phaseRuntime->pedInterval == INTERSECTION_PED_INTERVAL_DONT_WALK)
      && (phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_GREEN))
  {
    phaseRuntime->pedServicePending = 1U;
  }

  if ((phaseRuntime->pedInterval == INTERSECTION_PED_INTERVAL_DONT_WALK)
      && (phaseRuntime->pedServicePending != 0U)
      && (phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_GREEN)
      && (PhaseDontWalkRevertActive(engine, phaseIndex) == 0U)
      && (PhasePedWalkServiceAvailable(engine, phaseIndex) != 0U)
      && (phaseRuntime->intervalElapsedTicks >= PhasePedStartDelayTicks(
            engine,
            phaseIndex)))
  {
    StartPhasePedWalk(engine, phaseIndex);
  }

  switch (phaseRuntime->pedInterval)
  {
      case INTERSECTION_PED_INTERVAL_WALK:
      {
        if (RemoteManualControlActive(engine) != 0U)
        {
          break;
        }

        phaseRuntime->pedIntervalElapsedTicks++;

        if (phaseRuntime->pedIntervalElapsedTicks >= PhaseCurrentWalkTicks(
              engine,
              phaseIndex))
        {
          if (PhaseRestInWalkActive(engine, phaseIndex) != 0U)
          {
            phaseRuntime->pedIntervalElapsedTicks = PhaseCurrentWalkTicks(
              engine,
              phaseIndex);
          }
          else if (PhaseCurrentPedClearTicks(engine, phaseIndex) == 0U)
          {
            EndPhasePedService(engine, phaseIndex);
          }
          else
          {
            StartPhasePedClear(engine, phaseIndex);
          }
        }

        break;
      }

      case INTERSECTION_PED_INTERVAL_CLEAR:
      {
        if ((phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_YELLOW)
            || (phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_RED_CLEAR))
        {
          uint16_t clearanceTicks = PhasePedClearVehicleClearanceTicks(
            engine,
            phaseIndex);

          if (clearanceTicks == 0U)
          {
            EndPhasePedService(engine, phaseIndex);

            break;
          }

          if (engine->pedClearClearanceTicks[phaseIndex] < UINT16_MAX)
          {
            engine->pedClearClearanceTicks[phaseIndex]++;
          }

          if (engine->pedClearClearanceTicks[phaseIndex] >= clearanceTicks)
          {
            EndPhasePedService(engine, phaseIndex);

            break;
          }
        }

        if ((RemoteManualControlActive(engine) != 0U)
            && (engine->config.unit.autoPedestrianClear
                != (uint8_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_ENABLE))
        {
          break;
        }

        phaseRuntime->pedIntervalElapsedTicks++;

        if (phaseRuntime->pedIntervalElapsedTicks >= PhaseCurrentPedClearTicks(
              engine,
              phaseIndex))
        {
          EndPhasePedService(engine, phaseIndex);
        }

        break;
      }

      case INTERSECTION_PED_INTERVAL_DONT_WALK:
      default:
      {
        break;
      }
  }
} /* TickPhasePedState */

static void SetRingPhasesRed(IntersectionEngine_t *engine, uint8_t ringIndex)
{
  const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];
  uint8_t position;

  for (position = 0U; position < ringPlan->phaseCount; position++)
  {
    uint8_t phaseIndex = ringPlan->phaseOrder[position];

    engine->runtime.phases[phaseIndex].interval =
      INTERSECTION_PHASE_INTERVAL_RED;
    engine->runtime.phases[phaseIndex].intervalElapsedTicks = 0U;
    EndPhasePedService(engine, phaseIndex);
  }
}

static void SetRingRedRestStage(IntersectionEngine_t *engine,
                                uint8_t ringIndex,
                                uint8_t position)
{
  IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];
  uint8_t phaseIndex = engine->config.rings[ringIndex].phaseOrder[position];

  SetRingPhasesRed(engine, ringIndex);
  ringRuntime->activePosition = position;
  ringRuntime->pendingPosition = position;
  ringRuntime->activePhaseIndex = phaseIndex;
  ringRuntime->barrierWaiting = 0U;
  ringRuntime->stage = INTERSECTION_RING_STAGE_RED_REST;
  ringRuntime->statusCode = INTERSECTION_RING_STATUS_RED_REST;
  ringRuntime->terminationReasonBits = INTERSECTION_RING_TERMINATION_NONE;
  ringRuntime->stageElapsedTicks = 0U;
}

static uint8_t AutomaticFlashEntryCompleted(const IntersectionEngine_t *engine)
{
  uint8_t ringIndex;

  if ((engine == NULL) || (AutomaticFlashTransitionConfigured(engine) == 0U))
  {
    return 0U;
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    const IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[
      ringIndex];

    if ((ringRuntime->stage != INTERSECTION_RING_STAGE_RED_REST)
        || (ringRuntime->activePosition
            != engine->automaticFlashEntryPositions[ringIndex]))
    {
      return 0U;
    }
  }

  return 1U;
}

static void SetAllPhaseCalls(IntersectionEngine_t *engine)
{
  uint8_t phaseIndex;

  if (engine == NULL)
  {
    return;
  }

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    engine->runtime.phases[phaseIndex].callLatched = 1U;

    if (PhasePedConfigured(engine, phaseIndex) != 0U)
    {
      engine->runtime.phases[phaseIndex].pedCallLatched = 1U;
      engine->runtime.phases[phaseIndex].pedServicePending = 1U;
    }
  }
}

static void StartAutomaticFlashExit(IntersectionEngine_t *engine)
{
  uint8_t ringIndex;

  if ((engine == NULL) || (AutomaticFlashTransitionConfigured(engine) == 0U))
  {
    return;
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    StartRingGreenStage(engine,
                        ringIndex,
                        engine->automaticFlashExitPositions[ringIndex],
                        1U);
  }

  SetAllPhaseCalls(engine);
}

static void ForceControllerRedRest(IntersectionEngine_t *engine)
{
  uint8_t phaseIndex;
  uint8_t ringIndex;

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    engine->runtime.phases[phaseIndex].interval =
      INTERSECTION_PHASE_INTERVAL_RED;
    engine->runtime.phases[phaseIndex].intervalElapsedTicks = 0U;
    engine->runtime.phases[phaseIndex].next = 0U;
    EndPhasePedService(engine, phaseIndex);
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];

    ringRuntime->activePosition = 0U;
    ringRuntime->pendingPosition = 0U;
    ringRuntime->activePhaseIndex =
      engine->config.rings[ringIndex].phaseOrder[0];
    ringRuntime->barrierWaiting = 0U;
    ringRuntime->stage = INTERSECTION_RING_STAGE_RED_REST;
    ringRuntime->statusCode = INTERSECTION_RING_STATUS_RED_REST;
    ringRuntime->terminationReasonBits = INTERSECTION_RING_TERMINATION_NONE;
    ringRuntime->stageElapsedTicks = 0U;
  }
}

static uint16_t MinUInt16(uint16_t left, uint16_t right)
{
  return (left < right) ? left : right;
}

static uint32_t MaxUInt32(uint32_t left, uint32_t right)
{
  return (left > right) ? left : right;
}

static uint32_t RemainingTicks(uint32_t requiredTicks, uint32_t elapsedTicks)
{
  return (requiredTicks > elapsedTicks) ? (requiredTicks - elapsedTicks) : 0U;
}

static void CapturePreemptEntrySnapshot(IntersectionEngine_t *engine)
{
  uint8_t phaseIndex;

  if (engine == NULL)
  {
    return;
  }

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    engine->preemptEntryPhaseIntervals[phaseIndex] =
      engine->runtime.phases[phaseIndex].interval;
    engine->preemptEntryPedIntervals[phaseIndex] =
      engine->runtime.phases[phaseIndex].pedInterval;
    engine->preemptEntryPhaseElapsedTicks[phaseIndex] =
      engine->runtime.phases[phaseIndex].intervalElapsedTicks;
    engine->preemptEntryPedElapsedTicks[phaseIndex] =
      engine->runtime.phases[phaseIndex].pedIntervalElapsedTicks;
  }
}

static uint32_t PreemptEntryWalkTicks(const IntersectionEngine_t *engine,
                                      uint8_t preemptIndex,
                                      uint8_t phaseIndex)
{
  uint32_t requiredTicks;

  if ((engine == NULL)
      || (phaseIndex >= engine->config.phaseCount)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (engine->preemptEntryPedIntervals[phaseIndex]
          != INTERSECTION_PED_INTERVAL_WALK))
  {
    return 0U;
  }

  requiredTicks = MinUInt16(PhaseCurrentWalkTicks(engine, phaseIndex),
                            engine->preemptMinimumWalkTicks[preemptIndex]);

  return RemainingTicks(requiredTicks,
                        engine->preemptEntryPedElapsedTicks[phaseIndex]);
}

static uint32_t PreemptEntryPedClearTicks(const IntersectionEngine_t *engine,
                                          uint8_t preemptIndex,
                                          uint8_t phaseIndex)
{
  uint32_t requiredTicks;

  if ((engine == NULL)
      || (phaseIndex >= engine->config.phaseCount)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (engine->preemptEntryPedIntervals[phaseIndex]
          == INTERSECTION_PED_INTERVAL_DONT_WALK))
  {
    return 0U;
  }

  requiredTicks = MinUInt16(PhaseCurrentPedClearTicks(engine, phaseIndex),
                            engine->preemptEnterPedClearTicks[preemptIndex]);

  if (engine->preemptEntryPedIntervals[phaseIndex]
      == INTERSECTION_PED_INTERVAL_CLEAR)
  {
    return RemainingTicks(requiredTicks,
                          engine->preemptEntryPedElapsedTicks[phaseIndex]);
  }

  return requiredTicks;
}

static uint32_t PreemptEntryGreenTicks(const IntersectionEngine_t *engine,
                                       uint8_t preemptIndex,
                                       uint8_t phaseIndex)
{
  uint32_t requiredTicks;
  uint32_t walkTicks;

  if ((engine == NULL)
      || (phaseIndex >= engine->config.phaseCount)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (engine->preemptEntryPhaseIntervals[phaseIndex]
          != INTERSECTION_PHASE_INTERVAL_GREEN))
  {
    return 0U;
  }

  requiredTicks = MinUInt16(engine->minGreenTicks[phaseIndex],
                            engine->preemptMinimumGreenTicks[preemptIndex]);
  requiredTicks = RemainingTicks(requiredTicks,
                                 engine->preemptEntryPhaseElapsedTicks[phaseIndex]);
  walkTicks = PreemptEntryWalkTicks(engine, preemptIndex, phaseIndex);

  return MaxUInt32(requiredTicks, walkTicks);
}

static uint32_t PreemptEntryYellowTicks(const IntersectionEngine_t *engine,
                                        uint8_t preemptIndex,
                                        uint8_t phaseIndex)
{
  uint32_t requiredTicks;

  if ((engine == NULL)
      || (phaseIndex >= engine->config.phaseCount)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  requiredTicks = MinUInt16(engine->yellowTicks[phaseIndex],
                            engine->preemptEnterYellowTicks[preemptIndex]);

  if (engine->preemptEntryPhaseIntervals[phaseIndex]
      == INTERSECTION_PHASE_INTERVAL_GREEN)
  {
    return requiredTicks;
  }

  if (engine->preemptEntryPhaseIntervals[phaseIndex]
      == INTERSECTION_PHASE_INTERVAL_YELLOW)
  {
    return RemainingTicks(requiredTicks,
                          engine->preemptEntryPhaseElapsedTicks[phaseIndex]);
  }

  return 0U;
}

static uint32_t PreemptEntryRedClearTicks(const IntersectionEngine_t *engine,
                                          uint8_t preemptIndex,
                                          uint8_t phaseIndex)
{
  uint32_t requiredTicks;

  if ((engine == NULL)
      || (phaseIndex >= engine->config.phaseCount)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  requiredTicks = MinUInt16(engine->redClearTicks[phaseIndex],
                            engine->preemptEnterRedClearTicks[preemptIndex]);

  if ((engine->preemptEntryPhaseIntervals[phaseIndex]
       == INTERSECTION_PHASE_INTERVAL_GREEN)
      || (engine->preemptEntryPhaseIntervals[phaseIndex]
          == INTERSECTION_PHASE_INTERVAL_YELLOW))
  {
    return requiredTicks;
  }

  if (engine->preemptEntryPhaseIntervals[phaseIndex]
      == INTERSECTION_PHASE_INTERVAL_RED_CLEAR)
  {
    return RemainingTicks(requiredTicks,
                          engine->preemptEntryPhaseElapsedTicks[phaseIndex]);
  }

  return 0U;
}

static uint32_t PreemptEntryVehicleCompleteTicks(const IntersectionEngine_t *engine,
                                                 uint8_t preemptIndex,
                                                 uint8_t phaseIndex)
{
  uint32_t greenTicks = PreemptEntryGreenTicks(engine, preemptIndex, phaseIndex);
  uint32_t yellowTicks = PreemptEntryYellowTicks(engine, preemptIndex, phaseIndex);
  uint32_t redClearTicks = PreemptEntryRedClearTicks(engine,
                                                     preemptIndex,
                                                     phaseIndex);

  switch (engine->preemptEntryPhaseIntervals[phaseIndex])
  {
      case INTERSECTION_PHASE_INTERVAL_GREEN:
      {
        return greenTicks + yellowTicks + redClearTicks;
      }

      case INTERSECTION_PHASE_INTERVAL_YELLOW:
      {
        return yellowTicks + redClearTicks;
      }

      case INTERSECTION_PHASE_INTERVAL_RED_CLEAR:
      {
        return redClearTicks;
      }

      case INTERSECTION_PHASE_INTERVAL_RED:
      default:
      {
        return 0U;
      }
  }
}

static uint32_t PreemptEntryPedCompleteTicks(const IntersectionEngine_t *engine,
                                             uint8_t preemptIndex,
                                             uint8_t phaseIndex)
{
  uint32_t walkTicks = PreemptEntryWalkTicks(engine, preemptIndex, phaseIndex);
  uint32_t pedClearTicks = PreemptEntryPedClearTicks(engine,
                                                     preemptIndex,
                                                     phaseIndex);

  if (engine->preemptEntryPedIntervals[phaseIndex] == INTERSECTION_PED_INTERVAL_WALK)
  {
    return walkTicks + pedClearTicks;
  }

  if (engine->preemptEntryPedIntervals[phaseIndex]
      == INTERSECTION_PED_INTERVAL_CLEAR)
  {
    return pedClearTicks;
  }

  return 0U;
}

static uint32_t PreemptEntryCompleteTicks(const IntersectionEngine_t *engine,
                                          uint8_t preemptIndex)
{
  uint8_t phaseIndex;
  uint32_t requiredTicks = 0U;

  if ((engine == NULL) || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    uint32_t phaseTicks = MaxUInt32(
      PreemptEntryVehicleCompleteTicks(engine, preemptIndex, phaseIndex),
      PreemptEntryPedCompleteTicks(engine, preemptIndex, phaseIndex));

    if (phaseTicks > requiredTicks)
    {
      requiredTicks = phaseTicks;
    }
  }

  return requiredTicks;
}

static void ApplyEntryPreemptOutputs(IntersectionEngine_t *engine,
                                     uint8_t preemptIndex)
{
  uint8_t phaseIndex;
  uint32_t elapsedTicks;

  if ((engine == NULL) || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return;
  }

  elapsedTicks = (engine->preemptStageTicks > 0U)
                 ? (engine->preemptStageTicks - 1U)
                 : 0U;

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    uint32_t greenTicks = PreemptEntryGreenTicks(engine, preemptIndex, phaseIndex);
    uint32_t yellowTicks = PreemptEntryYellowTicks(engine,
                                                   preemptIndex,
                                                   phaseIndex);
    uint32_t redClearTicks = PreemptEntryRedClearTicks(engine,
                                                       preemptIndex,
                                                       phaseIndex);
    uint32_t walkTicks = PreemptEntryWalkTicks(engine, preemptIndex, phaseIndex);
    uint32_t pedClearTicks = PreemptEntryPedClearTicks(engine,
                                                       preemptIndex,
                                                       phaseIndex);

    switch (engine->preemptEntryPhaseIntervals[phaseIndex])
    {
        case INTERSECTION_PHASE_INTERVAL_GREEN:
        {
          if (elapsedTicks < greenTicks)
          {
            engine->runtime.phases[phaseIndex].interval =
              INTERSECTION_PHASE_INTERVAL_GREEN;
          }
          else if (elapsedTicks < (greenTicks + yellowTicks))
          {
            engine->runtime.phases[phaseIndex].interval =
              INTERSECTION_PHASE_INTERVAL_YELLOW;
          }
          else if (elapsedTicks < (greenTicks + yellowTicks + redClearTicks))
          {
            engine->runtime.phases[phaseIndex].interval =
              INTERSECTION_PHASE_INTERVAL_RED_CLEAR;
          }

          break;
        }

        case INTERSECTION_PHASE_INTERVAL_YELLOW:
        {
          if (elapsedTicks < yellowTicks)
          {
            engine->runtime.phases[phaseIndex].interval =
              INTERSECTION_PHASE_INTERVAL_YELLOW;
          }
          else if (elapsedTicks < (yellowTicks + redClearTicks))
          {
            engine->runtime.phases[phaseIndex].interval =
              INTERSECTION_PHASE_INTERVAL_RED_CLEAR;
          }

          break;
        }

        case INTERSECTION_PHASE_INTERVAL_RED_CLEAR:
        {
          if (elapsedTicks < redClearTicks)
          {
            engine->runtime.phases[phaseIndex].interval =
              INTERSECTION_PHASE_INTERVAL_RED_CLEAR;
          }

          break;
        }

        case INTERSECTION_PHASE_INTERVAL_RED:
        default:
        {
          break;
        }
    }

    switch (engine->preemptEntryPedIntervals[phaseIndex])
    {
        case INTERSECTION_PED_INTERVAL_WALK:
        {
          if (elapsedTicks < walkTicks)
          {
            engine->runtime.phases[phaseIndex].pedInterval =
              INTERSECTION_PED_INTERVAL_WALK;
            engine->runtime.phases[phaseIndex].pedServiceActive = 1U;
          }
          else if (elapsedTicks < (walkTicks + pedClearTicks))
          {
            engine->runtime.phases[phaseIndex].pedInterval =
              INTERSECTION_PED_INTERVAL_CLEAR;
            engine->runtime.phases[phaseIndex].pedServiceActive = 1U;
          }

          break;
        }

        case INTERSECTION_PED_INTERVAL_CLEAR:
        {
          if (elapsedTicks < pedClearTicks)
          {
            engine->runtime.phases[phaseIndex].pedInterval =
              INTERSECTION_PED_INTERVAL_CLEAR;
            engine->runtime.phases[phaseIndex].pedServiceActive = 1U;
          }

          break;
        }

        case INTERSECTION_PED_INTERVAL_DONT_WALK:
        default:
        {
          break;
        }
    }
  }
}

static uint16_t PreemptTrackYellowTicks(const IntersectionEngine_t *engine,
                                        uint8_t preemptIndex,
                                        uint8_t phaseIndex)
{
  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  return MinUInt16(engine->yellowTicks[phaseIndex],
                   engine->preemptTrackYellowTicks[preemptIndex]);
}

static uint16_t PreemptTrackRedClearTicks(const IntersectionEngine_t *engine,
                                          uint8_t preemptIndex,
                                          uint8_t phaseIndex)
{
  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  return MinUInt16(engine->redClearTicks[phaseIndex],
                   engine->preemptTrackRedClearTicks[preemptIndex]);
}

static uint32_t PreemptTrackClearCompleteTicks(
  const IntersectionEngine_t *engine,
  const IntersectionPhaseReferenceList_t *phases,
  uint8_t preemptIndex)
{
  uint8_t index;
  uint32_t requiredTicks = 0U;

  if ((engine == NULL) || (phases == NULL)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  for (index = 0U; index < phases->length; index++)
  {
    uint8_t phaseNumber = phases->values[index];
    uint8_t phaseIndex = (uint8_t) (phaseNumber - 1U);

    if ((phaseNumber == 0U) || (phaseIndex >= engine->config.phaseCount))
    {
      continue;
    }

    {
      uint32_t phaseTicks =
        (uint32_t) PreemptTrackYellowTicks(engine, preemptIndex, phaseIndex)
        + (uint32_t) PreemptTrackRedClearTicks(engine, preemptIndex, phaseIndex);

      if (phaseTicks > requiredTicks)
      {
        requiredTicks = phaseTicks;
      }
    }
  }

  return requiredTicks;
}

static void ApplyAdvancedPreemptOutputs(IntersectionEngine_t *engine,
                                        const IntersectionPreemptConfig_t *preempt,
                                        uint8_t preemptIndex)
{
  uint8_t index;
  uint32_t elapsedTicks;

  if ((engine == NULL) || (preempt == NULL)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return;
  }

  elapsedTicks = (engine->preemptStageTicks > 0U)
                 ? (engine->preemptStageTicks - 1U)
                 : 0U;

  for (index = 0U; index < preempt->trackPhases.length; index++)
  {
    uint8_t phaseNumber = preempt->trackPhases.values[index];
    uint8_t phaseIndex = (uint8_t) (phaseNumber - 1U);
    uint16_t yellowTicks;
    uint16_t redClearTicks;

    if ((phaseNumber == 0U) || (phaseIndex >= engine->config.phaseCount))
    {
      continue;
    }

    yellowTicks = PreemptTrackYellowTicks(engine, preemptIndex, phaseIndex);
    redClearTicks = PreemptTrackRedClearTicks(engine, preemptIndex, phaseIndex);

    if (elapsedTicks < yellowTicks)
    {
      engine->runtime.phases[phaseIndex].interval =
        INTERSECTION_PHASE_INTERVAL_YELLOW;
    }
    else if (elapsedTicks < ((uint32_t) yellowTicks + (uint32_t) redClearTicks))
    {
      engine->runtime.phases[phaseIndex].interval =
        INTERSECTION_PHASE_INTERVAL_RED_CLEAR;
    }
  }
}

static void CapturePreemptShortServiceCandidates(IntersectionEngine_t *engine,
                                                 uint8_t preemptIndex)
{
  const IntersectionPreemptConfig_t *preempt;
  uint32_t entryCompleteTicks;
  uint8_t phaseIndex;
  uint8_t index;

  if ((engine == NULL) || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return;
  }

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    engine->preemptShortServiceOrder[phaseIndex] = UINT32_MAX;
  }

  preempt = &engine->config.preempts[preemptIndex];
  entryCompleteTicks = PreemptEntryCompleteTicks(engine, preemptIndex);

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    uint32_t greenTicks = PreemptEntryGreenTicks(engine, preemptIndex, phaseIndex);

    if ((engine->preemptEntryPhaseIntervals[phaseIndex]
         == INTERSECTION_PHASE_INTERVAL_GREEN)
        && (greenTicks != 0U))
    {
      engine->preemptShortServiceOrder[phaseIndex] = greenTicks;
    }
  }

  if ((engine->preemptTrackGreenTicks[preemptIndex] == 0U)
      || (engine->preemptTrackGreenTicks[preemptIndex]
          > engine->preemptMinimumGreenTicks[preemptIndex]))
  {
    return;
  }

  for (index = 0U; index < preempt->trackPhases.length; index++)
  {
    uint8_t phaseNumber = preempt->trackPhases.values[index];

    if ((phaseNumber == 0U) || (phaseNumber > engine->config.phaseCount))
    {
      continue;
    }

    phaseIndex = (uint8_t) (phaseNumber - 1U);

    if (engine->preemptShortServiceOrder[phaseIndex]
        > (entryCompleteTicks + engine->preemptTrackGreenTicks[preemptIndex]))
    {
      engine->preemptShortServiceOrder[phaseIndex] =
        entryCompleteTicks + engine->preemptTrackGreenTicks[preemptIndex];
    }
  }
}

static uint8_t FindRingPositionForPhase(const IntersectionEngine_t *engine,
                                        uint8_t phaseIndex,
                                        uint8_t *ringIndex,
                                        uint8_t *position)
{
  uint8_t localRingIndex;

  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  for (localRingIndex = 0U;
       localRingIndex < engine->config.ringCount;
       localRingIndex++)
  {
    const IntersectionRingPlan_t *ringPlan = &engine->config.rings[localRingIndex];
    uint8_t localPosition;

    for (localPosition = 0U; localPosition < ringPlan->phaseCount;
         localPosition++)
    {
      if (ringPlan->phaseOrder[localPosition] != phaseIndex)
      {
        continue;
      }

      if (ringIndex != NULL)
      {
        *ringIndex = localRingIndex;
      }

      if (position != NULL)
      {
        *position = localPosition;
      }

      return 1U;
    }
  }

  return 0U;
}

static uint8_t PhaseReferenceListContainsPhase(
  const IntersectionPhaseReferenceList_t *list,
  uint8_t phaseIndex)
{
  uint8_t index;
  uint8_t phaseNumber = (uint8_t) (phaseIndex + 1U);

  if (list == NULL)
  {
    return 0U;
  }

  for (index = 0U; index < list->length; index++)
  {
    if (list->values[index] == phaseNumber)
    {
      return 1U;
    }
  }

  return 0U;
}

static uint8_t PhaseReferenceListContainsRingPhase(
  const IntersectionEngine_t *engine,
  const IntersectionPhaseReferenceList_t *list,
  uint8_t ringIndex)
{
  uint8_t index;

  if ((engine == NULL) || (list == NULL) || (ringIndex >= engine->config.ringCount))
  {
    return 0U;
  }

  for (index = 0U; index < list->length; index++)
  {
    uint8_t phaseNumber = list->values[index];
    uint8_t phaseIndex = (uint8_t) (phaseNumber - 1U);
    uint8_t phaseRingIndex = 0U;

    if ((phaseNumber == 0U)
        || (phaseIndex >= engine->config.phaseCount)
        || (PhaseRingIndex(engine, phaseIndex, &phaseRingIndex) == 0U))
    {
      continue;
    }

    if (phaseRingIndex == ringIndex)
    {
      return 1U;
    }
  }

  return 0U;
}

static uint8_t PreemptCyclingConfigured(const IntersectionEngine_t *engine,
                                        uint8_t preemptIndex)
{
  if ((engine == NULL) || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  return (uint8_t) ((PreemptFlashDwell(engine, preemptIndex) == 0U)
                    && (engine->config.preempts[preemptIndex].cyclingPhases.length
                        != 0U));
}

static uint8_t PreemptCyclingRunning(const IntersectionEngine_t *engine,
                                     uint8_t preemptIndex)
{
  return (uint8_t) ((engine != NULL)
                    && (preemptIndex < INTERSECTION_PREEMPT_COUNT_MAX)
                    && (engine->preemptCyclingInitialized != 0U)
                    && (engine->activePreemptIndex == preemptIndex)
                    && (engine->runtime.preemptStates[preemptIndex]
                        == INTERSECTION_PREEMPT_STATE_DWELL)
                    && (PreemptCyclingConfigured(engine, preemptIndex) != 0U));
}

static uint8_t PreemptCyclingPhaseAllowed(const IntersectionEngine_t *engine,
                                          uint8_t phaseIndex)
{
  uint8_t preemptIndex;
  uint8_t ringIndex = 0U;

  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount)
      || (engine->preemptCyclingDemandFilterActive == 0U))
  {
    return 1U;
  }

  preemptIndex = engine->activePreemptIndex;

  if (PreemptCyclingRunning(engine, preemptIndex) == 0U)
  {
    return 1U;
  }

  if (PhaseRingIndex(engine, phaseIndex, &ringIndex) == 0U)
  {
    return 0U;
  }

  if (PhaseReferenceListContainsRingPhase(
        engine,
        &engine->config.preempts[preemptIndex].cyclingPhases,
        ringIndex) == 0U)
  {
    return PhaseReferenceListContainsPhase(
      &engine->config.preempts[preemptIndex].dwellPhases,
      phaseIndex);
  }

  return PhaseReferenceListContainsPhase(
    &engine->config.preempts[preemptIndex].cyclingPhases,
    phaseIndex);
}

static uint8_t PreemptCyclingPedAllowed(const IntersectionEngine_t *engine,
                                        uint8_t phaseIndex)
{
  uint8_t preemptIndex;
  uint8_t ringIndex = 0U;

  if ((engine == NULL) || (phaseIndex >= engine->config.phaseCount)
      || (engine->preemptCyclingDemandFilterActive == 0U))
  {
    return 1U;
  }

  preemptIndex = engine->activePreemptIndex;

  if (PreemptCyclingRunning(engine, preemptIndex) == 0U)
  {
    return 1U;
  }

  if (PhaseRingIndex(engine, phaseIndex, &ringIndex) == 0U)
  {
    return 0U;
  }

  if (PhaseReferenceListContainsRingPhase(
        engine,
        &engine->config.preempts[preemptIndex].cyclingPeds,
        ringIndex) == 0U)
  {
    return PhaseReferenceListContainsPhase(
      &engine->config.preempts[preemptIndex].dwellPeds,
      phaseIndex);
  }

  return PhaseReferenceListContainsPhase(
    &engine->config.preempts[preemptIndex].cyclingPeds,
    phaseIndex);
}

static uint8_t FindConfiguredCyclingPositionForRing(
  const IntersectionEngine_t *engine,
  uint8_t preemptIndex,
  uint8_t ringIndex,
  uint8_t demandOnly,
  uint8_t *position)
{
  const IntersectionRingPlan_t *ringPlan;
  uint8_t ringPosition;

  if ((engine == NULL) || (position == NULL)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (ringIndex >= engine->config.ringCount))
  {
    return 0U;
  }

  ringPlan = &engine->config.rings[ringIndex];

  for (ringPosition = 0U; ringPosition < ringPlan->phaseCount; ringPosition++)
  {
    uint8_t phaseIndex = ringPlan->phaseOrder[ringPosition];

    if (PhaseReferenceListContainsPhase(
          &engine->config.preempts[preemptIndex].cyclingPhases,
          phaseIndex) == 0U)
    {
      continue;
    }

    if ((demandOnly != 0U) && (PhaseHasDemand(engine, phaseIndex) == 0U))
    {
      continue;
    }

    *position = ringPosition;

    return 1U;
  }

  return 0U;
}

static uint8_t FindConfiguredDwellPositionForRing(const IntersectionEngine_t *engine,
                                                  uint8_t preemptIndex,
                                                  uint8_t ringIndex,
                                                  uint8_t *position)
{
  const IntersectionRingPlan_t *ringPlan;
  uint8_t ringPosition;

  if ((engine == NULL) || (position == NULL)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (ringIndex >= engine->config.ringCount))
  {
    return 0U;
  }

  ringPlan = &engine->config.rings[ringIndex];

  for (ringPosition = 0U; ringPosition < ringPlan->phaseCount; ringPosition++)
  {
    if (PhaseReferenceListContainsPhase(
          &engine->config.preempts[preemptIndex].dwellPhases,
          ringPlan->phaseOrder[ringPosition]) != 0U)
    {
      *position = ringPosition;

      return 1U;
    }
  }

  return 0U;
}

static void InitializePreemptExitRecoveryTargets(
  const IntersectionEngine_t *engine,
  IntersectionPreemptExitRecoveryTarget_t *targets)
{
  uint8_t ringIndex;

  if ((engine == NULL) || (targets == NULL))
  {
    return;
  }

  for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
  {
    targets[ringIndex].valid = 0U;
    targets[ringIndex].position = 0U;
    targets[ringIndex].stage = INTERSECTION_RING_STAGE_RED_REST;
    targets[ringIndex].elapsedTicks = 0U;
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    targets[ringIndex].valid = 1U;
    targets[ringIndex].position = engine->runtime.rings[ringIndex].activePosition;
  }
}

static uint32_t QueueDelayDemandScoreForPhase(const IntersectionEngine_t *engine,
                                              uint8_t preemptIndex,
                                              uint8_t phaseIndex)
{
  uint32_t score = 0U;
  uint8_t detectorIndex;
  uint8_t phaseNumber;

  if ((engine == NULL) || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (phaseIndex >= engine->config.phaseCount))
  {
    return 0U;
  }

  phaseNumber = (uint8_t) (phaseIndex + 1U);

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    if ((engine->config.vehicleDetectors[detectorIndex].callPhase != phaseNumber)
        || (engine->runtime.vehicleDetectors[detectorIndex].recognitionActive
            == 0U))
    {
      continue;
    }

    score += engine->config.preemptQueueDelayWeights[preemptIndex][detectorIndex];
  }

  return score;
}

static uint8_t SelectQueueDelayRecoveryPhase(const IntersectionEngine_t *engine,
                                             uint8_t preemptIndex,
                                             uint8_t *phaseIndex)
{
  uint8_t bestPhaseIndex = 0xFFU;
  uint32_t bestScore = 0U;
  uint32_t bestWaitTicks = 0U;
  uint8_t candidatePhaseIndex;

  if ((engine == NULL) || (phaseIndex == NULL)
      || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  for (candidatePhaseIndex = 0U;
       candidatePhaseIndex < engine->config.phaseCount;
       candidatePhaseIndex++)
  {
    uint32_t candidateScore;
    uint32_t candidateWaitTicks;

    if (PhaseHasDemand(engine, candidatePhaseIndex) == 0U)
    {
      continue;
    }

    candidateScore = QueueDelayDemandScoreForPhase(engine,
                                                   preemptIndex,
                                                   candidatePhaseIndex);
    candidateWaitTicks = engine->phaseDemandWaitTicks[candidatePhaseIndex];

    if ((bestPhaseIndex == 0xFFU)
        || (candidateScore > bestScore)
        || ((candidateScore == bestScore)
            && (candidateWaitTicks > bestWaitTicks))
        || ((candidateScore == bestScore)
            && (candidateWaitTicks == bestWaitTicks)
            && (candidatePhaseIndex < bestPhaseIndex)))
    {
      bestPhaseIndex = candidatePhaseIndex;
      bestScore = candidateScore;
      bestWaitTicks = candidateWaitTicks;
    }
  }

  if (bestPhaseIndex >= engine->config.phaseCount)
  {
    return 0U;
  }

  *phaseIndex = bestPhaseIndex;

  return 1U;
}

static uint8_t SelectShortServicePhase(const IntersectionEngine_t *engine,
                                       uint8_t *phaseIndex)
{
  uint8_t bestPhaseIndex = 0xFFU;
  uint8_t candidatePhaseIndex;
  uint32_t bestOrder = UINT32_MAX;

  if ((engine == NULL) || (phaseIndex == NULL))
  {
    return 0U;
  }

  for (candidatePhaseIndex = 0U;
       candidatePhaseIndex < engine->config.phaseCount;
       candidatePhaseIndex++)
  {
    uint32_t candidateOrder = engine->preemptShortServiceOrder[candidatePhaseIndex];

    if ((candidateOrder == UINT32_MAX)
        || (candidateOrder > bestOrder)
        || ((candidateOrder == bestOrder)
            && (bestPhaseIndex != 0xFFU)
            && (candidatePhaseIndex > bestPhaseIndex)))
    {
      continue;
    }

    bestPhaseIndex = candidatePhaseIndex;
    bestOrder = candidateOrder;
  }

  if (bestPhaseIndex >= engine->config.phaseCount)
  {
    return 0U;
  }

  *phaseIndex = bestPhaseIndex;

  return 1U;
}

static uint8_t SelectConfiguredExitPhaseTarget(
  const IntersectionEngine_t *engine,
  const IntersectionPreemptConfig_t *preempt,
  uint8_t ringIndex,
  IntersectionPreemptExitRecoveryTarget_t *target)
{
  uint8_t listIndex;

  if ((engine == NULL) || (preempt == NULL) || (target == NULL)
      || (ringIndex >= engine->config.ringCount))
  {
    return 0U;
  }

  for (listIndex = 0U; listIndex < preempt->exitPhases.length; listIndex++)
  {
    uint8_t phaseNumber = preempt->exitPhases.values[listIndex];
    uint8_t phaseIndex;
    uint8_t position;

    if ((phaseNumber == 0U) || (phaseNumber > engine->config.phaseCount))
    {
      continue;
    }

    phaseIndex = (uint8_t) (phaseNumber - 1U);

    if ((engine->config.phases[phaseIndex].ring != ringIndex)
        || (FindRingPositionForPhase(engine, phaseIndex, NULL, &position) == 0U))
    {
      continue;
    }

    target->valid = 1U;
    target->position = position;
    target->stage = INTERSECTION_RING_STAGE_GREEN;
    target->elapsedTicks = 0U;

    return 1U;
  }

  return 0U;
}

static uint8_t SelectCoordinatedExitTarget(
  const IntersectionEngine_t *engine,
  uint8_t ringIndex,
  IntersectionPreemptExitRecoveryTarget_t *target)
{
  const IntersectionPatternConfig_t *pattern;
  const IntersectionRingPlan_t *ringPlan;
  uint32_t cycleTicks;
  uint32_t offsetTicks;
  uint32_t syncTicks;
  uint32_t localTicks;
  uint32_t elapsedTicks = 0U;
  uint8_t patternStatus;
  uint8_t splitIndex;
  uint8_t position;

  if ((engine == NULL) || (target == NULL)
      || (ringIndex >= engine->config.ringCount))
  {
    return 0U;
  }

  patternStatus = engine->runtime.coordPatternStatus;

  if ((patternStatus == 0U) || (patternStatus > INTERSECTION_PATTERN_COUNT_MAX))
  {
    return 0U;
  }

  pattern = &engine->config.coordination.patterns[patternStatus - 1U];

  if ((pattern->splitNumber == 0U)
      || (pattern->splitNumber > INTERSECTION_SPLIT_COUNT_MAX)
      || (pattern->cycleTimeSeconds == 0U)
      || (pattern->offsetTimeSeconds >= pattern->cycleTimeSeconds))
  {
    return 0U;
  }

  ringPlan = &engine->config.rings[ringIndex];
  splitIndex = (uint8_t) (pattern->splitNumber - 1U);
  cycleTicks = (uint32_t) pattern->cycleTimeSeconds * 100U;
  offsetTicks = (uint32_t) pattern->offsetTimeSeconds * 100U;
  syncTicks = engine->coordSyncTicks % cycleTicks;
  localTicks = (syncTicks + cycleTicks - (offsetTicks % cycleTicks)) % cycleTicks;

  for (position = 0U; position < ringPlan->phaseCount; position++)
  {
    uint8_t phaseIndex = ringPlan->phaseOrder[position];
    uint32_t splitTicks =
      (uint32_t) engine->config.coordination.splits[splitIndex][phaseIndex].
      timeSeconds * 100U;

    if (splitTicks == 0U)
    {
      continue;
    }

    if (localTicks < (elapsedTicks + splitTicks))
    {
      uint32_t offsetIntoPhase = localTicks - elapsedTicks;
      uint32_t yellowTicks = engine->yellowTicks[phaseIndex];
      uint32_t redClearTicks = engine->redClearTicks[phaseIndex];
      uint32_t greenTicks = (splitTicks > (yellowTicks + redClearTicks))
                            ? (splitTicks - yellowTicks - redClearTicks)
                            : 0U;

      target->valid = 1U;
      target->position = position;

      if (offsetIntoPhase < greenTicks)
      {
        target->stage = INTERSECTION_RING_STAGE_GREEN;
        target->elapsedTicks = offsetIntoPhase;
      }
      else if (offsetIntoPhase < (greenTicks + yellowTicks))
      {
        target->stage = INTERSECTION_RING_STAGE_YELLOW;
        target->elapsedTicks = offsetIntoPhase - greenTicks;
      }
      else if (offsetIntoPhase < (greenTicks + yellowTicks + redClearTicks))
      {
        target->stage = INTERSECTION_RING_STAGE_RED_CLEAR;
        target->elapsedTicks = offsetIntoPhase - greenTicks - yellowTicks;
      }
      else
      {
        target->stage = INTERSECTION_RING_STAGE_RED_REST;
        target->elapsedTicks = 0U;
      }

      return 1U;
    }

    elapsedTicks += splitTicks;
  }

  if (ringPlan->phaseCount == 0U)
  {
    return 0U;
  }

  target->valid = 1U;
  target->position = (uint8_t) (ringPlan->phaseCount - 1U);
  target->stage = INTERSECTION_RING_STAGE_RED_REST;
  target->elapsedTicks = 0U;

  return 1U;
}

static void ApplyRingExitRecoveryTarget(
  IntersectionEngine_t *engine,
  uint8_t ringIndex,
  const IntersectionPreemptExitRecoveryTarget_t *target)
{
  const IntersectionRingPlan_t *ringPlan;
  IntersectionRingRuntime_t *ringRuntime;
  uint8_t position;
  uint8_t phaseIndex;

  if ((engine == NULL) || (target == NULL)
      || (ringIndex >= engine->config.ringCount))
  {
    return;
  }

  ringPlan = &engine->config.rings[ringIndex];
  ringRuntime = &engine->runtime.rings[ringIndex];
  position = target->position;

  if ((target->valid == 0U) || (position >= ringPlan->phaseCount))
  {
    position = ringRuntime->activePosition;
  }

  if (position >= ringPlan->phaseCount)
  {
    position = 0U;
  }

  phaseIndex = ringPlan->phaseOrder[position];

  switch (target->stage)
  {
      case INTERSECTION_RING_STAGE_GREEN:
      {
        StartRingGreenStage(engine, ringIndex, position, 0U);
        ringRuntime->stageElapsedTicks = target->elapsedTicks;
        engine->runtime.phases[phaseIndex].intervalElapsedTicks =
          target->elapsedTicks;
        break;
      }

      case INTERSECTION_RING_STAGE_YELLOW:
      {
        SetRingPhasesRed(engine, ringIndex);
        ringRuntime->activePosition = position;
        ringRuntime->pendingPosition = position;
        ringRuntime->activePhaseIndex = phaseIndex;
        ringRuntime->barrierWaiting = 0U;
        ringRuntime->stage = INTERSECTION_RING_STAGE_YELLOW;
        ringRuntime->statusCode = INTERSECTION_RING_STATUS_YELLOW_CHANGE;
        ringRuntime->terminationReasonBits = INTERSECTION_RING_TERMINATION_NONE;
        ringRuntime->stageElapsedTicks = target->elapsedTicks;
        engine->runtime.phases[phaseIndex].interval =
          INTERSECTION_PHASE_INTERVAL_YELLOW;
        engine->runtime.phases[phaseIndex].intervalElapsedTicks =
          target->elapsedTicks;
        engine->runtime.phases[phaseIndex].callLatched = 0U;
        engine->runtime.phases[phaseIndex].next = 0U;
        EndPhasePedService(engine, phaseIndex);
        break;
      }

      case INTERSECTION_RING_STAGE_RED_CLEAR:
      {
        SetRingPhasesRed(engine, ringIndex);
        ringRuntime->activePosition = position;
        ringRuntime->pendingPosition = position;
        ringRuntime->activePhaseIndex = phaseIndex;
        ringRuntime->barrierWaiting = 0U;
        ringRuntime->stage = INTERSECTION_RING_STAGE_RED_CLEAR;
        ringRuntime->statusCode = INTERSECTION_RING_STATUS_RED_CLEARANCE;
        ringRuntime->terminationReasonBits = INTERSECTION_RING_TERMINATION_NONE;
        ringRuntime->stageElapsedTicks = target->elapsedTicks;
        engine->runtime.phases[phaseIndex].interval =
          INTERSECTION_PHASE_INTERVAL_RED_CLEAR;
        engine->runtime.phases[phaseIndex].intervalElapsedTicks =
          target->elapsedTicks;
        engine->runtime.phases[phaseIndex].callLatched = 0U;
        engine->runtime.phases[phaseIndex].next = 0U;
        EndPhasePedService(engine, phaseIndex);
        StartPhaseRedRevertTimer(engine, phaseIndex);
        break;
      }

      case INTERSECTION_RING_STAGE_WAIT_BARRIER:
      case INTERSECTION_RING_STAGE_RED_REST:
      default:
      {
        SetRingRedRestStage(engine, ringIndex, position);
        break;
      }
  }
}

static void SetPreemptStateDefaults(IntersectionEngine_t *engine)
{
  uint8_t index;

  for (index = 0U; index < INTERSECTION_PREEMPT_COUNT_MAX; index++)
  {
    engine->runtime.preemptStates[index] =
      (PreemptInputIsPresent(engine, index) != 0U)
      ? INTERSECTION_PREEMPT_STATE_NOT_ACTIVE_WITH_CALL
      : INTERSECTION_PREEMPT_STATE_NOT_ACTIVE;
  }

  engine->runtime.preemptStatus = 0U;
}

static void ApplyDwellPreemptOutputs(IntersectionEngine_t *engine,
                                     const IntersectionPreemptConfig_t *preempt,
                                     uint8_t preemptIndex)
{
  if ((engine == NULL) || (preempt == NULL))
  {
    return;
  }

  if (PreemptFlashDwell(engine, preemptIndex) != 0U)
  {
    ApplyPhaseListInterval(engine,
                           &preempt->dwellPhases,
                           INTERSECTION_PHASE_INTERVAL_YELLOW);
    ApplyOverlapListAspect(engine,
                           &preempt->dwellOverlaps,
                           INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW);
  }
  else
  {
    ApplyPhaseListInterval(engine,
                           &preempt->dwellPhases,
                           INTERSECTION_PHASE_INTERVAL_GREEN);
    ApplyPedListInterval(engine,
                         &preempt->dwellPeds,
                         INTERSECTION_PED_INTERVAL_WALK);
    ApplyOverlapListAspect(engine,
                           &preempt->dwellOverlaps,
                           INTERSECTION_OUTPUT_ASPECT_GREEN);
  }
}

static void ResetPreemptCyclingRuntime(IntersectionEngine_t *engine)
{
  if (engine == NULL)
  {
    return;
  }

  engine->preemptCyclingInitialized = 0U;
  engine->preemptCyclingDemandFilterActive = 0U;
}

static void InitializePreemptCyclingRuntime(IntersectionEngine_t *engine,
                                            uint8_t preemptIndex)
{
  uint8_t ringIndex;
  uint8_t phaseIndex;

  if ((engine == NULL) || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (PreemptCyclingConfigured(engine, preemptIndex) == 0U)
      || (engine->preemptCyclingInitialized != 0U))
  {
    return;
  }

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    ResetPhasePedState(engine, phaseIndex);
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    uint8_t position = 0U;
    uint8_t useDwellPosition = FindConfiguredDwellPositionForRing(engine,
                                                                  preemptIndex,
                                                                  ringIndex,
                                                                  &position);
    uint8_t phaseInCyclingPeds;
    uint8_t phasePositionFound = useDwellPosition;

    if (phasePositionFound == 0U)
    {
      phasePositionFound = FindConfiguredCyclingPositionForRing(engine,
                                                                preemptIndex,
                                                                ringIndex,
                                                                1U,
                                                                &position);
    }

    if (phasePositionFound == 0U)
    {
      phasePositionFound = FindConfiguredCyclingPositionForRing(engine,
                                                                preemptIndex,
                                                                ringIndex,
                                                                0U,
                                                                &position);
    }

    if (phasePositionFound == 0U)
    {
      SetRingRedRestStage(engine, ringIndex, 0U);
      continue;
    }

    StartRingGreenStage(engine, ringIndex, position, 0U);
    phaseIndex = engine->runtime.rings[ringIndex].activePhaseIndex;
    phaseInCyclingPeds = PhaseReferenceListContainsPhase(
      &engine->config.preempts[preemptIndex].cyclingPeds,
      phaseIndex);

    if (useDwellPosition != 0U)
    {
      engine->runtime.rings[ringIndex].stageElapsedTicks = engine->preemptStageTicks;
      engine->runtime.phases[phaseIndex].intervalElapsedTicks =
        engine->preemptStageTicks;
    }

    if (phaseInCyclingPeds == 0U)
    {
      ResetPhasePedState(engine, phaseIndex);
    }
  }

  engine->preemptCyclingInitialized = 1U;
}

static void TickPreemptCyclingRuntime(IntersectionEngine_t *engine,
                                      uint8_t preemptIndex)
{
  if ((engine == NULL) || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX)
      || (PreemptCyclingConfigured(engine, preemptIndex) == 0U))
  {
    return;
  }

  InitializePreemptCyclingRuntime(engine, preemptIndex);
  engine->preemptCyclingDemandFilterActive = 1U;
  TickControllerRings(engine);
  TickInactivePedStates(engine);
  engine->preemptCyclingDemandFilterActive = 0U;
}

static void ApplyPreemptCyclingOverlapOutputs(
  IntersectionEngine_t *engine,
  const IntersectionPreemptConfig_t *preempt)
{
  uint8_t ringIndex;
  uint8_t dwellOverlapActive = 0U;
  uint8_t cyclingOverlapActive = 0U;

  if ((engine == NULL) || (preempt == NULL))
  {
    return;
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    const IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[
      ringIndex];
    uint8_t phaseIndex = ringRuntime->activePhaseIndex;

    if ((ringRuntime->stage != INTERSECTION_RING_STAGE_GREEN)
        || (phaseIndex >= engine->config.phaseCount))
    {
      continue;
    }

    if (PhaseReferenceListContainsPhase(&preempt->dwellPhases, phaseIndex) != 0U)
    {
      dwellOverlapActive = 1U;
    }

    if (PhaseReferenceListContainsPhase(&preempt->cyclingPhases, phaseIndex)
        != 0U)
    {
      cyclingOverlapActive = 1U;
    }
  }

  if (dwellOverlapActive != 0U)
  {
    ApplyOverlapListAspect(engine,
                           &preempt->dwellOverlaps,
                           INTERSECTION_OUTPUT_ASPECT_GREEN);
  }

  if (cyclingOverlapActive != 0U)
  {
    ApplyOverlapListAspect(engine,
                           &preempt->cyclingOverlaps,
                           INTERSECTION_OUTPUT_ASPECT_GREEN);
  }
}

static void ApplyActivePreemptOutputs(IntersectionEngine_t *engine)
{
  const IntersectionPreemptConfig_t *preempt;
  IntersectionPreemptState_t state;
  uint8_t preemptIndex;

  if (PreemptModeActive(engine) == 0U)
  {
    return;
  }

  preemptIndex = engine->activePreemptIndex;
  preempt = &engine->config.preempts[preemptIndex];
  state = engine->runtime.preemptStates[preemptIndex];

  if ((state == INTERSECTION_PREEMPT_STATE_DWELL)
      && (PreemptCyclingRunning(engine, preemptIndex) != 0U))
  {
    ClearPreemptOverlapOutputs(engine);
    ApplyPreemptCyclingOverlapOutputs(engine, preempt);

    return;
  }

  ClearPreemptPhaseOutputs(engine);
  ClearPreemptOverlapOutputs(engine);

  switch (state)
  {
      case INTERSECTION_PREEMPT_STATE_TRACK_SERVICE:
      {
        ApplyPhaseListInterval(engine,
                               &preempt->trackPhases,
                               INTERSECTION_PHASE_INTERVAL_GREEN);
        ApplyOverlapListAspect(engine,
                               &preempt->trackOverlaps,
                               INTERSECTION_OUTPUT_ASPECT_GREEN);
        break;
      }

      case INTERSECTION_PREEMPT_STATE_ENTRY_STARTED:
      {
        ApplyEntryPreemptOutputs(engine, preemptIndex);
        break;
      }

      case INTERSECTION_PREEMPT_STATE_ADVANCED_PREEMPT:
      {
        ApplyAdvancedPreemptOutputs(engine, preempt, preemptIndex);
        break;
      }

      case INTERSECTION_PREEMPT_STATE_DWELL:
      case INTERSECTION_PREEMPT_STATE_LINK_ACTIVE:
      {
        ApplyDwellPreemptOutputs(engine, preempt, preemptIndex);
        break;
      }

      case INTERSECTION_PREEMPT_STATE_EXIT_STARTED:
      {
        ApplyPhaseListInterval(engine,
                               &preempt->exitPhases,
                               INTERSECTION_PHASE_INTERVAL_GREEN);
        break;
      }

      case INTERSECTION_PREEMPT_STATE_MAX_PRESENCE:
      {
        if (PreemptAllRedFlashOnMaxPresence(engine,
                                            engine->activePreemptIndex) != 0U)
        {
          uint8_t overlapIndex;

          for (overlapIndex = 0U;
               overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX;
               overlapIndex++)
          {
            engine->runtime.overlaps[overlapIndex].aspect =
              INTERSECTION_OUTPUT_ASPECT_FLASH_RED;
          }
        }

        break;
      }

      case INTERSECTION_PREEMPT_STATE_OTHER:
      case INTERSECTION_PREEMPT_STATE_NOT_ACTIVE:
      case INTERSECTION_PREEMPT_STATE_NOT_ACTIVE_WITH_CALL:
      default:
      {
        if ((state == INTERSECTION_PREEMPT_STATE_NOT_ACTIVE_WITH_CALL)
            && (engine->linkedPreemptTargetIndex == preemptIndex)
            && (engine->linkedPreemptSourceIndex
                < INTERSECTION_PREEMPT_COUNT_MAX))
        {
          const IntersectionPreemptConfig_t *linkedPreempt =
            &engine->config.preempts[engine->linkedPreemptSourceIndex];

          ApplyDwellPreemptOutputs(engine,
                                   linkedPreempt,
                                   engine->linkedPreemptSourceIndex);
        }

        break;
      }
  } /* switch */
} /* ApplyActivePreemptOutputs */

static void ResetPreemptRuntime(IntersectionEngine_t *engine)
{
  uint8_t phaseIndex;

  engine->activePreemptIndex = 0xFFU;
  engine->preemptStageTicks = 0U;
  engine->preemptPresenceTicks = 0U;
  ResetPreemptCyclingRuntime(engine);
  ClearLinkedPreemptCall(engine);

  for (phaseIndex = 0U; phaseIndex < INTERSECTION_PHASE_COUNT_MAX; phaseIndex++)
  {
    engine->preemptShortServiceOrder[phaseIndex] = UINT32_MAX;
  }

  SetPreemptStateDefaults(engine);
}

static uint8_t BeginPreemptExitRecovery(IntersectionEngine_t *engine,
                                        uint8_t preemptIndex)
{
  const IntersectionPreemptConfig_t *preempt;
  IntersectionPreemptExitRecoveryTarget_t targets[INTERSECTION_RING_COUNT_MAX];
  uint8_t selectedPhaseIndex = 0xFFU;
  uint8_t ringIndex;

  if ((engine == NULL) || (preemptIndex >= INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  preempt = &engine->config.preempts[preemptIndex];
  InitializePreemptExitRecoveryTargets(engine, targets);

  switch ((IntersectionPreemptExitType_t) preempt->exitType)
  {
      case INTERSECTION_PREEMPT_EXIT_TYPE_QUEUE_DELAY_RECOVERY:
      {
        uint8_t targetPosition = 0U;

        if ((SelectQueueDelayRecoveryPhase(engine,
                                           preemptIndex,
                                           &selectedPhaseIndex) != 0U)
            && (FindRingPositionForPhase(engine,
                                         selectedPhaseIndex,
                                         &ringIndex,
                                         &targetPosition) != 0U))
        {
          targets[ringIndex].valid = 1U;
          targets[ringIndex].position = targetPosition;
          targets[ringIndex].stage = INTERSECTION_RING_STAGE_GREEN;
          targets[ringIndex].elapsedTicks = 0U;
        }

        break;
      }

      case INTERSECTION_PREEMPT_EXIT_TYPE_SHORT_SERVICE:
      {
        uint8_t targetPosition = 0U;

        if ((SelectShortServicePhase(engine, &selectedPhaseIndex) != 0U)
            && (FindRingPositionForPhase(engine,
                                         selectedPhaseIndex,
                                         &ringIndex,
                                         &targetPosition) != 0U))
        {
          targets[ringIndex].valid = 1U;
          targets[ringIndex].position = targetPosition;
          targets[ringIndex].stage = INTERSECTION_RING_STAGE_GREEN;
          targets[ringIndex].elapsedTicks = 0U;
        }

        break;
      }

      case INTERSECTION_PREEMPT_EXIT_TYPE_EXIT_COORD:
      {
        for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
        {
          (void) SelectCoordinatedExitTarget(engine,
                                             ringIndex,
                                             &targets[ringIndex]);
        }

        break;
      }

      case INTERSECTION_PREEMPT_EXIT_TYPE_EXIT_PHASES:
      default:
      {
        for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
        {
          (void) SelectConfiguredExitPhaseTarget(engine,
                                                 preempt,
                                                 ringIndex,
                                                 &targets[ringIndex]);
        }

        break;
      }
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    ApplyRingExitRecoveryTarget(engine, ringIndex, &targets[ringIndex]);
  }

  ResetPreemptRuntime(engine);
  UpdateCoordinationRuntime(engine);

  return 1U;
}

static void StartPreempt(IntersectionEngine_t *engine, uint8_t preemptIndex)
{
  const IntersectionPreemptConfig_t *preempt =
    &engine->config.preempts[preemptIndex];
  uint32_t entryTicks;

  engine->activePreemptIndex = preemptIndex;
  engine->preemptStageTicks = 0U;
  engine->preemptPresenceTicks = 0U;
  ResetPreemptCyclingRuntime(engine);
  CapturePreemptEntrySnapshot(engine);
  CapturePreemptShortServiceCandidates(engine, preemptIndex);
  SetPreemptStateDefaults(engine);
  engine->runtime.preemptStatus = (uint8_t) (preemptIndex + 1U);
  engine->runtime.preemptStates[preemptIndex] =
    INTERSECTION_PREEMPT_STATE_ENTRY_STARTED;
  engine->runtime.mode = INTERSECTION_CONTROL_MODE_PREEMPT;
  entryTicks = PreemptEntryCompleteTicks(engine, preemptIndex);

  if (entryTicks == 0U)
  {
    if ((preempt->trackGreenSeconds != 0U)
        && (preempt->trackPhases.length != 0U))
    {
      engine->runtime.preemptStates[preemptIndex] =
        INTERSECTION_PREEMPT_STATE_TRACK_SERVICE;
    }
    else
    {
      engine->runtime.preemptStates[preemptIndex] =
        INTERSECTION_PREEMPT_STATE_DWELL;
    }
  }

  RefreshLinkedPreemptCall(engine);
}

static uint8_t HandlePreemptStateMachine(IntersectionEngine_t *engine)
{
  int16_t candidateIndex;
  uint8_t hadActiveState = 0U;
  uint8_t activePreemptIndex = 0xFFU;
  IntersectionPreemptState_t activeState = INTERSECTION_PREEMPT_STATE_NOT_ACTIVE;

  if (engine == NULL)
  {
    return 0U;
  }

  if (engine->mmuFlashActive != 0U)
  {
    SetPreemptStateDefaults(engine);
    engine->activePreemptIndex = 0xFFU;
    engine->preemptStageTicks = 0U;
    engine->preemptPresenceTicks = 0U;
    ResetPreemptCyclingRuntime(engine);
    ClearLinkedPreemptCall(engine);

    return 0U;
  }

  if (engine->activePreemptIndex < INTERSECTION_PREEMPT_COUNT_MAX)
  {
    activePreemptIndex = engine->activePreemptIndex;
    activeState = engine->runtime.preemptStates[activePreemptIndex];
    hadActiveState = PreemptStateIsServicing(activeState);
  }

  SetPreemptStateDefaults(engine);
  RefreshLinkedPreemptCall(engine);

  if ((hadActiveState != 0U)
      && (activePreemptIndex < INTERSECTION_PREEMPT_COUNT_MAX))
  {
    engine->runtime.preemptStates[activePreemptIndex] = activeState;
  }

  candidateIndex = SelectPreemptCandidate(engine);

  if (candidateIndex < 0)
  {
    engine->activePreemptIndex = 0xFFU;
    engine->preemptStageTicks = 0U;
    engine->preemptPresenceTicks = 0U;
    ResetPreemptCyclingRuntime(engine);
    ClearLinkedPreemptCall(engine);

    return 0U;
  }

  if ((engine->runtime.mode == INTERSECTION_CONTROL_MODE_FLASH)
      && (PreemptOverridesFlash(engine, (uint8_t) candidateIndex) == 0U))
  {
    return 0U;
  }

  if ((engine->activePreemptIndex == 0xFFU)
      || (engine->activePreemptIndex != (uint8_t) candidateIndex)
      || (engine->runtime.preemptStates[engine->activePreemptIndex]
          == INTERSECTION_PREEMPT_STATE_NOT_ACTIVE_WITH_CALL))
  {
    if (engine->activePreemptIndex != (uint8_t) candidateIndex)
    {
      engine->activePreemptIndex = (uint8_t) candidateIndex;
      engine->preemptStageTicks = 0U;
      engine->preemptPresenceTicks = 0U;
    }

    engine->runtime.preemptStates[engine->activePreemptIndex] =
      INTERSECTION_PREEMPT_STATE_NOT_ACTIVE_WITH_CALL;
    engine->preemptStageTicks++;

    if ((PreemptNonLocking(engine, engine->activePreemptIndex) != 0U)
        && (PreemptInputIsPresent(engine, engine->activePreemptIndex) == 0U))
    {
      ResetPreemptRuntime(engine);

      return 0U;
    }

    if ((engine->preemptDelayTicks[engine->activePreemptIndex] == 0U)
        || (engine->preemptStageTicks
            >= engine->preemptDelayTicks[engine->activePreemptIndex]))
    {
      StartPreempt(engine, (uint8_t) candidateIndex);
    }
    else
    {
      return 0U;
    }
  }

  engine->runtime.preemptStatus = (uint8_t) (engine->activePreemptIndex + 1U);
  engine->runtime.mode = INTERSECTION_CONTROL_MODE_PREEMPT;
  engine->preemptStageTicks++;
  engine->preemptPresenceTicks++;

  {
    const IntersectionPreemptConfig_t *preempt =
      &engine->config.preempts[engine->activePreemptIndex];
    IntersectionPreemptState_t *state =
      &engine->runtime.preemptStates[engine->activePreemptIndex];

    if ((preempt->maximumPresenceSeconds != 0U)
        && (engine->preemptPresenceTicks
            >= engine->preemptMaximumPresenceTicks[engine->activePreemptIndex]))
    {
      *state = INTERSECTION_PREEMPT_STATE_MAX_PRESENCE;

      if (PreemptAllRedFlashOnMaxPresence(engine,
                                          engine->activePreemptIndex) != 0U)
      {
        engine->runtime.mode = INTERSECTION_CONTROL_MODE_PREEMPT;
      }
      else
      {
        ResetPreemptRuntime(engine);

        return 0U;
      }
    }
    else
    {
      switch (*state)
      {
          case INTERSECTION_PREEMPT_STATE_ENTRY_STARTED:
          {
            uint32_t entryTicks = PreemptEntryCompleteTicks(
              engine,
              engine->activePreemptIndex);

            if ((entryTicks == 0U) || (engine->preemptStageTicks >= entryTicks))
            {
              engine->preemptStageTicks = 0U;
              *state = ((preempt->trackGreenSeconds != 0U)
                        && (preempt->trackPhases.length != 0U))
                       ? INTERSECTION_PREEMPT_STATE_TRACK_SERVICE
                       : INTERSECTION_PREEMPT_STATE_DWELL;
            }

            break;
          }

          case INTERSECTION_PREEMPT_STATE_TRACK_SERVICE:
          {
            if ((engine->preemptTrackGreenTicks[engine->activePreemptIndex]
                 == 0U)
                || (engine->preemptStageTicks
                    >= engine->preemptTrackGreenTicks[engine->activePreemptIndex]))
            {
              uint32_t trackClearTicks = PreemptTrackClearCompleteTicks(
                engine,
                &preempt->trackPhases,
                engine->activePreemptIndex);

              engine->preemptStageTicks = 0U;
              *state = (trackClearTicks != 0U)
                       ? INTERSECTION_PREEMPT_STATE_ADVANCED_PREEMPT
                       : INTERSECTION_PREEMPT_STATE_DWELL;
            }

            break;
          }

          case INTERSECTION_PREEMPT_STATE_ADVANCED_PREEMPT:
          {
            uint32_t trackClearTicks = PreemptTrackClearCompleteTicks(
              engine,
              &preempt->trackPhases,
              engine->activePreemptIndex);

            if ((trackClearTicks == 0U)
                || (engine->preemptStageTicks >= trackClearTicks))
            {
              engine->preemptStageTicks = 0U;
              *state = INTERSECTION_PREEMPT_STATE_DWELL;
            }

            break;
          }

          case INTERSECTION_PREEMPT_STATE_DWELL:
          {
            uint8_t linkedTarget = 0xFFU;

            if ((engine->preemptStageTicks
                 >= engine->preemptDwellGreenTicks[engine->activePreemptIndex])
                && (PreemptBaseDemandIsPresent(engine,
                                              engine->activePreemptIndex)
                    != 0U)
                && (engine->linkedPreemptSourceIndex
                    >= INTERSECTION_PREEMPT_COUNT_MAX)
                && (ResolveLinkedPreemptTarget(engine,
                                              engine->activePreemptIndex,
                                              &linkedTarget)
                    != 0U))
            {
              engine->linkedPreemptSourceIndex = engine->activePreemptIndex;
              engine->linkedPreemptTargetIndex = linkedTarget;
              *state = INTERSECTION_PREEMPT_STATE_LINK_ACTIVE;

              break;
            }

            if ((engine->preemptStageTicks
                 >= engine->preemptDwellGreenTicks[engine->activePreemptIndex])
                && (PreemptBaseDemandIsPresent(engine,
                                              engine->activePreemptIndex)
                    != 0U))
            {
              TickPreemptCyclingRuntime(engine, engine->activePreemptIndex);
            }

            if ((engine->preemptPresenceTicks
                 >= engine->preemptMinimumDurationTicks[engine->
                                                        activePreemptIndex])
                && (engine->preemptStageTicks
                    >= engine->preemptDwellGreenTicks[engine->activePreemptIndex])
                && (PreemptInputIsPresent(engine,
                                          engine->activePreemptIndex) == 0U))
            {
              engine->preemptStageTicks = 0U;
              *state = INTERSECTION_PREEMPT_STATE_EXIT_STARTED;
            }

            break;
          }

          case INTERSECTION_PREEMPT_STATE_EXIT_STARTED:
          {
            (void) preempt;
            (void) BeginPreemptExitRecovery(engine, engine->activePreemptIndex);

            break;
          }

          case INTERSECTION_PREEMPT_STATE_MAX_PRESENCE:
          {
            if (PreemptInputIsPresent(engine, engine->activePreemptIndex) == 0U)
            {
              ResetPreemptRuntime(engine);

              return 0U;
            }

            break;
          }

          default:
          {
            break;
          }
      } /* switch */
    }
  }

  ApplyActivePreemptOutputs(engine);

  return 1U;
} /* HandlePreemptStateMachine */

static void UpdateCoordinationRuntime(IntersectionEngine_t *engine)
{
  uint8_t command = engine->config.coordination.operationalMode;
  const IntersectionTimebaseActionConfig_t *timebaseAction = NULL;
  uint8_t interconnectCommand = 0U;
  uint8_t controlFromSystem = 0U;
  uint8_t controlFromManual = (uint8_t) (command != 0U);
  uint8_t controlFromTimebase = 0U;
  uint8_t controlFromInterconnect = 0U;
  uint8_t calledPattern = 0U;
  const IntersectionPatternConfig_t *pattern = NULL;
  uint32_t cycleTicks = 0U;
  uint32_t offsetTicks = 0U;
  uint8_t diagnosticTrackingActive = 0U;
  uint8_t diagnosticCycleZeroActive = 0U;
  IntersectionLocalFreeStatus_t localFreeStatus =
    INTERSECTION_LOCAL_FREE_STATUS_NOT_FREE;
  IntersectionControlMode_t mode = INTERSECTION_CONTROL_MODE_FREE;

  engine->runtime.systemPatternControl = engine->systemPatternControl;
  engine->runtime.systemSyncControlSeconds = engine->systemSyncControlSeconds;
  engine->runtime.actionPlanControl = engine->actionPlanControl;
  engine->runtime.timebaseActionStatus = 0U;
  engine->runtime.timebaseAuxiliaryFunctionStatus = 0U;
  engine->runtime.interconnectCommand = engine->localInterconnectCommand;
  engine->runtime.interconnectInputsValid = engine->localInterconnectInputsValid;
  engine->runtime.startUpFlashActive = StartUpFlashActive(engine);
  engine->runtime.unitControlStatus =
    (uint8_t) INTERSECTION_UNIT_CONTROL_STATUS_OTHER;
  engine->runtime.remoteManualControlTimeout =
    engine->remoteManualControlTimeout;
  engine->runtime.remoteManualIntervalAdvance =
    engine->remoteManualIntervalAdvance;
  engine->runtime.localDimmingInputActive = engine->localDimmingInputActive;
  engine->runtime.backupModeActive = engine->backupModeActive;
  engine->runtime.dimmingActive = TimebaseActionDimmingRequested(engine);
  engine->runtime.coordCycleStatusSeconds = 0U;
  engine->runtime.coordSyncStatusSeconds = 0U;
  engine->runtime.mmuFlashActive = engine->mmuFlashActive;

  timebaseAction = GetSelectedTimebaseAction(engine);

  if (timebaseAction != NULL)
  {
    engine->runtime.timebaseActionStatus = engine->actionPlanControl;
    engine->runtime.timebaseAuxiliaryFunctionStatus =
      timebaseAction->auxiliaryFunction;
  }

  if (InterconnectCommandAvailable(engine) != 0U)
  {
    interconnectCommand = engine->localInterconnectCommand;
  }

  if (StartUpFlashActive(engine) != 0U)
  {
    ResetCoordinationCycleFaultDiagnostics(engine);
    ResetCoordinationAlarmDiagnostics(engine);
    engine->coordDiagnosticCycleTicks = 0U;
    engine->coordDiagnosticCycleZeroActive = 0U;
    engine->shortAlarmCycleZeroActive = 0U;
    engine->runtime.coordPatternStatus = 255U;
    engine->runtime.localFreeStatus =
      (uint8_t) INTERSECTION_LOCAL_FREE_STATUS_NOT_FREE;
    engine->runtime.mode = INTERSECTION_CONTROL_MODE_FLASH;

    return;
  }

  if (engine->mmuFlashActive != 0U)
  {
    ResetCoordinationCycleFaultDiagnostics(engine);
    ResetCoordinationAlarmDiagnostics(engine);
    engine->coordDiagnosticCycleTicks = 0U;
    engine->coordDiagnosticCycleZeroActive = 0U;
    engine->shortAlarmCycleZeroActive = 0U;

    if (RemoteManualControlActive(engine) != 0U)
    {
      engine->runtime.unitControlStatus =
        (uint8_t) INTERSECTION_UNIT_CONTROL_STATUS_REMOTE_MANUAL_CONTROL;
    }

    engine->runtime.coordPatternStatus = 255U;
    engine->runtime.localFreeStatus =
      (uint8_t) INTERSECTION_LOCAL_FREE_STATUS_NOT_FREE;
    engine->runtime.mode = INTERSECTION_CONTROL_MODE_FLASH;

    return;
  }

  if (command == 0U)
  {
    if (engine->systemPatternControl != 0U)
    {
      command = engine->systemPatternControl;
      controlFromSystem = 1U;
    }
    else if ((UnitControlInterconnectPriorityActive(engine) != 0U)
             && (interconnectCommand != 0U))
    {
      command = interconnectCommand;
      controlFromInterconnect = 1U;
    }
    else if (TimebaseActionControlsPattern(engine, &command) != 0U)
    {
      if (command == 0U)
      {
        if (interconnectCommand != 0U)
        {
          command = interconnectCommand;
          controlFromInterconnect = 1U;
        }
      }
      else
      {
        controlFromTimebase = 1U;
      }
    }
    else if (interconnectCommand != 0U)
    {
      command = interconnectCommand;
      controlFromInterconnect = 1U;
    }
  }

  if ((controlFromTimebase != 0U)
      && (UnitControlInterconnectPriorityActive(engine) != 0U)
      && (engine->localInterconnectInputsValid == 0U)
      && (timebaseAction != NULL) && (timebaseAction->pattern != 0U))
  {
    engine->runtime.unitControlStatus =
      (uint8_t) INTERSECTION_UNIT_CONTROL_STATUS_INTERCONNECT_BACKUP;
  }
  else if (controlFromInterconnect != 0U)
  {
    engine->runtime.unitControlStatus =
      (uint8_t) INTERSECTION_UNIT_CONTROL_STATUS_INTERCONNECT;
  }
  else if (controlFromTimebase != 0U)
  {
    engine->runtime.unitControlStatus =
      (uint8_t) INTERSECTION_UNIT_CONTROL_STATUS_TIMEBASE;
  }
  else if (controlFromSystem != 0U)
  {
    engine->runtime.unitControlStatus =
      (uint8_t) INTERSECTION_UNIT_CONTROL_STATUS_SYSTEM_CONTROL;
  }
  else if (controlFromManual != 0U)
  {
    engine->runtime.unitControlStatus =
      (uint8_t) INTERSECTION_UNIT_CONTROL_STATUS_MANUAL;
  }

  if (RemoteManualControlActive(engine) != 0U)
  {
    engine->runtime.unitControlStatus =
      (uint8_t) INTERSECTION_UNIT_CONTROL_STATUS_REMOTE_MANUAL_CONTROL;
  }

  if (engine->backupModeActive != 0U)
  {
    engine->runtime.unitControlStatus =
      (uint8_t) INTERSECTION_UNIT_CONTROL_STATUS_BACKUP_MODE;
  }

  if (command == 255U)
  {
    ResetCoordinationCycleFaultDiagnostics(engine);
    ResetCoordinationAlarmDiagnostics(engine);
    engine->runtime.coordPatternStatus = 255U;
    engine->runtime.localFreeStatus =
      (uint8_t) INTERSECTION_LOCAL_FREE_STATUS_NOT_FREE;
    engine->runtime.mode = INTERSECTION_CONTROL_MODE_FLASH;

    return;
  }

  if (command == 254U)
  {
    localFreeStatus = INTERSECTION_LOCAL_FREE_STATUS_COMMAND_FREE;
  }
  else if ((command >= 1U) && (command <= INTERSECTION_PATTERN_COUNT_MAX))
  {
    calledPattern = command;
    pattern = &engine->config.coordination.patterns[calledPattern - 1U];

    if ((pattern->splitNumber == 0U)
        || (pattern->splitNumber > INTERSECTION_SPLIT_COUNT_MAX))
    {
      localFreeStatus = INTERSECTION_LOCAL_FREE_STATUS_BAD_PLAN;
      calledPattern = 0U;
    }
    else if (pattern->cycleTimeSeconds == 0U)
    {
      localFreeStatus = INTERSECTION_LOCAL_FREE_STATUS_COORD_FREE;
      calledPattern = 0U;
    }
    else if (pattern->offsetTimeSeconds >= pattern->cycleTimeSeconds)
    {
      localFreeStatus = INTERSECTION_LOCAL_FREE_STATUS_INVALID_OFFSET;
      calledPattern = 0U;
    }
    else if (CoordinationCriticalPathSeconds(engine)
             > pattern->cycleTimeSeconds)
    {
      localFreeStatus = INTERSECTION_LOCAL_FREE_STATUS_BAD_CYCLE_TIME;
      calledPattern = 0U;
    }
    else if (CoordinationSplitOverrunSeconds(engine,
                                             (uint8_t) (pattern->splitNumber
                                                        - 1U))
             > pattern->cycleTimeSeconds)
    {
      localFreeStatus = INTERSECTION_LOCAL_FREE_STATUS_SPLIT_OVERRUN;
      calledPattern = 0U;
    }
    else
    {
      cycleTicks = (uint32_t) pattern->cycleTimeSeconds * 100U;
      offsetTicks = (uint32_t) pattern->offsetTimeSeconds * 100U;
      mode = INTERSECTION_CONTROL_MODE_COORDINATED;
      localFreeStatus = INTERSECTION_LOCAL_FREE_STATUS_NOT_FREE;
    }
  }
  else if (command != 0U)
  {
    localFreeStatus = INTERSECTION_LOCAL_FREE_STATUS_BAD_PLAN;
  }
  else
  {
    localFreeStatus = INTERSECTION_LOCAL_FREE_STATUS_OTHER;
  }

  engine->coordDiagnosticCycleTicks = cycleTicks;

  diagnosticTrackingActive = (uint8_t) ((cycleTicks != 0U)
                                        && (((mode
                                              == INTERSECTION_CONTROL_MODE_COORDINATED)
                                             && (calledPattern != 0U))
                                            || (engine->runtime.coordCycleFaultActive
                                                != 0U)
                                            || (engine->runtime.coordFaultActive
                                                != 0U)
                                            || (engine->runtime.coordFailActive
                                                != 0U)
                                            || (engine->runtime.cycleFailActive
                                                != 0U)));

  if ((diagnosticTrackingActive != 0U) && (pattern != NULL))
  {
    uint32_t syncTicks;
    uint32_t localTicks;
    uint32_t remainingTicks;

    syncTicks = engine->coordSyncTicks % cycleTicks;
    localTicks = (syncTicks + cycleTicks - (offsetTicks % cycleTicks))
                 % cycleTicks;
    remainingTicks = (cycleTicks - localTicks) % cycleTicks;
    diagnosticCycleZeroActive = (uint8_t) (localTicks == 0U);

    if ((diagnosticCycleZeroActive != 0U)
        && (engine->coordDiagnosticCycleZeroActive == 0U))
    {
      ResetPedWalkServiceCycleCounts(engine);
      engine->pedWalkServiceCyclePattern = calledPattern;

      if (mode == INTERSECTION_CONTROL_MODE_COORDINATED)
      {
        engine->shortAlarmCycleZeroLatched = 1U;
        engine->shortAlarmCycleZeroActive = 1U;
      }

      UpdateCoordinationCycleFaultDiagnostics(
        engine,
        (uint8_t) ((mode == INTERSECTION_CONTROL_MODE_COORDINATED)
                   && (engine->runtime.coordCycleFaultActive == 0U)
                   && (engine->runtime.coordFailActive == 0U)
                   && (engine->runtime.cycleFailActive == 0U)));
      UpdateCoordinationAlarmDiagnostics(
        engine,
        calledPattern,
        (uint8_t) ((mode == INTERSECTION_CONTROL_MODE_COORDINATED)
                   && (engine->runtime.coordCycleFaultActive == 0U)
                   && (engine->runtime.coordFailActive == 0U)
                   && (engine->runtime.cycleFailActive == 0U)));
    }
    else if (diagnosticCycleZeroActive == 0U)
    {
      engine->shortAlarmCycleZeroActive = 0U;
    }

    engine->coordDiagnosticCycleZeroActive = diagnosticCycleZeroActive;

    if ((engine->runtime.coordCycleFaultActive != 0U)
        || (engine->runtime.coordFailActive != 0U)
        || (engine->runtime.cycleFailActive != 0U))
    {
      mode = INTERSECTION_CONTROL_MODE_FREE;
      localFreeStatus = INTERSECTION_LOCAL_FREE_STATUS_FAILED;
    }

    if (mode == INTERSECTION_CONTROL_MODE_COORDINATED)
    {
      engine->runtime.coordPatternStatus = calledPattern;
      engine->runtime.coordSyncStatusSeconds = (uint16_t) (syncTicks / 100U);
      engine->runtime.coordCycleStatusSeconds = (uint16_t) (remainingTicks
                                                            / 100U);
    }
    else
    {
      engine->runtime.coordPatternStatus = 254U;
      engine->runtime.coordSyncStatusSeconds = 0U;
      engine->runtime.coordCycleStatusSeconds = 0U;
    }
  }
  else
  {
    ResetCoordinationCycleFaultDiagnostics(engine);
    ResetCoordinationAlarmDiagnostics(engine);
    engine->coordDiagnosticCycleTicks = 0U;
    engine->coordDiagnosticCycleZeroActive = 0U;
    engine->pedWalkServiceCyclePattern = 0U;
    ResetPedWalkServiceCycleCounts(engine);
    engine->shortAlarmCycleZeroActive = 0U;
    engine->runtime.coordPatternStatus = 254U;
    engine->runtime.coordSyncStatusSeconds = 0U;
    engine->runtime.coordCycleStatusSeconds = 0U;
  }

  if (((engine->runtime.coordCycleFaultActive != 0U)
       || (engine->runtime.coordFailActive != 0U)
       || (engine->runtime.cycleFailActive != 0U))
      && (engine->coordDiagnosticCycleTicks != 0U))
  {
    mode = INTERSECTION_CONTROL_MODE_FREE;
    localFreeStatus = INTERSECTION_LOCAL_FREE_STATUS_FAILED;
    engine->runtime.coordPatternStatus = 254U;
    engine->runtime.coordSyncStatusSeconds = 0U;
    engine->runtime.coordCycleStatusSeconds = 0U;
  }

  engine->runtime.localFreeStatus = (uint8_t) localFreeStatus;
  engine->runtime.mode = mode;

  if ((mode != INTERSECTION_CONTROL_MODE_COORDINATED) || (calledPattern == 0U))
  {
    engine->pedWalkServiceCyclePattern = 0U;
    ResetPedWalkServiceCycleCounts(engine);
  }
  else if (engine->pedWalkServiceCyclePattern != calledPattern)
  {
    engine->pedWalkServiceCyclePattern = calledPattern;
    ResetPedWalkServiceCycleCounts(engine);
  }
} /* UpdateCoordinationRuntime */

static void StartRingGreenStage(IntersectionEngine_t *engine,
                                uint8_t ringIndex,
                                uint8_t nextPosition,
                                uint8_t startPedWalk)
{
  IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];
  uint8_t phaseIndex = engine->config.rings[ringIndex].phaseOrder[nextPosition];
  uint8_t detectorIndex;

  SetRingPhasesRed(engine, ringIndex);
  ringRuntime->activePosition = nextPosition;
  ringRuntime->activePhaseIndex = phaseIndex;
  ringRuntime->pendingPosition = nextPosition;
  ringRuntime->barrierWaiting = 0U;
  ringRuntime->stage = INTERSECTION_RING_STAGE_GREEN;
  ringRuntime->statusCode = INTERSECTION_RING_STATUS_MIN_GREEN;
  ringRuntime->terminationReasonBits = INTERSECTION_RING_TERMINATION_NONE;
  ringRuntime->stageElapsedTicks = 0U;
  engine->coordCycleServedThisCycle[phaseIndex] = 1U;
  engine->coordCycleFaultAges[phaseIndex] = 0U;
  engine->runtime.phases[phaseIndex].interval =
    INTERSECTION_PHASE_INTERVAL_GREEN;
  engine->runtime.phases[phaseIndex].intervalElapsedTicks = 0U;
  engine->runtime.phases[phaseIndex].callLatched = 0U;
  engine->runtime.phases[phaseIndex].next = 0U;
  engine->passageTimerTicks[phaseIndex] = engine->passageTicks[phaseIndex];
  engine->gapTimerTicks[phaseIndex] = engine->passageTicks[phaseIndex];
  engine->currentGapTicks[phaseIndex] = engine->passageTicks[phaseIndex];
  engine->initialGreenTicks[phaseIndex] = PhaseInitialGreenTicks(engine,
                                                                 phaseIndex);
  engine->initialActuationCount[phaseIndex] = 0U;
  engine->remoteManualPedAutoAdvance[ringIndex] = 0U;
  engine->vehicleCountGreen[phaseIndex] = 0U;
  engine->conflictingVehicleCountGreen[phaseIndex] = 0U;
  engine->reductionElapsedTicks[phaseIndex] = 0U;
  engine->reductionActive[phaseIndex] = 0U;
  RefreshPhaseRunningMax(engine, phaseIndex);

  if ((phaseIndex < 8U)
      && ((engine->coordDiagnosticFaultPhaseMask
           & (uint8_t) (1U << phaseIndex))
          != 0U))
  {
    uint8_t retryQualified = 0U;

    engine->coordDiagnosticFaultPhaseMask = (uint8_t) (
      engine->coordDiagnosticFaultPhaseMask & (uint8_t) ~(1U << phaseIndex));

    if ((engine->coordDiagnosticFaultPhaseMask == 0U)
        && (engine->runtime.coordCycleFaultActive != 0U)
        && (engine->coordDiagnosticRecoveryCyclesRemaining > 0U)
        && (engine->runtime.coordFailActive == 0U))
    {
      retryQualified = 1U;
    }

    if (engine->coordDiagnosticFaultPhaseMask == 0U)
    {
      engine->runtime.coordCycleFaultActive = 0U;
      engine->runtime.coordFailActive = 0U;
      engine->runtime.cycleFailActive = 0U;
      engine->coordDiagnosticRecoveryCyclesRemaining = 0U;

      if (retryQualified != 0U)
      {
        engine->runtime.coordFaultActive = 1U;
        engine->coordDiagnosticRetryCyclesRemaining = 2U;
      }
      else
      {
        engine->runtime.coordFaultActive = 0U;
        engine->coordDiagnosticRetryCyclesRemaining = 0U;
      }
    }
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    if (engine->config.vehicleDetectors[detectorIndex].callPhase
        == (uint8_t) (phaseIndex + 1U))
    {
      engine->runtime.vehicleDetectors[detectorIndex].addedInitialCount = 0U;
    }
  }

  if ((startPedWalk != 0U) && (PhasePedConfigured(engine, phaseIndex) != 0U))
  {
    if (PhasePedWalkServiceAvailable(engine, phaseIndex) != 0U)
    {
      StartPhasePedWalk(engine, phaseIndex);
    }
    else
    {
      engine->runtime.phases[phaseIndex].pedServicePending = 1U;
    }
  }
  else if ((PhasePedConfigured(engine, phaseIndex) != 0U)
      && (PhaseSystemPedOmitActive(engine, phaseIndex) == 0U)
      && ((engine->runtime.phases[phaseIndex].pedInputActive != 0U)
          || (engine->runtime.phases[phaseIndex].pedCallLatched != 0U)
          || (PhaseSystemPedCallActive(engine, phaseIndex) != 0U)
          || (RingSystemPedRecycleActive(engine, ringIndex) != 0U)
          || (RemoteManualPedCallActive(engine, phaseIndex) != 0U)))
  {
    if ((engine->runtime.phases[phaseIndex].pedInterval
         == INTERSECTION_PED_INTERVAL_DONT_WALK)
        && (PhasePedAdvanceTicks(engine, phaseIndex) != 0U)
        && (PhaseDontWalkRevertActive(engine, phaseIndex) == 0U)
        && (PhasePedWalkServiceAvailable(engine, phaseIndex) != 0U))
    {
      StartPhasePedWalk(engine, phaseIndex);
    }
    else
    {
      engine->runtime.phases[phaseIndex].pedServicePending = 1U;
    }
  }
}

static void ApplyRingStartupState(IntersectionEngine_t *engine,
                                  uint8_t ringIndex)
{
  IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];
  uint8_t phaseIndex = engine->config.rings[ringIndex].phaseOrder[0];
  uint8_t startup = engine->config.phases[phaseIndex].startup;

  engine->startupPending[ringIndex] = 0U;
  engine->startupHold[ringIndex] = 0U;

  switch ((IntersectionPhaseStartup_t) startup)
  {
      case INTERSECTION_PHASE_STARTUP_GREEN_WALK:
      {
        StartRingGreenStage(engine, ringIndex, 0U, 1U);
        break;
      }

      case INTERSECTION_PHASE_STARTUP_GREEN_NO_WALK:
      {
        StartRingGreenStage(engine, ringIndex, 0U, 0U);
        break;
      }

      case INTERSECTION_PHASE_STARTUP_YELLOW_CHANGE:
      {
        SetRingPhasesRed(engine, ringIndex);
        ringRuntime->activePosition = 0U;
        ringRuntime->activePhaseIndex = phaseIndex;
        ringRuntime->pendingPosition = 0U;
        ringRuntime->barrierWaiting = 0U;
        ringRuntime->stage = INTERSECTION_RING_STAGE_YELLOW;
        ringRuntime->statusCode = INTERSECTION_RING_STATUS_YELLOW_CHANGE;
        ringRuntime->terminationReasonBits = INTERSECTION_RING_TERMINATION_NONE;
        ringRuntime->stageElapsedTicks = 0U;
        engine->runtime.phases[phaseIndex].interval =
          INTERSECTION_PHASE_INTERVAL_YELLOW;
        engine->runtime.phases[phaseIndex].intervalElapsedTicks = 0U;
        break;
      }

      case INTERSECTION_PHASE_STARTUP_RED_CLEAR:
      {
        SetRingPhasesRed(engine, ringIndex);
        ringRuntime->activePosition = 0U;
        ringRuntime->activePhaseIndex = phaseIndex;
        ringRuntime->pendingPosition = 0U;
        ringRuntime->barrierWaiting = 0U;
        ringRuntime->stage = INTERSECTION_RING_STAGE_RED_CLEAR;
        ringRuntime->statusCode = INTERSECTION_RING_STATUS_RED_CLEARANCE;
        ringRuntime->terminationReasonBits = INTERSECTION_RING_TERMINATION_NONE;
        ringRuntime->stageElapsedTicks = 0U;
        engine->runtime.phases[phaseIndex].interval =
          INTERSECTION_PHASE_INTERVAL_RED_CLEAR;
        engine->runtime.phases[phaseIndex].intervalElapsedTicks = 0U;
        break;
      }

      case INTERSECTION_PHASE_STARTUP_PHASE_NOT_ON:
      case INTERSECTION_PHASE_STARTUP_OTHER:
      default:
      {
        engine->startupHold[ringIndex] = 1U;
        break;
      }
  }
}

static void EnterGreen(IntersectionEngine_t *engine,
                       uint8_t ringIndex,
                       uint8_t nextPosition)
{
  uint8_t phaseIndex = engine->config.rings[ringIndex].phaseOrder[nextPosition];

  if (PhaseRedRevertActive(engine, phaseIndex) != 0U)
  {
    SetRingRedRestStage(engine, ringIndex, nextPosition);

    return;
  }

  StartRingGreenStage(engine, ringIndex, nextPosition, 0U);
}

static void EnterYellow(IntersectionEngine_t *engine,
                        uint8_t ringIndex,
                        uint8_t terminationReasonBits)
{
  IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];
  uint8_t phaseIndex = ringRuntime->activePhaseIndex;

  if ((PhaseUsesDetectorOptionBasedLocking(engine, phaseIndex) == 0U)
      && (engine->runtime.phases[phaseIndex].detectorActive != 0U))
  {
    engine->runtime.phases[phaseIndex].callLatched = 1U;
  }

  ApplyDynamicMaxTermination(engine, phaseIndex, terminationReasonBits);
  engine->systemPhaseForceOff[phaseIndex] = 0U;

  ringRuntime->stage = INTERSECTION_RING_STAGE_YELLOW;
  ringRuntime->statusCode = INTERSECTION_RING_STATUS_YELLOW_CHANGE;
  ringRuntime->terminationReasonBits = terminationReasonBits;
  ringRuntime->stageElapsedTicks = 0U;
  engine->runtime.phases[phaseIndex].interval =
    INTERSECTION_PHASE_INTERVAL_YELLOW;
  engine->runtime.phases[phaseIndex].intervalElapsedTicks = 0U;
}

static void EnterRedClear(IntersectionEngine_t *engine, uint8_t ringIndex)
{
  IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];
  uint8_t phaseIndex = ringRuntime->activePhaseIndex;

  StartPhaseRedRevertTimer(engine, phaseIndex);

  ringRuntime->stage = INTERSECTION_RING_STAGE_RED_CLEAR;
  ringRuntime->statusCode = INTERSECTION_RING_STATUS_RED_CLEARANCE;
  ringRuntime->stageElapsedTicks = 0U;
  engine->runtime.phases[phaseIndex].interval =
    INTERSECTION_PHASE_INTERVAL_RED_CLEAR;
  engine->runtime.phases[phaseIndex].intervalElapsedTicks = 0U;
}

static void AdvanceAfterRedClear(IntersectionEngine_t *engine,
                                 uint8_t ringIndex)
{
  IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];
  uint8_t nextPosition;

  if ((engine->automaticFlashState
       == (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_ENTRY)
      && (ringRuntime->activePosition
          == engine->automaticFlashEntryPositions[ringIndex]))
  {
    SetRingRedRestStage(engine, ringIndex, ringRuntime->activePosition);
    return;
  }

  nextPosition = ResolveRequestedPosition(engine,
                                          ringIndex,
                                          ringRuntime->activePosition);
  ringRuntime->pendingPosition = nextPosition;

  if ((nextPosition == ringRuntime->activePosition)
      && (RingSystemRedRestActive(engine, ringIndex) != 0U))
  {
    SetRingRedRestStage(engine, ringIndex, ringRuntime->activePosition);

    return;
  }

  if (nextPosition == ringRuntime->activePosition)
  {
    EnterGreen(engine, ringIndex, ringRuntime->activePosition);

    return;
  }

  if (IsBarrierCrossing(engine,
                        ringIndex,
                        ringRuntime->activePosition,
                        nextPosition) != 0U)
  {
    EnterBarrierWait(engine, ringIndex, nextPosition);

    return;
  }

  EnterGreen(engine, ringIndex, nextPosition);
}

static void EnterBarrierWait(IntersectionEngine_t *engine,
                             uint8_t ringIndex,
                             uint8_t pendingPosition)
{
  IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];
  uint8_t phaseIndex = ringRuntime->activePhaseIndex;

  ringRuntime->pendingPosition = pendingPosition;
  ringRuntime->barrierWaiting = 1U;
  ringRuntime->stage = INTERSECTION_RING_STAGE_WAIT_BARRIER;
  ringRuntime->statusCode = INTERSECTION_RING_STATUS_RED_REST;
  ringRuntime->stageElapsedTicks = 0U;
  engine->runtime.phases[phaseIndex].interval = INTERSECTION_PHASE_INTERVAL_RED;
  engine->runtime.phases[phaseIndex].intervalElapsedTicks = 0U;
}

static uint8_t FindPreviousPositionInBarrierGroup(
  const IntersectionRingPlan_t *ringPlan,
  uint8_t currentPosition,
  uint8_t *previousPosition)
{
  uint8_t barrierGroup;
  uint8_t position;

  if ((ringPlan == NULL) || (currentPosition >= ringPlan->phaseCount))
  {
    return 0U;
  }

  barrierGroup = BarrierGroupForPosition(ringPlan, currentPosition);
  position = currentPosition;

  while (position > 0U)
  {
    position--;

    if (BarrierGroupForPosition(ringPlan, position) != barrierGroup)
    {
      break;
    }

    if (previousPosition != NULL)
    {
      *previousPosition = position;
    }

    return 1U;
  }

  return 0U;
}

static uint8_t ConditionalServiceWindowAvailable(IntersectionEngine_t *engine,
                                                 uint8_t ringIndex,
                                                 uint8_t phaseIndex)
{
  uint8_t otherRingIndex;
  uint32_t minimumServiceTicks = PhaseMinimumServiceTicks(engine, phaseIndex);

  for (otherRingIndex = 0U; otherRingIndex < engine->config.ringCount;
       otherRingIndex++)
  {
    IntersectionRingRuntime_t *otherRuntime;
    uint8_t otherPhaseIndex;
    uint32_t effectiveMaxTicks;

    if (otherRingIndex == ringIndex)
    {
      continue;
    }

    otherRuntime = &engine->runtime.rings[otherRingIndex];

    if ((otherRuntime->stage == INTERSECTION_RING_STAGE_WAIT_BARRIER)
        || (otherRuntime->stage == INTERSECTION_RING_STAGE_YELLOW)
        || (otherRuntime->stage == INTERSECTION_RING_STAGE_RED_CLEAR))
    {
      return 0U;
    }

    if (otherRuntime->stage != INTERSECTION_RING_STAGE_GREEN)
    {
      continue;
    }

    otherPhaseIndex = otherRuntime->activePhaseIndex;

    if (PhaseMaxTimingActive(engine, otherPhaseIndex) == 0U)
    {
      continue;
    }

    effectiveMaxTicks = EffectiveMaxGreenTicks(engine, otherPhaseIndex);

    if ((effectiveMaxTicks == UINT32_MAX)
        || (otherRuntime->stageElapsedTicks >= effectiveMaxTicks)
        || ((effectiveMaxTicks - otherRuntime->stageElapsedTicks)
            < minimumServiceTicks))
    {
      return 0U;
    }
  }

  return 1U;
}

static uint8_t FindConditionalServicePosition(IntersectionEngine_t *engine,
                                              uint8_t ringIndex,
                                              uint8_t *servicePosition)
{
  const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];
  const IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[
    ringIndex];
  const IntersectionPhaseConfig_t *phaseConfig = &engine->config.phases[
    ringRuntime->activePhaseIndex];
  uint8_t previousPosition;
  uint8_t previousPhaseIndex;

  if (((phaseConfig->phaseOptions & PHASE_OPTIONS_COND_SERVICE) == 0U)
      || ((ringRuntime->terminationReasonBits
           & (INTERSECTION_RING_TERMINATION_GAP_OUT
              | INTERSECTION_RING_TERMINATION_MAX_OUT)) == 0U)
      || (FindPreviousPositionInBarrierGroup(ringPlan,
                                             ringRuntime->activePosition,
                                             &previousPosition) == 0U))
  {
    return 0U;
  }

  previousPhaseIndex = ringPlan->phaseOrder[previousPosition];

  if ((PhaseHasVehicleServiceDemand(engine, previousPhaseIndex) == 0U)
      || (ConditionalServiceWindowAvailable(engine,
                                            ringIndex,
                                            previousPhaseIndex) == 0U))
  {
    return 0U;
  }

  if (servicePosition != NULL)
  {
    *servicePosition = previousPosition;
  }

  return 1U;
}

static uint8_t CanResumeGapFromBarrierWait(const IntersectionEngine_t *engine,
                                           uint8_t ringIndex)
{
  const IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[
    ringIndex];
  const IntersectionPhaseRuntime_t *phaseRuntime = &engine->runtime.phases[
    ringRuntime->activePhaseIndex];
  const IntersectionPhaseConfig_t *phaseConfig = &engine->config.phases[
    ringRuntime->activePhaseIndex];
  uint8_t otherRingIndex;

  if (((ringRuntime->terminationReasonBits & INTERSECTION_RING_TERMINATION_GAP_OUT)
       == 0U)
      || ((phaseConfig->phaseOptions & PHASE_OPTIONS_SIMUL_GAP_DISABLE) != 0U)
      || ((phaseRuntime->detectorActive == 0U)
          && (phaseRuntime->callLatched == 0U)))
  {
    return 0U;
  }

  for (otherRingIndex = 0U; otherRingIndex < engine->config.ringCount;
       otherRingIndex++)
  {
    if ((otherRingIndex != ringIndex)
        && (engine->runtime.rings[otherRingIndex].stage
            != INTERSECTION_RING_STAGE_WAIT_BARRIER))
    {
      return 1U;
    }
  }

  return 0U;
}

static void ResumeGapPhaseFromBarrierWait(IntersectionEngine_t *engine,
                                          uint8_t ringIndex)
{
  IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];
  uint8_t phaseIndex = ringRuntime->activePhaseIndex;

  ringRuntime->pendingPosition = ringRuntime->activePosition;
  ringRuntime->barrierWaiting = 0U;
  ringRuntime->stage = INTERSECTION_RING_STAGE_GREEN;
  ringRuntime->statusCode = INTERSECTION_RING_STATUS_EXTENSION;
  ringRuntime->terminationReasonBits = INTERSECTION_RING_TERMINATION_NONE;
  ringRuntime->stageElapsedTicks = (uint32_t) engine->minGreenTicks[phaseIndex];
  engine->runtime.phases[phaseIndex].interval =
    INTERSECTION_PHASE_INTERVAL_GREEN;
  engine->runtime.phases[phaseIndex].intervalElapsedTicks =
    ringRuntime->stageElapsedTicks;
  engine->runtime.phases[phaseIndex].callLatched = 0U;
  engine->gapTimerTicks[phaseIndex] = engine->currentGapTicks[phaseIndex];
  engine->passageTimerTicks[phaseIndex] = engine->passageTicks[phaseIndex];
}

static void TickBarrierWaitStage(IntersectionEngine_t *engine, uint8_t ringIndex)
{
  uint8_t conditionalPosition;
  IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];

  ringRuntime->stageElapsedTicks++;

  if (CanResumeGapFromBarrierWait(engine, ringIndex) != 0U)
  {
    ResumeGapPhaseFromBarrierWait(engine, ringIndex);

    return;
  }

  if (FindConditionalServicePosition(engine,
                                     ringIndex,
                                     &conditionalPosition) != 0U)
  {
    EnterGreen(engine, ringIndex, conditionalPosition);
  }
}

static void RefreshPhaseNextFlags(IntersectionEngine_t *engine)
{
  uint8_t phaseIndex;
  uint8_t ringIndex;

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    engine->runtime.phases[phaseIndex].next = 0U;
  }

  if (PreemptModeActive(engine) != 0U)
  {
    return;
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    const IntersectionRingRuntime_t *ringRuntime =
      &engine->runtime.rings[ringIndex];
    const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];
    uint8_t pendingPosition = ringRuntime->pendingPosition;

    if (ringRuntime->stage == INTERSECTION_RING_STAGE_GREEN)
    {
      pendingPosition = ResolveRequestedPosition(engine,
                                                 ringIndex,
                                                 ringRuntime->activePosition);
    }

    if ((pendingPosition < ringPlan->phaseCount)
        && (pendingPosition != ringRuntime->activePosition))
    {
      uint8_t pendingPhaseIndex = ringPlan->phaseOrder[pendingPosition];

      engine->runtime.phases[pendingPhaseIndex].next = 1U;
    }
  }
}

static void RefreshRingStatusCodes(IntersectionEngine_t *engine)
{
  uint8_t ringIndex;

  if (PreemptModeActive(engine) != 0U)
  {
    for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
    {
      engine->runtime.rings[ringIndex].statusCode =
        INTERSECTION_RING_STATUS_RED_REST;
      engine->runtime.rings[ringIndex].terminationReasonBits =
        INTERSECTION_RING_TERMINATION_NONE;
    }

    return;
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];

    switch (ringRuntime->stage)
    {
        case INTERSECTION_RING_STAGE_GREEN:
        {
          uint8_t phaseIndex = ringRuntime->activePhaseIndex;
          uint8_t nextPosition = ResolveRequestedPosition(engine,
                                                          ringIndex,
                                                          ringRuntime->
                                                          activePosition);
          uint32_t effectiveMaxTicks = EffectiveMaxGreenTicks(engine,
                                                              phaseIndex);

          if (ringRuntime->stageElapsedTicks
              < engine->initialGreenTicks[phaseIndex])
          {
            ringRuntime->statusCode = INTERSECTION_RING_STATUS_MIN_GREEN;
          }
          else if (nextPosition == ringRuntime->activePosition)
          {
            ringRuntime->statusCode = INTERSECTION_RING_STATUS_GREEN_REST;
          }
          else if ((PhaseMaxTimingActive(engine, phaseIndex) != 0U)
                   && (effectiveMaxTicks != UINT32_MAX)
                   && ((ringRuntime->stageElapsedTicks + 1U)
                       >= effectiveMaxTicks))
          {
            ringRuntime->statusCode = INTERSECTION_RING_STATUS_MAXIMUM;
          }
          else
          {
            ringRuntime->statusCode = INTERSECTION_RING_STATUS_EXTENSION;
          }

          break;
        }

        case INTERSECTION_RING_STAGE_YELLOW:
        {
          ringRuntime->statusCode = INTERSECTION_RING_STATUS_YELLOW_CHANGE;
          break;
        }

        case INTERSECTION_RING_STAGE_RED_CLEAR:
        {
          ringRuntime->statusCode = INTERSECTION_RING_STATUS_RED_CLEARANCE;
          break;
        }

        case INTERSECTION_RING_STAGE_WAIT_BARRIER:
        case INTERSECTION_RING_STAGE_RED_REST:
        default:
        {
          ringRuntime->statusCode = INTERSECTION_RING_STATUS_RED_REST;
          break;
        }
    } /* switch */
  }
} /* RefreshRingStatusCodes */

static void RefreshOverlapOutputs(IntersectionEngine_t *engine)
{
  uint8_t overlapIndex;

  if (PreemptModeActive(engine) != 0U)
  {
    return;
  }

  for (overlapIndex = 0U;
       overlapIndex < INTERSECTION_OVERLAP_COUNT_MAX;
       overlapIndex++)
  {
    const IntersectionOverlapConfig_t *overlap =
      &engine->config.overlaps[overlapIndex];
    IntersectionOutputAspect_t aspect = INTERSECTION_OUTPUT_ASPECT_RED;
    uint8_t listIndex;
    uint8_t anyIncludedGreen = 0U;
    uint8_t anyIncludedYellow = 0U;
    uint8_t anyIncludedYellowOrRedClear = 0U;
    uint8_t anyIncludedNext = 0U;
    uint8_t conflictingPedActive = 0U;

    if ((overlap->type != (uint8_t) INTERSECTION_OVERLAP_TYPE_NORMAL)
        || (overlap->includedPhases.length == 0U))
    {
      engine->runtime.overlaps[overlapIndex].aspect = aspect;
      continue;
    }

    for (listIndex = 0U;
         listIndex < overlap->includedPhases.length;
         listIndex++)
    {
      uint8_t phaseNumber = overlap->includedPhases.values[listIndex];
      uint8_t phaseIndex = (uint8_t) (phaseNumber - 1U);
      const IntersectionPhaseRuntime_t *phaseRuntime =
        &engine->runtime.phases[phaseIndex];

      if (phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_GREEN)
      {
        anyIncludedGreen = 1U;
      }

      if (phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_YELLOW)
      {
        anyIncludedYellow = 1U;
        anyIncludedYellowOrRedClear = 1U;
      }

      if (phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_RED_CLEAR)
      {
        anyIncludedYellowOrRedClear = 1U;
      }

      if (phaseRuntime->next != 0U)
      {
        anyIncludedNext = 1U;
      }
    }

    for (listIndex = 0U;
         listIndex < overlap->conflictingPedPhases.length;
         listIndex++)
    {
      uint8_t phaseNumber = overlap->conflictingPedPhases.values[listIndex];
      uint8_t phaseIndex = (uint8_t) (phaseNumber - 1U);
      const IntersectionPhaseRuntime_t *phaseRuntime =
        &engine->runtime.phases[phaseIndex];

      if ((phaseRuntime->pedInterval == INTERSECTION_PED_INTERVAL_WALK)
          || (phaseRuntime->pedInterval == INTERSECTION_PED_INTERVAL_CLEAR))
      {
        conflictingPedActive = 1U;
        break;
      }
    }

    if (conflictingPedActive != 0U)
    {
      aspect = INTERSECTION_OUTPUT_ASPECT_RED;
    }
    else if (anyIncludedGreen != 0U)
    {
      aspect = INTERSECTION_OUTPUT_ASPECT_GREEN;
    }
    else if ((anyIncludedYellowOrRedClear != 0U)
             && (anyIncludedNext != 0U))
    {
      aspect = INTERSECTION_OUTPUT_ASPECT_GREEN;
    }
    else if (anyIncludedYellow != 0U)
    {
      aspect = INTERSECTION_OUTPUT_ASPECT_YELLOW;
    }

    engine->runtime.overlaps[overlapIndex].aspect = aspect;
  }
} /* RefreshOverlapOutputs */

static uint8_t ChannelAspectShouldDim(const IntersectionEngine_t *engine,
                                      uint8_t channelIndex,
                                      IntersectionOutputAspect_t aspect)
{
  const IntersectionChannelConfig_t *channel;
  uint8_t mask;

  if ((engine == NULL) || (channelIndex >= INTERSECTION_CHANNEL_COUNT_MAX)
      || (engine->runtime.dimmingActive == 0U))
  {
    return 0U;
  }

  channel = &engine->config.channels[channelIndex];
  mask = channel->dimMask;

  switch (aspect)
  {
      case INTERSECTION_OUTPUT_ASPECT_RED:
      case INTERSECTION_OUTPUT_ASPECT_FLASH_RED:
      {
        return (uint8_t) ((mask & 0x04U) != 0U);
      }

      case INTERSECTION_OUTPUT_ASPECT_YELLOW:
      case INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW:
      {
        return (uint8_t) ((mask & 0x02U) != 0U);
      }

      case INTERSECTION_OUTPUT_ASPECT_GREEN:
      {
        return (uint8_t) ((mask & 0x01U) != 0U);
      }

      case INTERSECTION_OUTPUT_ASPECT_DARK:
      default:
      {
        return 0U;
      }
  }
}

static void RefreshChannelOutputs(IntersectionEngine_t *engine)
{
  uint8_t channelIndex;

  if (StartUpFlashActive(engine) != 0U)
  {
    for (channelIndex = 0U;
         channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
         channelIndex++)
    {
      const IntersectionChannelConfig_t *channel =
        &engine->config.channels[channelIndex];
      IntersectionOutputAspect_t aspect = INTERSECTION_OUTPUT_ASPECT_DARK;
      uint8_t dimmed;
      uint8_t dimAlternateHalfCycle;

      if (StartUpFlashUsesAutoFlashMode(engine) != 0U)
      {
        if ((channel->flashMask & 0x04U) != 0U)
        {
          aspect = INTERSECTION_OUTPUT_ASPECT_FLASH_RED;
        }
        else if ((channel->flashMask & 0x02U) != 0U)
        {
          aspect = INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW;
        }
      }
      else
      {
        aspect = INTERSECTION_OUTPUT_ASPECT_FLASH_RED;
      }

      dimmed = ChannelAspectShouldDim(engine, channelIndex, aspect);
      dimAlternateHalfCycle = (uint8_t) ((dimmed != 0U)
                                         && ((channel->dimMask & 0x08U) != 0U));
      engine->runtime.channels[channelIndex].aspect = aspect;
      engine->runtime.channels[channelIndex].dimmed = dimmed;
      engine->runtime.channels[channelIndex].dimAlternateHalfCycle =
        dimAlternateHalfCycle;
      engine->runtime.outputIntentImage.channels[channelIndex] = aspect;
      engine->runtime.outputIntentImage.channelDimmed[channelIndex] = dimmed;
      engine->runtime.outputIntentImage.channelDimAlternateHalfCycle[
        channelIndex] = dimAlternateHalfCycle;
    }

    return;
  }

  if (PreemptModeActive(engine) != 0U)
  {
    const IntersectionPreemptConfig_t *preempt =
      &engine->config.preempts[engine->activePreemptIndex];
    IntersectionPreemptState_t state =
      engine->runtime.preemptStates[engine->activePreemptIndex];
    uint8_t flashDwell = (uint8_t) ((state == INTERSECTION_PREEMPT_STATE_DWELL)
                                    && (PreemptFlashDwell(engine,
                                                          engine->
                                                          activePreemptIndex)
                                        != 0U));

    for (channelIndex = 0U;
         channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
         channelIndex++)
    {
      const IntersectionChannelConfig_t *channel =
        &engine->config.channels[channelIndex];
      IntersectionOutputAspect_t aspect = INTERSECTION_OUTPUT_ASPECT_RED;
      uint8_t dimmed;
      uint8_t dimAlternateHalfCycle;

      if ((state == INTERSECTION_PREEMPT_STATE_MAX_PRESENCE)
          && (PreemptAllRedFlashOnMaxPresence(engine,
                                              engine->activePreemptIndex)
              != 0U))
      {
        aspect = INTERSECTION_OUTPUT_ASPECT_FLASH_RED;
      }
      else if ((channel->controlSource != 0U)
               && (channel->controlSource <= engine->config.phaseCount)
               && ((IntersectionChannelControlType_t) channel->controlType
                   == INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE))
      {
        aspect = PhaseIntervalToAspect(
          engine->runtime.phases[channel->controlSource - 1U].interval);
      }
      else if ((channel->controlSource != 0U)
               && (channel->controlSource <= engine->config.phaseCount)
               && ((IntersectionChannelControlType_t) channel->controlType
                   == INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN))
      {
        aspect = PedIntervalToAspect(
          engine->runtime.phases[channel->controlSource - 1U].pedInterval);
      }
      else if ((channel->controlSource != 0U)
               && (channel->controlSource <= INTERSECTION_OVERLAP_COUNT_MAX)
               && ((IntersectionChannelControlType_t) channel->controlType
                   == INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP))
      {
        aspect = engine->runtime.overlaps[channel->controlSource - 1U].aspect;
      }
      else if (flashDwell != 0U)
      {
        aspect = INTERSECTION_OUTPUT_ASPECT_FLASH_RED;
      }

      dimmed = ChannelAspectShouldDim(engine, channelIndex, aspect);
      dimAlternateHalfCycle = (uint8_t) ((dimmed != 0U)
                                         && ((channel->dimMask & 0x08U) != 0U));
      engine->runtime.channels[channelIndex].aspect = aspect;
      engine->runtime.channels[channelIndex].dimmed = dimmed;
      engine->runtime.channels[channelIndex].dimAlternateHalfCycle =
        dimAlternateHalfCycle;
      engine->runtime.outputIntentImage.channels[channelIndex] = aspect;
      engine->runtime.outputIntentImage.channelDimmed[channelIndex] = dimmed;
      engine->runtime.outputIntentImage.channelDimAlternateHalfCycle[
        channelIndex] = dimAlternateHalfCycle;
    }

    (void) preempt;

    return;
  }

  for (channelIndex = 0U;
       channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    const IntersectionChannelConfig_t *channel =
      &engine->config.channels[channelIndex];
    IntersectionOutputAspect_t aspect = INTERSECTION_OUTPUT_ASPECT_DARK;
    uint8_t dimmed;
    uint8_t dimAlternateHalfCycle;

    if (AutomaticFlashOutputsActive(engine) != 0U)
    {
      if ((channel->flashMask & 0x04U) != 0U)
      {
        aspect = INTERSECTION_OUTPUT_ASPECT_FLASH_RED;
      }
      else if ((channel->flashMask & 0x02U) != 0U)
      {
        aspect = INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW;
      }

      dimmed = ChannelAspectShouldDim(engine, channelIndex, aspect);
      dimAlternateHalfCycle = (uint8_t) ((dimmed != 0U)
                                         && ((channel->dimMask & 0x08U) != 0U));
      engine->runtime.channels[channelIndex].aspect = aspect;
      engine->runtime.channels[channelIndex].dimmed = dimmed;
      engine->runtime.channels[channelIndex].dimAlternateHalfCycle =
        dimAlternateHalfCycle;
      engine->runtime.outputIntentImage.channels[channelIndex] = aspect;
      engine->runtime.outputIntentImage.channelDimmed[channelIndex] = dimmed;
      engine->runtime.outputIntentImage.channelDimAlternateHalfCycle[
        channelIndex] = dimAlternateHalfCycle;
      continue;
    }

    if (channel->controlSource != 0U)
    {
      switch ((IntersectionChannelControlType_t) channel->controlType)
      {
          case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE:
          {
            if (channel->controlSource <= engine->config.phaseCount)
            {
              aspect = PhaseIntervalToAspect(
                engine->runtime.phases[channel->controlSource - 1U].interval);
            }
            else
            {
              aspect = INTERSECTION_OUTPUT_ASPECT_RED;
            }

            break;
          }

          case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN:
          {
            if (channel->controlSource <= engine->config.phaseCount)
            {
              aspect = PedIntervalToAspect(
                engine->runtime.phases[channel->controlSource
                                       - 1U].pedInterval);
            }
            else
            {
              aspect = INTERSECTION_OUTPUT_ASPECT_RED;
            }

            break;
          }

          case INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP:
          {
            if (channel->controlSource <= INTERSECTION_OVERLAP_COUNT_MAX)
            {
              aspect = engine->runtime.overlaps[channel->controlSource
                                                - 1U].aspect;
            }
            else
            {
              aspect = INTERSECTION_OUTPUT_ASPECT_RED;
            }

            break;
          }

          case INTERSECTION_CHANNEL_CONTROL_TYPE_PED_OVERLAP:
          case INTERSECTION_CHANNEL_CONTROL_TYPE_QUEUE_JUMP:
          case INTERSECTION_CHANNEL_CONTROL_TYPE_OTHER:
          default:
          {
            aspect = INTERSECTION_OUTPUT_ASPECT_RED;
            break;
          }
      } /* switch */
    }

    dimmed = ChannelAspectShouldDim(engine, channelIndex, aspect);
    dimAlternateHalfCycle = (uint8_t) ((dimmed != 0U)
                                       && ((channel->dimMask & 0x08U) != 0U));
    engine->runtime.channels[channelIndex].aspect = aspect;
    engine->runtime.channels[channelIndex].dimmed = dimmed;
    engine->runtime.channels[channelIndex].dimAlternateHalfCycle =
      dimAlternateHalfCycle;
    engine->runtime.outputIntentImage.channels[channelIndex] = aspect;
    engine->runtime.outputIntentImage.channelDimmed[channelIndex] = dimmed;
    engine->runtime.outputIntentImage.channelDimAlternateHalfCycle[
      channelIndex] = dimAlternateHalfCycle;
  }
} /* RefreshChannelOutputs */

static void RefreshRuntimeViews(IntersectionEngine_t *engine)
{
  engine->runtime.remoteManualControlTimeout =
    engine->remoteManualControlTimeout;
  engine->runtime.remoteManualIntervalAdvance =
    engine->remoteManualIntervalAdvance;
  engine->runtime.specialFunctionControl = engine->specialFunctionControl;
  engine->runtime.specialFunctionStatus =
    (uint8_t) (engine->specialFunctionControl
               | TimebaseActionSpecialFunctionMask(engine));
  RefreshPhaseNextFlags(engine);
  RefreshRingStatusCodes(engine);
  RefreshOverlapOutputs(engine);
  RefreshChannelOutputs(engine);
}

static void TickGreenStage(IntersectionEngine_t *engine, uint8_t ringIndex)
{
  IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];
  uint8_t phaseIndex = ringRuntime->activePhaseIndex;
  uint8_t nextPosition;

  TickPhasePedState(engine, phaseIndex);

  ringRuntime->stageElapsedTicks++;
  engine->runtime.phases[phaseIndex].intervalElapsedTicks =
    ringRuntime->stageElapsedTicks;

  if (PhaseHasConflictingDemand(engine, phaseIndex) != 0U)
  {
    if ((engine->config.phases[phaseIndex].timeBeforeReductionSec > 0U)
        && (ringRuntime->stageElapsedTicks
            >= (uint32_t)
            engine->config.phases[phaseIndex].timeBeforeReductionSec * 100U))
    {
      engine->reductionActive[phaseIndex] = 1U;
    }

    if ((engine->config.phases[phaseIndex].carsBeforeReduction > 0U)
        && (engine->conflictingVehicleCountGreen[phaseIndex]
            >= engine->config.phases[phaseIndex].carsBeforeReduction))
    {
      engine->reductionActive[phaseIndex] = 1U;
    }

    if ((engine->reductionActive[phaseIndex] != 0U)
        && (engine->config.phases[phaseIndex].timeToReduceSec > 0U)
        && (engine->passageTicks[phaseIndex]
            > engine->minimumGapTicks[phaseIndex]))
    {
      uint32_t ttrTicks =
        (uint32_t) engine->config.phases[phaseIndex].timeToReduceSec * 100U;
      uint32_t span =
        (uint32_t) (engine->passageTicks[phaseIndex]
                    - engine->minimumGapTicks[phaseIndex]);

      if (engine->reductionElapsedTicks[phaseIndex] < ttrTicks)
      {
        engine->reductionElapsedTicks[phaseIndex]++;
      }

      if (engine->reductionElapsedTicks[phaseIndex] >= ttrTicks)
      {
        engine->currentGapTicks[phaseIndex] =
          engine->minimumGapTicks[phaseIndex];
      }
      else
      {
        uint32_t reduced = (span * engine->reductionElapsedTicks[phaseIndex])
                           / ttrTicks;

        engine->currentGapTicks[phaseIndex] =
          (uint16_t) (engine->passageTicks[phaseIndex] - reduced);
      }
    }
  }
  else
  {
    engine->reductionActive[phaseIndex] = 0U;
    engine->reductionElapsedTicks[phaseIndex] = 0U;
    engine->currentGapTicks[phaseIndex] = engine->passageTicks[phaseIndex];
  }

  if (engine->runtime.phases[phaseIndex].detectorActive != 0U)
  {
    engine->gapTimerTicks[phaseIndex] = engine->currentGapTicks[phaseIndex];
    engine->passageTimerTicks[phaseIndex] = engine->passageTicks[phaseIndex];
  }
  else
  {
    if (engine->gapTimerTicks[phaseIndex] > 0U)
    {
      engine->gapTimerTicks[phaseIndex]--;
    }

    if (engine->passageTimerTicks[phaseIndex] > 0U)
    {
      engine->passageTimerTicks[phaseIndex]--;
    }
  }

  nextPosition = ResolveRequestedPosition(engine,
                                          ringIndex,
                                          ringRuntime->activePosition);
  ringRuntime->pendingPosition = nextPosition;

  if (ringRuntime->stageElapsedTicks < engine->initialGreenTicks[phaseIndex])
  {
    return;
  }

  if (RemoteManualControlActive(engine) != 0U)
  {
    IntersectionPhaseRuntime_t *phaseRuntime =
      &engine->runtime.phases[phaseIndex];

    if ((engine->remoteManualPedAutoAdvance[ringIndex] != 0U)
        && (phaseRuntime->pedInterval == INTERSECTION_PED_INTERVAL_DONT_WALK))
    {
      engine->remoteManualPedAutoAdvance[ringIndex] = 0U;
      EnterYellow(engine, ringIndex, INTERSECTION_RING_TERMINATION_FORCE_OFF);

      return;
    }

    if (engine->remoteManualIntervalAdvance == 0U)
    {
      return;
    }

    if (phaseRuntime->pedInterval == INTERSECTION_PED_INTERVAL_WALK)
    {
      if (PhaseCurrentPedClearTicks(engine, phaseIndex) == 0U)
      {
        EndPhasePedService(engine, phaseIndex);
        EnterYellow(engine, ringIndex, INTERSECTION_RING_TERMINATION_FORCE_OFF);
      }
      else
      {
        StartPhasePedClear(engine, phaseIndex);

        if (engine->config.unit.autoPedestrianClear
            == (uint8_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_ENABLE)
        {
          engine->remoteManualPedAutoAdvance[ringIndex] = 1U;
        }
      }

      return;
    }

    if (phaseRuntime->pedServicePending != 0U)
    {
      return;
    }

    EnterYellow(engine, ringIndex, INTERSECTION_RING_TERMINATION_FORCE_OFF);

    return;
  }

  if (engine->runtime.phases[phaseIndex].pedInterval
      == INTERSECTION_PED_INTERVAL_WALK)
  {
    return;
  }

  if (engine->runtime.phases[phaseIndex].pedServicePending != 0U)
  {
    return;
  }

  if (engine->automaticFlashState
      == (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_ENTRY)
  {
    EnterYellow(engine, ringIndex, INTERSECTION_RING_TERMINATION_FORCE_OFF);

    return;
  }

  if (PhaseGuaranteedPassageActive(engine, phaseIndex) != 0U)
  {
    return;
  }

  if (PhaseSystemHoldActive(engine, phaseIndex) != 0U)
  {
    return;
  }

  if (PhaseSystemForceOffActive(engine, phaseIndex) != 0U)
  {
    EnterYellow(engine, ringIndex, INTERSECTION_RING_TERMINATION_FORCE_OFF);

    return;
  }

  if (RingSystemForceOffActive(engine, ringIndex) != 0U)
  {
    EnterYellow(engine, ringIndex, INTERSECTION_RING_TERMINATION_FORCE_OFF);

    return;
  }

  if ((nextPosition == ringRuntime->activePosition)
      && (PhaseHasDemand(engine, phaseIndex) == 0U)
      && (RingSystemRedRestActive(engine, ringIndex) != 0U))
  {
    EnterYellow(engine, ringIndex, INTERSECTION_RING_TERMINATION_GAP_OUT);

    return;
  }

  if (nextPosition == ringRuntime->activePosition)
  {
    return;
  }

  if (PhaseHasDemand(engine, phaseIndex) != 0U)
  {
    uint32_t effectiveMaxTicks = EffectiveMaxGreenTicks(engine, phaseIndex);

    if ((PhaseMaxTimingActive(engine, phaseIndex) == 0U)
        || (effectiveMaxTicks == UINT32_MAX)
        || (ringRuntime->stageElapsedTicks < effectiveMaxTicks))
    {
      return;
    }

    EnterYellow(engine, ringIndex, INTERSECTION_RING_TERMINATION_MAX_OUT);
  }
  else
  {
    EnterYellow(engine, ringIndex, INTERSECTION_RING_TERMINATION_GAP_OUT);
  }
} /* TickGreenStage */

static void TickYellowStage(IntersectionEngine_t *engine, uint8_t ringIndex)
{
  IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];
  uint8_t phaseIndex = ringRuntime->activePhaseIndex;

  TickPhasePedState(engine, phaseIndex);

  ringRuntime->stageElapsedTicks++;
  engine->runtime.phases[phaseIndex].intervalElapsedTicks =
    ringRuntime->stageElapsedTicks;

  if (ringRuntime->stageElapsedTicks >= engine->yellowTicks[phaseIndex])
  {
    if (RingSystemOmitRedClearActive(engine, ringIndex) != 0U)
    {
      StartPhaseRedRevertTimer(engine, phaseIndex);
      AdvanceAfterRedClear(engine, ringIndex);

      return;
    }

    EnterRedClear(engine, ringIndex);
  }
}

static void TickRedClearStage(IntersectionEngine_t *engine, uint8_t ringIndex)
{
  IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];
  uint8_t phaseIndex = ringRuntime->activePhaseIndex;

  TickPhasePedState(engine, phaseIndex);

  ringRuntime->stageElapsedTicks++;
  engine->runtime.phases[phaseIndex].intervalElapsedTicks =
    ringRuntime->stageElapsedTicks;

  if (ringRuntime->stageElapsedTicks < engine->redClearTicks[phaseIndex])
  {
    return;
  }

  AdvanceAfterRedClear(engine, ringIndex);
}

static uint8_t AllRingsReadyForBarrierCrossing(
  const IntersectionEngine_t *engine)
{
  uint8_t ringIndex;

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    if (engine->runtime.rings[ringIndex].stage
        != INTERSECTION_RING_STAGE_WAIT_BARRIER)
    {
      return 0U;
    }
  }

  return 1U;
}

static void CommitBarrierCrossing(IntersectionEngine_t *engine)
{
  uint8_t ringIndex;

  if (AllRingsReadyForBarrierCrossing(engine) == 0U)
  {
    return;
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    EnterGreen(engine,
               ringIndex,
               engine->runtime.rings[ringIndex].pendingPosition);
  }
}

static void TickControllerRings(IntersectionEngine_t *engine)
{
  uint8_t ringIndex;

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    if (RingSystemStopTimeActive(engine, ringIndex) != 0U)
    {
      continue;
    }

    switch (engine->runtime.rings[ringIndex].stage)
    {
        case INTERSECTION_RING_STAGE_GREEN:
        {
          TickGreenStage(engine, ringIndex);
          break;
        }

        case INTERSECTION_RING_STAGE_YELLOW:
        {
          TickYellowStage(engine, ringIndex);
          break;
        }

        case INTERSECTION_RING_STAGE_RED_CLEAR:
        {
          TickRedClearStage(engine, ringIndex);
          break;
        }

        case INTERSECTION_RING_STAGE_RED_REST:
        {
          if ((engine->startupPending[ringIndex] == 0U)
              && (engine->startupHold[ringIndex] == 0U)
              && (RingSystemRedRestActive(engine, ringIndex) != 0U))
          {
            break;
          }
          else if (engine->startupPending[ringIndex] != 0U)
          {
            ApplyRingStartupState(engine, ringIndex);
          }
          else if (engine->startupHold[ringIndex] != 0U)
          {
            if (RingHasDemand(engine, ringIndex) != 0U)
            {
              engine->startupHold[ringIndex] = 0U;
              EnterGreen(engine,
                         ringIndex,
                         FindFirstDemandPosition(engine, ringIndex));
            }
          }
          else
          {
            EnterGreen(engine,
                       ringIndex,
                       engine->runtime.rings[ringIndex].activePosition);
          }

          break;
        }

        case INTERSECTION_RING_STAGE_WAIT_BARRIER:
        {
          TickBarrierWaitStage(engine, ringIndex);
          break;
        }

        default:
        {
          break;
        }
    }
  }

  CommitBarrierCrossing(engine);
}

static void TickInactivePedStates(IntersectionEngine_t *engine)
{
  uint8_t phaseIndex;

  if (engine == NULL)
  {
    return;
  }

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    if ((engine->runtime.phases[phaseIndex].interval
         == INTERSECTION_PHASE_INTERVAL_RED)
        && (engine->runtime.phases[phaseIndex].pedServiceActive != 0U))
    {
      TickPhasePedState(engine, phaseIndex);
    }
  }
}

static void TryStartAdvancePedWalks(IntersectionEngine_t *engine)
{
  uint8_t ringIndex;

  if ((engine == NULL) || (PreemptModeActive(engine) != 0U)
      || (engine->runtime.mode == INTERSECTION_CONTROL_MODE_FLASH)
      || (RemoteManualControlActive(engine) != 0U))
  {
    return;
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    const IntersectionRingRuntime_t *ringRuntime =
      &engine->runtime.rings[ringIndex];
    const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];
    uint8_t pendingPosition = 0U;
    uint8_t pendingPhaseIndex;
    uint32_t ticksToGreen = 0U;
    uint16_t advanceTicks;

    if (GetRingDeterministicTicksToPendingGreen(engine,
                                                ringIndex,
                                                &pendingPosition,
                                                &ticksToGreen) == 0U)
    {
      continue;
    }

    pendingPhaseIndex = ringPlan->phaseOrder[pendingPosition];
    advanceTicks = PhasePedAdvanceTicks(engine, pendingPhaseIndex);

    if ((advanceTicks == 0U)
        || (PhaseAdvanceWalkReady(engine, pendingPhaseIndex) == 0U))
    {
      continue;
    }

    if (IsBarrierCrossing(engine,
                          ringIndex,
                          ringRuntime->activePosition,
                          pendingPosition) != 0U)
    {
      uint8_t otherRingIndex;

      for (otherRingIndex = 0U; otherRingIndex < engine->config.ringCount;
           otherRingIndex++)
      {
        const IntersectionRingRuntime_t *otherRingRuntime;
        uint8_t otherPendingPosition = 0U;
        uint32_t otherTicksToGreen = 0U;

        if (otherRingIndex == ringIndex)
        {
          continue;
        }

        if (GetRingDeterministicTicksToPendingGreen(engine,
                                                    otherRingIndex,
                                                    &otherPendingPosition,
                                                    &otherTicksToGreen) == 0U)
        {
          ticksToGreen = UINT32_MAX;
          break;
        }

        otherRingRuntime = &engine->runtime.rings[otherRingIndex];

        if (IsBarrierCrossing(engine,
                              otherRingIndex,
                              otherRingRuntime->activePosition,
                              otherPendingPosition) == 0U)
        {
          ticksToGreen = UINT32_MAX;
          break;
        }

        if (otherTicksToGreen > ticksToGreen)
        {
          ticksToGreen = otherTicksToGreen;
        }
      }
    }

    if (ticksToGreen <= advanceTicks)
    {
      StartPhasePedWalk(engine, pendingPhaseIndex);
    }
  }
}

void IntersectionEngineInit(IntersectionEngine_t *engine)
{
  if (engine != NULL)
  {
    memset(engine, 0, sizeof(*engine));
  }
}

uint8_t IntersectionEngineLoadConfig(IntersectionEngine_t *engine,
                                     const IntersectionConfig_t *config)
{
  IntersectionConfigErrorInfo_t errorInfo;
  uint8_t phaseIndex;
  uint8_t preemptIndex;

  if ((engine == NULL) || (config == NULL))
  {
    return 0U;
  }

  if (IntersectionConfigValidate(config, &errorInfo) == 0U)
  {
    return 0U;
  }

  IntersectionEngineInit(engine);
  engine->config = *config;
  engine->configLoaded = 1U;
  engine->systemPatternControl = 0U;
  engine->systemSyncControlSeconds = 0U;
  engine->actionPlanControl = 0U;
  engine->unitControl = 0U;
  engine->specialFunctionControl = 0U;
  engine->localInterconnectCommand = 0U;
  engine->localInterconnectInputsValid = 1U;
  engine->localDimmingInputActive = 0U;
  engine->automaticFlashState = (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_IDLE;
  engine->shortAlarmCycleZeroLatched = 0U;
  engine->shortAlarmCycleZeroActive = 0U;
  engine->coordDiagnosticCycleZeroActive = 0U;
  engine->coordinationAlarmPattern = 0U;
  engine->coordinationAlarmMissedCycles = 0U;
  engine->backupTimerArmed = 0U;
  engine->userDefinedBackupTimerArmed = 0U;
  engine->backupModeActive = 0U;
  engine->mmuFlashActive = 0U;
  engine->backupTimerTicksRemaining = 0U;
  engine->userDefinedBackupTimerTicksRemaining = 0U;
  engine->coordSyncTicks = 0U;
  engine->coordDiagnosticCycleTicks = 0U;
  engine->coordDiagnosticFaultPhaseMask = 0U;
  engine->coordDiagnosticRecoveryCyclesRemaining = 0U;
  engine->coordDiagnosticRetryCyclesRemaining = 0U;
  engine->pedWalkServiceCyclePattern = 0U;
  engine->activePreemptIndex = 0xFFU;
  engine->preemptStageTicks = 0U;
  engine->preemptPresenceTicks = 0U;
  memset(engine->phaseDemandWaitTicks, 0, sizeof(engine->phaseDemandWaitTicks));
  memset(engine->preemptShortServiceOrder,
         0xFF,
         sizeof(engine->preemptShortServiceOrder));
  engine->linkedPreemptSourceIndex = 0xFFU;
  engine->linkedPreemptTargetIndex = 0xFFU;
  engine->runtime.configLoaded = 1U;
  engine->runtime.mode = INTERSECTION_CONTROL_MODE_FREE;

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    engine->minGreenTicks[phaseIndex] =
      (uint16_t) (engine->config.phases[phaseIndex].minGreenDs * 10U);
    engine->minimumGapTicks[phaseIndex] =
      (uint16_t) (engine->config.phases[phaseIndex].minimumGapDs * 10U);
    engine->maxGreenTicks[phaseIndex] =
      (uint32_t) engine->config.phases[phaseIndex].maxGreenDs * 10U;
    engine->runningMaxTicks[phaseIndex] = engine->maxGreenTicks[phaseIndex];
    engine->selectedNormalMaxTicks[phaseIndex] =
      engine->maxGreenTicks[phaseIndex];
    engine->yellowTicks[phaseIndex] =
      (uint16_t) (engine->config.phases[phaseIndex].yellowChangeDs * 10U);
    engine->redClearTicks[phaseIndex] =
      (uint16_t) (engine->config.phases[phaseIndex].redClearDs * 10U);
    engine->redRevertTicks[phaseIndex] = 0U;
    engine->dontWalkRevertTicks[phaseIndex] = 0U;
    engine->pedClearClearanceTicks[phaseIndex] = 0U;
    engine->passageTicks[phaseIndex] =
      (uint16_t) (engine->config.phases[phaseIndex].passageDs * 10U);
    engine->passageTimerTicks[phaseIndex] = engine->passageTicks[phaseIndex];
    engine->currentGapTicks[phaseIndex] = engine->passageTicks[phaseIndex];
    engine->maxInitialTicks[phaseIndex] =
      (uint16_t) (engine->config.phases[phaseIndex].maxInitialDs * 10U);
    engine->pedWalkServicesThisCycle[phaseIndex] = 0U;
    engine->runtime.phases[phaseIndex].interval =
      INTERSECTION_PHASE_INTERVAL_RED;
  }

  for (preemptIndex = 0U;
       preemptIndex < INTERSECTION_PREEMPT_COUNT_MAX;
       preemptIndex++)
  {
    const IntersectionPreemptConfig_t *preempt =
      &engine->config.preempts[preemptIndex];

    engine->preemptDelayTicks[preemptIndex] =
      (uint16_t) ((uint32_t) preempt->delaySeconds * 100U);
    engine->preemptMinimumDurationTicks[preemptIndex] =
      (uint16_t) ((uint32_t) preempt->minimumDurationSeconds * 100U);
    engine->preemptMinimumGreenTicks[preemptIndex] =
      (uint16_t) ((uint32_t) preempt->minimumGreenSeconds * 100U);
    engine->preemptMinimumWalkTicks[preemptIndex] =
      (uint16_t) ((uint32_t) preempt->minimumWalkSeconds * 100U);
    engine->preemptEnterPedClearTicks[preemptIndex] =
      (uint16_t) ((uint32_t) preempt->enterPedClearSeconds * 100U);
    engine->preemptTrackGreenTicks[preemptIndex] =
      (uint16_t) ((uint32_t) preempt->trackGreenSeconds * 100U);
    engine->preemptDwellGreenTicks[preemptIndex] =
      (uint16_t) ((uint32_t) preempt->dwellGreenSeconds * 100U);
    engine->preemptMaximumPresenceTicks[preemptIndex] =
      (uint32_t) preempt->maximumPresenceSeconds * 100U;
    engine->preemptEnterYellowTicks[preemptIndex] =
      (uint16_t) ((uint32_t) preempt->enterYellowChangeDs * 10U);
    engine->preemptEnterRedClearTicks[preemptIndex] =
      (uint16_t) ((uint32_t) preempt->enterRedClearDs * 10U);
    engine->preemptTrackYellowTicks[preemptIndex] =
      (uint16_t) ((uint32_t) preempt->trackYellowChangeDs * 10U);
    engine->preemptTrackRedClearTicks[preemptIndex] =
      (uint16_t) ((uint32_t) preempt->trackRedClearDs * 10U);
  }

  RefreshAutomaticFlashPhasePositions(engine);

  IntersectionEngineReset(engine);

  return 1U;
} /* IntersectionEngineLoadConfig */

void IntersectionEngineReset(IntersectionEngine_t *engine)
{
  uint8_t ringIndex;
  uint8_t phaseIndex;

  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return;
  }

  memset(&engine->runtime, 0, sizeof(engine->runtime));
  memset(engine->directVehicleInputs, 0, sizeof(engine->directVehicleInputs));
  memset(engine->directPedInputs, 0, sizeof(engine->directPedInputs));
  memset(engine->systemPhaseOmit, 0, sizeof(engine->systemPhaseOmit));
  memset(engine->systemPedOmit, 0, sizeof(engine->systemPedOmit));
  memset(engine->systemPhaseHold, 0, sizeof(engine->systemPhaseHold));
  memset(engine->systemPhaseForceOff, 0, sizeof(engine->systemPhaseForceOff));
  memset(engine->systemVehCalls, 0, sizeof(engine->systemVehCalls));
  memset(engine->systemPedCalls, 0, sizeof(engine->systemPedCalls));
  memset(engine->systemRingStopTime, 0, sizeof(engine->systemRingStopTime));
  memset(engine->systemRingForceOff, 0, sizeof(engine->systemRingForceOff));
  memset(engine->systemRingMax2, 0, sizeof(engine->systemRingMax2));
  memset(engine->systemRingMaxInhibit, 0, sizeof(engine->systemRingMaxInhibit));
  memset(engine->systemRingPedRecycle, 0, sizeof(engine->systemRingPedRecycle));
  memset(engine->systemRingRedRest, 0, sizeof(engine->systemRingRedRest));
  memset(engine->systemRingOmitRedClear,
         0,
         sizeof(engine->systemRingOmitRedClear));
  memset(engine->systemRingMax3, 0, sizeof(engine->systemRingMax3));
  ResetCoordinationCycleFaultDiagnostics(engine);
  ResetCoordinationAlarmDiagnostics(engine);
  engine->coordSyncTicks = (uint32_t) engine->systemSyncControlSeconds * 100U;
  engine->coordDiagnosticCycleTicks = 0U;
  engine->automaticFlashState = (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_IDLE;
  engine->shortAlarmCycleZeroLatched = 0U;
  engine->shortAlarmCycleZeroActive = 0U;
  engine->coordDiagnosticCycleZeroActive = 0U;
  engine->backupTimerArmed = 0U;
  engine->userDefinedBackupTimerArmed = 0U;
  engine->backupModeActive = 0U;
  engine->startUpFlashTicksRemaining =
    (uint32_t) engine->config.unit.startUpFlashSeconds * 100U;
  engine->backupTimerTicksRemaining = 0U;
  engine->userDefinedBackupTimerTicksRemaining = 0U;
  engine->remoteManualControlTimeout = 0U;
  engine->remoteManualIntervalAdvance = 0U;
  engine->remoteManualTickAccumulator = 0U;
  memset(engine->remoteManualPedAutoAdvance,
         0,
         sizeof(engine->remoteManualPedAutoAdvance));
  engine->activePreemptIndex = 0xFFU;
  engine->preemptStageTicks = 0U;
  engine->preemptPresenceTicks = 0U;
  ResetPreemptCyclingRuntime(engine);
  ClearLinkedPreemptCall(engine);
  memset(engine->preemptEntryPhaseIntervals,
         0,
         sizeof(engine->preemptEntryPhaseIntervals));
  memset(engine->preemptEntryPedIntervals,
         0,
         sizeof(engine->preemptEntryPedIntervals));
  memset(engine->preemptEntryPhaseElapsedTicks,
         0,
         sizeof(engine->preemptEntryPhaseElapsedTicks));
  memset(engine->preemptEntryPedElapsedTicks,
         0,
         sizeof(engine->preemptEntryPedElapsedTicks));
  memset(engine->phaseDemandWaitTicks, 0, sizeof(engine->phaseDemandWaitTicks));
  memset(engine->preemptShortServiceOrder,
         0xFF,
         sizeof(engine->preemptShortServiceOrder));
  engine->runtime.configLoaded = 1U;
  engine->runtime.mode = INTERSECTION_CONTROL_MODE_FREE;
  engine->runtime.coordPatternStatus = 254U;
  engine->runtime.localFreeStatus =
    (uint8_t) INTERSECTION_LOCAL_FREE_STATUS_OTHER;
  engine->runtime.systemPatternControl = engine->systemPatternControl;
  engine->runtime.systemSyncControlSeconds = engine->systemSyncControlSeconds;
  engine->runtime.actionPlanControl = engine->actionPlanControl;
  engine->runtime.timebaseActionStatus = 0U;
  engine->runtime.timebaseAuxiliaryFunctionStatus = 0U;
  engine->runtime.interconnectCommand = engine->localInterconnectCommand;
  engine->runtime.interconnectInputsValid = engine->localInterconnectInputsValid;
  engine->runtime.coordCycleFaultActive = 0U;
  engine->runtime.coordFaultActive = 0U;
  engine->runtime.coordFailActive = 0U;
  engine->runtime.cycleFailActive = 0U;
  engine->runtime.coordinationAlarmActive = 0U;
  engine->runtime.unitControlStatus =
    (uint8_t) INTERSECTION_UNIT_CONTROL_STATUS_OTHER;
  engine->runtime.unitControl = engine->unitControl;
  engine->runtime.remoteManualControlTimeout = 0U;
  engine->runtime.remoteManualIntervalAdvance = 0U;
  engine->runtime.specialFunctionControl = engine->specialFunctionControl;
  engine->runtime.specialFunctionStatus = engine->specialFunctionControl;
  engine->runtime.backupModeActive = 0U;
  engine->runtime.startUpFlashActive = StartUpFlashActive(engine);
  engine->runtime.localDimmingInputActive = engine->localDimmingInputActive;
  engine->runtime.dimmingActive = 0U;
  engine->runtime.mmuFlashActive = engine->mmuFlashActive;
  SetPreemptStateDefaults(engine);

  for (phaseIndex = 0U; phaseIndex < engine->config.phaseCount; phaseIndex++)
  {
    engine->runtime.phases[phaseIndex].interval =
      INTERSECTION_PHASE_INTERVAL_RED;
    engine->passageTimerTicks[phaseIndex] = engine->passageTicks[phaseIndex];
    engine->gapTimerTicks[phaseIndex] = engine->passageTicks[phaseIndex];
    engine->currentGapTicks[phaseIndex] = engine->passageTicks[phaseIndex];
    engine->initialGreenTicks[phaseIndex] = PhaseInitialGreenTicks(engine,
                                                                   phaseIndex);
    engine->initialActuationCount[phaseIndex] = 0U;
    engine->vehicleCountGreen[phaseIndex] = 0U;
    engine->conflictingVehicleCountGreen[phaseIndex] = 0U;
    engine->reductionElapsedTicks[phaseIndex] = 0U;
    engine->reductionActive[phaseIndex] = 0U;
    engine->runningMaxTicks[phaseIndex] = BaseMaximumGreenTicks(engine,
                                                                phaseIndex);
    engine->selectedNormalMaxTicks[phaseIndex] = engine->runningMaxTicks[
      phaseIndex];
    engine->dynamicMaxGapOutCount[phaseIndex] = 0U;
    engine->dynamicMaxMaxOutCount[phaseIndex] = 0U;
  }

  for (ringIndex = 0U; ringIndex < engine->config.ringCount; ringIndex++)
  {
    const IntersectionRingPlan_t *ringPlan = &engine->config.rings[ringIndex];
    IntersectionRingRuntime_t *ringRuntime = &engine->runtime.rings[ringIndex];

    ringRuntime->activePosition = 0U;
    ringRuntime->pendingPosition = 0U;
    ringRuntime->activePhaseIndex = ringPlan->phaseOrder[0];
    ringRuntime->barrierWaiting = 0U;
    ringRuntime->stage = INTERSECTION_RING_STAGE_RED_REST;
    ringRuntime->statusCode = INTERSECTION_RING_STATUS_RED_REST;
    ringRuntime->terminationReasonBits = INTERSECTION_RING_TERMINATION_NONE;
    ringRuntime->stageElapsedTicks = 0U;
    engine->startupPending[ringIndex] = 1U;
    engine->startupHold[ringIndex] = 0U;
  }

  UpdateCoordinationRuntime(engine);
  RefreshRuntimeViews(engine);
} /* IntersectionEngineReset */

void IntersectionEngineTick(IntersectionEngine_t *engine)
{
  IntersectionControlMode_t previousMode;

  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return;
  }

  engine->runtime.monotonicTicks++;
  previousMode = engine->runtime.mode;
  RefreshDetectorDerivedInputs(engine);
  TickStartUpFlashTimer(engine);
  TickPhaseRedRevertTimers(engine);
  TickPhaseDontWalkRevertTimers(engine);
  TickBackupTimer(engine);
  TickUserDefinedBackupTimer(engine);
  TickRemoteManualControlTimer(engine);

  UpdateCoordinationRuntime(engine);

  if ((engine->coordDiagnosticCycleTicks != 0U)
      && ((CoordinationPatternIsActive(engine) != 0U)
          || (engine->runtime.coordCycleFaultActive != 0U)
          || (engine->runtime.coordFaultActive != 0U)
          || (engine->runtime.coordFailActive != 0U)
          || (engine->runtime.cycleFailActive != 0U)))
  {
    engine->coordSyncTicks = (engine->coordSyncTicks + 1U)
                             % engine->coordDiagnosticCycleTicks;
    UpdateCoordinationRuntime(engine);
  }

  RefreshPhaseDemandWaitTimes(engine);

  if (HandlePreemptStateMachine(engine) != 0U)
  {
    RefreshRuntimeViews(engine);
    FinalizeRemoteManualAdvanceCommand(engine);

    return;
  }

  if ((engine->automaticFlashState
       == (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_FLASHING)
      && (engine->runtime.mode != INTERSECTION_CONTROL_MODE_FLASH))
  {
    StartAutomaticFlashExit(engine);
    engine->automaticFlashState =
      (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_IDLE;
    RefreshRuntimeViews(engine);
    FinalizeRemoteManualAdvanceCommand(engine);

    return;
  }

  if (engine->runtime.mode == INTERSECTION_CONTROL_MODE_FLASH)
  {
    if (engine->mmuFlashActive != 0U)
    {
      engine->automaticFlashState =
        (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_IDLE;
      ForceControllerRedRest(engine);
      RefreshRuntimeViews(engine);
      FinalizeRemoteManualAdvanceCommand(engine);

      return;
    }

    if (AutomaticFlashTransitionConfigured(engine) != 0U)
    {
      if (engine->automaticFlashState
          == (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_IDLE)
      {
        engine->automaticFlashState =
          (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_ENTRY;
      }

      if (engine->automaticFlashState
          == (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_ENTRY)
      {
        TickControllerRings(engine);

        if (AutomaticFlashEntryCompleted(engine) != 0U)
        {
          engine->automaticFlashState =
            (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_FLASHING;
          ForceControllerRedRest(engine);
        }

        RefreshRuntimeViews(engine);
        FinalizeRemoteManualAdvanceCommand(engine);

        return;
      }
    }

    ForceControllerRedRest(engine);
    RefreshRuntimeViews(engine);
    FinalizeRemoteManualAdvanceCommand(engine);

    return;
  }

  if (previousMode == INTERSECTION_CONTROL_MODE_FLASH)
  {
    engine->automaticFlashState =
      (uint8_t) INTERSECTION_AUTOMATIC_FLASH_STATE_IDLE;
    ForceControllerRedRest(engine);
  }

  TickControllerRings(engine);
  TickInactivePedStates(engine);
  TryStartAdvancePedWalks(engine);
  RefreshRuntimeViews(engine);
  FinalizeRemoteManualAdvanceCommand(engine);
} /* IntersectionEngineTick */

uint8_t IntersectionEngineSetDetectorCall(IntersectionEngine_t *engine,
                                          uint8_t phaseNumber,
                                          uint8_t active)
{
  uint8_t phaseIndex;
  uint8_t wasActive;

  if ((engine == NULL) || (engine->configLoaded == 0U) || (phaseNumber == 0U))
  {
    return 0U;
  }

  phaseIndex = (uint8_t) (phaseNumber - 1U);

  if (phaseIndex >= engine->config.phaseCount)
  {
    return 0U;
  }

  wasActive = engine->directVehicleInputs[phaseIndex];
  engine->directVehicleInputs[phaseIndex] = (uint8_t) (active != 0U);

  if (active != 0U)
  {
    if ((wasActive == 0U)
        && (engine->runtime.phases[phaseIndex].interval
            != INTERSECTION_PHASE_INTERVAL_GREEN)
        && (engine->initialActuationCount[phaseIndex] < 0xFFFFU))
    {
      engine->initialActuationCount[phaseIndex]++;
    }

    if ((engine->runtime.phases[phaseIndex].interval
         != INTERSECTION_PHASE_INTERVAL_GREEN)
        || (PhaseUsesDetectorOptionBasedLocking(engine, phaseIndex) != 0U))
    {
      engine->runtime.phases[phaseIndex].callLatched = 1U;
    }

    if (engine->runtime.phases[phaseIndex].interval
        == INTERSECTION_PHASE_INTERVAL_GREEN)
    {
      engine->passageTimerTicks[phaseIndex] = engine->passageTicks[phaseIndex];
      engine->gapTimerTicks[phaseIndex] = engine->currentGapTicks[phaseIndex];

      if ((wasActive == 0U) && (engine->vehicleCountGreen[phaseIndex] < 0xFFFFU))
      {
        engine->vehicleCountGreen[phaseIndex]++;
      }
    }

    if ((wasActive == 0U)
        && (engine->runtime.phases[phaseIndex].interval
            != INTERSECTION_PHASE_INTERVAL_GREEN)
        && (PhaseHasVehicleServiceDemand(engine, phaseIndex) != 0U))
    {
      uint8_t activePhaseIndex;

      for (activePhaseIndex = 0U;
           activePhaseIndex < engine->config.phaseCount;
           activePhaseIndex++)
      {
        if ((activePhaseIndex == phaseIndex)
            || (engine->runtime.phases[activePhaseIndex].interval
                != INTERSECTION_PHASE_INTERVAL_GREEN)
            || (PhasesMayRunConcurrently(engine,
                                         activePhaseIndex,
                                         phaseIndex) != 0U))
        {
          continue;
        }

        if (engine->conflictingVehicleCountGreen[activePhaseIndex] < 0xFFFFU)
        {
          engine->conflictingVehicleCountGreen[activePhaseIndex]++;
        }
      }
    }
  }

  RefreshDetectorDerivedInputs(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetVehicleDetectorInput(IntersectionEngine_t *engine,
                                                  uint8_t detectorNumber,
                                                  uint8_t active)
{
  uint8_t detectorIndex;

  if ((engine == NULL) || (engine->configLoaded == 0U) || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  detectorIndex = (uint8_t) (detectorNumber - 1U);
  engine->runtime.vehicleDetectors[detectorIndex].inputActive =
    (uint8_t) (active != 0U);
  RefreshDetectorDerivedInputs(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetVehicleDetectorRemoteActuation(
  IntersectionEngine_t *engine,
  uint8_t detectorNumber,
  uint8_t active)
{
  uint8_t detectorIndex;

  if ((engine == NULL) || (engine->configLoaded == 0U) || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  detectorIndex = (uint8_t) (detectorNumber - 1U);
  engine->runtime.vehicleDetectors[detectorIndex].remoteActuation =
    (uint8_t) (active != 0U);
  ResetBackupTimer(engine);
  RefreshDetectorDerivedInputs(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetPedCall(IntersectionEngine_t *engine,
                                     uint8_t phaseNumber,
                                     uint8_t active)
{
  uint8_t phaseIndex;

  if ((engine == NULL) || (engine->configLoaded == 0U) || (phaseNumber == 0U))
  {
    return 0U;
  }

  phaseIndex = (uint8_t) (phaseNumber - 1U);

  if (phaseIndex >= engine->config.phaseCount)
  {
    return 0U;
  }

  engine->directPedInputs[phaseIndex] = (uint8_t) (active != 0U);

  if (active != 0U)
  {
    engine->runtime.phases[phaseIndex].pedCallLatched = 1U;
    engine->runtime.phases[phaseIndex].pedServicePending = 1U;
  }

  RefreshDetectorDerivedInputs(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetPedestrianDetectorInput(
  IntersectionEngine_t *engine,
  uint8_t detectorNumber,
  uint8_t active)
{
  uint8_t detectorIndex;

  if ((engine == NULL) || (engine->configLoaded == 0U) || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  detectorIndex = (uint8_t) (detectorNumber - 1U);
  engine->runtime.pedestrianDetectors[detectorIndex].inputActive =
    (uint8_t) (active != 0U);
  RefreshDetectorDerivedInputs(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetPedestrianDetectorRemoteActuation(
  IntersectionEngine_t *engine,
  uint8_t detectorNumber,
  uint8_t active)
{
  uint8_t detectorIndex;

  if ((engine == NULL) || (engine->configLoaded == 0U) || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  detectorIndex = (uint8_t) (detectorNumber - 1U);
  engine->runtime.pedestrianDetectors[detectorIndex].remoteActuation =
    (uint8_t) (active != 0U);
  ResetBackupTimer(engine);
  RefreshDetectorDerivedInputs(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetSystemPatternControl(IntersectionEngine_t *engine,
                                                  uint8_t systemPatternControl)
{
  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  engine->systemPatternControl = systemPatternControl;
  engine->runtime.systemPatternControl = systemPatternControl;
  ResetBackupTimer(engine);
  UpdateCoordinationRuntime(engine);

  if (AutomaticFlashOutputsActive(engine) != 0U)
  {
    ForceControllerRedRest(engine);
  }

  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetSystemSyncControl(IntersectionEngine_t *engine,
                                               uint8_t systemSyncControlSeconds)
{
  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  engine->systemSyncControlSeconds = systemSyncControlSeconds;
  engine->runtime.systemSyncControlSeconds = systemSyncControlSeconds;
  ResetBackupTimer(engine);

  if (systemSyncControlSeconds != 255U)
  {
    engine->coordSyncTicks = (uint32_t) systemSyncControlSeconds * 100U;
  }

  UpdateCoordinationRuntime(engine);

  if (AutomaticFlashOutputsActive(engine) != 0U)
  {
    ForceControllerRedRest(engine);
  }

  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetActionPlanControl(IntersectionEngine_t *engine,
                                               uint8_t actionPlanControl)
{
  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  engine->actionPlanControl = actionPlanControl;
  engine->runtime.actionPlanControl = actionPlanControl;
  ResetBackupTimer(engine);
  UpdateCoordinationRuntime(engine);

  if (AutomaticFlashOutputsActive(engine) != 0U)
  {
    ForceControllerRedRest(engine);
  }

  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineGetActionPlanControl(const IntersectionEngine_t *engine,
                                               uint8_t *actionPlanControl)
{
  if ((engine == NULL) || (actionPlanControl == NULL)
      || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  *actionPlanControl = engine->actionPlanControl;

  return 1U;
}

uint8_t IntersectionEngineSetUnitControl(IntersectionEngine_t *engine,
                                         uint8_t unitControl)
{
  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  engine->unitControl = unitControl;
  engine->runtime.unitControl = unitControl;
  ResetBackupTimer(engine);
  UpdateCoordinationRuntime(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineGetUnitControl(const IntersectionEngine_t *engine,
                                         uint8_t *unitControl)
{
  if ((engine == NULL) || (unitControl == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  *unitControl = engine->unitControl;

  return 1U;
}

uint8_t IntersectionEngineSetRemoteManualControlTimeout(
  IntersectionEngine_t *engine,
  uint8_t timeoutSeconds)
{
  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  if (timeoutSeconds == 0U)
  {
    ClearRemoteManualControl(engine);
  }
  else
  {
    engine->remoteManualControlTimeout = timeoutSeconds;
    engine->remoteManualIntervalAdvance = 0U;
    memset(engine->remoteManualPedAutoAdvance,
           0,
           sizeof(engine->remoteManualPedAutoAdvance));
    engine->remoteManualTickAccumulator = 100U;
  }

  ResetBackupTimer(engine);
  UpdateCoordinationRuntime(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineGetRemoteManualControlTimeout(
  const IntersectionEngine_t *engine,
  uint8_t *timeoutSeconds)
{
  if ((engine == NULL) || (timeoutSeconds == NULL)
      || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  *timeoutSeconds = engine->remoteManualControlTimeout;

  return 1U;
}

uint8_t IntersectionEngineSetRemoteManualIntervalAdvance(
  IntersectionEngine_t *engine,
  uint8_t active)
{
  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  if (active == 0U)
  {
    engine->remoteManualIntervalAdvance = 0U;
    engine->runtime.remoteManualIntervalAdvance = 0U;

    return 1U;
  }

  if (RemoteManualControlActive(engine) == 0U)
  {
    return 0U;
  }

  engine->remoteManualIntervalAdvance = 1U;
  engine->runtime.remoteManualIntervalAdvance = 1U;
  ResetBackupTimer(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineGetRemoteManualIntervalAdvance(
  const IntersectionEngine_t *engine,
  uint8_t *active)
{
  if ((engine == NULL) || (active == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  *active = (uint8_t) (engine->remoteManualIntervalAdvance != 0U);

  return 1U;
}

uint8_t IntersectionEngineResetUserDefinedBackupTimer(
  IntersectionEngine_t *engine)
{
  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  ResetUserDefinedBackupTimer(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetLocalInterconnectCommand(
  IntersectionEngine_t *engine,
  uint8_t command)
{
  if ((engine == NULL) || (engine->configLoaded == 0U)
      || (PatternCommandValid(command) == 0U))
  {
    return 0U;
  }

  if (engine->localInterconnectCommand == command)
  {
    return 1U;
  }

  engine->localInterconnectCommand = command;
  engine->runtime.interconnectCommand = command;
  UpdateCoordinationRuntime(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetLocalInterconnectInputsValid(
  IntersectionEngine_t *engine,
  uint8_t valid)
{
  uint8_t normalizedValid;

  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  normalizedValid = (uint8_t) (valid != 0U);

  if (engine->localInterconnectInputsValid == normalizedValid)
  {
    return 1U;
  }

  engine->localInterconnectInputsValid = normalizedValid;
  engine->runtime.interconnectInputsValid = normalizedValid;
  UpdateCoordinationRuntime(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetLocalDimmingInput(IntersectionEngine_t *engine,
                                               uint8_t active)
{
  uint8_t normalizedActive;

  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  normalizedActive = (uint8_t) (active != 0U);

  if (engine->localDimmingInputActive == normalizedActive)
  {
    return 1U;
  }

  engine->localDimmingInputActive = normalizedActive;
  engine->runtime.localDimmingInputActive = normalizedActive;
  UpdateCoordinationRuntime(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

static uint8_t ValidatePhaseControlAccess(const IntersectionEngine_t *engine,
                                          uint8_t phaseNumber,
                                          uint8_t *phaseIndex)
{
  if ((engine == NULL) || (engine->configLoaded == 0U) || (phaseNumber == 0U))
  {
    return 0U;
  }

  if (phaseNumber > engine->config.phaseCount)
  {
    return 0U;
  }

  if (phaseIndex != NULL)
  {
    *phaseIndex = (uint8_t) (phaseNumber - 1U);
  }

  return 1U;
}

static uint8_t ValidateRingControlAccess(const IntersectionEngine_t *engine,
                                         uint8_t ringNumber,
                                         uint8_t *ringIndex)
{
  if ((engine == NULL) || (engine->configLoaded == 0U) || (ringNumber == 0U))
  {
    return 0U;
  }

  if (ringNumber > engine->config.ringCount)
  {
    return 0U;
  }

  if (ringIndex != NULL)
  {
    *ringIndex = (uint8_t) (ringNumber - 1U);
  }

  return 1U;
}

static uint8_t SetRingControlState(IntersectionEngine_t *engine,
                                   uint8_t ringNumber,
                                   uint8_t active,
                                   uint8_t *controls)
{
  uint8_t ringIndex;

  if ((controls == NULL)
      || (ValidateRingControlAccess(engine, ringNumber, &ringIndex) == 0U))
  {
    return 0U;
  }

  controls[ringIndex] = (uint8_t) (active != 0U);
  ResetBackupTimer(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

static uint8_t GetRingControlState(const IntersectionEngine_t *engine,
                                   uint8_t ringNumber,
                                   const uint8_t *controls,
                                   uint8_t *active)
{
  uint8_t ringIndex;

  if ((controls == NULL) || (active == NULL)
      || (ValidateRingControlAccess(engine, ringNumber, &ringIndex) == 0U))
  {
    return 0U;
  }

  *active = controls[ringIndex];

  return 1U;
}

uint8_t IntersectionEngineSetPhaseOmitControl(IntersectionEngine_t *engine,
                                              uint8_t phaseNumber,
                                              uint8_t active)
{
  uint8_t phaseIndex;

  if (ValidatePhaseControlAccess(engine, phaseNumber, &phaseIndex) == 0U)
  {
    return 0U;
  }

  engine->systemPhaseOmit[phaseIndex] = (uint8_t) (active != 0U);
  ResetBackupTimer(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineGetPhaseOmitControl(const IntersectionEngine_t *engine,
                                              uint8_t phaseNumber,
                                              uint8_t *active)
{
  uint8_t phaseIndex;

  if ((active == NULL)
      || (ValidatePhaseControlAccess(engine, phaseNumber, &phaseIndex) == 0U))
  {
    return 0U;
  }

  *active = engine->systemPhaseOmit[phaseIndex];

  return 1U;
}

uint8_t IntersectionEngineSetPedOmitControl(IntersectionEngine_t *engine,
                                            uint8_t phaseNumber,
                                            uint8_t active)
{
  uint8_t phaseIndex;

  if (ValidatePhaseControlAccess(engine, phaseNumber, &phaseIndex) == 0U)
  {
    return 0U;
  }

  engine->systemPedOmit[phaseIndex] = (uint8_t) (active != 0U);
  ResetBackupTimer(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineGetPedOmitControl(const IntersectionEngine_t *engine,
                                            uint8_t phaseNumber,
                                            uint8_t *active)
{
  uint8_t phaseIndex;

  if ((active == NULL)
      || (ValidatePhaseControlAccess(engine, phaseNumber, &phaseIndex) == 0U))
  {
    return 0U;
  }

  *active = engine->systemPedOmit[phaseIndex];

  return 1U;
}

uint8_t IntersectionEngineSetPhaseHoldControl(IntersectionEngine_t *engine,
                                              uint8_t phaseNumber,
                                              uint8_t active)
{
  uint8_t phaseIndex;

  if (ValidatePhaseControlAccess(engine, phaseNumber, &phaseIndex) == 0U)
  {
    return 0U;
  }

  engine->systemPhaseHold[phaseIndex] = (uint8_t) (active != 0U);
  ResetBackupTimer(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineGetPhaseHoldControl(const IntersectionEngine_t *engine,
                                              uint8_t phaseNumber,
                                              uint8_t *active)
{
  uint8_t phaseIndex;

  if ((active == NULL)
      || (ValidatePhaseControlAccess(engine, phaseNumber, &phaseIndex) == 0U))
  {
    return 0U;
  }

  *active = engine->systemPhaseHold[phaseIndex];

  return 1U;
}

uint8_t IntersectionEngineSetPhaseForceOffControl(IntersectionEngine_t *engine,
                                                  uint8_t phaseNumber,
                                                  uint8_t active)
{
  uint8_t phaseIndex;

  if (ValidatePhaseControlAccess(engine, phaseNumber, &phaseIndex) == 0U)
  {
    return 0U;
  }

  engine->systemPhaseForceOff[phaseIndex] = (uint8_t) (active != 0U);
  ResetBackupTimer(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineGetPhaseForceOffControl(
  const IntersectionEngine_t *engine,
  uint8_t phaseNumber,
  uint8_t *active)
{
  uint8_t phaseIndex;

  if ((active == NULL)
      || (ValidatePhaseControlAccess(engine, phaseNumber, &phaseIndex) == 0U))
  {
    return 0U;
  }

  *active = engine->systemPhaseForceOff[phaseIndex];

  return 1U;
}

uint8_t IntersectionEngineSetVehCallControl(IntersectionEngine_t *engine,
                                            uint8_t phaseNumber,
                                            uint8_t active)
{
  uint8_t phaseIndex;

  if (ValidatePhaseControlAccess(engine, phaseNumber, &phaseIndex) == 0U)
  {
    return 0U;
  }

  engine->systemVehCalls[phaseIndex] = (uint8_t) (active != 0U);
  ResetBackupTimer(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineGetVehCallControl(const IntersectionEngine_t *engine,
                                            uint8_t phaseNumber,
                                            uint8_t *active)
{
  uint8_t phaseIndex;

  if ((active == NULL)
      || (ValidatePhaseControlAccess(engine, phaseNumber, &phaseIndex) == 0U))
  {
    return 0U;
  }

  *active = engine->systemVehCalls[phaseIndex];

  return 1U;
}

uint8_t IntersectionEngineSetPedCallControl(IntersectionEngine_t *engine,
                                            uint8_t phaseNumber,
                                            uint8_t active)
{
  uint8_t phaseIndex;

  if (ValidatePhaseControlAccess(engine, phaseNumber, &phaseIndex) == 0U)
  {
    return 0U;
  }

  engine->systemPedCalls[phaseIndex] = (uint8_t) (active != 0U);
  ResetBackupTimer(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineGetPedCallControl(const IntersectionEngine_t *engine,
                                            uint8_t phaseNumber,
                                            uint8_t *active)
{
  uint8_t phaseIndex;

  if ((active == NULL)
      || (ValidatePhaseControlAccess(engine, phaseNumber, &phaseIndex) == 0U))
  {
    return 0U;
  }

  *active = engine->systemPedCalls[phaseIndex];

  return 1U;
}

uint8_t IntersectionEngineSetRingStopTimeControl(IntersectionEngine_t *engine,
                                                 uint8_t ringNumber,
                                                 uint8_t active)
{
  return SetRingControlState(engine,
                             ringNumber,
                             active,
                             engine->systemRingStopTime);
}

uint8_t IntersectionEngineGetRingStopTimeControl(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active)
{
  return GetRingControlState(engine,
                             ringNumber,
                             engine->systemRingStopTime,
                             active);
}

uint8_t IntersectionEngineSetRingForceOffControl(IntersectionEngine_t *engine,
                                                 uint8_t ringNumber,
                                                 uint8_t active)
{
  return SetRingControlState(engine,
                             ringNumber,
                             active,
                             engine->systemRingForceOff);
}

uint8_t IntersectionEngineGetRingForceOffControl(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active)
{
  return GetRingControlState(engine,
                             ringNumber,
                             engine->systemRingForceOff,
                             active);
}

uint8_t IntersectionEngineSetRingMaximum2Control(IntersectionEngine_t *engine,
                                                 uint8_t ringNumber,
                                                 uint8_t active)
{
  return SetRingControlState(engine,
                             ringNumber,
                             active,
                             engine->systemRingMax2);
}

uint8_t IntersectionEngineGetRingMaximum2Control(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active)
{
  return GetRingControlState(engine,
                             ringNumber,
                             engine->systemRingMax2,
                             active);
}

uint8_t IntersectionEngineSetRingMaximumInhibitControl(
  IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t active)
{
  return SetRingControlState(engine,
                             ringNumber,
                             active,
                             engine->systemRingMaxInhibit);
}

uint8_t IntersectionEngineGetRingMaximumInhibitControl(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active)
{
  return GetRingControlState(engine,
                             ringNumber,
                             engine->systemRingMaxInhibit,
                             active);
}

uint8_t IntersectionEngineSetRingPedRecycleControl(
  IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t active)
{
  return SetRingControlState(engine,
                             ringNumber,
                             active,
                             engine->systemRingPedRecycle);
}

uint8_t IntersectionEngineGetRingPedRecycleControl(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active)
{
  return GetRingControlState(engine,
                             ringNumber,
                             engine->systemRingPedRecycle,
                             active);
}

uint8_t IntersectionEngineSetRingRedRestControl(IntersectionEngine_t *engine,
                                                uint8_t ringNumber,
                                                uint8_t active)
{
  return SetRingControlState(engine,
                             ringNumber,
                             active,
                             engine->systemRingRedRest);
}

uint8_t IntersectionEngineGetRingRedRestControl(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active)
{
  return GetRingControlState(engine,
                             ringNumber,
                             engine->systemRingRedRest,
                             active);
}

uint8_t IntersectionEngineSetRingOmitRedClearControl(
  IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t active)
{
  return SetRingControlState(engine,
                             ringNumber,
                             active,
                             engine->systemRingOmitRedClear);
}

uint8_t IntersectionEngineGetRingOmitRedClearControl(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active)
{
  return GetRingControlState(engine,
                             ringNumber,
                             engine->systemRingOmitRedClear,
                             active);
}

uint8_t IntersectionEngineSetRingMaximum3Control(IntersectionEngine_t *engine,
                                                 uint8_t ringNumber,
                                                 uint8_t active)
{
  return SetRingControlState(engine,
                             ringNumber,
                             active,
                             engine->systemRingMax3);
}

uint8_t IntersectionEngineGetRingMaximum3Control(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active)
{
  return GetRingControlState(engine,
                             ringNumber,
                             engine->systemRingMax3,
                             active);
}

uint8_t IntersectionEngineSetPreemptInput(IntersectionEngine_t *engine,
                                          uint8_t preemptNumber,
                                          uint8_t active)
{
  uint8_t preemptIndex;

  if ((engine == NULL) || (engine->configLoaded == 0U) || (preemptNumber == 0U)
      || (preemptNumber > INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  preemptIndex = (uint8_t) (preemptNumber - 1U);
  engine->runtime.preemptInputStatus[preemptIndex] = (uint8_t) (active != 0U);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetPreemptControlState(IntersectionEngine_t *engine,
                                                 uint8_t preemptNumber,
                                                 uint8_t active)
{
  uint8_t preemptIndex;

  if ((engine == NULL) || (engine->configLoaded == 0U) || (preemptNumber == 0U)
      || (preemptNumber > INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  preemptIndex = (uint8_t) (preemptNumber - 1U);
  engine->runtime.preemptControlState[preemptIndex] = (uint8_t) (active != 0U);
  ResetBackupTimer(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineSetMmuFlashControl(IntersectionEngine_t *engine,
                                             uint8_t active)
{
  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  engine->mmuFlashActive = (uint8_t) (active != 0U);
  engine->runtime.mmuFlashActive = engine->mmuFlashActive;
  UpdateCoordinationRuntime(engine);

  if (AutomaticFlashOutputsActive(engine) != 0U)
  {
    ForceControllerRedRest(engine);
  }

  RefreshRuntimeViews(engine);

  return 1U;
}

const IntersectionConfig_t *IntersectionEngineGetConfig(
  const IntersectionEngine_t *engine)
{
  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return NULL;
  }

  return &engine->config;
}

const IntersectionRuntime_t *IntersectionEngineGetRuntime(
  const IntersectionEngine_t *engine)
{
  if ((engine == NULL) || (engine->configLoaded == 0U))
  {
    return NULL;
  }

  return &engine->runtime;
}

uint8_t IntersectionEngineGetActivePhaseForRing(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *phaseNumber)
{
  uint8_t ringIndex;

  if ((engine == NULL) || (phaseNumber == NULL) || (ringNumber == 0U)
      || (ringNumber > engine->config.ringCount))
  {
    return 0U;
  }

  ringIndex = (uint8_t) (ringNumber - 1U);
  *phaseNumber = (uint8_t) (engine->runtime.rings[ringIndex].activePhaseIndex
                            + 1U);

  return 1U;
}

uint8_t IntersectionEngineGetOutputIntentImage(
  const IntersectionEngine_t *engine,
  IntersectionOutputIntentImage_t *
  outputIntentImage)
{
  if ((engine == NULL) || (outputIntentImage == NULL)
      || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  *outputIntentImage = engine->runtime.outputIntentImage;

  return 1U;
}

uint8_t IntersectionEngineGetPhaseStatusGroup(
  const IntersectionEngine_t *engine,
  uint8_t groupNumber,
  IntersectionPhaseStatusGroup_t *
  statusGroup)
{
  uint8_t groupIndex;
  uint8_t phaseIndex;

  if ((engine == NULL) || (statusGroup == NULL) || (groupNumber == 0U)
      || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  groupIndex = (uint8_t) (groupNumber - 1U);

  if (groupIndex >= ((engine->config.phaseCount + 7U) / 8U))
  {
    return 0U;
  }

  memset(statusGroup, 0, sizeof(*statusGroup));

  for (phaseIndex = (uint8_t) (groupIndex * 8U);
       (phaseIndex < engine->config.phaseCount)
       && (phaseIndex < (uint8_t) ((groupIndex + 1U) * 8U));
       phaseIndex++)
  {
    const IntersectionPhaseRuntime_t *phaseRuntime =
      &engine->runtime.phases[phaseIndex];
    const IntersectionPhaseConfig_t *phaseConfig =
      &engine->config.phases[phaseIndex];
    uint8_t bitMask = GroupBitMask(phaseIndex);

    if ((phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_RED)
        || (phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_RED_CLEAR))
    {
      statusGroup->reds |= bitMask;
    }

    if (phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_YELLOW)
    {
      statusGroup->yellows |= bitMask;
    }

    if (phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_GREEN)
    {
      statusGroup->greens |= bitMask;
    }

    if (phaseRuntime->pedInterval == INTERSECTION_PED_INTERVAL_DONT_WALK)
    {
      statusGroup->dontWalks |= bitMask;
    }

    if (phaseRuntime->pedInterval == INTERSECTION_PED_INTERVAL_CLEAR)
    {
      statusGroup->pedClears |= bitMask;
    }

    if (phaseRuntime->pedInterval == INTERSECTION_PED_INTERVAL_WALK)
    {
      statusGroup->walks |= bitMask;
    }

    if ((phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_GREEN)
        || (phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_YELLOW)
        || (phaseRuntime->interval == INTERSECTION_PHASE_INTERVAL_RED_CLEAR))
    {
      statusGroup->phaseOns |= bitMask;
    }

    if (phaseRuntime->next != 0U)
    {
      statusGroup->phaseNexts |= bitMask;
    }

    if ((PhaseSystemVehCallActive(engine, phaseIndex) != 0U)
        || (RemoteManualVehicleCallActive(engine, phaseIndex) != 0U)
        || (UnitControlExternalMinRecallActive(engine) != 0U)
        || (PhaseUnitControlledNonActuatedActive(engine, phaseIndex) != 0U)
        || (phaseRuntime->detectorActive != 0U)
        || (phaseRuntime->callLatched != 0U)
        || (PhaseHasVehicleRecall(phaseConfig) != 0U))
    {
      statusGroup->vehCalls |= bitMask;
    }

    if ((PhaseSystemPedCallActive(engine, phaseIndex) != 0U)
        || (RemoteManualPedCallActive(engine, phaseIndex) != 0U)
        || (phaseRuntime->pedInputActive != 0U)
        || (phaseRuntime->pedCallLatched != 0U)
        || (phaseRuntime->pedServicePending != 0U))
    {
      statusGroup->pedCalls |= bitMask;
    }
  }

  return 1U;
} /* IntersectionEngineGetPhaseStatusGroup */

uint8_t IntersectionEngineGetRingStatus(const IntersectionEngine_t *engine,
                                        uint8_t ringNumber,
                                        uint8_t *ringStatus)
{
  uint8_t ringIndex;

  if ((engine == NULL) || (ringStatus == NULL) || (ringNumber == 0U))
  {
    return 0U;
  }

  if ((engine->configLoaded == 0U) || (ringNumber > engine->config.ringCount))
  {
    return 0U;
  }

  ringIndex = (uint8_t) (ringNumber - 1U);
  *ringStatus =
    (uint8_t) (engine->runtime.rings[ringIndex].terminationReasonBits
               | (uint8_t) engine->runtime.rings[ringIndex].
               statusCode);

  return 1U;
}

uint8_t IntersectionEngineGetChannelStatusGroup(
  const IntersectionEngine_t *engine,
  uint8_t groupNumber,
  IntersectionChannelStatusGroup_t
  *statusGroup)
{
  uint8_t groupIndex;
  uint8_t channelIndex;

  if ((engine == NULL) || (statusGroup == NULL) || (groupNumber == 0U)
      || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  groupIndex = (uint8_t) (groupNumber - 1U);

  if (groupIndex >= ((INTERSECTION_CHANNEL_COUNT_MAX + 7U) / 8U))
  {
    return 0U;
  }

  memset(statusGroup, 0, sizeof(*statusGroup));

  for (channelIndex = (uint8_t) (groupIndex * 8U);
       channelIndex < (uint8_t) ((groupIndex + 1U) * 8U);
       channelIndex++)
  {
    uint8_t bitMask = GroupBitMask(channelIndex);
    IntersectionOutputAspect_t aspect =
      engine->runtime.channels[channelIndex].aspect;

    if (AspectIsRed(aspect) != 0U)
    {
      statusGroup->reds |= bitMask;
    }

    if (AspectIsYellow(aspect) != 0U)
    {
      statusGroup->yellows |= bitMask;
    }

    if (AspectIsGreen(aspect) != 0U)
    {
      statusGroup->greens |= bitMask;
    }
  }

  return 1U;
} /* IntersectionEngineGetChannelStatusGroup */

uint8_t IntersectionEngineGetOverlapStatusGroup(
  const IntersectionEngine_t *engine,
  uint8_t groupNumber,
  IntersectionOverlapStatusGroup_t
  *statusGroup)
{
  uint8_t groupIndex;
  uint8_t overlapIndex;

  if ((engine == NULL) || (statusGroup == NULL) || (groupNumber == 0U)
      || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  groupIndex = (uint8_t) (groupNumber - 1U);

  if (groupIndex >= ((INTERSECTION_OVERLAP_COUNT_MAX + 7U) / 8U))
  {
    return 0U;
  }

  memset(statusGroup, 0, sizeof(*statusGroup));

  for (overlapIndex = (uint8_t) (groupIndex * 8U);
       overlapIndex < (uint8_t) ((groupIndex + 1U) * 8U);
       overlapIndex++)
  {
    uint8_t bitMask = GroupBitMask(overlapIndex);
    IntersectionOutputAspect_t aspect =
      engine->runtime.overlaps[overlapIndex].aspect;

    if (AspectIsRed(aspect) != 0U)
    {
      statusGroup->reds |= bitMask;
    }

    if (AspectIsYellow(aspect) != 0U)
    {
      statusGroup->yellows |= bitMask;
    }

    if (AspectIsGreen(aspect) != 0U)
    {
      statusGroup->greens |= bitMask;
    }
  }

  return 1U;
} /* IntersectionEngineGetOverlapStatusGroup */

uint8_t IntersectionEngineGetPreemptStatusGroup(
  const IntersectionEngine_t *engine,
  uint8_t groupNumber,
  uint8_t *statusGroup)
{
  uint8_t groupIndex;
  uint8_t preemptIndex;

  if ((engine == NULL) || (statusGroup == NULL) || (groupNumber == 0U)
      || (groupNumber > (uint8_t) ((INTERSECTION_PREEMPT_COUNT_MAX + 7U) / 8U))
      || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  groupIndex = (uint8_t) (groupNumber - 1U);
  *statusGroup = 0U;

  for (preemptIndex = (uint8_t) (groupIndex * 8U);
       (preemptIndex < INTERSECTION_PREEMPT_COUNT_MAX)
       && (preemptIndex < (uint8_t) ((groupIndex + 1U) * 8U));
       preemptIndex++)
  {
    if (PreemptInputIsPresent(engine, preemptIndex) != 0U)
    {
      *statusGroup |= GroupBitMask(preemptIndex);
    }
  }

  return 1U;
}

uint8_t IntersectionEngineGetVehicleDetectorStatusGroup(
  const IntersectionEngine_t *engine,
  uint8_t groupNumber,
  uint8_t *statusGroup)
{
  uint8_t detectorIndex;
  uint8_t groupIndex;

  if ((engine == NULL) || (statusGroup == NULL) || (groupNumber == 0U)
      || (engine->configLoaded == 0U)
      || (groupNumber
          > (uint8_t) ((INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX + 7U) / 8U)))
  {
    return 0U;
  }

  groupIndex = (uint8_t) (groupNumber - 1U);
  *statusGroup = 0U;

  for (detectorIndex = (uint8_t) (groupIndex * 8U);
       (detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
       && (detectorIndex < (uint8_t) ((groupIndex + 1U) * 8U));
       detectorIndex++)
  {
    if ((engine->runtime.vehicleDetectors[detectorIndex].inputActive != 0U)
        || (engine->runtime.vehicleDetectors[detectorIndex].remoteActuation
            != 0U))
    {
      *statusGroup |= GroupBitMask(detectorIndex);
    }
  }

  return 1U;
}

uint8_t IntersectionEngineGetPedestrianDetectorStatusGroup(
  const IntersectionEngine_t *engine,
  uint8_t groupNumber,
  uint8_t *statusGroup)
{
  uint8_t detectorIndex;
  uint8_t groupIndex;

  if ((engine == NULL) || (statusGroup == NULL) || (groupNumber == 0U)
      || (engine->configLoaded == 0U)
      || (groupNumber > (uint8_t) ((INTERSECTION_PED_INPUT_COUNT_MAX + 7U)
                                   / 8U)))
  {
    return 0U;
  }

  groupIndex = (uint8_t) (groupNumber - 1U);
  *statusGroup = 0U;

  for (detectorIndex = (uint8_t) (groupIndex * 8U);
       (detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX)
       && (detectorIndex < (uint8_t) ((groupIndex + 1U) * 8U));
       detectorIndex++)
  {
    if ((engine->runtime.pedestrianDetectors[detectorIndex].inputActive != 0U)
        || (engine->runtime.pedestrianDetectors[detectorIndex].remoteActuation
            != 0U))
    {
      *statusGroup |= GroupBitMask(detectorIndex);
    }
  }

  return 1U;
}

uint8_t IntersectionEngineSetSpecialFunctionOutputControl(
  IntersectionEngine_t *engine,
  uint8_t outputNumber,
  uint8_t active)
{
  uint8_t bitMask;

  if ((engine == NULL) || (engine->configLoaded == 0U) || (outputNumber == 0U)
      || (outputNumber > INTERSECTION_SPECIAL_FUNCTION_OUTPUT_COUNT))
  {
    return 0U;
  }

  bitMask = (uint8_t) (1U << (outputNumber - 1U));

  if (active != 0U)
  {
    engine->specialFunctionControl |= bitMask;
  }
  else
  {
    engine->specialFunctionControl &= (uint8_t) ~bitMask;
  }

  ResetBackupTimer(engine);
  RefreshRuntimeViews(engine);

  return 1U;
}

uint8_t IntersectionEngineGetSpecialFunctionOutputControl(
  const IntersectionEngine_t *engine,
  uint8_t outputNumber,
  uint8_t *active)
{
  uint8_t bitMask;

  if ((engine == NULL) || (active == NULL) || (engine->configLoaded == 0U)
      || (outputNumber == 0U)
      || (outputNumber > INTERSECTION_SPECIAL_FUNCTION_OUTPUT_COUNT))
  {
    return 0U;
  }

  bitMask = (uint8_t) (1U << (outputNumber - 1U));
  *active = (uint8_t) ((engine->specialFunctionControl & bitMask) != 0U);

  return 1U;
}

uint8_t IntersectionEngineGetSpecialFunctionOutputStatus(
  const IntersectionEngine_t *engine,
  uint8_t outputNumber,
  uint8_t *active)
{
  uint8_t bitMask;

  if ((engine == NULL) || (active == NULL) || (engine->configLoaded == 0U)
      || (outputNumber == 0U)
      || (outputNumber > INTERSECTION_SPECIAL_FUNCTION_OUTPUT_COUNT))
  {
    return 0U;
  }

  bitMask = (uint8_t) (1U << (outputNumber - 1U));
  *active = (uint8_t) ((engine->runtime.specialFunctionStatus & bitMask) != 0U);

  return 1U;
}

uint8_t IntersectionEngineGetShortAlarmCycleZeroLatched(
  const IntersectionEngine_t *engine,
  uint8_t *active)
{
  if ((engine == NULL) || (active == NULL) || (engine->configLoaded == 0U))
  {
    return 0U;
  }

  *active = engine->shortAlarmCycleZeroLatched;

  return 1U;
}

void IntersectionEngineAcknowledgeShortAlarmStatusRead(
  IntersectionEngine_t *engine)
{
  if (engine == NULL)
  {
    return;
  }

  engine->shortAlarmCycleZeroLatched = 0U;
}
