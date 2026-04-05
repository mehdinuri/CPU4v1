#pragma once

/*
 * App/Domain/Intersection/Types.h
 *
 * Core Domain type definitions for the NTCIP Intersection controller.
 * Field names follow NTCIP 1201/1202 terminology where applicable.
 * NO hardware, RTOS, or network stack headers included here.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ---------------------------------------------------------------------------
 * Capacity constants (must match hardware SSM configuration)
 * ---------------------------------------------------------------------------*/
#define PHASES_MAX               16
#define SIGNAL_GROUPS_MAX        32
#define SIGNAL_OUTPUTS_MAX       96
#define DETECTORS_MAX            32
#define DIGITAL_INPUTS_MAX       32
#define SEQUENCES_MAX             8
#define SEQUENCE_STEPS_MAX       48
#define TRANSITIONS_MAX          64
#define SIGNAL_PROGRAMS_MAX      16
#define WORK_PLANS_MAX           13
#define WORK_PLAN_ENTRIES_MAX    16
#define SP_PLANS_MAX             16
#define SP_PLAN_ENTRIES_MAX      16
#define WORK_SCHEDULE_ENTRIES_MAX 16
#define SIGNALS_MAX              16   /* Custom signal definitions */
#define SUBSIGNALS_MAX            3   /* Red, yellow, green sub-channels */
#define COUNTERS_MAX             12

/* ---------------------------------------------------------------------------
 * Controller state machine states
 * ---------------------------------------------------------------------------*/
typedef enum
{
  CTRL_STATE_NONE               = 0,
  CTRL_STATE_ANY                = 1,    /* Wildcard — matches any state in rules */
  CTRL_STATE_DARK               = 2,    /* All outputs off */
  CTRL_STATE_FLASH              = 3,    /* All-yellow flash */
  CTRL_STATE_ALL_RED            = 4,    /* Safety hold — all red */
  CTRL_STATE_PHASE              = 5,    /* Normal phase-based operation */
  CTRL_STATE_PHASE_TRANSITION   = 6,    /* Between phases (clearance active) */
  CTRL_STATE_SEQUENCE           = 7,    /* Pre-timed Sequence execution */
  CTRL_STATE_SECURE_TRANSITION  = 8,    /* Secure inter-Program transition */
} ControllerState_t;

/* ---------------------------------------------------------------------------
 * Control modes (how timing is determined)
 * ---------------------------------------------------------------------------*/
typedef enum
{
  CONTROL_MODE_FIXED_PLAN       = 1,    /* Pre-timed, no Detectors */
  CONTROL_MODE_HALF_ACTUATED    = 2,    /* Some movements Detector-actuated */
  CONTROL_MODE_FULLY_ACTUATED   = 3,    /* All movements Detector-actuated */
  CONTROL_MODE_CENTRAL_ADAPTIVE = 4,    /* Central system commands timings */
  CONTROL_MODE_FLASH            = 5,
  CONTROL_MODE_DARK             = 6,
  CONTROL_MODE_LOCAL_ADAPTIVE   = 7,
} ControlMode_t;

/* ---------------------------------------------------------------------------
 * Signal group types
 * ---------------------------------------------------------------------------*/
typedef enum
{
  SG_TYPE_NONE              = 0,
  SG_TYPE_VEHICLE_MAINWAY   = 1,
  SG_TYPE_VEHICLE_SUBWAY    = 2,
  SG_TYPE_PEDESTRIAN        = 3,
  SG_TYPE_BICYCLE           = 4,
  SG_TYPE_TRAM              = 5,
  SG_TYPE_FLASHER           = 6,
} SignalGroupType_t;

/* ---------------------------------------------------------------------------
 * Signal group runtime states (state machine within each phase)
 * ---------------------------------------------------------------------------*/
typedef enum
{
  SG_STATE_NONE       = 0,
  SG_STATE_CLOSING    = 1,    /* Showing yellow (yellowChangeInterval) */
  SG_STATE_CLOSED     = 2,    /* Red — either in clearance or waiting */
  SG_STATE_OPENING    = 3,    /* Showing opening signal before full green */
  SG_STATE_OPEN       = 4,    /* Full green */
  SG_STATE_GREEN_FLASH= 5,    /* Green flashing before yellow (pedestrianClearance) */
  SG_STATE_FLASH      = 6,    /* All-yellow flash mode */
  SG_STATE_SEQUENCE   = 7,    /* Controlled by Sequence step */
} SignalGroupState_t;

