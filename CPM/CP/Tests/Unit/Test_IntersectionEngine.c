/*
 * Tests/Unit/Test_IntersectionEngine.c
 *
 * Unit tests for the new dual-ring controller-core timing engine.
 */
#include "unity.h"

#include "Domain/Intersection/IntersectionEngine.h"

#include <stdio.h>

static IntersectionEngine_t s_engine;

void setUp(void)
{
  IntersectionEngineInit(&s_engine);
}

void tearDown(void)
{
}

static void TickForTicks(uint32_t tickCount)
{
  uint32_t tickIndex;

  for (tickIndex = 0U; tickIndex < tickCount; tickIndex++)
  {
    IntersectionEngineTick(&s_engine);
  }
}

static const IntersectionRuntime_t *GetRuntime(void)
{
  return IntersectionEngineGetRuntime(&s_engine);
}

static void TickUntilPhaseInterval(uint8_t phaseNumber,
                                   IntersectionPhaseInterval_t interval,
                                   uint32_t maxTicks)
{
  uint32_t tickIndex;
  uint8_t phaseIndex = (uint8_t) (phaseNumber - 1U);
  char message[96];

  for (tickIndex = 0U; tickIndex < maxTicks; tickIndex++)
  {
    if (GetRuntime()->phases[phaseIndex].interval == interval)
    {
      return;
    }

    IntersectionEngineTick(&s_engine);
  }

  (void) snprintf(message,
                  sizeof(message),
                  "phase interval did not reach expected state (phase=%u interval=%d)",
                  (unsigned int) phaseNumber,
                  (int) GetRuntime()->phases[phaseIndex].interval);
  TEST_FAIL_MESSAGE(message);
}

static void TickUntilPedInterval(uint8_t phaseNumber,
                                 IntersectionPedInterval_t pedInterval,
                                 uint32_t maxTicks)
{
  uint32_t tickIndex;
  uint8_t phaseIndex = (uint8_t) (phaseNumber - 1U);

  for (tickIndex = 0U; tickIndex < maxTicks; tickIndex++)
  {
    if (GetRuntime()->phases[phaseIndex].pedInterval == pedInterval)
    {
      return;
    }

    IntersectionEngineTick(&s_engine);
  }

  TEST_FAIL_MESSAGE("ped interval did not reach expected state");
}

static void TickUntilActivePhase(uint8_t ringNumber,
                                 uint8_t phaseNumber,
                                 uint32_t maxTicks)
{
  uint32_t tickIndex;
  uint8_t activePhaseNumber = 0U;
  uint8_t otherActivePhaseNumber = 0U;
  char message[96];
  uint8_t ringIndex = (uint8_t) (ringNumber - 1U);
  uint8_t otherRingNumber = (uint8_t) ((ringNumber == 1U) ? 2U : 1U);
  uint8_t otherRingIndex = (uint8_t) (otherRingNumber - 1U);

  for (tickIndex = 0U; tickIndex < maxTicks; tickIndex++)
  {
    if (IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                ringNumber,
                                                &activePhaseNumber) != 0U)
    {
      if (activePhaseNumber == phaseNumber)
      {
        return;
      }
    }

    IntersectionEngineTick(&s_engine);
  }

  (void) snprintf(
    message,
    sizeof(message),
    "active phase did not reach expected value (r%u=%u st%d r%u=%u st%d)",
    (unsigned int) ringNumber,
    (unsigned int) activePhaseNumber,
    (int) GetRuntime()->rings[ringIndex].stage,
    (unsigned int) otherRingNumber,
    (unsigned int) ((IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                             otherRingNumber,
                                                             &otherActivePhaseNumber)
                      != 0U)
                       ? otherActivePhaseNumber
                       : 0U),
    (int) GetRuntime()->rings[otherRingIndex].stage);
  TEST_FAIL_MESSAGE(message);
}

static void TickUntilRingStage(uint8_t ringNumber,
                               IntersectionRingStage_t stage,
                               uint32_t maxTicks)
{
  uint32_t tickIndex;
  uint8_t ringIndex = (uint8_t) (ringNumber - 1U);

  for (tickIndex = 0U; tickIndex < maxTicks; tickIndex++)
  {
    if (GetRuntime()->rings[ringIndex].stage == stage)
    {
      return;
    }

    IntersectionEngineTick(&s_engine);
  }

  TEST_FAIL_MESSAGE("ring stage did not reach expected value");
}

static void TickUntilPreemptState(uint8_t preemptNumber,
                                  IntersectionPreemptState_t state,
                                  uint32_t maxTicks)
{
  uint32_t tickIndex;
  uint8_t preemptIndex = (uint8_t) (preemptNumber - 1U);
  char message[96];

  for (tickIndex = 0U; tickIndex < maxTicks; tickIndex++)
  {
    if (GetRuntime()->preemptStates[preemptIndex] == state)
    {
      return;
    }

    IntersectionEngineTick(&s_engine);
  }

  (void) snprintf(message,
                  sizeof(message),
                  "preempt state did not reach expected value (preempt=%u state=%d)",
                  (unsigned int) preemptNumber,
                  (int) GetRuntime()->preemptStates[preemptIndex]);
  TEST_FAIL_MESSAGE(message);
}

static IntersectionConfig_t MakeTwoPhasePerRingConfig(void)
{
  IntersectionConfig_t config;
  const uint8_t overlapIncluded[] = { 1U, 2U };
  uint8_t phaseIndex;
  uint8_t detectorIndex;

  IntersectionConfigInitDefaults(&config);
  config.phaseCount = 4U;
  config.ringCount = 2U;
  config.barrierCount = 2U;

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].phaseOptions = PHASE_OPTIONS_ENABLED;
    config.phases[phaseIndex].ring = (phaseIndex < 2U) ? 0U : 1U;
  }

  config.rings[0].phaseCount = 2U;
  config.rings[0].barrierPhaseCount = 1U;
  config.rings[0].phaseOrder[0] = 0U;
  config.rings[0].phaseOrder[1] = 1U;
  config.rings[1].phaseCount = 2U;
  config.rings[1].barrierPhaseCount = 1U;
  config.rings[1].phaseOrder[0] = 2U;
  config.rings[1].phaseOrder[1] = 3U;

  config.channels[0].controlSource = 1U;
  config.channels[0].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  config.channels[1].controlSource = 1U;
  config.channels[1].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN;
  config.channels[2].controlSource = 1U;
  config.channels[2].controlType = INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP;
  config.overlaps[0].type = INTERSECTION_OVERLAP_TYPE_NORMAL;
  config.overlaps[0].includedPhases.length = 2U;
  config.overlaps[0].includedPhases.values[0] = overlapIncluded[0];
  config.overlaps[0].includedPhases.values[1] = overlapIncluded[1];

  for (detectorIndex = config.phaseCount;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    config.vehicleDetectors[detectorIndex].callPhase = 0U;
  }

  for (detectorIndex = config.phaseCount;
       detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       detectorIndex++)
  {
    config.pedestrianDetectors[detectorIndex].callPhase = 0U;
  }

  return config;
}

static IntersectionConfig_t MakeThreePhasePerRingConfig(void)
{
  IntersectionConfig_t config;
  uint8_t phaseIndex;
  uint8_t detectorIndex;

  IntersectionConfigInitDefaults(&config);
  config.phaseCount = 6U;
  config.ringCount = 2U;
  config.barrierCount = 2U;

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].phaseOptions = PHASE_OPTIONS_ENABLED;
    config.phases[phaseIndex].ring = (phaseIndex < 3U) ? 0U : 1U;
  }

  config.rings[0].phaseCount = 3U;
  config.rings[0].barrierPhaseCount = 2U;
  config.rings[0].phaseOrder[0] = 0U;
  config.rings[0].phaseOrder[1] = 1U;
  config.rings[0].phaseOrder[2] = 2U;
  config.rings[1].phaseCount = 3U;
  config.rings[1].barrierPhaseCount = 2U;
  config.rings[1].phaseOrder[0] = 3U;
  config.rings[1].phaseOrder[1] = 4U;
  config.rings[1].phaseOrder[2] = 5U;

  for (detectorIndex = config.phaseCount;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    config.vehicleDetectors[detectorIndex].callPhase = 0U;
  }

  for (detectorIndex = config.phaseCount;
       detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       detectorIndex++)
  {
    config.pedestrianDetectors[detectorIndex].callPhase = 0U;
  }

  return config;
}

static IntersectionConfig_t MakeSingleModifierOverlapConfig(
  IntersectionOverlapType_t overlapType)
{
  IntersectionConfig_t config = MakeThreePhasePerRingConfig();
  uint8_t phaseIndex;

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].minGreenDs = 10U;
    config.phases[phaseIndex].yellowChangeDs = 3U;
    config.phases[phaseIndex].redClearDs = 2U;
    config.phases[phaseIndex].walkSeconds = 0U;
    config.phases[phaseIndex].pedClearSeconds = 0U;
  }

  config.channels[0].controlSource = 1U;
  config.channels[0].controlType = INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP;
  config.overlaps[0].type = (uint8_t) overlapType;
  config.overlaps[0].includedPhases.length = 1U;
  config.overlaps[0].includedPhases.values[0] = 1U;
  config.overlaps[0].modifierPhases.length = 1U;
  config.overlaps[0].modifierPhases.values[0] = 2U;

  return config;
}

void test_load_config_starts_all_red_until_first_tick(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeTwoPhasePerRingConfig();

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_RED_REST,
                        runtime->rings[0].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[0].interval);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_RED,
                        outputIntentImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_RED,
                        outputIntentImage.channels[1]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_RED,
                        outputIntentImage.channels[2]);

  IntersectionEngineTick(&s_engine);
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[2].interval);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_GREEN,
                        outputIntentImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_RED,
                        outputIntentImage.channels[1]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_GREEN,
                        outputIntentImage.channels[2]);
}

void test_competing_call_advances_to_next_phase_after_clearance(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));

  TickForTicks(500U);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[0].interval);

  TickForTicks(400U);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED_CLEAR,
                        runtime->phases[0].interval);

  TickForTicks(200U);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_WAIT_BARRIER,
                        runtime->rings[0].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[0].interval);
}

void test_phase_status_group_sets_next_and_vehicle_call_bits(void)
{
  IntersectionConfig_t config;
  IntersectionPhaseStatusGroup_t statusGroup;

  config = MakeTwoPhasePerRingConfig();

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));

  TEST_ASSERT_TRUE(IntersectionEngineGetPhaseStatusGroup(&s_engine,
                                                         1U,
                                                         &statusGroup));
  TEST_ASSERT_BITS_HIGH(0x05U, statusGroup.greens);
  TEST_ASSERT_BITS_HIGH(0x02U, statusGroup.vehCalls);
}

void test_barrier_crossing_waits_until_both_rings_are_ready(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  uint8_t phaseNumber;

  config = MakeTwoPhasePerRingConfig();

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));

  TickForTicks(1100U);
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_WAIT_BARRIER,
                        runtime->rings[0].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[0].interval);
  TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                           2U,
                                                           &phaseNumber));
  TEST_ASSERT_EQUAL_UINT8(3U, phaseNumber);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[2].interval);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 0U));

  TickForTicks(610U);
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                           1U,
                                                           &phaseNumber));
  TEST_ASSERT_EQUAL_UINT8(2U, phaseNumber);
  TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                           2U,
                                                           &phaseNumber));
  TEST_ASSERT_EQUAL_UINT8(4U, phaseNumber);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[1].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[3].interval);
} /* test_barrier_crossing_waits_until_both_rings_are_ready */

void test_ped_call_services_walk_then_clear_on_phase_pedestrian_channel(void)
{
  IntersectionConfig_t config;
  IntersectionPhaseStatusGroup_t statusGroup;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].walkSeconds = 1U;
  config.phases[0].pedClearSeconds = 2U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 0U));

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineGetPhaseStatusGroup(&s_engine,
                                                         1U,
                                                         &statusGroup));
  TEST_ASSERT_BITS_HIGH(0x01U, statusGroup.walks);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_GREEN,
                        outputIntentImage.channels[1]);

  TickForTicks(100U);
  TEST_ASSERT_TRUE(IntersectionEngineGetPhaseStatusGroup(&s_engine,
                                                         1U,
                                                         &statusGroup));
  TEST_ASSERT_BITS_HIGH(0x01U, statusGroup.pedClears);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_YELLOW,
                        outputIntentImage.channels[1]);

  TickForTicks(200U);
  TEST_ASSERT_TRUE(IntersectionEngineGetPhaseStatusGroup(&s_engine,
                                                         1U,
                                                         &statusGroup));
  TEST_ASSERT_BITS_HIGH(0x01U, statusGroup.dontWalks);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_RED,
                        outputIntentImage.channels[1]);
} /* test_ped_call_services_walk_then_clear_on_phase_pedestrian_channel */

