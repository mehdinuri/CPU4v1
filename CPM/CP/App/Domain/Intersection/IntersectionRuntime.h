/* App/Domain/Intersection/IntersectionRuntime.h
 *
 * Runtime state for the new controller-core engine.
 */
#ifndef INTERSECTION_RUNTIME_H
#define INTERSECTION_RUNTIME_H

#include <stdint.h>

#include "Domain/Intersection/IntersectionConfig.h"

typedef enum
{
  INTERSECTION_CONTROL_MODE_FREE = 0,
  INTERSECTION_CONTROL_MODE_COORDINATED,
  INTERSECTION_CONTROL_MODE_PREEMPT,
  INTERSECTION_CONTROL_MODE_FLASH,
  INTERSECTION_CONTROL_MODE_ALL_RED,
  INTERSECTION_CONTROL_MODE_DARK
} IntersectionControlMode_t;

typedef enum
{
  INTERSECTION_LOCAL_FREE_STATUS_OTHER = 1,
  INTERSECTION_LOCAL_FREE_STATUS_NOT_FREE = 2,
  INTERSECTION_LOCAL_FREE_STATUS_COMMAND_FREE = 3,
  INTERSECTION_LOCAL_FREE_STATUS_TRANSITION_FREE = 4,
  INTERSECTION_LOCAL_FREE_STATUS_INPUT_FREE = 5,
  INTERSECTION_LOCAL_FREE_STATUS_COORD_FREE = 6,
  INTERSECTION_LOCAL_FREE_STATUS_BAD_PLAN = 7,
  INTERSECTION_LOCAL_FREE_STATUS_BAD_CYCLE_TIME = 8,
  INTERSECTION_LOCAL_FREE_STATUS_SPLIT_OVERRUN = 9,
  INTERSECTION_LOCAL_FREE_STATUS_INVALID_OFFSET = 10,
  INTERSECTION_LOCAL_FREE_STATUS_FAILED = 11
} IntersectionLocalFreeStatus_t;

typedef enum
{
  INTERSECTION_UNIT_CONTROL_STATUS_OTHER = 1,
  INTERSECTION_UNIT_CONTROL_STATUS_SYSTEM_CONTROL = 2,
  INTERSECTION_UNIT_CONTROL_STATUS_BACKUP_MODE = 4,
  INTERSECTION_UNIT_CONTROL_STATUS_MANUAL = 5,
  INTERSECTION_UNIT_CONTROL_STATUS_TIMEBASE = 6,
  INTERSECTION_UNIT_CONTROL_STATUS_INTERCONNECT = 7,
  INTERSECTION_UNIT_CONTROL_STATUS_INTERCONNECT_BACKUP = 8,
  INTERSECTION_UNIT_CONTROL_STATUS_REMOTE_MANUAL_CONTROL = 9,
  INTERSECTION_UNIT_CONTROL_STATUS_LOCAL_MANUAL_CONTROL = 10
} IntersectionUnitControlStatus_t;

typedef enum
{
  INTERSECTION_PREEMPT_STATE_OTHER = 1,
  INTERSECTION_PREEMPT_STATE_NOT_ACTIVE = 2,
  INTERSECTION_PREEMPT_STATE_NOT_ACTIVE_WITH_CALL = 3,
  INTERSECTION_PREEMPT_STATE_ENTRY_STARTED = 4,
  INTERSECTION_PREEMPT_STATE_TRACK_SERVICE = 5,
  INTERSECTION_PREEMPT_STATE_DWELL = 6,
  INTERSECTION_PREEMPT_STATE_LINK_ACTIVE = 7,
  INTERSECTION_PREEMPT_STATE_EXIT_STARTED = 8,
  INTERSECTION_PREEMPT_STATE_MAX_PRESENCE = 9,
  INTERSECTION_PREEMPT_STATE_ADVANCED_PREEMPT = 10
} IntersectionPreemptState_t;

