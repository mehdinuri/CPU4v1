/* App/Domain/Intersection/IntersectionEngine.h
 *
 * Dual-ring controller-core timing engine using the canonical
 * IntersectionConfig model.
 */
#ifndef INTERSECTION_ENGINE_H
#define INTERSECTION_ENGINE_H

#include <stdint.h>

#include "Domain/Intersection/IntersectionRuntime.h"

#define INTERSECTION_ENGINE_TICK_MS 10U
#define INTERSECTION_SPECIAL_FUNCTION_OUTPUT_COUNT 8U

typedef struct
{
  uint8_t configLoaded;
  uint8_t systemPatternControl;
  uint8_t systemSyncControlSeconds;
  uint8_t actionPlanControl;
  uint8_t unitControl;
  uint8_t remoteManualControlTimeout;
  uint8_t remoteManualIntervalAdvance;
  uint8_t specialFunctionControl;
  uint8_t localInterconnectCommand;
  uint8_t localInterconnectInputsValid;
  uint8_t localDimmingInputActive;
  uint8_t automaticFlashState;
  uint8_t shortAlarmCycleZeroLatched;
  uint8_t shortAlarmCycleZeroActive;
  uint8_t coordDiagnosticCycleZeroActive;
  uint8_t backupTimerArmed;
  uint8_t userDefinedBackupTimerArmed;
  uint8_t backupModeActive;
  uint8_t mmuFlashActive;
  uint8_t activePreemptIndex;
  uint8_t remoteManualPedAutoAdvance[INTERSECTION_RING_COUNT_MAX];
  uint8_t remoteManualTickAccumulator;
  uint8_t pedWalkServiceCyclePattern;
  uint8_t activeSequenceNumber;
  uint8_t automaticFlashEntryPositions[INTERSECTION_RING_COUNT_MAX];
  uint8_t automaticFlashExitPositions[INTERSECTION_RING_COUNT_MAX];
  IntersectionConfig_t config;
  IntersectionRingPlan_t activeRingPlans[INTERSECTION_RING_COUNT_MAX];
  uint16_t minGreenTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint32_t maxGreenTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint32_t runningMaxTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint32_t selectedNormalMaxTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t yellowTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t redClearTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t redRevertTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t dontWalkRevertTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t pedClearClearanceTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t passageTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t passageTimerTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t minimumGapTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t currentGapTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t gapTimerTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t maxInitialTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t initialGreenTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t initialActuationCount[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t pedWalkServicesThisCycle[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t vehicleCountGreen[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t conflictingVehicleCountGreen[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t reductionElapsedTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t reductionActive[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t dynamicMaxGapOutCount[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t dynamicMaxMaxOutCount[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t startupPending[INTERSECTION_RING_COUNT_MAX];
  uint8_t startupHold[INTERSECTION_RING_COUNT_MAX];
  uint8_t directVehicleInputs[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t directPedInputs[INTERSECTION_PHASE_COUNT_MAX];
  uint32_t coordSyncTicks;
  uint16_t preemptDelayTicks[INTERSECTION_PREEMPT_COUNT_MAX];
  uint16_t preemptMinimumDurationTicks[INTERSECTION_PREEMPT_COUNT_MAX];
  uint16_t preemptMinimumGreenTicks[INTERSECTION_PREEMPT_COUNT_MAX];
  uint16_t preemptMinimumWalkTicks[INTERSECTION_PREEMPT_COUNT_MAX];
  uint16_t preemptEnterPedClearTicks[INTERSECTION_PREEMPT_COUNT_MAX];
  uint16_t preemptTrackGreenTicks[INTERSECTION_PREEMPT_COUNT_MAX];
  uint16_t preemptDwellGreenTicks[INTERSECTION_PREEMPT_COUNT_MAX];
  uint32_t preemptMaximumPresenceTicks[INTERSECTION_PREEMPT_COUNT_MAX];
  uint16_t preemptEnterYellowTicks[INTERSECTION_PREEMPT_COUNT_MAX];
  uint16_t preemptEnterRedClearTicks[INTERSECTION_PREEMPT_COUNT_MAX];
  uint16_t preemptTrackYellowTicks[INTERSECTION_PREEMPT_COUNT_MAX];
  uint16_t preemptTrackRedClearTicks[INTERSECTION_PREEMPT_COUNT_MAX];
  uint32_t backupTimerTicksRemaining;
  uint32_t userDefinedBackupTimerTicksRemaining;
  uint32_t startUpFlashTicksRemaining;
  uint32_t preemptStageTicks;
  uint32_t preemptPresenceTicks;
  uint32_t phaseDemandWaitTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint32_t preemptShortServiceOrder[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t preemptCyclingInitialized;
  uint8_t preemptCyclingDemandFilterActive;
  uint8_t linkedPreemptSourceIndex;
  uint8_t linkedPreemptTargetIndex;
  IntersectionPedInterval_t overlapPedIntervals[
    INTERSECTION_OVERLAP_COUNT_MAX];
  uint16_t overlapPedIntervalElapsedTicks[INTERSECTION_OVERLAP_COUNT_MAX];
  uint8_t overlapPedServiceWindowActive[INTERSECTION_OVERLAP_COUNT_MAX];
  uint8_t overlapTrailStage[INTERSECTION_OVERLAP_COUNT_MAX];
  IntersectionOutputAspect_t overlapLastBaseAspect[
    INTERSECTION_OVERLAP_COUNT_MAX];
  uint16_t overlapTrailTicksRemaining[
    INTERSECTION_OVERLAP_COUNT_MAX];
  IntersectionPhaseInterval_t preemptEntryPhaseIntervals[
    INTERSECTION_PHASE_COUNT_MAX];
  IntersectionPedInterval_t preemptEntryPedIntervals[
    INTERSECTION_PHASE_COUNT_MAX];
  uint32_t preemptEntryPhaseElapsedTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint16_t preemptEntryPedElapsedTicks[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t systemPhaseOmit[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t systemPedOmit[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t systemPhaseHold[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t systemPhaseForceOff[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t systemVehCalls[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t systemPedCalls[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t systemRingStopTime[INTERSECTION_RING_COUNT_MAX];
  uint8_t systemRingForceOff[INTERSECTION_RING_COUNT_MAX];
  uint8_t systemRingMax2[INTERSECTION_RING_COUNT_MAX];
  uint8_t systemRingMaxInhibit[INTERSECTION_RING_COUNT_MAX];
  uint8_t systemRingPedRecycle[INTERSECTION_RING_COUNT_MAX];
  uint8_t systemRingRedRest[INTERSECTION_RING_COUNT_MAX];
  uint8_t systemRingOmitRedClear[INTERSECTION_RING_COUNT_MAX];
  uint8_t systemRingMax3[INTERSECTION_RING_COUNT_MAX];
  uint8_t coordinationAlarmPattern;
  uint8_t coordinationAlarmMissedCycles;
  uint8_t coordDiagnosticFaultPhaseMask;
  uint8_t coordDiagnosticRecoveryCyclesRemaining;
  uint8_t coordDiagnosticRetryCyclesRemaining;
  uint8_t coordCycleFaultAges[INTERSECTION_PHASE_COUNT_MAX];
  uint8_t coordCycleServedThisCycle[INTERSECTION_PHASE_COUNT_MAX];
  uint32_t coordDiagnosticCycleTicks;
  IntersectionRuntime_t runtime;
} IntersectionEngine_t;

void IntersectionEngineInit(IntersectionEngine_t *engine);
uint8_t IntersectionEngineLoadConfig(IntersectionEngine_t *engine,
                                     const IntersectionConfig_t *config);
void IntersectionEngineReset(IntersectionEngine_t *engine);
void IntersectionEngineTick(IntersectionEngine_t *engine);
uint8_t IntersectionEngineSetDetectorCall(IntersectionEngine_t *engine,
                                          uint8_t phaseNumber,
                                          uint8_t active);
uint8_t IntersectionEngineSetVehicleDetectorInput(IntersectionEngine_t *engine,
                                                  uint8_t detectorNumber,
                                                  uint8_t active);
uint8_t IntersectionEngineSetVehicleDetectorRemoteActuation(
  IntersectionEngine_t *engine,
  uint8_t detectorNumber,
  uint8_t active);
uint8_t IntersectionEngineSetPedCall(IntersectionEngine_t *engine,
                                     uint8_t phaseNumber,
                                     uint8_t active);
uint8_t IntersectionEngineSetPedestrianDetectorInput(
  IntersectionEngine_t *engine,
  uint8_t detectorNumber,
  uint8_t active);
uint8_t IntersectionEngineSetPedestrianDetectorRemoteActuation(
  IntersectionEngine_t *engine,
  uint8_t detectorNumber,
  uint8_t active);
uint8_t IntersectionEngineSetSystemPatternControl(IntersectionEngine_t *engine,
                                                  uint8_t systemPatternControl);
uint8_t IntersectionEngineSetSystemSyncControl(IntersectionEngine_t *engine,
                                               uint8_t systemSyncControlSeconds);
uint8_t IntersectionEngineSetActionPlanControl(IntersectionEngine_t *engine,
                                               uint8_t actionPlanControl);
uint8_t IntersectionEngineGetActionPlanControl(const IntersectionEngine_t *engine,
                                               uint8_t *actionPlanControl);
uint8_t IntersectionEngineSetUnitControl(IntersectionEngine_t *engine,
                                         uint8_t unitControl);
uint8_t IntersectionEngineGetUnitControl(const IntersectionEngine_t *engine,
                                         uint8_t *unitControl);
uint8_t IntersectionEngineSetRemoteManualControlTimeout(
  IntersectionEngine_t *engine,
  uint8_t timeoutSeconds);
uint8_t IntersectionEngineGetRemoteManualControlTimeout(
  const IntersectionEngine_t *engine,
  uint8_t *timeoutSeconds);
uint8_t IntersectionEngineSetRemoteManualIntervalAdvance(
  IntersectionEngine_t *engine,
  uint8_t active);
uint8_t IntersectionEngineGetRemoteManualIntervalAdvance(
  const IntersectionEngine_t *engine,
  uint8_t *active);
uint8_t IntersectionEngineResetUserDefinedBackupTimer(
  IntersectionEngine_t *engine);
uint8_t IntersectionEngineSetLocalInterconnectCommand(
  IntersectionEngine_t *engine,
  uint8_t command);
uint8_t IntersectionEngineSetLocalInterconnectInputsValid(
  IntersectionEngine_t *engine,
  uint8_t valid);
uint8_t IntersectionEngineSetLocalDimmingInput(IntersectionEngine_t *engine,
                                               uint8_t active);
uint8_t IntersectionEngineSetPhaseOmitControl(IntersectionEngine_t *engine,
                                              uint8_t phaseNumber,
                                              uint8_t active);
uint8_t IntersectionEngineGetPhaseOmitControl(const IntersectionEngine_t *engine,
                                              uint8_t phaseNumber,
                                              uint8_t *active);
uint8_t IntersectionEngineSetPedOmitControl(IntersectionEngine_t *engine,
                                            uint8_t phaseNumber,
                                            uint8_t active);
uint8_t IntersectionEngineGetPedOmitControl(const IntersectionEngine_t *engine,
                                            uint8_t phaseNumber,
                                            uint8_t *active);
uint8_t IntersectionEngineSetPhaseHoldControl(IntersectionEngine_t *engine,
                                              uint8_t phaseNumber,
                                              uint8_t active);
uint8_t IntersectionEngineGetPhaseHoldControl(const IntersectionEngine_t *engine,
                                              uint8_t phaseNumber,
                                              uint8_t *active);
uint8_t IntersectionEngineSetPhaseForceOffControl(IntersectionEngine_t *engine,
                                                  uint8_t phaseNumber,
                                                  uint8_t active);
uint8_t IntersectionEngineGetPhaseForceOffControl(
  const IntersectionEngine_t *engine,
  uint8_t phaseNumber,
  uint8_t *active);
uint8_t IntersectionEngineSetVehCallControl(IntersectionEngine_t *engine,
                                            uint8_t phaseNumber,
                                            uint8_t active);
uint8_t IntersectionEngineGetVehCallControl(const IntersectionEngine_t *engine,
                                            uint8_t phaseNumber,
                                            uint8_t *active);
uint8_t IntersectionEngineSetPedCallControl(IntersectionEngine_t *engine,
                                            uint8_t phaseNumber,
                                            uint8_t active);
uint8_t IntersectionEngineGetPedCallControl(const IntersectionEngine_t *engine,
                                            uint8_t phaseNumber,
                                            uint8_t *active);
uint8_t IntersectionEngineSetPreemptInput(IntersectionEngine_t *engine,
                                          uint8_t preemptNumber,
                                          uint8_t active);
uint8_t IntersectionEngineSetPreemptControlState(IntersectionEngine_t *engine,
                                                 uint8_t preemptNumber,
                                                 uint8_t active);
uint8_t IntersectionEngineSetMmuFlashControl(IntersectionEngine_t *engine,
                                             uint8_t active);
uint8_t IntersectionEngineSetRingStopTimeControl(IntersectionEngine_t *engine,
                                                 uint8_t ringNumber,
                                                 uint8_t active);
uint8_t IntersectionEngineGetRingStopTimeControl(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active);
uint8_t IntersectionEngineSetRingForceOffControl(IntersectionEngine_t *engine,
                                                 uint8_t ringNumber,
                                                 uint8_t active);
uint8_t IntersectionEngineGetRingForceOffControl(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active);
uint8_t IntersectionEngineSetRingMaximum2Control(IntersectionEngine_t *engine,
                                                 uint8_t ringNumber,
                                                 uint8_t active);
uint8_t IntersectionEngineGetRingMaximum2Control(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active);
uint8_t IntersectionEngineSetRingMaximumInhibitControl(
  IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t active);
uint8_t IntersectionEngineGetRingMaximumInhibitControl(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active);
uint8_t IntersectionEngineSetRingPedRecycleControl(
  IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t active);
uint8_t IntersectionEngineGetRingPedRecycleControl(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active);
uint8_t IntersectionEngineSetRingRedRestControl(IntersectionEngine_t *engine,
                                                uint8_t ringNumber,
                                                uint8_t active);
uint8_t IntersectionEngineGetRingRedRestControl(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active);
uint8_t IntersectionEngineSetRingOmitRedClearControl(
  IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t active);
uint8_t IntersectionEngineGetRingOmitRedClearControl(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active);
uint8_t IntersectionEngineSetRingMaximum3Control(IntersectionEngine_t *engine,
                                                 uint8_t ringNumber,
                                                 uint8_t active);
uint8_t IntersectionEngineGetRingMaximum3Control(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *active);
const IntersectionConfig_t *IntersectionEngineGetConfig(
  const IntersectionEngine_t *engine);
const IntersectionRuntime_t *IntersectionEngineGetRuntime(
  const IntersectionEngine_t *engine);
uint8_t IntersectionEngineGetActivePhaseForRing(
  const IntersectionEngine_t *engine,
  uint8_t ringNumber,
  uint8_t *phaseNumber);
uint8_t IntersectionEngineGetOutputIntentImage(
  const IntersectionEngine_t *engine,
  IntersectionOutputIntentImage_t *
  outputIntentImage);
uint8_t IntersectionEngineGetPhaseStatusGroup(
  const IntersectionEngine_t *engine,
  uint8_t groupNumber,
  IntersectionPhaseStatusGroup_t *
  statusGroup);
uint8_t IntersectionEngineGetRingStatus(const IntersectionEngine_t *engine,
                                        uint8_t ringNumber,
                                        uint8_t *ringStatus);
uint8_t IntersectionEngineGetChannelStatusGroup(
  const IntersectionEngine_t *engine,
  uint8_t groupNumber,
  IntersectionChannelStatusGroup_t
  *statusGroup);
uint8_t IntersectionEngineGetOverlapStatusGroup(
  const IntersectionEngine_t *engine,
  uint8_t groupNumber,
  IntersectionOverlapStatusGroup_t
  *statusGroup);
uint8_t IntersectionEngineGetPreemptStatusGroup(
  const IntersectionEngine_t *engine,
  uint8_t groupNumber,
  uint8_t *statusGroup);
uint8_t IntersectionEngineGetVehicleDetectorStatusGroup(
  const IntersectionEngine_t *engine,
  uint8_t groupNumber,
  uint8_t *statusGroup);
uint8_t IntersectionEngineGetPedestrianDetectorStatusGroup(
  const IntersectionEngine_t *engine,
  uint8_t groupNumber,
  uint8_t *statusGroup);
uint8_t IntersectionEngineSetSpecialFunctionOutputControl(
  IntersectionEngine_t *engine,
  uint8_t outputNumber,
  uint8_t active);
uint8_t IntersectionEngineGetSpecialFunctionOutputControl(
  const IntersectionEngine_t *engine,
  uint8_t outputNumber,
  uint8_t *active);
uint8_t IntersectionEngineGetSpecialFunctionOutputStatus(
  const IntersectionEngine_t *engine,
  uint8_t outputNumber,
  uint8_t *active);
uint8_t IntersectionEngineGetShortAlarmCycleZeroLatched(
  const IntersectionEngine_t *engine,
  uint8_t *active);
void IntersectionEngineAcknowledgeShortAlarmStatusRead(
  IntersectionEngine_t *engine);

#endif /* INTERSECTION_ENGINE_H */