void test_remote_preempt_control_drives_runtime_state_and_status_group(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  uint8_t statusGroup;

  config = MakeTwoPhasePerRingConfig();
  config.preempts[0].control = 0x10U;
  config.preempts[0].minimumGreenSeconds = 0U;
  config.preempts[0].trackGreenSeconds = 1U;
  config.preempts[0].dwellGreenSeconds = 1U;
  config.preempts[0].minimumDurationSeconds = 1U;
  config.preempts[0].enterYellowChangeDs = 0U;
  config.preempts[0].enterRedClearDs = 0U;
  config.preempts[0].trackYellowChangeDs = 0U;
  config.preempts[0].trackRedClearDs = 0U;
  config.preempts[0].trackPhases.length = 1U;
  config.preempts[0].trackPhases.values[0] = 2U;
  config.preempts[0].dwellPhases.length = 1U;
  config.preempts[0].dwellPhases.values[0] = 2U;
  config.preempts[0].exitType = INTERSECTION_PREEMPT_EXIT_TYPE_SHORT_SERVICE;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_PREEMPT, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->preemptStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_TRACK_SERVICE,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[1].interval);
  TEST_ASSERT_TRUE(IntersectionEngineGetPreemptStatusGroup(&s_engine,
                                                           1U,
                                                           &statusGroup));
  TEST_ASSERT_EQUAL_UINT8(0x01U, statusGroup);

  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 0U));
  TickForTicks(220U);

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->preemptStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_NOT_ACTIVE,
                        runtime->preemptStates[0]);
  TEST_ASSERT_TRUE(IntersectionEngineGetPreemptStatusGroup(&s_engine,
                                                           1U,
                                                           &statusGroup));
  TEST_ASSERT_EQUAL_UINT8(0x00U, statusGroup);
} /* test_remote_preempt_control_drives_runtime_state_and_status_group */

void test_preempt_entry_outputs_follow_enter_yellow_and_red_clear(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.preempts[0].control = 0x10U;
  config.preempts[0].minimumGreenSeconds = 0U;
  config.preempts[0].minimumWalkSeconds = 0U;
  config.preempts[0].enterPedClearSeconds = 0U;
  config.preempts[0].trackGreenSeconds = 0U;
  config.preempts[0].dwellGreenSeconds = 1U;
  config.preempts[0].enterYellowChangeDs = 20U;
  config.preempts[0].enterRedClearDs = 10U;
  config.preempts[0].dwellPhases.length = 1U;
  config.preempts[0].dwellPhases.values[0] = 2U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_ENTRY_STARTED,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[2].interval);

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_RED_CLEAR, 250U);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_ENTRY_STARTED,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED_CLEAR,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED_CLEAR,
                        runtime->phases[2].interval);

  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_DWELL, 150U);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_DWELL,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[1].interval);
} /* test_preempt_entry_outputs_follow_enter_yellow_and_red_clear */

void test_preempt_track_clear_advances_through_advanced_preempt(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.preempts[0].control = 0x10U;
  config.preempts[0].minimumGreenSeconds = 0U;
  config.preempts[0].minimumWalkSeconds = 0U;
  config.preempts[0].enterPedClearSeconds = 0U;
  config.preempts[0].enterYellowChangeDs = 0U;
  config.preempts[0].enterRedClearDs = 0U;
  config.preempts[0].trackGreenSeconds = 1U;
  config.preempts[0].trackYellowChangeDs = 20U;
  config.preempts[0].trackRedClearDs = 10U;
  config.preempts[0].dwellGreenSeconds = 1U;
  config.preempts[0].trackPhases.length = 1U;
  config.preempts[0].trackPhases.values[0] = 2U;
  config.preempts[0].dwellPhases.length = 1U;
  config.preempts[0].dwellPhases.values[0] = 2U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_TRACK_SERVICE,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[1].interval);

  TickUntilPreemptState(1U,
                        INTERSECTION_PREEMPT_STATE_ADVANCED_PREEMPT,
                        150U);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_ADVANCED_PREEMPT,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[1].interval);

  TickUntilPhaseInterval(2U, INTERSECTION_PHASE_INTERVAL_RED_CLEAR, 250U);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_ADVANCED_PREEMPT,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED_CLEAR,
                        runtime->phases[1].interval);

  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_DWELL, 150U);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_DWELL,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[1].interval);
} /* test_preempt_track_clear_advances_through_advanced_preempt */

void test_preempt_link_activates_higher_priority_preempt_call(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.preempts[0].control = 0x10U;
  config.preempts[0].minimumGreenSeconds = 0U;
  config.preempts[0].minimumWalkSeconds = 0U;
  config.preempts[0].enterPedClearSeconds = 0U;
  config.preempts[0].trackGreenSeconds = 1U;
  config.preempts[0].dwellGreenSeconds = 1U;
  config.preempts[0].minimumDurationSeconds = 1U;
  config.preempts[0].enterYellowChangeDs = 0U;
  config.preempts[0].enterRedClearDs = 0U;
  config.preempts[0].trackYellowChangeDs = 0U;
  config.preempts[0].trackRedClearDs = 0U;
  config.preempts[0].trackPhases.length = 1U;
  config.preempts[0].trackPhases.values[0] = 2U;
  config.preempts[0].dwellPhases.length = 1U;
  config.preempts[0].dwellPhases.values[0] = 2U;

  config.preempts[1].control = 0x10U;
  config.preempts[1].minimumGreenSeconds = 0U;
  config.preempts[1].minimumWalkSeconds = 0U;
  config.preempts[1].enterPedClearSeconds = 0U;
  config.preempts[1].trackGreenSeconds = 0U;
  config.preempts[1].dwellGreenSeconds = 1U;
  config.preempts[1].minimumDurationSeconds = 1U;
  config.preempts[1].enterYellowChangeDs = 0U;
  config.preempts[1].enterRedClearDs = 0U;
  config.preempts[1].dwellPhases.length = 1U;
  config.preempts[1].dwellPhases.values[0] = 4U;
  config.preempts[1].link = 1U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 2U, 1U));
  IntersectionEngineTick(&s_engine);

  TickUntilPreemptState(2U, INTERSECTION_PREEMPT_STATE_DWELL, 150U);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_UINT8(2U, runtime->preemptStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[3].interval);

  TickUntilPreemptState(2U, INTERSECTION_PREEMPT_STATE_LINK_ACTIVE, 150U);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_LINK_ACTIVE,
                        runtime->preemptStates[1]);

  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_TRACK_SERVICE, 150U);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->preemptStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_LINK_ACTIVE,
                        runtime->preemptStates[1]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[1].interval);

  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 2U, 0U));
  TickForTicks(250U);

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->preemptStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_NOT_ACTIVE,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_NOT_ACTIVE,
                        runtime->preemptStates[1]);
} /* test_preempt_link_activates_higher_priority_preempt_call */

void test_preempt_exit_phases_transfer_runtime_to_configured_exit_phases(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.preempts[0].control = 0x10U;
  config.preempts[0].minimumGreenSeconds = 0U;
  config.preempts[0].minimumWalkSeconds = 0U;
  config.preempts[0].enterPedClearSeconds = 0U;
  config.preempts[0].trackGreenSeconds = 0U;
  config.preempts[0].dwellGreenSeconds = 1U;
  config.preempts[0].minimumDurationSeconds = 1U;
  config.preempts[0].enterYellowChangeDs = 0U;
  config.preempts[0].enterRedClearDs = 0U;
  config.preempts[0].dwellPhases.length = 2U;
  config.preempts[0].dwellPhases.values[0] = 2U;
  config.preempts[0].dwellPhases.values[1] = 4U;
  config.preempts[0].exitPhases.length = 2U;
  config.preempts[0].exitPhases.values[0] = 1U;
  config.preempts[0].exitPhases.values[1] = 3U;
  config.preempts[0].exitType = INTERSECTION_PREEMPT_EXIT_TYPE_EXIT_PHASES;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_DWELL, 25U);

  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 0U));
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_EXIT_STARTED, 150U);
  IntersectionEngineTick(&s_engine);

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->preemptStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_NOT_ACTIVE,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_GREEN,
                        runtime->rings[0].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_GREEN,
                        runtime->rings[1].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[2].interval);
} /* test_preempt_exit_phases_transfer_runtime_to_configured_exit_phases */

void test_preempt_queue_delay_recovery_enters_highest_weighted_demand_phase(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.preempts[0].control = 0x10U;
  config.preempts[0].minimumGreenSeconds = 0U;
  config.preempts[0].minimumWalkSeconds = 0U;
  config.preempts[0].enterPedClearSeconds = 0U;
  config.preempts[0].trackGreenSeconds = 0U;
  config.preempts[0].dwellGreenSeconds = 1U;
  config.preempts[0].minimumDurationSeconds = 1U;
  config.preempts[0].enterYellowChangeDs = 0U;
  config.preempts[0].enterRedClearDs = 0U;
  config.preempts[0].dwellPhases.length = 2U;
  config.preempts[0].dwellPhases.values[0] = 1U;
  config.preempts[0].dwellPhases.values[1] = 3U;
  config.preempts[0].exitType =
    INTERSECTION_PREEMPT_EXIT_TYPE_QUEUE_DELAY_RECOVERY;
  config.preemptQueueDelayWeights[0][1] = 200U;
  config.preemptQueueDelayWeights[0][3] = 500U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_DWELL, 25U);
  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 4U, 1U));
  TickForTicks(5U);

  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 0U));
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_EXIT_STARTED, 150U);
  IntersectionEngineTick(&s_engine);

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->preemptStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_RED_REST,
                        runtime->rings[0].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_GREEN,
                        runtime->rings[1].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[3].interval);
} /* test_preempt_queue_delay_recovery_enters_highest_weighted_demand_phase */

void test_preempt_queue_delay_recovery_falls_back_to_longest_waiting_phase(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.preempts[0].control = 0x10U;
  config.preempts[0].minimumGreenSeconds = 0U;
  config.preempts[0].minimumWalkSeconds = 0U;
  config.preempts[0].enterPedClearSeconds = 0U;
  config.preempts[0].trackGreenSeconds = 0U;
  config.preempts[0].dwellGreenSeconds = 1U;
  config.preempts[0].minimumDurationSeconds = 1U;
  config.preempts[0].enterYellowChangeDs = 0U;
  config.preempts[0].enterRedClearDs = 0U;
  config.preempts[0].dwellPhases.length = 2U;
  config.preempts[0].dwellPhases.values[0] = 1U;
  config.preempts[0].dwellPhases.values[1] = 3U;
  config.preempts[0].exitType =
    INTERSECTION_PREEMPT_EXIT_TYPE_QUEUE_DELAY_RECOVERY;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_DWELL, 25U);
  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 2U, 1U));
  TickForTicks(50U);
  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 4U, 1U));
  TickForTicks(5U);

  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 0U));
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_EXIT_STARTED, 150U);
  IntersectionEngineTick(&s_engine);

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->preemptStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_GREEN,
                        runtime->rings[0].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_RED_REST,
                        runtime->rings[1].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[1].interval);
} /* test_preempt_queue_delay_recovery_falls_back_to_longest_waiting_phase */