typedef enum
{
  INTERSECTION_PHASE_INTERVAL_RED = 0,
  INTERSECTION_PHASE_INTERVAL_GREEN,
  INTERSECTION_PHASE_INTERVAL_YELLOW,
  INTERSECTION_PHASE_INTERVAL_RED_CLEAR
} IntersectionPhaseInterval_t;

typedef enum
{
  INTERSECTION_PED_INTERVAL_DONT_WALK = 0,
  INTERSECTION_PED_INTERVAL_WALK,
  INTERSECTION_PED_INTERVAL_CLEAR
} IntersectionPedInterval_t;

typedef enum
{
  INTERSECTION_RING_STAGE_GREEN = 0,
  INTERSECTION_RING_STAGE_YELLOW,
  INTERSECTION_RING_STAGE_RED_CLEAR,
  INTERSECTION_RING_STAGE_WAIT_BARRIER,
  INTERSECTION_RING_STAGE_RED_REST
} IntersectionRingStage_t;

typedef struct
{
  uint8_t detectorActive;
  uint8_t callLatched;
  uint8_t pedInputActive;
  uint8_t pedCallLatched;
  uint8_t pedAlternateTimingPending;
  uint8_t pedAlternateTimingActive;
  uint8_t next;
  IntersectionPhaseInterval_t interval;
  IntersectionPedInterval_t pedInterval;
  uint32_t intervalElapsedTicks;
  uint16_t pedIntervalElapsedTicks;
  uint8_t pedServicePending;
  uint8_t pedServiceActive;
} IntersectionPhaseRuntime_t;

typedef enum
{
  INTERSECTION_RING_STATUS_MIN_GREEN = 0,
  INTERSECTION_RING_STATUS_EXTENSION = 1,
  INTERSECTION_RING_STATUS_MAXIMUM = 2,
  INTERSECTION_RING_STATUS_GREEN_REST = 3,
  INTERSECTION_RING_STATUS_YELLOW_CHANGE = 4,
  INTERSECTION_RING_STATUS_RED_CLEARANCE = 5,
  INTERSECTION_RING_STATUS_RED_REST = 6
} IntersectionRingStatusCode_t;

typedef enum
{
  INTERSECTION_RING_TERMINATION_NONE = 0x00,
  INTERSECTION_RING_TERMINATION_GAP_OUT = 0x08,
  INTERSECTION_RING_TERMINATION_MAX_OUT = 0x10,
  INTERSECTION_RING_TERMINATION_FORCE_OFF = 0x20
} IntersectionRingTerminationReason_t;

typedef struct
{
  uint8_t activePosition;
  uint8_t activePhaseIndex;
  uint8_t pendingPosition;
  uint8_t barrierWaiting;
  IntersectionRingStage_t stage;
  IntersectionRingStatusCode_t statusCode;
  uint8_t terminationReasonBits;
  uint32_t stageElapsedTicks;
} IntersectionRingRuntime_t;

typedef enum
{
  INTERSECTION_OUTPUT_ASPECT_RED = 0,
  INTERSECTION_OUTPUT_ASPECT_YELLOW,
  INTERSECTION_OUTPUT_ASPECT_GREEN,
  INTERSECTION_OUTPUT_ASPECT_DARK,
  INTERSECTION_OUTPUT_ASPECT_FLASH_RED,
  INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW,
  INTERSECTION_OUTPUT_ASPECT_FLASH_GREEN
} IntersectionOutputAspect_t;

typedef struct
{
  IntersectionOutputAspect_t aspect;
  uint8_t dimmed;
  uint8_t dimAlternateHalfCycle;
} IntersectionChannelRuntime_t;

typedef struct
{
  IntersectionOutputAspect_t aspect;
} IntersectionOverlapRuntime_t;

typedef struct
{
  uint8_t inputActive;
  uint8_t remoteActuation;
  uint8_t previousPresenceActive;
  uint8_t recognitionActive;
  uint16_t delayTimerTicks;
  uint16_t extendTimerTicks;
  uint16_t addedInitialCount;
} IntersectionVehicleDetectorRuntime_t;