/* ---------------------------------------------------------------------------
 * Conflict types between signal groups
 * ---------------------------------------------------------------------------*/
typedef enum
{
  CONFLICT_NONE                   = 0,
  CONFLICT_GREEN_GREEN            = 1,    /* Safety-critical */
  CONFLICT_YELLOW_GREEN           = 2,    /* Safety-critical */
  CONFLICT_YELLOW_YELLOW          = 3,
  CONFLICT_MALFUNCTION            = 4,
  CONFLICT_VOLTAGE_LIMIT          = 5,
  CONFLICT_FREQUENCY_ERROR        = 6,
  CONFLICT_INVALID_SIGNAL         = 7,
  CONFLICT_INVALID_SIGNAL_SEQUENCE= 8,
} ConflictType_t;

/* ---------------------------------------------------------------------------
 * RPN rule operators
 * ---------------------------------------------------------------------------*/
typedef enum
{
  OPR_NONE         = 0,
  OPR_EQUAL        = 1,
  OPR_NOTEQUAL     = 2,
  OPR_LESS         = 3,
  OPR_LESS_EQUAL   = 4,
  OPR_GREATER      = 5,
  OPR_GREATER_EQUAL= 6,
  OPR_ADD          = 7,
  OPR_SUB          = 8,
  OPR_MUL          = 9,
  OPR_DIV          = 10,
  OPR_MODULO       = 11,
  OPR_AND          = 12,
  OPR_OR           = 13,
  OPR_GG_CONFLICT  = 14,
  OPR_GY_CONFLICT  = 15,
  OPR_YY_CONFLICT  = 16,
} Operator_t;

/* ---------------------------------------------------------------------------
 * Statement commands (transition action bytecode)
 * ---------------------------------------------------------------------------*/
typedef enum
{
  CMD_NONE                     = 0,
  CMD_MEMORY_ALLOCATE          = 1,
  CMD_MEMORY_DEALLOCATE        = 2,
  CMD_MEMORY_INIT              = 3,
  CMD_MEMORY_ADD               = 4,
  CMD_COUNTER_START            = 5,
  CMD_COUNTER_STOP             = 6,
  CMD_PHASE_START              = 7,
  CMD_PHASE_STOP               = 8,
  CMD_PHASE_EXTEND             = 9,     /* param3 is signed extension in seconds */
  CMD_PHASE_END                = 10,
  CMD_SEQ_START                = 11,
  CMD_SEQ_STOP                 = 12,
  CMD_SEQ_ADD_STEP             = 13,
  CMD_SEQ_REMOVE_SECONDS       = 14,
  CMD_USER_STATE_REQ_END       = 15,
  CMD_USER_STATE_TO_CURRENT    = 16,
  CMD_TRANSITIONS_LOCK_ADD     = 17,
  CMD_TRANSITIONS_LOCK_END     = 18,
  CMD_SG_ADD_TO_FLASHER        = 19,
  CMD_SG_REMOVE_FROM_FLASHER   = 20,
  CMD_SG_ADD_TO_PHASE          = 21,
  CMD_SG_REMOVE_FROM_PHASE     = 22,
  CMD_PHASE_DELETE_RUN_INFO    = 23,
  CMD_SIG_PROG_RESTART         = 24,
} Command_t;

/* ---------------------------------------------------------------------------
 * Phase definition (NTCIP 1201 phaseTable row)
 * ---------------------------------------------------------------------------*/
typedef struct
{
  uint32_t signalGroupMask;     /* Bitmask of SG indices (bit 0 = SG 0) */
  uint8_t minGreenTime;         /* NTCIP phaseMinGreenTime    (seconds) */
  uint8_t maxGreenTime;         /* NTCIP phaseMaxGreenTime    (seconds) */
} PhaseConfig_t;