void test_preempt_short_service_enters_first_short_service_phase(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 20U;
  config.phases[2].minGreenDs = 10U;
  config.preempts[0].control = 0x10U;
  config.preempts[0].minimumGreenSeconds = 2U;
  config.preempts[0].minimumWalkSeconds = 0U;
  config.preempts[0].enterPedClearSeconds = 0U;
  config.preempts[0].trackGreenSeconds = 0U;
  config.preempts[0].dwellGreenSeconds = 1U;
  config.preempts[0].minimumDurationSeconds = 1U;
  config.preempts[0].enterYellowChangeDs = 0U;
  config.preempts[0].enterRedClearDs = 0U;
  config.preempts[0].dwellPhases.length = 2U;
  config.preempts[0].dwellPhases.values[0] = 2U;
  config.preempts[0].dwellPhases.values[1] = 4U;
  config.preempts[0].exitType = INTERSECTION_PREEMPT_EXIT_TYPE_SHORT_SERVICE;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TickForTicks(150U);
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_DWELL, 150U);

  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 0U));
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_EXIT_STARTED, 150U);
  IntersectionEngineTick(&s_engine);

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->preemptStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_GREEN,
                        runtime->rings[0].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_RED_REST,
                        runtime->rings[1].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);
} /* test_preempt_short_service_enters_first_short_service_phase */

void test_preempt_exit_coord_returns_to_current_coordination_cycle_position(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  uint8_t phaseIndex;

  config = MakeTwoPhasePerRingConfig();
  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].walkSeconds = 0U;
    config.phases[phaseIndex].pedClearSeconds = 0U;
  }

  config.coordination.patterns[0].cycleTimeSeconds = 60U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;
  config.preempts[0].control = 0x10U;
  config.preempts[0].minimumGreenSeconds = 0U;
  config.preempts[0].minimumWalkSeconds = 0U;
  config.preempts[0].enterPedClearSeconds = 0U;
  config.preempts[0].trackGreenSeconds = 0U;
  config.preempts[0].dwellGreenSeconds = 1U;
  config.preempts[0].minimumDurationSeconds = 1U;
  config.preempts[0].enterYellowChangeDs = 0U;
  config.preempts[0].enterRedClearDs = 0U;
  config.preempts[0].dwellPhases.length = 2U;
  config.preempts[0].dwellPhases.values[0] = 1U;
  config.preempts[0].dwellPhases.values[1] = 3U;
  config.preempts[0].exitType = INTERSECTION_PREEMPT_EXIT_TYPE_EXIT_COORD;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_DWELL, 25U);
  TickForTicks(2500U);

  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 0U));
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_EXIT_STARTED, 150U);
  IntersectionEngineTick(&s_engine);

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->preemptStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_COORDINATED, runtime->mode);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_GREEN,
                        runtime->rings[0].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_GREEN,
                        runtime->rings[1].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[1].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[3].interval);
} /* test_preempt_exit_coord_returns_to_current_coordination_cycle_position */

void test_preempt_flash_dwell_ignores_cycling_phase_programming(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  IntersectionOutputIntentImage_t outputIntentImage;
  uint8_t phaseIndex;

  config = MakeThreePhasePerRingConfig();

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].minGreenDs = 1U;
    config.phases[phaseIndex].yellowChangeDs = 1U;
    config.phases[phaseIndex].redClearDs = 1U;
    config.phases[phaseIndex].walkSeconds = 0U;
    config.phases[phaseIndex].pedClearSeconds = 0U;
  }

  config.preempts[0].control = 0x18U;
  config.preempts[0].minimumGreenSeconds = 0U;
  config.preempts[0].minimumWalkSeconds = 0U;
  config.preempts[0].enterPedClearSeconds = 0U;
  config.preempts[0].trackGreenSeconds = 0U;
  config.preempts[0].dwellGreenSeconds = 1U;
  config.preempts[0].minimumDurationSeconds = 5U;
  config.preempts[0].enterYellowChangeDs = 0U;
  config.preempts[0].enterRedClearDs = 0U;
  config.preempts[0].dwellPhases.length = 2U;
  config.preempts[0].dwellPhases.values[0] = 1U;
  config.preempts[0].dwellPhases.values[1] = 4U;
  config.preempts[0].dwellOverlaps.length = 1U;
  config.preempts[0].dwellOverlaps.values[0] = 1U;
  config.preempts[0].cyclingPhases.length = 2U;
  config.preempts[0].cyclingPhases.values[0] = 2U;
  config.preempts[0].cyclingPhases.values[1] = 5U;
  config.preempts[0].cyclingOverlaps.length = 1U;
  config.preempts[0].cyclingOverlaps.values[0] = 2U;
  config.channels[0].controlSource = 1U;
  config.channels[0].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  config.channels[1].controlSource = 2U;
  config.channels[1].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  config.channels[2].controlSource = 1U;
  config.channels[2].controlType = INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP;
  config.channels[3].controlSource = 2U;
  config.channels[3].controlType = INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP;
  config.overlaps[0].type = INTERSECTION_OVERLAP_TYPE_NORMAL;
  config.overlaps[0].includedPhases.length = 1U;
  config.overlaps[0].includedPhases.values[0] = 1U;
  config.overlaps[1].type = INTERSECTION_OVERLAP_TYPE_NORMAL;
  config.overlaps[1].includedPhases.length = 1U;
  config.overlaps[1].includedPhases.values[0] = 2U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_DWELL, 25U);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 5U, 1U));
  TickForTicks(200U);

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_DWELL,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[3].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[1].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[4].interval);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW,
                        outputIntentImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_RED,
                        outputIntentImage.channels[1]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW,
                        outputIntentImage.channels[2]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_RED,
                        outputIntentImage.channels[3]);
} /* test_preempt_flash_dwell_ignores_cycling_phase_programming */

void test_preempt_cycling_phases_begin_after_dwell_interval(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  IntersectionOutputIntentImage_t outputIntentImage;
  uint8_t activePhaseNumber = 0U;
  uint8_t phaseIndex;

  config = MakeThreePhasePerRingConfig();

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].minGreenDs = 1U;
    config.phases[phaseIndex].yellowChangeDs = 1U;
    config.phases[phaseIndex].redClearDs = 1U;
    config.phases[phaseIndex].walkSeconds = 0U;
    config.phases[phaseIndex].pedClearSeconds = 0U;
  }

  config.preempts[0].control = 0x10U;
  config.preempts[0].minimumGreenSeconds = 0U;
  config.preempts[0].minimumWalkSeconds = 0U;
  config.preempts[0].enterPedClearSeconds = 0U;
  config.preempts[0].trackGreenSeconds = 0U;
  config.preempts[0].dwellGreenSeconds = 1U;
  config.preempts[0].minimumDurationSeconds = 5U;
  config.preempts[0].enterYellowChangeDs = 0U;
  config.preempts[0].enterRedClearDs = 0U;
  config.preempts[0].dwellPhases.length = 2U;
  config.preempts[0].dwellPhases.values[0] = 1U;
  config.preempts[0].dwellPhases.values[1] = 4U;
  config.preempts[0].dwellOverlaps.length = 1U;
  config.preempts[0].dwellOverlaps.values[0] = 1U;
  config.preempts[0].cyclingPhases.length = 2U;
  config.preempts[0].cyclingPhases.values[0] = 2U;
  config.preempts[0].cyclingPhases.values[1] = 5U;
  config.preempts[0].cyclingOverlaps.length = 1U;
  config.preempts[0].cyclingOverlaps.values[0] = 2U;
  config.channels[2].controlSource = 1U;
  config.channels[2].controlType = INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP;
  config.channels[3].controlSource = 2U;
  config.channels[3].controlType = INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP;
  config.overlaps[0].type = INTERSECTION_OVERLAP_TYPE_NORMAL;
  config.overlaps[0].includedPhases.length = 1U;
  config.overlaps[0].includedPhases.values[0] = 1U;
  config.overlaps[1].type = INTERSECTION_OVERLAP_TYPE_NORMAL;
  config.overlaps[1].includedPhases.length = 1U;
  config.overlaps[1].includedPhases.values[0] = 2U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_DWELL, 25U);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 5U, 1U));
  TickForTicks(50U);

  TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                           1U,
                                                           &activePhaseNumber));
  TEST_ASSERT_EQUAL_UINT8(1U, activePhaseNumber);
  TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                           2U,
                                                           &activePhaseNumber));
  TEST_ASSERT_EQUAL_UINT8(4U, activePhaseNumber);

  TickUntilActivePhase(1U, 2U, 200U);
  TickUntilActivePhase(2U, 5U, 200U);

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_DWELL,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[1].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[4].interval);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_RED,
                        outputIntentImage.channels[2]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_GREEN,
                        outputIntentImage.channels[3]);
} /* test_preempt_cycling_phases_begin_after_dwell_interval */

void test_preempt_cycling_ped_service_starts_only_after_cycling_begins(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  uint8_t phaseIndex;

  config = MakeThreePhasePerRingConfig();

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].minGreenDs = 1U;
    config.phases[phaseIndex].yellowChangeDs = 1U;
    config.phases[phaseIndex].redClearDs = 1U;
    config.phases[phaseIndex].walkSeconds = 0U;
    config.phases[phaseIndex].pedClearSeconds = 0U;
  }

  config.phases[1].walkSeconds = 1U;
  config.phases[1].pedClearSeconds = 1U;
  config.phases[1].pedDelayDs = 0U;
  config.preempts[0].control = 0x10U;
  config.preempts[0].minimumGreenSeconds = 0U;
  config.preempts[0].minimumWalkSeconds = 0U;
  config.preempts[0].enterPedClearSeconds = 0U;
  config.preempts[0].trackGreenSeconds = 0U;
  config.preempts[0].dwellGreenSeconds = 1U;
  config.preempts[0].minimumDurationSeconds = 5U;
  config.preempts[0].enterYellowChangeDs = 0U;
  config.preempts[0].enterRedClearDs = 0U;
  config.preempts[0].dwellPhases.length = 2U;
  config.preempts[0].dwellPhases.values[0] = 1U;
  config.preempts[0].dwellPhases.values[1] = 4U;
  config.preempts[0].cyclingPhases.length = 2U;
  config.preempts[0].cyclingPhases.values[0] = 2U;
  config.preempts[0].cyclingPhases.values[1] = 5U;
  config.preempts[0].cyclingPeds.length = 1U;
  config.preempts[0].cyclingPeds.values[0] = 2U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);
  TickUntilPreemptState(1U, INTERSECTION_PREEMPT_STATE_DWELL, 25U);
  TickForTicks(50U);

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_DONT_WALK,
                        runtime->phases[1].pedInterval);

  TickUntilActivePhase(1U, 2U, 200U);
  TickUntilPedInterval(2U, INTERSECTION_PED_INTERVAL_WALK, 50U);

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PREEMPT_STATE_DWELL,
                        runtime->preemptStates[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        runtime->phases[1].pedInterval);
} /* test_preempt_cycling_ped_service_starts_only_after_cycling_begins */

void test_system_pattern_command_activates_coordination_status(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.coordination.patterns[0].cycleTimeSeconds = 60U;
  config.coordination.patterns[0].offsetTimeSeconds = 10U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemSyncControl(&s_engine, 15U));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));

  IntersectionEngineTick(&s_engine);
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_COORDINATED, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->coordPatternStatus);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_LOCAL_FREE_STATUS_NOT_FREE,
                          runtime->localFreeStatus);
  TEST_ASSERT_EQUAL_UINT16(15U, runtime->coordSyncStatusSeconds);
  TEST_ASSERT_EQUAL_UINT16(54U, runtime->coordCycleStatusSeconds);
}

void test_short_alarm_cycle_zero_latches_until_acknowledged(void)
{
  IntersectionConfig_t config;
  uint8_t active = 0U;

  config = MakeTwoPhasePerRingConfig();
  config.coordination.patterns[0].cycleTimeSeconds = 80U;
  config.coordination.patterns[0].offsetTimeSeconds = 10U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemSyncControl(&s_engine, 10U));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));

  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineGetShortAlarmCycleZeroLatched(&s_engine,
                                                                   &active));
  TEST_ASSERT_EQUAL_UINT8(1U, active);

  IntersectionEngineAcknowledgeShortAlarmStatusRead(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineGetShortAlarmCycleZeroLatched(&s_engine,
                                                                   &active));
  TEST_ASSERT_EQUAL_UINT8(0U, active);
}

void test_coord_cycle_fault_sets_after_two_coordinated_cycles_and_calls_free(
  void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.coordination.patterns[0].cycleTimeSeconds = 60U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));

  TickForTicks(16100U);
  runtime = GetRuntime();

  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FREE, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_LOCAL_FREE_STATUS_FAILED,
                          runtime->localFreeStatus);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->coordCycleFaultActive);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->coordFaultActive);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->coordFailActive);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->cycleFailActive);
}