typedef struct
{
  uint8_t inputActive;
  uint8_t remoteActuation;
  uint8_t alternateTimingRequest;
  uint8_t reserved0;
  uint16_t apsElapsedTicks;
  uint16_t reserved1;
} IntersectionPedestrianDetectorRuntime_t;

typedef struct
{
  IntersectionOutputAspect_t channels[INTERSECTION_CHANNEL_COUNT_MAX];
  uint8_t channelDimmed[INTERSECTION_CHANNEL_COUNT_MAX];
  uint8_t channelDimAlternateHalfCycle[INTERSECTION_CHANNEL_COUNT_MAX];
} IntersectionOutputIntentImage_t;

typedef struct
{
  uint8_t reds;
  uint8_t yellows;
  uint8_t greens;
  uint8_t dontWalks;
  uint8_t pedClears;
  uint8_t walks;
  uint8_t vehCalls;
  uint8_t pedCalls;
  uint8_t phaseOns;
  uint8_t phaseNexts;
} IntersectionPhaseStatusGroup_t;

typedef struct
{
  uint8_t reds;
  uint8_t yellows;
  uint8_t greens;
} IntersectionChannelStatusGroup_t;

typedef struct
{
  uint8_t reds;
  uint8_t yellows;
  uint8_t greens;
} IntersectionOverlapStatusGroup_t;

typedef struct
{
  uint8_t configLoaded;
  uint8_t reserved0;
  uint8_t reserved1;
  uint8_t reserved2;
  IntersectionControlMode_t mode;
  uint8_t coordPatternStatus;
  uint8_t localFreeStatus;
  uint8_t systemPatternControl;
  uint8_t systemSyncControlSeconds;
  uint8_t actionPlanControl;
  uint8_t timebaseActionStatus;
  uint8_t timebaseAuxiliaryFunctionStatus;
  uint8_t interconnectCommand;
  uint8_t interconnectInputsValid;
  uint8_t coordCycleFaultActive;
  uint8_t coordFaultActive;
  uint8_t coordFailActive;
  uint8_t cycleFailActive;
  uint8_t coordinationAlarmActive;
  uint8_t unitControlStatus;
  uint8_t unitControl;
  uint8_t remoteManualControlTimeout;
  uint8_t remoteManualIntervalAdvance;
  uint8_t specialFunctionControl;
  uint8_t specialFunctionStatus;
  uint8_t backupModeActive;
  uint8_t startUpFlashActive;
  uint8_t localDimmingInputActive;
  uint8_t dimmingActive;
  uint16_t coordCycleStatusSeconds;
  uint16_t coordSyncStatusSeconds;
  uint8_t preemptStatus;
  uint8_t mmuFlashActive;
  uint8_t preemptInputStatus[INTERSECTION_PREEMPT_COUNT_MAX];
  uint8_t preemptControlState[INTERSECTION_PREEMPT_COUNT_MAX];
  IntersectionPreemptState_t preemptStates[INTERSECTION_PREEMPT_COUNT_MAX];
  uint32_t monotonicTicks;
  IntersectionPhaseRuntime_t phases[INTERSECTION_PHASE_COUNT_MAX];
  IntersectionRingRuntime_t rings[INTERSECTION_RING_COUNT_MAX];
  IntersectionChannelRuntime_t channels[INTERSECTION_CHANNEL_COUNT_MAX];
  IntersectionOverlapRuntime_t overlaps[INTERSECTION_OVERLAP_COUNT_MAX];
  IntersectionVehicleDetectorRuntime_t vehicleDetectors[
    INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX];
  IntersectionPedestrianDetectorRuntime_t pedestrianDetectors[
    INTERSECTION_PED_INPUT_COUNT_MAX];
  IntersectionOutputIntentImage_t outputIntentImage;
} IntersectionRuntime_t;

#endif /* INTERSECTION_RUNTIME_H */