/* Phase runtime state (lives in RAM, reset each time phase starts) */
typedef struct
{
  uint16_t elapsedSeconds;      /* Seconds elapsed in current green */
  int8_t extensionSeconds;      /* Signed extension (+/- from actuated control) */
  bool maxTimeElapsed;          /* True once elapsedSeconds >= maxGreenTime */
} PhaseRuntime_t;

/* ---------------------------------------------------------------------------
 * Sub-signal (one color channel within a signal definition)
 * ---------------------------------------------------------------------------*/
typedef struct
{
  uint16_t period_10ms;         /* Flash period in 10 ms units (0 = steady) */
} SubsignalConfig_t;

/* Signal definition (custom signal pattern for flash/opening/closing etc.) */
typedef struct
{
  bool isValid;
  bool isValidForFlash;
  bool durationUnlimited;
  SubsignalConfig_t subsignals[SUBSIGNALS_MAX];   /* [0]=red [1]=yellow [2]=green */
  uint16_t followers;           /* Bitmask of other signals that follow this one */
  uint8_t minDuration;          /* Minimum display time (seconds) */
  uint8_t maxDuration;          /* Maximum display time (seconds) */
} SignalConfig_t;

/* ---------------------------------------------------------------------------
 * Signal group per-output Conflict entry (within tSSGDef.SaConflicts)
 * ---------------------------------------------------------------------------*/
typedef struct
{
  bool hasConflict;
  uint8_t redClearanceInterval;   /* NTCIP phaseRedClearanceInterval (seconds) */
} ConflictEntry_t;

/* ---------------------------------------------------------------------------
 * Signal group configuration (NTCIP 1201 row)
 * ---------------------------------------------------------------------------*/
typedef struct
{
  SignalGroupType_t type;
  uint8_t openingSignalIdx;         /* Signal to show while transitioning to green */
  uint8_t closingSignalIdx;         /* Signal to show while transitioning to red */
  uint8_t openingDuration;          /* Time in opening signal (seconds) */
  uint8_t yellowChangeInterval;     /* NTCIP phaseYellowChangeInterval (seconds) */
  uint8_t pedestrianClearance;      /* NTCIP phasePedestrianClearance / green-flash (s) */
  uint8_t pedestrianWalk;           /* NTCIP phaseWalk (seconds) — for ped SGs */
  uint8_t flashSignalIdx;           /* Signal shown in flash mode */
  uint8_t firstOutputIndex;         /* First lamp output index in this SG */
  uint8_t criticalRedLampCount;     /* Minimum broken red lamps to trigger alarm */
  ConflictEntry_t Conflicts[SIGNAL_GROUPS_MAX];   /* Conflict matrix row */
} SignalGroupConfig_t;

/* Signal group runtime state */
typedef struct
{
  SignalGroupState_t state;
  uint8_t currentSignalIdx;         /* Which signal pattern is currently shown */
  uint8_t stateElapsedSeconds;      /* Time in current state */

  /* Lamp fault tracking (set by adapter when SSM rePorts faults) */
  bool redLampBroken;
  bool redLampCritical;         /* >= criticalRedLampCount lamps broken */
  bool yellowLampBroken;
  bool greenLampBroken;
} SignalGroupRuntime_t;

/* ---------------------------------------------------------------------------
 * Detector configuration and runtime
 * ---------------------------------------------------------------------------*/
typedef struct
{
  uint8_t ownerSignalGroup;         /* SG index this Detector serves */
  uint8_t greenExtensionPerDemand;  /* Phase green += this × demand count (s) */
  uint8_t redTimeIfBroken;          /* Min phase duration if Detector broken (s) */
  uint8_t fallbackPhaseIfBroken;    /* Phase to force if Detector broken */
  bool activeLevelHigh;             /* True = vehicle present = pin HIGH */
} DetectorConfig_t;