void test_coord_fault_sets_when_faulted_call_is_served_within_two_cycles(
  void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.coordination.patterns[0].cycleTimeSeconds = 80U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));

  TickForTicks(16100U);
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TickUntilActivePhase(1U, 2U, 10000U);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 0U));
  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();

  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_COORDINATED, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_LOCAL_FREE_STATUS_NOT_FREE,
                          runtime->localFreeStatus);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->coordCycleFaultActive);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->coordFaultActive);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->coordFailActive);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->cycleFailActive);
}

void test_coord_fail_sets_when_cycle_fault_reoccurs_during_retry_window(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.coordination.patterns[0].cycleTimeSeconds = 80U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));

  TickForTicks(16100U);
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TickUntilActivePhase(1U, 2U, 10000U);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 0U));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 3U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetRingForceOffControl(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetRingForceOffControl(&s_engine, 2U, 1U));
  TickUntilActivePhase(1U, 1U, 10000U);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 3U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));
  {
    uint32_t tick;
    uint8_t coordFailReached = 0U;

    for (tick = 0U; tick < 30000U; tick++)
    {
      IntersectionEngineTick(&s_engine);
      runtime = GetRuntime();

      if (runtime->coordFailActive != 0U)
      {
        coordFailReached = 1U;
        break;
      }
    }

    TEST_ASSERT_EQUAL_UINT8(1U, coordFailReached);
  }
  runtime = GetRuntime();

  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FREE, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_LOCAL_FREE_STATUS_FAILED,
                          runtime->localFreeStatus);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->coordCycleFaultActive);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->coordFaultActive);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->coordFailActive);
}

void test_cycle_fail_sets_when_unserved_call_persists_in_failed_free_mode(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.coordination.patterns[0].cycleTimeSeconds = 80U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));

  TickForTicks(16100U);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  {
    uint32_t tick;
    uint8_t cycleFailReached = 0U;

    for (tick = 0U; tick < 25000U; tick++)
    {
      IntersectionEngineTick(&s_engine);
      runtime = GetRuntime();

      if (runtime->cycleFailActive != 0U)
      {
        cycleFailReached = 1U;
        break;
      }
    }

    TEST_ASSERT_EQUAL_UINT8(1U, cycleFailReached);
  }
  runtime = GetRuntime();

  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FREE, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_LOCAL_FREE_STATUS_FAILED,
                          runtime->localFreeStatus);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->coordCycleFaultActive);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->cycleFailActive);
}

void test_coordination_alarm_sets_after_three_cycles_not_running_called_pattern(
  void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  uint8_t coordinationAlarmReached = 0U;

  config = MakeTwoPhasePerRingConfig();
  config.coordination.patterns[0].cycleTimeSeconds = 80U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));
  TickForTicks(16100U);

  for (uint32_t tick = 0U; tick < 25000U; tick++)
  {
    IntersectionEngineTick(&s_engine);
    runtime = GetRuntime();

    if (runtime->coordinationAlarmActive != 0U)
    {
      coordinationAlarmReached = 1U;
      break;
    }
  }

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(1U, coordinationAlarmReached);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->coordinationAlarmActive);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FREE, runtime->mode);
}

void test_unit_red_revert_holds_phase_red_until_unit_minimum_expires(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].yellowChangeDs = 1U;
  config.phases[0].redClearDs = 1U;
  config.phases[0].redRevertDs = 0U;
  config.unit.redRevertDs = 5U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineSetRingForceOffControl(&s_engine, 1U, 1U));
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_RED_CLEAR, 200U);
  TEST_ASSERT_TRUE(IntersectionEngineSetRingForceOffControl(&s_engine, 1U, 0U));

  TickForTicks(20U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_RED_REST,
                        runtime->rings[0].stage);

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_GREEN, 100U);
}

void test_phase_red_revert_extends_unit_red_revert_when_larger(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].yellowChangeDs = 1U;
  config.phases[0].redClearDs = 1U;
  config.phases[0].redRevertDs = 7U;
  config.unit.redRevertDs = 3U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineSetRingForceOffControl(&s_engine, 1U, 1U));
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_RED_CLEAR, 200U);
  TEST_ASSERT_TRUE(IntersectionEngineSetRingForceOffControl(&s_engine, 1U, 0U));

  TickForTicks(40U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_RED_REST,
                        runtime->rings[0].stage);

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_GREEN, 100U);
}

void test_phase_dont_walk_revert_delays_next_walk_until_minimum_expires(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].yellowChangeDs = 1U;
  config.phases[0].redClearDs = 1U;
  config.phases[0].walkSeconds = 1U;
  config.phases[0].pedClearSeconds = 1U;
  config.phases[0].dontWalkRevertDs = 100U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 1U));
  TickUntilPedInterval(1U, INTERSECTION_PED_INTERVAL_WALK, 50U);
  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 0U));
  TickUntilPedInterval(1U, INTERSECTION_PED_INTERVAL_DONT_WALK, 300U);
  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 1U));

  TickForTicks(20U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_DONT_WALK,
                        runtime->phases[0].pedInterval);

  TickUntilPedInterval(1U, INTERSECTION_PED_INTERVAL_WALK, 2000U);
}

void test_phase_ped_clear_zero_clearance_limit_ends_on_first_yellow_tick(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].yellowChangeDs = 10U;
  config.phases[0].redClearDs = 10U;
  config.phases[0].walkSeconds = 1U;
  config.phases[0].pedClearSeconds = 30U;
  config.phases[0].yellowRedBeforeEndPedClearDs = 0U;
  config.unit.autoPedestrianClear =
    (uint8_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_DISABLE;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualControlTimeout(&s_engine, 5U));

  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        runtime->phases[0].pedInterval);

  TickForTicks(100U);
  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualIntervalAdvance(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_CLEAR,
                        runtime->phases[0].pedInterval);

  TickForTicks(50U);
  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualIntervalAdvance(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_CLEAR,
                        runtime->phases[0].pedInterval);

  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_DONT_WALK,
                        runtime->phases[0].pedInterval);
}

void test_phase_ped_clear_limit_counts_across_yellow_and_red_clear(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].yellowChangeDs = 1U;
  config.phases[0].redClearDs = 10U;
  config.phases[0].walkSeconds = 1U;
  config.phases[0].pedClearSeconds = 30U;
  config.phases[0].yellowRedBeforeEndPedClearDs = 2U;
  config.unit.autoPedestrianClear =
    (uint8_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_DISABLE;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualControlTimeout(&s_engine, 5U));

  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        runtime->phases[0].pedInterval);

  TickForTicks(100U);
  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualIntervalAdvance(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_CLEAR,
                        runtime->phases[0].pedInterval);

  TickForTicks(50U);
  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualIntervalAdvance(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_CLEAR,
                        runtime->phases[0].pedInterval);

  TickForTicks(10U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED_CLEAR,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_CLEAR,
                        runtime->phases[0].pedInterval);

  TickForTicks(9U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_CLEAR,
                        runtime->phases[0].pedInterval);

  TickForTicks(1U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_DONT_WALK,
                        runtime->phases[0].pedInterval);
}

void test_phase_ped_walk_service_one_blocks_recycle_until_next_coord_cycle(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].walkSeconds = 1U;
  config.phases[0].pedClearSeconds = 1U;
  config.phases[0].pedWalkService = 1U;
  config.coordination.patterns[0].cycleTimeSeconds = 60U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemSyncControl(&s_engine, 15U));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_COORDINATED, runtime->mode);

  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 1U));
  TickUntilPedInterval(1U, INTERSECTION_PED_INTERVAL_WALK, 50U);
  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 0U));
  TickUntilPedInterval(1U, INTERSECTION_PED_INTERVAL_DONT_WALK, 300U);

  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 1U));
  TickForTicks(100U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_DONT_WALK,
                        runtime->phases[0].pedInterval);

  TickUntilPedInterval(1U, INTERSECTION_PED_INTERVAL_WALK, 6200U);
}

void test_phase_ped_walk_service_two_allows_one_recycle_in_same_coord_cycle(
  void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].walkSeconds = 1U;
  config.phases[0].pedClearSeconds = 1U;
  config.phases[0].pedWalkService = 2U;
  config.coordination.patterns[0].cycleTimeSeconds = 60U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.splits[0][0].timeSeconds = 20U;
  config.coordination.splits[0][1].timeSeconds = 20U;
  config.coordination.splits[0][2].timeSeconds = 20U;
  config.coordination.splits[0][3].timeSeconds = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemSyncControl(&s_engine, 15U));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_COORDINATED, runtime->mode);

  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 1U));
  TickUntilPedInterval(1U, INTERSECTION_PED_INTERVAL_WALK, 50U);
  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 0U));
  TickUntilPedInterval(1U, INTERSECTION_PED_INTERVAL_DONT_WALK, 300U);

  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 1U, 1U));
  TickUntilPedInterval(1U, INTERSECTION_PED_INTERVAL_WALK, 100U);
}

void test_startup_flash_uses_configured_mode_and_expires_to_normal_operation(
  void)
{
  IntersectionConfig_t config;
  IntersectionOutputIntentImage_t outputIntentImage;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.unit.startUpFlashSeconds = 1U;
  config.unit.startUpFlashMode =
    (uint8_t) INTERSECTION_UNIT_STARTUP_FLASH_MODE_ALL_RED_CONTROLLER_FLASH;
  config.channels[0].flashMask = 0x02U;
  config.channels[1].flashMask = 0x02U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FLASH, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->startUpFlashActive);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_RED,
                        outputIntentImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_RED,
                        outputIntentImage.channels[1]);

  TickForTicks(100U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->startUpFlashActive);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FREE, runtime->mode);
}

void test_system_flash_command_forces_flash_output_image(void)
{
  IntersectionConfig_t config;
  IntersectionOutputIntentImage_t outputIntentImage;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.channels[0].flashMask = 0x04U;
  config.channels[1].flashMask = 0x02U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 255U));

  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FLASH, runtime->mode);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_RED,
                        outputIntentImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW,
                        outputIntentImage.channels[1]);
}

void test_minus_green_yellow_overlap_follows_included_and_modifier_phases(void)
{
  IntersectionConfig_t config;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeSingleModifierOverlapConfig(
    INTERSECTION_OVERLAP_TYPE_MINUS_GREEN_YELLOW);

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_GREEN,
                        outputIntentImage.channels[0]);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 200U);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_YELLOW,
                        outputIntentImage.channels[0]);

  TickUntilActivePhase(1U, 2U, 200U);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_RED,
                        outputIntentImage.channels[0]);
}

void test_fya_three_section_overlap_flashes_yellow_then_turns_green(void)
{
  IntersectionConfig_t config;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeSingleModifierOverlapConfig(
    INTERSECTION_OVERLAP_TYPE_FYA_THREE_SECTION);

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW,
                        outputIntentImage.channels[0]);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 200U);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_YELLOW,
                        outputIntentImage.channels[0]);

  TickUntilActivePhase(1U, 2U, 200U);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_GREEN,
                        outputIntentImage.channels[0]);
}

void test_fya_four_section_overlap_flashes_yellow_then_turns_dark(void)
{
  IntersectionConfig_t config;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeSingleModifierOverlapConfig(
    INTERSECTION_OVERLAP_TYPE_FYA_FOUR_SECTION);

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW,
                        outputIntentImage.channels[0]);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TickUntilActivePhase(1U, 2U, 300U);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_DARK,
                        outputIntentImage.channels[0]);
}

void test_fra_three_section_overlap_flashes_red_then_turns_green(void)
{
  IntersectionConfig_t config;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeSingleModifierOverlapConfig(
    INTERSECTION_OVERLAP_TYPE_FRA_THREE_SECTION);

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_RED,
                        outputIntentImage.channels[0]);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TickUntilActivePhase(1U, 2U, 300U);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_GREEN,
                        outputIntentImage.channels[0]);
}

void test_fra_four_section_overlap_flashes_red_then_turns_dark(void)
{
  IntersectionConfig_t config;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeSingleModifierOverlapConfig(
    INTERSECTION_OVERLAP_TYPE_FRA_FOUR_SECTION);

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_RED,
                        outputIntentImage.channels[0]);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TickUntilActivePhase(1U, 2U, 300U);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_DARK,
                        outputIntentImage.channels[0]);
}

