#pragma once

/*
 * App/Domain/Intersection/Program.h
 *
 * Top-level Intersection Program coordinator.
 * Called every 100 ms by ProgramTask. Orchestrates phase/Sequence
 * state machines, transition rule evaluation, Conflict detection,
 * and signal output updates.
 *
 * All port dependencies are injected at init time — no global state.
 */
#include "Types.h"
#include "Ports/ISignalOutputPort.h"
#include "Ports/IDetectorInputPort.h"
#include "Ports/ISystemClockPort.h"
#include "Ports/ISNMPNotifierPort.h"

/* ---------------------------------------------------------------------------
 * Static configuration — loaded from persistent storage at startup
 * ---------------------------------------------------------------------------*/
typedef struct
{
  /* Core objects */
  PhaseConfig_t phases[PHASES_MAX];
  SignalGroupConfig_t signalGroups[SIGNAL_GROUPS_MAX];
  DetectorConfig_t Detectors[DETECTORS_MAX];
  SequenceConfig_t Sequences[SEQUENCES_MAX];
  SignalConfig_t signals[SIGNALS_MAX];

  /* Signal Programs (one per active timing plan) */
  SignalProgramConfig_t signalPrograms[SIGNAL_PROGRAMS_MAX];
  TransitionConfig_t transitions[SIGNAL_PROGRAMS_MAX][TRANSITIONS_MAX];
  RuleConfig_t rules[SIGNAL_PROGRAMS_MAX][TRANSITIONS_MAX];
  OperationConfig_t operations[SIGNAL_PROGRAMS_MAX][256];
  StatementConfig_t statements[SIGNAL_PROGRAMS_MAX][128];

  /* Scheduling */
  WorkPlanEntry_t workPlans[WORK_PLANS_MAX][WORK_PLAN_ENTRIES_MAX];
  SPPlanEntry_t spPlans[SP_PLANS_MAX][SP_PLAN_ENTRIES_MAX];
  WorkScheduleEntry_t workSchedule[WORK_SCHEDULE_ENTRIES_MAX];

  /* Consumed counts (how many of each pool are actually configured) */
  uint8_t phaseCount;
  uint8_t signalGroupCount;
  uint8_t DetectorCount;
  uint8_t SequenceCount;
  uint8_t signalCount;
  uint8_t transitionCounts[SIGNAL_PROGRAMS_MAX];
  uint8_t workPlanCount;
  uint8_t spPlanCount;
  uint8_t workScheduleEntryCount;

  /* Runtime settings */
  ControlMode_t controlMode;
  uint8_t activeSignalProgram;
  uint8_t activeWorkPlan;
  uint8_t activeSPPlan;
} ProgramConfig_t;

/* ---------------------------------------------------------------------------
 * Runtime state — lives in RAM, reset when controller restarts
 * ---------------------------------------------------------------------------*/
typedef struct
{
  ControllerState_t currentState;
  ControllerState_t requestedState;

  PhaseRuntime_t phases[PHASES_MAX];
  SignalGroupRuntime_t signalGroups[SIGNAL_GROUPS_MAX];
  DetectorRuntime_t Detectors[DETECTORS_MAX];
  SequenceRuntime_t Sequences[SEQUENCES_MAX];
  CounterRuntime_t counters[COUNTERS_MAX];

  uint8_t activePhase;          /* Currently executing phase index */
  uint8_t nextPhase;            /* Pending phase after transition */
  uint8_t activeSequence;       /* Currently executing Sequence index */

  uint32_t tickCount;           /* Total 100 ms ticks since startup */

  /* Transition locking (prevents re-triggering during transition) */
  bool transitionsLocked;
  uint8_t transitionLockCount;
} ProgramRuntime_t;

/* ---------------------------------------------------------------------------
 * Full context — config + runtime + injected Ports
 * ---------------------------------------------------------------------------*/
typedef struct
{
  ProgramConfig_t config;
  ProgramRuntime_t runtime;

  /* Injected port references (set once at init, never NULL after init) */
  ISignalOutputPort_t  *signalOutput;
  IDetectorInputPort_t *DetectorInput;
  ISystemClockPort_t   *systemClock;
  ISnmpNotifierPort_t  *snmpNotifier;
} ProgramCtx_t;

/* ---------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/

/**
 * Initialize the Program context with injected Ports and load the default
 * signal Program. Must be called once before ProgramTick().
 */
void ProgramInit(ProgramCtx_t *ctx,
                 ISignalOutputPort_t  *signalOutput,
                 IDetectorInputPort_t *DetectorInput,
                 ISystemClockPort_t   *systemClock,
                 ISnmpNotifierPort_t  *snmpNotifier);

/**
 * Advance the Intersection controller by one 100 ms tick.
 * Evaluates transition rules, updates signal group states,
 * performs Conflict checking, and flushes signal outputs.
 */
void ProgramTick(ProgramCtx_t *ctx);

/**
 * Load a timing plan from the configuration. Called by the storage
 * task after reading EEPROM/flash at startup or on plan change command.
 */
void ProgramLoadConfig(ProgramCtx_t *ctx, const ProgramConfig_t *config);

/**
 * Request a state change (e.g. from SNMP SET or user input).
 * Actual transition occurs on the next ProgramTick().
 */
void ProgramRequestState(ProgramCtx_t *ctx, ControllerState_t newState);

/**
 * Return the current controller state (safe to read from any task
 * after acquiring the SignalGroups mutex in the STM32 adapter).
 */
ControllerState_t ProgramGetState(const ProgramCtx_t *ctx);