typedef struct
{
  uint8_t demandCountInPeriod;      /* DetectorCallStatus (NTCIP 1202) */
  uint8_t demandCountInRed;
  uint8_t demandCountInGreen;
  uint16_t firstDemandTimeMs;       /* Time until first demand since period start */
  uint16_t occupancyTimeMs;         /* DetectorOccupancy (NTCIP 1202) */
  uint16_t occupancyInRedMs;
  uint16_t occupancyInGreenMs;
  uint16_t maxGapInGreenMs;         /* Longest gap with no vehicle */
  uint16_t brokenDurationMs;        /* How long Detector has been broken */
  bool isBroken;
} DetectorRuntime_t;

/* ---------------------------------------------------------------------------
 * Pre-timed Sequence definition
 * ---------------------------------------------------------------------------*/
typedef struct
{
  uint8_t stepCount;
  uint8_t stepDurations[SEQUENCE_STEPS_MAX];           /* Duration per step (seconds) */
  /* Signal index for each SG in each step. Packed: 4 bits per SG. */
  uint8_t stepSignals[SEQUENCE_STEPS_MAX][SIGNAL_GROUPS_MAX / 2];
} SequenceConfig_t;

typedef struct
{
  uint8_t currentStep;
  uint8_t stepElapsedSeconds;
  uint8_t loopCount;
} SequenceRuntime_t;

/* ---------------------------------------------------------------------------
 * Transition (event → state change) and rule (condition bytecode)
 * ---------------------------------------------------------------------------*/
typedef struct
{
  uint8_t fromState;            /* ControllerState_t or CTRL_STATE_ANY */
  uint8_t toState;              /* ControllerState_t */
  uint8_t param1;               /* Phase number, Sequence number, etc. */
  uint8_t param2;
  uint8_t ruleIndex;            /* Index into the rule pool */
  uint8_t priority;             /* Higher priority wins when multiple fire */
} TransitionConfig_t;

/* RPN rule — references a sub-range of the operation pool */
typedef struct
{
  uint16_t operationStart;      /* First operation index */
  uint8_t trueActionStart;      /* Statement pool: actions when rule is true */
  uint8_t trueActionEnd;
  uint8_t falseActionStart;     /* Statement pool: actions when rule is false */
  uint8_t falseActionEnd;
} RuleConfig_t;

/* RPN operation (one node in the expression tree) */
typedef struct
{
  Operator_t op;
  uint8_t operandA;             /* Index into operand pool, or literal value */
  uint8_t operandB;
} OperationConfig_t;

/* Bytecode statement (action to execute on transition) */
typedef struct
{
  Command_t cmd;
  uint8_t param1;
  uint8_t param2;
  int8_t param3;                /* Signed — used for phase extension (+/-) */
} StatementConfig_t;

/* ---------------------------------------------------------------------------
 * Timing plans
 * ---------------------------------------------------------------------------*/

/* One row in a daily work plan — time-of-day → per-phase durations */
typedef struct
{
  uint8_t hours;
  uint8_t minutes;
  uint8_t phaseDurations[PHASES_MAX];   /* Duration for each phase (seconds) */
} WorkPlanEntry_t;

/* One row in a signal Program plan — time-of-day → which Program to run */
typedef struct
{
  uint8_t hours;
  uint8_t minutes;
  uint8_t signalProgramIndex;
} SPPlanEntry_t;

/* Work schedule entry — date range + weekday mask → plan selection */
typedef struct
{
  uint8_t weekdayMask;          /* Bit 0 = Mon, Bit 6 = Sun */
  uint8_t startDay;
  uint8_t startMonth;
  uint8_t startYear;            /* Last 2 digits (e.g. 25 for 2025) */
  uint8_t endDay;
  uint8_t endMonth;
  uint8_t endYear;
  uint8_t workPlanIndex;
  uint8_t spPlanIndex;
} WorkScheduleEntry_t;

/* Signal Program additional entry/exit actions */
typedef struct
{
  uint8_t entryActionStart;     /* Statement pool index */
  uint8_t entryActionEnd;
  uint8_t exitActionStart;
  uint8_t exitActionEnd;
} SignalProgramConfig_t;

/* ---------------------------------------------------------------------------
 * Counter (general-purpose, used by transition rules)
 * ---------------------------------------------------------------------------*/
typedef struct
{
  uint32_t value;
  uint16_t periodSeconds;
  bool allocated;
  bool running;
  bool overflowed;
} CounterRuntime_t;