void test_minus_green_yellow_alternate_stays_green_when_modifier_is_not_next(
  void)
{
  IntersectionConfig_t config;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeSingleModifierOverlapConfig(
    INTERSECTION_OVERLAP_TYPE_MINUS_GREEN_YELLOW_ALTERNATE);
  config.overlaps[0].includedPhases.length = 2U;
  config.overlaps[0].includedPhases.values[0] = 1U;
  config.overlaps[0].includedPhases.values[1] = 2U;
  config.overlaps[0].modifierPhases.values[0] = 3U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_GREEN,
                        outputIntentImage.channels[0]);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 200U);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_GREEN,
                        outputIntentImage.channels[0]);
}

void test_minus_green_yellow_alternate_turns_red_when_modifier_is_next(void)
{
  IntersectionConfig_t config;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeSingleModifierOverlapConfig(
    INTERSECTION_OVERLAP_TYPE_MINUS_GREEN_YELLOW_ALTERNATE);
  config.overlaps[0].includedPhases.length = 2U;
  config.overlaps[0].includedPhases.values[0] = 1U;
  config.overlaps[0].includedPhases.values[1] = 2U;
  config.overlaps[0].modifierPhases.values[0] = 2U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_GREEN,
                        outputIntentImage.channels[0]);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 200U);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_RED,
                        outputIntentImage.channels[0]);
}

void test_programmed_automatic_flash_entry_and_exit_phases_follow_ts2_sequence(
  void)
{
  IntersectionConfig_t config;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeTwoPhasePerRingConfig();
  config.channels[0].flashMask = 0x04U;
  config.channels[1].flashMask = 0x02U;

  for (uint8_t phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].minGreenDs = 10U;
    config.phases[phaseIndex].yellowChangeDs = 10U;
    config.phases[phaseIndex].redClearDs = 10U;
  }

  config.phases[1].phaseOptions |= PHASE_OPTIONS_AUTO_FLASH_ENTRY;
  config.phases[3].phaseOptions |= PHASE_OPTIONS_AUTO_FLASH_ENTRY;
  config.phases[0].phaseOptions |= PHASE_OPTIONS_AUTO_FLASH_EXIT;
  config.phases[2].phaseOptions |= PHASE_OPTIONS_AUTO_FLASH_EXIT;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 255U));

  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_GREEN,
                        outputIntentImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_RED,
                        outputIntentImage.channels[1]);

  TickUntilActivePhase(1U, 2U, 400U);
  TickUntilActivePhase(2U, 4U, 400U);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        GetRuntime()->phases[1].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        GetRuntime()->phases[3].interval);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_RED,
                        outputIntentImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_RED,
                        outputIntentImage.channels[1]);

  TickForTicks(350U);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_RED,
                        outputIntentImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW,
                        outputIntentImage.channels[1]);

  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 0U));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        GetRuntime()->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        GetRuntime()->phases[2].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        GetRuntime()->phases[0].pedInterval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        GetRuntime()->phases[2].pedInterval);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_GREEN,
                        outputIntentImage.channels[0]);
}

void test_phase_call_does_not_hold_green_past_gap_timer(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].passageDs = 20U;
  config.phases[0].minimumGapDs = 10U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 0U));

  TickForTicks(250U);
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_RING_TERMINATION_GAP_OUT,
                          runtime->rings[0].terminationReasonBits);
}

void test_volume_density_reduces_current_gap_after_tbr(void)
{
  IntersectionConfig_t config;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].passageDs = 50U;
  config.phases[0].minimumGapDs = 5U;
  config.phases[0].timeBeforeReductionSec = 2U;
  config.phases[0].timeToReduceSec = 3U;
  config.phases[0].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;
  config.phases[1].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TickForTicks(199U);
  TEST_ASSERT_EQUAL_UINT16(500U, s_engine.currentGapTicks[0]);

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_LESS_THAN_UINT16(500U, s_engine.currentGapTicks[0]);

  TickForTicks(299U);
  TEST_ASSERT_EQUAL_UINT16(50U, s_engine.currentGapTicks[0]);
}

void test_phase_startup_phase_not_on_holds_red_until_ring_demand(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].startup =
    (uint8_t) INTERSECTION_PHASE_STARTUP_PHASE_NOT_ON;
  config.phases[2].startup =
    (uint8_t) INTERSECTION_PHASE_STARTUP_PHASE_NOT_ON;
  config.phases[1].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;
  config.phases[3].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));

  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_RED_REST,
                        runtime->rings[0].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[0].interval);

  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[1].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[3].interval);
}

void test_phase_startup_yellow_change_initializes_ring_in_yellow(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].startup =
    (uint8_t) INTERSECTION_PHASE_STARTUP_YELLOW_CHANGE;
  config.phases[2].startup =
    (uint8_t) INTERSECTION_PHASE_STARTUP_YELLOW_CHANGE;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();

  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[2].interval);
}

void test_coordination_maximum2_selects_alternate_phase_maximum(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  uint8_t phaseIndex;

  config = MakeTwoPhasePerRingConfig();

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].minGreenDs = 10U;
    config.phases[phaseIndex].yellowChangeDs = 5U;
    config.phases[phaseIndex].redClearDs = 5U;
  }

  config.phases[0].maxGreenDs = 300U;
  config.phases[0].phaseMaximum2Ds = 20U;
  config.phases[0].phaseOptions |= PHASE_OPTIONS_MAX_RECALL;
  config.phases[1].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;
  config.phases[2].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;
  config.phases[3].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;
  config.coordination.maximumMode =
    (uint8_t) INTERSECTION_COORD_MAXIMUM_MODE_MAXIMUM2;
  config.coordination.patterns[0].cycleTimeSeconds = 120U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.patterns[0].options =
    (uint8_t) INTERSECTION_PATTERN_OPTIONS_COORD_MAXIMUM_MODE;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);
  TickForTicks(189U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 40U);
}

void test_coordination_maximum3_uses_32bit_running_maximum(void)
{
  IntersectionConfig_t config;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].phaseMaximum3Ds = 7000U;
  config.coordination.patterns[0].cycleTimeSeconds = 120U;
  config.coordination.patterns[0].offsetTimeSeconds = 0U;
  config.coordination.patterns[0].splitNumber = 1U;
  config.coordination.patterns[0].options =
    (uint8_t) INTERSECTION_PATTERN_OPTIONS_MAXIMUM3;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_EQUAL_UINT32(70000U, s_engine.runningMaxTicks[0]);
}

void test_dynamic_max_increases_after_two_consecutive_max_outs(void)
{
  IntersectionConfig_t config;
  uint8_t phaseIndex;

  config = MakeTwoPhasePerRingConfig();

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].minGreenDs = 10U;
    config.phases[phaseIndex].maxGreenDs = 20U;
    config.phases[phaseIndex].phaseMaximum2Ds = 20U;
    config.phases[phaseIndex].phaseMaximum3Ds = 20U;
    config.phases[phaseIndex].passageDs = 10U;
    config.phases[phaseIndex].minimumGapDs = 10U;
    config.phases[phaseIndex].maxInitialDs = 10U;
    config.phases[phaseIndex].yellowChangeDs = 5U;
    config.phases[phaseIndex].redClearDs = 5U;
  }

  config.phases[0].maxGreenDs = 20U;
  config.phases[0].dynamicMaxLimitSeconds = 4U;
  config.phases[0].dynamicMaxStepDs = 10U;
  config.phases[1].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;
  config.phases[2].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;
  config.phases[3].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 220U);
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_GREEN, 600U);
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 220U);

  TEST_ASSERT_EQUAL_UINT32(300U, s_engine.runningMaxTicks[0]);
}

void test_dynamic_max_decreases_after_two_consecutive_gap_outs(void)
{
  IntersectionConfig_t config;
  uint8_t phaseIndex;

  config = MakeTwoPhasePerRingConfig();

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].minGreenDs = 10U;
    config.phases[phaseIndex].maxGreenDs = 20U;
    config.phases[phaseIndex].phaseMaximum2Ds = 20U;
    config.phases[phaseIndex].phaseMaximum3Ds = 20U;
    config.phases[phaseIndex].passageDs = 10U;
    config.phases[phaseIndex].minimumGapDs = 10U;
    config.phases[phaseIndex].maxInitialDs = 10U;
    config.phases[phaseIndex].yellowChangeDs = 5U;
    config.phases[phaseIndex].redClearDs = 5U;
  }

  config.phases[0].maxGreenDs = 40U;
  config.phases[0].dynamicMaxLimitSeconds = 2U;
  config.phases[0].dynamicMaxStepDs = 10U;
  config.phases[1].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;
  config.phases[2].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;
  config.phases[3].phaseOptions |= PHASE_OPTIONS_MIN_RECALL;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 150U);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 0U));
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_GREEN, 600U);
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 150U);

  TEST_ASSERT_EQUAL_UINT32(300U, s_engine.runningMaxTicks[0]);
}

void test_dual_entry_phase_starts_on_barrier_cross_without_local_demand(void)
{
  IntersectionConfig_t config;
  uint8_t phaseIndex;
  uint8_t phaseNumber;

  config = MakeTwoPhasePerRingConfig();

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].minGreenDs = 10U;
    config.phases[phaseIndex].passageDs = 10U;
    config.phases[phaseIndex].minimumGapDs = 10U;
    config.phases[phaseIndex].yellowChangeDs = 5U;
    config.phases[phaseIndex].redClearDs = 5U;
  }

  config.phases[1].phaseOptions |= PHASE_OPTIONS_DUAL_ENTRY;
  config.phases[1].concurrency.length = 1U;
  config.phases[1].concurrency.values[0] = 4U;
  config.phases[3].concurrency.length = 1U;
  config.phases[3].concurrency.values[0] = 2U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 4U, 0U));

  TickUntilPhaseInterval(2U, INTERSECTION_PHASE_INTERVAL_GREEN, 400U);
  TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                           1U,
                                                           &phaseNumber));
  TEST_ASSERT_EQUAL_UINT8(2U, phaseNumber);
  TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                           2U,
                                                           &phaseNumber));
  TEST_ASSERT_EQUAL_UINT8(4U, phaseNumber);
}

void test_gap_out_phase_resumes_from_barrier_wait_when_simultaneous_gap_is_enabled(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].passageDs = 10U;
  config.phases[0].minimumGapDs = 10U;
  config.phases[0].yellowChangeDs = 5U;
  config.phases[0].redClearDs = 5U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TickForTicks(200U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_WAIT_BARRIER,
                        runtime->rings[0].stage);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();

  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_GREEN, runtime->rings[0].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);
}

void test_gap_out_phase_stays_waiting_when_simultaneous_gap_disable_is_set(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].phaseOptions |= PHASE_OPTIONS_SIMUL_GAP_DISABLE;
  config.phases[0].minGreenDs = 10U;
  config.phases[0].passageDs = 10U;
  config.phases[0].minimumGapDs = 10U;
  config.phases[0].yellowChangeDs = 5U;
  config.phases[0].redClearDs = 5U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TickForTicks(200U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_WAIT_BARRIER,
                        runtime->rings[0].stage);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();

  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_WAIT_BARRIER,
                        runtime->rings[0].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[0].interval);
}

void test_conditional_service_restarts_preceding_phase_while_waiting_at_barrier(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  uint8_t phaseIndex;
  uint8_t phaseNumber;

  config = MakeThreePhasePerRingConfig();

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].minGreenDs = 10U;
    config.phases[phaseIndex].passageDs = 10U;
    config.phases[phaseIndex].minimumGapDs = 10U;
    config.phases[phaseIndex].yellowChangeDs = 5U;
    config.phases[phaseIndex].redClearDs = 5U;
  }

  config.phases[1].phaseOptions |= PHASE_OPTIONS_COND_SERVICE;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TickUntilPhaseInterval(2U, INTERSECTION_PHASE_INTERVAL_GREEN, 300U);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 3U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 3U, 0U));

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_GREEN, 400U);
  runtime = GetRuntime();

  TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                           1U,
                                                           &phaseNumber));
  TEST_ASSERT_EQUAL_UINT8(1U, phaseNumber);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_GREEN, runtime->rings[0].stage);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);
}

void test_conflicting_vehicle_actuation_starts_gap_reduction_when_cbr_is_met(void)
{
  IntersectionConfig_t config;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 100U;
  config.phases[0].maxInitialDs = 100U;
  config.phases[0].passageDs = 50U;
  config.phases[0].minimumGapDs = 5U;
  config.phases[0].timeBeforeReductionSec = 10U;
  config.phases[0].carsBeforeReduction = 1U;
  config.phases[0].timeToReduceSec = 3U;

  TEST_ASSERT_EQUAL_UINT8(1U, IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_EQUAL_UINT16(0U, s_engine.conflictingVehicleCountGreen[0]);

  TEST_ASSERT_EQUAL_UINT8(1U,
                          IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TEST_ASSERT_EQUAL_UINT16(1U, s_engine.conflictingVehicleCountGreen[0]);

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_LESS_THAN_UINT16(500U, s_engine.currentGapTicks[0]);
}

void test_added_initial_extends_initial_green_from_red_actuation_count(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].addedInitialDs = 10U;
  config.phases[0].maxInitialDs = 30U;
  config.phases[0].passageDs = 10U;
  config.phases[0].minimumGapDs = 10U;
  config.phases[0].yellowChangeDs = 5U;
  config.phases[0].redClearDs = 5U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 0U));

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_EQUAL_UINT16(100U, s_engine.initialGreenTicks[0]);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));

  TickForTicks(90U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 30U);
}

void test_added_initial_is_limited_by_maximum_initial(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].addedInitialDs = 20U;
  config.phases[0].maxInitialDs = 30U;
  config.phases[0].passageDs = 10U;
  config.phases[0].minimumGapDs = 10U;
  config.phases[0].yellowChangeDs = 5U;
  config.phases[0].redClearDs = 5U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 0U));

  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_EQUAL_UINT16(300U, s_engine.initialGreenTicks[0]);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));

  TickForTicks(250U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 70U);
}

void test_green_actuation_without_non_lock_detector_memory_does_not_latch_before_yellow(void)
{
  IntersectionConfig_t config;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].passageDs = 10U;
  config.phases[0].minimumGapDs = 10U;
  config.phases[0].yellowChangeDs = 5U;
  config.phases[0].redClearDs = 5U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 0U));
  TEST_ASSERT_EQUAL_UINT8(0U, GetRuntime()->phases[0].callLatched);

  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 200U);

  TEST_ASSERT_EQUAL_UINT8(0U, GetRuntime()->phases[0].callLatched);
}

void test_green_detector_present_at_yellow_start_latches_when_non_lock_memory_is_clear(void)
{
  IntersectionConfig_t config;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].maxGreenDs = 20U;
  config.phases[0].maxInitialDs = 20U;
  config.phases[0].passageDs = 10U;
  config.phases[0].minimumGapDs = 10U;
  config.phases[0].yellowChangeDs = 5U;
  config.phases[0].redClearDs = 5U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 220U);

  TEST_ASSERT_EQUAL_UINT8(1U, GetRuntime()->phases[0].callLatched);
}

void test_non_lock_detector_memory_option_preserves_green_actuation_call_memory(void)
{
  IntersectionConfig_t config;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].phaseOptions |= PHASE_OPTIONS_NON_LOCK_DET_MEM;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 0U));

  TEST_ASSERT_EQUAL_UINT8(1U, GetRuntime()->phases[0].callLatched);
}

void test_vehicle_detector_delay_applies_before_non_green_call_is_recognized(void)
{
  IntersectionConfig_t config;

  config = MakeTwoPhasePerRingConfig();
  config.vehicleDetectors[1].delayDs = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 2U, 1U));
  TEST_ASSERT_EQUAL_UINT8(0U, GetRuntime()->phases[1].callLatched);

  TickForTicks(150U);
  TEST_ASSERT_EQUAL_UINT8(0U, GetRuntime()->phases[1].callLatched);

  TickForTicks(60U);
  TEST_ASSERT_EQUAL_UINT8(1U, GetRuntime()->phases[1].callLatched);
}

void test_vehicle_detector_extend_keeps_green_detection_active_after_release(void)
{
  IntersectionConfig_t config;

  config = MakeTwoPhasePerRingConfig();
  config.vehicleDetectors[0].extendDs = 15U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 1U, 1U));
  TEST_ASSERT_EQUAL_UINT8(1U, GetRuntime()->phases[0].detectorActive);

  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 1U, 0U));
  TEST_ASSERT_EQUAL_UINT8(1U, GetRuntime()->phases[0].detectorActive);

  TickForTicks(100U);
  TEST_ASSERT_EQUAL_UINT8(1U, GetRuntime()->phases[0].detectorActive);

  TickForTicks(60U);
  TEST_ASSERT_EQUAL_UINT8(0U, GetRuntime()->phases[0].detectorActive);
}

void test_vehicle_detector_switch_phase_routes_green_extension_to_switch_phase(void)
{
  IntersectionConfig_t config;

  config = MakeTwoPhasePerRingConfig();
  config.vehicleDetectors[1].switchPhase = 1U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 2U, 1U));

  TEST_ASSERT_EQUAL_UINT8(1U, GetRuntime()->phases[0].detectorActive);
  TEST_ASSERT_EQUAL_UINT8(0U, GetRuntime()->phases[1].callLatched);
}

void test_ped_detector_alternate_timing_uses_alternate_walk_and_clearance(void)
{
  IntersectionConfig_t config;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].walkSeconds = 1U;
  config.phases[0].pedClearSeconds = 1U;
  config.phases[0].pedAlternateWalkSeconds = 3U;
  config.phases[0].pedAlternateClearSeconds = 2U;
  config.phases[0].pedDelayDs = 20U;
  config.pedestrianDetectors[0].callPhase = 1U;
  config.pedestrianDetectors[0].apsMinimumActuationDs = 10U;
  config.pedestrianDetectors[0].options = PED_DETECTOR_OPTIONS_ALT_TIMING;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPedestrianDetectorInput(&s_engine,
                                                                1U,
                                                                1U));

  TickForTicks(210U);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        GetRuntime()->phases[0].pedInterval);
  TEST_ASSERT_EQUAL_UINT8(1U,
                          GetRuntime()->phases[0].pedAlternateTimingActive);

  TickForTicks(150U);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        GetRuntime()->phases[0].pedInterval);

  TickForTicks(170U);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_CLEAR,
                        GetRuntime()->phases[0].pedInterval);
}

void test_phase_ped_advance_walk_starts_before_green_and_ignores_delay_time(
  void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  uint16_t initialWalkTicks;

  config = MakeThreePhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].yellowChangeDs = 3U;
  config.phases[0].redClearDs = 2U;
  config.phases[1].pedAdvanceWalkDs = 40U;
  config.phases[1].pedDelayDs = 100U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPedCall(&s_engine, 2U, 1U));

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 200U);
  TickUntilPedInterval(2U, INTERSECTION_PED_INTERVAL_WALK, 20U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[1].interval);
  initialWalkTicks = runtime->phases[1].pedIntervalElapsedTicks;

  TickForTicks(5U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[1].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        runtime->phases[1].pedInterval);
  TEST_ASSERT_EQUAL_UINT16((uint16_t) (initialWalkTicks + 5U),
                           runtime->phases[1].pedIntervalElapsedTicks);
}

void test_phase_hold_control_keeps_active_phase_green_past_gap_out_point(void)
{
  IntersectionConfig_t config;

  config = MakeTwoPhasePerRingConfig();

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));

  TickForTicks(700U);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        GetRuntime()->phases[0].interval);

  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 0U));
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 10U);
}

void test_phase_force_off_control_terminates_green_and_clears_after_yellow_entry(void)
{
  IntersectionConfig_t config;
  uint8_t active = 0U;

  config = MakeTwoPhasePerRingConfig();

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseForceOffControl(&s_engine,
                                                             1U,
                                                             1U));
  TEST_ASSERT_TRUE(IntersectionEngineGetPhaseForceOffControl(&s_engine,
                                                             1U,
                                                             &active));
  TEST_ASSERT_EQUAL_UINT8(1U, active);

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 510U);
  TEST_ASSERT_TRUE(IntersectionEngineGetPhaseForceOffControl(&s_engine,
                                                             1U,
                                                             &active));
  TEST_ASSERT_EQUAL_UINT8(0U, active);
  TEST_ASSERT_BITS_HIGH(INTERSECTION_RING_TERMINATION_FORCE_OFF,
                        GetRuntime()->rings[0].terminationReasonBits);
}

void test_phase_omit_control_prevents_omitted_phase_from_being_selected(void)
{
  IntersectionConfig_t config;
  uint8_t activePhaseNumber = 0U;

  config = MakeTwoPhasePerRingConfig();

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseOmitControl(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));

  TickForTicks(1200U);
  TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                           1U,
                                                           &activePhaseNumber));
  TEST_ASSERT_EQUAL_UINT8(1U, activePhaseNumber);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        GetRuntime()->phases[0].interval);
}

void test_phase_ped_omit_control_blocks_remote_ped_service_start(void)
{
  IntersectionConfig_t config;
  IntersectionPhaseStatusGroup_t statusGroup;

  config = MakeTwoPhasePerRingConfig();

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetPedOmitControl(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPedCallControl(&s_engine, 1U, 1U));

  TickForTicks(20U);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_DONT_WALK,
                        GetRuntime()->phases[0].pedInterval);
  TEST_ASSERT_TRUE(IntersectionEngineGetPhaseStatusGroup(&s_engine,
                                                         1U,
                                                         &statusGroup));
  TEST_ASSERT_BITS_HIGH(0x01U, statusGroup.pedCalls);
  TEST_ASSERT_BITS_LOW(0x01U, statusGroup.walks);
}

void test_phase_remote_vehicle_call_control_creates_service_demand(void)
{
  IntersectionConfig_t config;
  IntersectionPhaseStatusGroup_t statusGroup;

  config = MakeThreePhasePerRingConfig();

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetVehCallControl(&s_engine, 2U, 1U));

  TEST_ASSERT_TRUE(IntersectionEngineGetPhaseStatusGroup(&s_engine,
                                                         1U,
                                                         &statusGroup));
  TEST_ASSERT_BITS_HIGH(0x02U, statusGroup.vehCalls);

  TickUntilActivePhase(1U, 2U, 1200U);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        GetRuntime()->phases[1].interval);
}

void test_unit_control_external_minimum_recall_sets_vehicle_calls_for_all_phases(
  void)
{
  IntersectionConfig_t config;
  IntersectionPhaseStatusGroup_t statusGroup;

  config = MakeTwoPhasePerRingConfig();

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetUnitControl(&s_engine, 0x04U));

  TEST_ASSERT_TRUE(IntersectionEngineGetPhaseStatusGroup(&s_engine,
                                                         1U,
                                                         &statusGroup));
  TEST_ASSERT_EQUAL_UINT8(0x0FU, statusGroup.vehCalls);
}

void test_unit_control_non_actuated_command_services_programmed_phase(void)
{
  IntersectionConfig_t config;

  config = MakeThreePhasePerRingConfig();
  config.phases[1].phaseOptions |= PHASE_OPTIONS_NON_ACTUATED_1;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetUnitControl(&s_engine, 0x08U));

  TickUntilActivePhase(1U, 2U, 1200U);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        GetRuntime()->phases[1].interval);
}

void test_unit_control_non_actuated_requires_pedestrian_equipped_phase(void)
{
  IntersectionConfig_t config;
  uint8_t activePhaseNumber = 0U;

  config = MakeThreePhasePerRingConfig();
  config.phases[1].phaseOptions |= PHASE_OPTIONS_NON_ACTUATED_1;
  config.phases[1].walkSeconds = 0U;
  config.phases[1].pedClearSeconds = 0U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetUnitControl(&s_engine, 0x08U));

  TickForTicks(1200U);
  TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(&s_engine,
                                                           1U,
                                                           &activePhaseNumber));
  TEST_ASSERT_EQUAL_UINT8(1U, activePhaseNumber);
}

void test_unit_control_walk_rest_modifier_holds_walk_on_non_actuated_phase(void)
{
  IntersectionConfig_t config;

  config = MakeThreePhasePerRingConfig();
  config.phases[1].phaseOptions |= PHASE_OPTIONS_NON_ACTUATED_1;
  config.phases[1].walkSeconds = 1U;
  config.phases[1].pedClearSeconds = 3U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetUnitControl(&s_engine, 0x28U));

  TickUntilActivePhase(1U, 2U, 1200U);
  TEST_ASSERT_TRUE(IntersectionEngineSetPedCallControl(&s_engine, 2U, 1U));
  TickUntilPhaseInterval(2U, INTERSECTION_PHASE_INTERVAL_GREEN, 20U);

  TickForTicks(20U);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        GetRuntime()->phases[1].pedInterval);

  TickForTicks(150U);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        GetRuntime()->phases[1].pedInterval);
}

void test_interconnect_relinquished_by_timebase_pattern_zero_controls_runtime(
  void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeTwoPhasePerRingConfig();
  config.channels[0].flashMask = 0x04U;
  config.timebase.actions[0].pattern = 0U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetActionPlanControl(&s_engine, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetLocalInterconnectInputsValid(&s_engine,
                                                                     1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetLocalInterconnectCommand(&s_engine,
                                                                 255U));

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(255U, runtime->interconnectCommand);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->interconnectInputsValid);
  TEST_ASSERT_EQUAL_UINT8(7U, runtime->unitControlStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FLASH, runtime->mode);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_RED,
                        outputIntentImage.channels[0]);
}

void test_unit_control_interconnect_priority_overrides_timebase(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeTwoPhasePerRingConfig();
  config.channels[0].flashMask = 0x04U;
  config.timebase.actions[0].pattern = 254U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetActionPlanControl(&s_engine, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetLocalInterconnectInputsValid(&s_engine,
                                                                     1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetLocalInterconnectCommand(&s_engine,
                                                                 255U));

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(6U, runtime->unitControlStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FREE, runtime->mode);

  TEST_ASSERT_TRUE(IntersectionEngineSetUnitControl(&s_engine, 0x40U));
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(7U, runtime->unitControlStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FLASH, runtime->mode);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_RED,
                        outputIntentImage.channels[0]);
}

void test_invalid_interconnect_inputs_report_interconnect_backup(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.timebase.actions[0].pattern = 254U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetUnitControl(&s_engine, 0x40U));
  TEST_ASSERT_TRUE(IntersectionEngineSetActionPlanControl(&s_engine, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetLocalInterconnectCommand(&s_engine,
                                                                 255U));
  TEST_ASSERT_TRUE(IntersectionEngineSetLocalInterconnectInputsValid(&s_engine,
                                                                     0U));

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->interconnectInputsValid);
  TEST_ASSERT_EQUAL_UINT8(8U, runtime->unitControlStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FREE, runtime->mode);
}

void test_timebase_dimming_requires_action_and_local_or_unit_enable(void)
{
  IntersectionConfig_t config;
  IntersectionOutputIntentImage_t outputIntentImage;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.channels[0].dimMask = 0x09U;
  config.timebase.actions[0].auxiliaryFunction =
    INTERSECTION_TIMEBASE_AUX_FUNCTION_DIMMING;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_UINT8(0U, outputIntentImage.channelDimmed[0]);
  TEST_ASSERT_EQUAL_UINT8(0U, outputIntentImage.channelDimAlternateHalfCycle[0]);

  TEST_ASSERT_TRUE(IntersectionEngineSetActionPlanControl(&s_engine, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_UINT8(0U, outputIntentImage.channelDimmed[0]);

  TEST_ASSERT_TRUE(IntersectionEngineSetLocalDimmingInput(&s_engine, 1U));
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->localDimmingInputActive);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_UINT8(1U, outputIntentImage.channelDimmed[0]);
  TEST_ASSERT_EQUAL_UINT8(1U, outputIntentImage.channelDimAlternateHalfCycle[0]);

  TEST_ASSERT_TRUE(IntersectionEngineSetLocalDimmingInput(&s_engine, 0U));
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->localDimmingInputActive);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_UINT8(0U, outputIntentImage.channelDimmed[0]);

  TEST_ASSERT_TRUE(IntersectionEngineSetUnitControl(&s_engine, 0x80U));
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_UINT8(1U, outputIntentImage.channelDimmed[0]);
  TEST_ASSERT_EQUAL_UINT8(1U, outputIntentImage.channelDimAlternateHalfCycle[0]);
}

void test_action_plan_control_reports_selected_timebase_action_and_can_flash(
  void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  IntersectionOutputIntentImage_t outputIntentImage;

  config = MakeTwoPhasePerRingConfig();
  config.timebase.actions[0].pattern = 255U;
  config.channels[0].flashMask = 0x04U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineSetActionPlanControl(&s_engine, 1U));

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->actionPlanControl);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->timebaseActionStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FLASH, runtime->mode);
  TEST_ASSERT_TRUE(IntersectionEngineGetOutputIntentImage(&s_engine,
                                                          &outputIntentImage));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_OUTPUT_ASPECT_FLASH_RED,
                        outputIntentImage.channels[0]);
}

void test_unit_backup_timer_enters_backup_mode_and_clears_remote_controls(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  uint8_t unitControl = 0U;
  uint8_t actionPlanControl = 0U;

  config = MakeTwoPhasePerRingConfig();
  config.unit.backupTimeSeconds = 1U;
  config.timebase.actions[0].pattern = 1U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineSetUnitControl(&s_engine, 0x04U));
  TEST_ASSERT_TRUE(IntersectionEngineSetActionPlanControl(&s_engine, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPhaseHoldControl(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetVehCallControl(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetPreemptControlState(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(
    IntersectionEngineSetVehicleDetectorRemoteActuation(&s_engine, 1U, 1U));

  TickForTicks(100U);
  runtime = GetRuntime();

  TEST_ASSERT_EQUAL_UINT8(1U, runtime->backupModeActive);
  TEST_ASSERT_EQUAL_UINT8(4U, runtime->unitControlStatus);
  TEST_ASSERT_TRUE(IntersectionEngineGetUnitControl(&s_engine, &unitControl));
  TEST_ASSERT_EQUAL_UINT8(0U, unitControl);
  TEST_ASSERT_TRUE(IntersectionEngineGetActionPlanControl(&s_engine,
                                                          &actionPlanControl));
  TEST_ASSERT_EQUAL_UINT8(0U, actionPlanControl);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->preemptControlState[0]);
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->vehicleDetectors[0].remoteActuation);
  TEST_ASSERT_TRUE(IntersectionEngineGetPhaseHoldControl(&s_engine, 1U,
                                                         &unitControl));
  TEST_ASSERT_EQUAL_UINT8(0U, unitControl);
  TEST_ASSERT_TRUE(IntersectionEngineGetVehCallControl(&s_engine, 1U,
                                                       &unitControl));
  TEST_ASSERT_EQUAL_UINT8(0U, unitControl);

  TEST_ASSERT_TRUE(IntersectionEngineSetSystemPatternControl(&s_engine, 1U));
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->backupModeActive);
  TEST_ASSERT_EQUAL_UINT8(2U, runtime->unitControlStatus);
}

void test_user_defined_backup_timer_requires_explicit_reset_and_enters_backup_mode(
  void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.unit.backupTimeSeconds = 1U;
  config.unit.userDefinedBackupTimeSeconds = 1U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TickForTicks(100U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->backupModeActive);

  TEST_ASSERT_TRUE(IntersectionEngineResetUserDefinedBackupTimer(&s_engine));
  TickForTicks(99U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->backupModeActive);

  TickForTicks(1U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->backupModeActive);
  TEST_ASSERT_EQUAL_UINT8(4U, runtime->unitControlStatus);
}

void test_remote_manual_control_holds_green_until_interval_advance_and_expires(
  void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  uint8_t timeoutSeconds = 0U;
  uint8_t phaseNumber = 0U;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].walkSeconds = 0U;
  config.phases[0].pedClearSeconds = 0U;
  config.phases[2].minGreenDs = 10U;
  config.phases[2].walkSeconds = 0U;
  config.phases[2].pedClearSeconds = 0U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualControlTimeout(&s_engine, 3U));

  TickForTicks(100U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(9U, runtime->unitControlStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[2].interval);
  TEST_ASSERT_TRUE(IntersectionEngineGetActivePhaseForRing(&s_engine, 1U,
                                                           &phaseNumber));
  TEST_ASSERT_EQUAL_UINT8(1U, phaseNumber);

  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualIntervalAdvance(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[2].interval);

  TickForTicks(100U);
  TEST_ASSERT_TRUE(
    IntersectionEngineGetRemoteManualControlTimeout(&s_engine, &timeoutSeconds));
  TEST_ASSERT_EQUAL_UINT8(1U, timeoutSeconds);
  TickForTicks(100U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(0U, runtime->remoteManualControlTimeout);
  TEST_ASSERT_NOT_EQUAL(9U, runtime->unitControlStatus);
}

void test_remote_manual_interval_advance_with_auto_ped_clear_advances_walk_then_clear(
  void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].walkSeconds = 1U;
  config.phases[0].pedClearSeconds = 1U;
  config.unit.autoPedestrianClear =
    (uint8_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_ENABLE;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualControlTimeout(&s_engine, 5U));

  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        runtime->phases[0].pedInterval);

  TickForTicks(100U);

  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualIntervalAdvance(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_CLEAR,
                        runtime->phases[0].pedInterval);

  TickForTicks(100U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[0].interval);
}

void test_remote_manual_interval_advance_requires_second_press_without_auto_ped_clear(
  void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].walkSeconds = 1U;
  config.phases[0].pedClearSeconds = 1U;
  config.unit.autoPedestrianClear =
    (uint8_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_DISABLE;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualControlTimeout(&s_engine, 5U));

  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        runtime->phases[0].pedInterval);

  TickForTicks(100U);

  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualIntervalAdvance(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_CLEAR,
                        runtime->phases[0].pedInterval);

  TickForTicks(50U);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_CLEAR,
                        runtime->phases[0].pedInterval);

  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualIntervalAdvance(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[0].interval);
}

void test_special_function_output_status_ors_runtime_control_with_timebase_bits(
  void)
{
  IntersectionConfig_t config;
  uint8_t active = 0U;

  config = MakeTwoPhasePerRingConfig();
  config.unit.backupTimeSeconds = 1U;
  config.timebase.actions[0].specialFunction = 0x01U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineSetSpecialFunctionOutputControl(&s_engine,
                                                                     2U,
                                                                     1U));
  TEST_ASSERT_TRUE(IntersectionEngineGetSpecialFunctionOutputControl(&s_engine,
                                                                     2U,
                                                                     &active));
  TEST_ASSERT_EQUAL_UINT8(1U, active);
  TEST_ASSERT_TRUE(IntersectionEngineGetSpecialFunctionOutputStatus(&s_engine,
                                                                    2U,
                                                                    &active));
  TEST_ASSERT_EQUAL_UINT8(1U, active);

  TEST_ASSERT_TRUE(IntersectionEngineSetActionPlanControl(&s_engine, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineGetSpecialFunctionOutputStatus(&s_engine,
                                                                    1U,
                                                                    &active));
  TEST_ASSERT_EQUAL_UINT8(1U, active);

  TickForTicks(100U);
  TEST_ASSERT_TRUE(IntersectionEngineGetSpecialFunctionOutputControl(&s_engine,
                                                                     2U,
                                                                     &active));
  TEST_ASSERT_EQUAL_UINT8(0U, active);
  TEST_ASSERT_TRUE(IntersectionEngineGetSpecialFunctionOutputStatus(&s_engine,
                                                                    1U,
                                                                    &active));
  TEST_ASSERT_EQUAL_UINT8(0U, active);
}

void test_ring_stop_time_control_freezes_elapsed_ticks(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;
  uint8_t active = 0U;
  uint32_t beforeElapsed;

  config = MakeTwoPhasePerRingConfig();
  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));

  IntersectionEngineTick(&s_engine);
  TickForTicks(20U);
  runtime = GetRuntime();
  beforeElapsed = runtime->rings[0].stageElapsedTicks;

  TEST_ASSERT_TRUE(IntersectionEngineSetRingStopTimeControl(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineGetRingStopTimeControl(&s_engine,
                                                            1U,
                                                            &active));
  TEST_ASSERT_EQUAL_UINT8(1U, active);

  TickForTicks(100U);
  runtime = GetRuntime();

  TEST_ASSERT_EQUAL_UINT32(beforeElapsed, runtime->rings[0].stageElapsedTicks);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        runtime->phases[0].interval);
}

void test_ring_force_off_control_terminates_green(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineSetRingForceOffControl(&s_engine, 1U, 1U));
  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 200U);

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_RING_TERMINATION_FORCE_OFF,
                          runtime->rings[0].terminationReasonBits);
}

void test_ring_maximum2_control_selects_alternate_maximum(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].maxGreenDs = 100U;
  config.phases[0].phaseMaximum2Ds = 15U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineSetRingMaximum2Control(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 300U);

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_RING_TERMINATION_MAX_OUT,
                          runtime->rings[0].terminationReasonBits);
}

void test_ring_maximum3_control_selects_third_maximum(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].maxGreenDs = 100U;
  config.phases[0].phaseMaximum3Ds = 18U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineSetRingMaximum3Control(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));

  TickUntilPhaseInterval(1U, INTERSECTION_PHASE_INTERVAL_YELLOW, 320U);

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_RING_TERMINATION_MAX_OUT,
                          runtime->rings[0].terminationReasonBits);
}

void test_ring_maximum_inhibit_control_prevents_max_out(void)
{
  IntersectionConfig_t config;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].maxGreenDs = 100U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(
    IntersectionEngineSetRingMaximumInhibitControl(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 1U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));

  TickForTicks(1100U);

  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_GREEN,
                        GetRuntime()->phases[0].interval);
  TEST_ASSERT_TRUE(GetRuntime()->rings[0].stageElapsedTicks > 1000U);
}

void test_ring_ped_recycle_control_requests_walk_on_startup_green(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].pedDelayDs = 0U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetRingPedRecycleControl(&s_engine,
                                                              1U,
                                                              1U));

  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->phases[0].pedServicePending);

  IntersectionEngineTick(&s_engine);
  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PED_INTERVAL_WALK,
                        runtime->phases[0].pedInterval);
}

void test_ring_red_rest_control_clears_to_red_rest_when_demand_drops(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].yellowChangeDs = 3U;
  config.phases[0].redClearDs = 2U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetRingRedRestControl(&s_engine, 1U, 1U));

  IntersectionEngineTick(&s_engine);
  TickUntilRingStage(1U, INTERSECTION_RING_STAGE_RED_REST, 200U);

  runtime = GetRuntime();
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_RED,
                        runtime->phases[0].interval);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_RED_REST,
                        runtime->rings[0].stage);
}

void test_ring_omit_red_clear_control_skips_red_clear_interval(void)
{
  IntersectionConfig_t config;
  uint32_t tickIndex;
  uint8_t sawRedClear = 0U;

  config = MakeTwoPhasePerRingConfig();
  config.phases[0].minGreenDs = 10U;
  config.phases[0].yellowChangeDs = 3U;
  config.phases[0].redClearDs = 20U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(IntersectionEngineSetRingOmitRedClearControl(&s_engine,
                                                                1U,
                                                                1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetDetectorCall(&s_engine, 2U, 0U));

  for (tickIndex = 0U; tickIndex < 250U; tickIndex++)
  {
    if (GetRuntime()->phases[0].interval == INTERSECTION_PHASE_INTERVAL_RED_CLEAR)
    {
      sawRedClear = 1U;
    }

    if (GetRuntime()->rings[0].stage == INTERSECTION_RING_STAGE_WAIT_BARRIER)
    {
      break;
    }

    IntersectionEngineTick(&s_engine);
  }

  TEST_ASSERT_EQUAL_UINT8(0U, sawRedClear);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_RING_STAGE_WAIT_BARRIER,
                        GetRuntime()->rings[0].stage);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_load_config_starts_all_red_until_first_tick);
  RUN_TEST(test_competing_call_advances_to_next_phase_after_clearance);
  RUN_TEST(test_phase_status_group_sets_next_and_vehicle_call_bits);
  RUN_TEST(test_barrier_crossing_waits_until_both_rings_are_ready);
  RUN_TEST(test_ped_call_services_walk_then_clear_on_phase_pedestrian_channel);
  RUN_TEST(test_system_pattern_command_activates_coordination_status);
  RUN_TEST(test_short_alarm_cycle_zero_latches_until_acknowledged);
  RUN_TEST(
    test_coord_cycle_fault_sets_after_two_coordinated_cycles_and_calls_free);
  RUN_TEST(test_coord_fault_sets_when_faulted_call_is_served_within_two_cycles);
  RUN_TEST(test_coord_fail_sets_when_cycle_fault_reoccurs_during_retry_window);
  RUN_TEST(test_cycle_fail_sets_when_unserved_call_persists_in_failed_free_mode);
  RUN_TEST(
    test_coordination_alarm_sets_after_three_cycles_not_running_called_pattern);
  RUN_TEST(test_unit_red_revert_holds_phase_red_until_unit_minimum_expires);
  RUN_TEST(test_phase_red_revert_extends_unit_red_revert_when_larger);
  RUN_TEST(test_phase_dont_walk_revert_delays_next_walk_until_minimum_expires);
  RUN_TEST(
    test_phase_ped_clear_zero_clearance_limit_ends_on_first_yellow_tick);
  RUN_TEST(test_phase_ped_clear_limit_counts_across_yellow_and_red_clear);
  RUN_TEST(
    test_phase_ped_walk_service_one_blocks_recycle_until_next_coord_cycle);
  RUN_TEST(
    test_phase_ped_walk_service_two_allows_one_recycle_in_same_coord_cycle);
  RUN_TEST(
    test_startup_flash_uses_configured_mode_and_expires_to_normal_operation);
  RUN_TEST(test_system_flash_command_forces_flash_output_image);
  RUN_TEST(test_minus_green_yellow_overlap_follows_included_and_modifier_phases);
  RUN_TEST(test_fya_three_section_overlap_flashes_yellow_then_turns_green);
  RUN_TEST(test_fya_four_section_overlap_flashes_yellow_then_turns_dark);
  RUN_TEST(test_fra_three_section_overlap_flashes_red_then_turns_green);
  RUN_TEST(test_fra_four_section_overlap_flashes_red_then_turns_dark);
  RUN_TEST(
    test_minus_green_yellow_alternate_stays_green_when_modifier_is_not_next);
  RUN_TEST(test_minus_green_yellow_alternate_turns_red_when_modifier_is_next);
  RUN_TEST(test_programmed_automatic_flash_entry_and_exit_phases_follow_ts2_sequence);
  RUN_TEST(test_remote_preempt_control_drives_runtime_state_and_status_group);
  RUN_TEST(test_preempt_entry_outputs_follow_enter_yellow_and_red_clear);
  RUN_TEST(test_preempt_track_clear_advances_through_advanced_preempt);
  RUN_TEST(test_preempt_link_activates_higher_priority_preempt_call);
  RUN_TEST(test_preempt_exit_phases_transfer_runtime_to_configured_exit_phases);
  RUN_TEST(test_preempt_queue_delay_recovery_enters_highest_weighted_demand_phase);
  RUN_TEST(test_preempt_queue_delay_recovery_falls_back_to_longest_waiting_phase);
  RUN_TEST(test_preempt_short_service_enters_first_short_service_phase);
  RUN_TEST(test_preempt_exit_coord_returns_to_current_coordination_cycle_position);
  RUN_TEST(test_preempt_flash_dwell_ignores_cycling_phase_programming);
  RUN_TEST(test_preempt_cycling_phases_begin_after_dwell_interval);
  RUN_TEST(test_preempt_cycling_ped_service_starts_only_after_cycling_begins);
  RUN_TEST(test_phase_call_does_not_hold_green_past_gap_timer);
  RUN_TEST(test_volume_density_reduces_current_gap_after_tbr);
  RUN_TEST(test_phase_startup_phase_not_on_holds_red_until_ring_demand);
  RUN_TEST(test_phase_startup_yellow_change_initializes_ring_in_yellow);
  RUN_TEST(test_coordination_maximum2_selects_alternate_phase_maximum);
  RUN_TEST(test_coordination_maximum3_uses_32bit_running_maximum);
  RUN_TEST(test_dynamic_max_increases_after_two_consecutive_max_outs);
  RUN_TEST(test_dynamic_max_decreases_after_two_consecutive_gap_outs);
  RUN_TEST(test_dual_entry_phase_starts_on_barrier_cross_without_local_demand);
  RUN_TEST(test_gap_out_phase_resumes_from_barrier_wait_when_simultaneous_gap_is_enabled);
  RUN_TEST(test_gap_out_phase_stays_waiting_when_simultaneous_gap_disable_is_set);
  RUN_TEST(test_conditional_service_restarts_preceding_phase_while_waiting_at_barrier);
  RUN_TEST(test_conflicting_vehicle_actuation_starts_gap_reduction_when_cbr_is_met);
  RUN_TEST(test_added_initial_extends_initial_green_from_red_actuation_count);
  RUN_TEST(test_added_initial_is_limited_by_maximum_initial);
  RUN_TEST(test_green_actuation_without_non_lock_detector_memory_does_not_latch_before_yellow);
  RUN_TEST(test_green_detector_present_at_yellow_start_latches_when_non_lock_memory_is_clear);
  RUN_TEST(test_non_lock_detector_memory_option_preserves_green_actuation_call_memory);
  RUN_TEST(test_vehicle_detector_delay_applies_before_non_green_call_is_recognized);
  RUN_TEST(test_vehicle_detector_extend_keeps_green_detection_active_after_release);
  RUN_TEST(test_vehicle_detector_switch_phase_routes_green_extension_to_switch_phase);
  RUN_TEST(test_ped_detector_alternate_timing_uses_alternate_walk_and_clearance);
  RUN_TEST(
    test_phase_ped_advance_walk_starts_before_green_and_ignores_delay_time);
  RUN_TEST(test_phase_hold_control_keeps_active_phase_green_past_gap_out_point);
  RUN_TEST(test_phase_force_off_control_terminates_green_and_clears_after_yellow_entry);
  RUN_TEST(test_phase_omit_control_prevents_omitted_phase_from_being_selected);
  RUN_TEST(test_phase_ped_omit_control_blocks_remote_ped_service_start);
  RUN_TEST(test_phase_remote_vehicle_call_control_creates_service_demand);
  RUN_TEST(test_unit_control_external_minimum_recall_sets_vehicle_calls_for_all_phases);
  RUN_TEST(test_unit_control_non_actuated_command_services_programmed_phase);
  RUN_TEST(test_unit_control_non_actuated_requires_pedestrian_equipped_phase);
  RUN_TEST(test_unit_control_walk_rest_modifier_holds_walk_on_non_actuated_phase);
  RUN_TEST(test_interconnect_relinquished_by_timebase_pattern_zero_controls_runtime);
  RUN_TEST(test_unit_control_interconnect_priority_overrides_timebase);
  RUN_TEST(test_invalid_interconnect_inputs_report_interconnect_backup);
  RUN_TEST(test_timebase_dimming_requires_action_and_local_or_unit_enable);
  RUN_TEST(test_action_plan_control_reports_selected_timebase_action_and_can_flash);
  RUN_TEST(test_unit_backup_timer_enters_backup_mode_and_clears_remote_controls);
  RUN_TEST(
    test_user_defined_backup_timer_requires_explicit_reset_and_enters_backup_mode);
  RUN_TEST(test_remote_manual_control_holds_green_until_interval_advance_and_expires);
  RUN_TEST(test_remote_manual_interval_advance_with_auto_ped_clear_advances_walk_then_clear);
  RUN_TEST(test_remote_manual_interval_advance_requires_second_press_without_auto_ped_clear);
  RUN_TEST(test_special_function_output_status_ors_runtime_control_with_timebase_bits);
  RUN_TEST(test_ring_stop_time_control_freezes_elapsed_ticks);
  RUN_TEST(test_ring_force_off_control_terminates_green);
  RUN_TEST(test_ring_maximum2_control_selects_alternate_maximum);
  RUN_TEST(test_ring_maximum3_control_selects_third_maximum);
  RUN_TEST(test_ring_maximum_inhibit_control_prevents_max_out);
  RUN_TEST(test_ring_ped_recycle_control_requests_walk_on_startup_green);
  RUN_TEST(test_ring_red_rest_control_clears_to_red_rest_when_demand_drops);
  RUN_TEST(test_ring_omit_red_clear_control_skips_red_clear_interval);

  return UNITY_END();
}
