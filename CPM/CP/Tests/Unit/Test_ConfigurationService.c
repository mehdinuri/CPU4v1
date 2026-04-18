/*
 * Tests/Unit/Test_ConfigurationService.c
 *
 * Unit tests for immutable config slots and transactional commit behavior.
 */
#include "unity.h"

#include "Domain/Intersection/ConfigurationService.h"
#include "MockConfigRepositoryAdapter.h"

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_service;

void setUp(void)
{
  MockConfigRepositoryAdapterInit(&s_repoCtx);
  s_repoPort = MockConfigRepositoryAdapterCreatePort(&s_repoCtx);
  ConfigurationServiceInit(&s_service, &s_repoPort);
}

void tearDown(void)
{
}

static void CommitPhaseOneMinGreen(uint16_t valueDs)
{
  TEST_ASSERT_EQUAL_UINT8(1U, ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          ConfigurationServiceSetPhaseMinGreenDs(&s_service,
                                                                 0U,
                                                                 valueDs));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          ConfigurationServiceSetPhaseMaxInitialDs(&s_service,
                                                                   0U,
                                                                   valueDs));
  TEST_ASSERT_EQUAL_UINT8(1U, ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_EQUAL_UINT8(1U, ConfigurationServiceCommit(&s_service));
}

void test_empty_repository_uses_defaults_until_first_commit(void)
{
  const IntersectionConfig_t *config =
    ConfigurationServiceGetActiveConfig(&s_service);

  TEST_ASSERT_NOT_NULL(config);
  TEST_ASSERT_FALSE(ConfigurationServiceIsLoadedFromRepository(&s_service));
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_PHASE_COUNT_MAX, config->phaseCount);
  TEST_ASSERT_EQUAL_UINT32(0U,
                           ConfigurationServiceGetActiveGeneration(&s_service));
  TEST_ASSERT_EQUAL_UINT8(CONFIGURATION_SLOT_NONE,
                          ConfigurationServiceGetActiveSlot(&s_service));
  TEST_ASSERT_EQUAL_UINT16(50U, config->phases[0].minGreenDs);
}

void test_first_commit_targets_slot_b_and_survives_reload(void)
{
  ConfigurationService_t reloaded;

  CommitPhaseOneMinGreen(120U);

  TEST_ASSERT_TRUE(ConfigurationServiceIsLoadedFromRepository(&s_service));
  TEST_ASSERT_EQUAL_UINT8(CONFIGURATION_SLOT_B,
                          ConfigurationServiceGetActiveSlot(&s_service));
  TEST_ASSERT_EQUAL_UINT32(1U,
                           ConfigurationServiceGetActiveGeneration(&s_service));
  TEST_ASSERT_EQUAL_UINT16(120U,
                           ConfigurationServiceGetActiveConfig(
                             &s_service)->phases[0].minGreenDs);
  TEST_ASSERT_FALSE(ConfigurationServiceHasOpenTransaction(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);

  TEST_ASSERT_TRUE(ConfigurationServiceIsLoadedFromRepository(&reloaded));
  TEST_ASSERT_EQUAL_UINT8(CONFIGURATION_SLOT_B,
                          ConfigurationServiceGetActiveSlot(&reloaded));
  TEST_ASSERT_EQUAL_UINT32(1U,
                           ConfigurationServiceGetActiveGeneration(&reloaded));
  TEST_ASSERT_EQUAL_UINT16(120U,
                           ConfigurationServiceGetActiveConfig(
                             &reloaded)->phases[0].minGreenDs);
}

void test_newer_valid_slot_wins_over_older_slot(void)
{
  ConfigurationService_t reloaded;

  CommitPhaseOneMinGreen(120U);
  CommitPhaseOneMinGreen(140U);

  TEST_ASSERT_EQUAL_UINT8(CONFIGURATION_SLOT_A,
                          ConfigurationServiceGetActiveSlot(&s_service));
  TEST_ASSERT_EQUAL_UINT32(2U,
                           ConfigurationServiceGetActiveGeneration(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);

  TEST_ASSERT_TRUE(ConfigurationServiceIsLoadedFromRepository(&reloaded));
  TEST_ASSERT_EQUAL_UINT8(CONFIGURATION_SLOT_A,
                          ConfigurationServiceGetActiveSlot(&reloaded));
  TEST_ASSERT_EQUAL_UINT32(2U,
                           ConfigurationServiceGetActiveGeneration(&reloaded));
  TEST_ASSERT_EQUAL_UINT16(140U,
                           ConfigurationServiceGetActiveConfig(
                             &reloaded)->phases[0].minGreenDs);
}

void test_corrupt_newer_slot_falls_back_to_previous_valid_slot(void)
{
  ConfigurationService_t reloaded;
  uint8_t corruptByte = 0U;

  CommitPhaseOneMinGreen(120U);
  CommitPhaseOneMinGreen(140U);

  TEST_ASSERT_TRUE(ConfigRepositoryWrite(&s_repoPort,
                                         CONFIG_REPOSITORY_OBJECT_SLOT_A,
                                         CONFIGURATION_IMAGE_HEADER_SIZE,
                                         &corruptByte,
                                         sizeof(corruptByte)));

  ConfigurationServiceInit(&reloaded, &s_repoPort);

  TEST_ASSERT_TRUE(ConfigurationServiceIsLoadedFromRepository(&reloaded));
  TEST_ASSERT_EQUAL_UINT8(CONFIGURATION_SLOT_B,
                          ConfigurationServiceGetActiveSlot(&reloaded));
  TEST_ASSERT_EQUAL_UINT32(1U,
                           ConfigurationServiceGetActiveGeneration(&reloaded));
  TEST_ASSERT_EQUAL_UINT16(120U,
                           ConfigurationServiceGetActiveConfig(
                             &reloaded)->phases[0].minGreenDs);
}

void test_rollback_discards_candidate_changes(void)
{
  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseMinGreenDs(&s_service, 0U,
                                                          115U));

  ConfigurationServiceRollback(&s_service);

  TEST_ASSERT_FALSE(ConfigurationServiceHasOpenTransaction(&s_service));
  TEST_ASSERT_EQUAL_UINT16(50U,
                           ConfigurationServiceGetActiveConfig(
                             &s_service)->phases[0].minGreenDs);
}

void test_active_set_id_changes_after_committed_static_data_change(void)
{
  uint16_t before = ConfigurationServiceGetActiveSetId(&s_service);
  uint16_t after;

  CommitPhaseOneMinGreen(120U);
  after = ConfigurationServiceGetActiveSetId(&s_service);

  TEST_ASSERT_NOT_EQUAL(before, after);
}

void test_channel_and_overlap_configuration_persist_across_reload(void)
{
  ConfigurationService_t reloaded;
  IntersectionPhaseConfig_t phase;
  IntersectionChannelConfig_t channel;
  IntersectionOverlapConfig_t overlap;
  const uint8_t includedPhases[] = { 1U, 2U };

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseWalkSeconds(&s_service, 0U, 9U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhasePedClearSeconds(&s_service,
                                                               0U,
                                                               15U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetChannelControlType(&s_service,
                                                             0U,
                                                             INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE));
  TEST_ASSERT_TRUE(ConfigurationServiceSetChannelControlSource(&s_service,
                                                               0U,
                                                               1U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetOverlapType(&s_service,
                                                      0U,
                                                      INTERSECTION_OVERLAP_TYPE_NORMAL));
  TEST_ASSERT_TRUE(ConfigurationServiceSetOverlapIncludedPhases(&s_service,
                                                                0U,
                                                                includedPhases,
                                                                2U));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);

  TEST_ASSERT_TRUE(ConfigurationServiceGetActivePhaseConfig(&reloaded,
                                                            0U,
                                                            &phase));
  TEST_ASSERT_EQUAL_UINT16(9U, phase.walkSeconds);
  TEST_ASSERT_EQUAL_UINT16(15U, phase.pedClearSeconds);
  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveChannelConfig(&reloaded,
                                                              0U,
                                                              &channel));
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE,
                          channel.controlType);
  TEST_ASSERT_EQUAL_UINT8(1U, channel.controlSource);
  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveOverlapConfig(&reloaded,
                                                              0U,
                                                              &overlap));
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_OVERLAP_TYPE_NORMAL, overlap.type);
  TEST_ASSERT_EQUAL_UINT8(2U, overlap.includedPhases.length);
  TEST_ASSERT_EQUAL_UINT8(1U, overlap.includedPhases.values[0]);
  TEST_ASSERT_EQUAL_UINT8(2U, overlap.includedPhases.values[1]);
} /* test_channel_and_overlap_configuration_persist_across_reload */

void test_coordination_configuration_persists_across_reload(void)
{
  ConfigurationService_t reloaded;
  IntersectionCoordinationConfig_t coordination;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetCoordOperationalMode(&s_service, 2U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetCoordCorrectionMode(
                     &s_service,
                     INTERSECTION_COORD_CORRECTION_MODE_SHORTWAY));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPatternCycleTimeSeconds(&s_service,
                                                                  1U,
                                                                  90U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPatternOffsetTimeSeconds(&s_service,
                                                                   1U,
                                                                   15U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPatternSplitNumber(&s_service, 1U,
                                                             2U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetSplitTimeSeconds(&s_service,
                                                           1U,
                                                           0U,
                                                           25U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetSplitMode(
                     &s_service,
                     1U,
                     0U,
                     INTERSECTION_SPLIT_MODE_NON_ACTUATED));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);

  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveCoordinationConfig(&reloaded,
                                                                   &coordination));
  TEST_ASSERT_EQUAL_UINT8(2U, coordination.operationalMode);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_COORD_CORRECTION_MODE_SHORTWAY,
                          coordination.correctionMode);
  TEST_ASSERT_EQUAL_UINT8(90U, coordination.patterns[1].cycleTimeSeconds);
  TEST_ASSERT_EQUAL_UINT8(15U, coordination.patterns[1].offsetTimeSeconds);
  TEST_ASSERT_EQUAL_UINT8(2U, coordination.patterns[1].splitNumber);
  TEST_ASSERT_EQUAL_UINT8(25U, coordination.splits[1][0].timeSeconds);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_SPLIT_MODE_NON_ACTUATED,
                          coordination.splits[1][0].mode);
} /* test_coordination_configuration_persists_across_reload */

void test_preempt_configuration_persists_across_reload(void)
{
  ConfigurationService_t reloaded;
  IntersectionPreemptConfig_t preempt;
  IntersectionPreemptGateConfig_t gate;
  uint16_t detectorWeight = 0U;
  const uint8_t trackPhases[] = { 2U };
  const uint8_t exitPhases[] = { 1U };
  static const uint8_t gateDescription[] = "RR gate northbound";

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptControl(&s_service, 0U,
                                                         0x10U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptDelaySeconds(&s_service, 0U,
                                                              3U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptMinimumDurationSeconds(
                     &s_service,
                     0U,
                     8U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptTrackPhases(&s_service,
                                                             0U,
                                                             trackPhases,
                                                             1U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptExitPhases(&s_service,
                                                            0U,
                                                            exitPhases,
                                                            1U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptSequenceNumber(&s_service,
                                                                0U,
                                                                1U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptExitType(
                     &s_service,
                     0U,
                     INTERSECTION_PREEMPT_EXIT_TYPE_EXIT_PHASES));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptQueueDelayWeight(&s_service,
                                                                  0U,
                                                                  0U,
                                                                  375U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptGateDescription(
                     &s_service,
                     0U,
                     gateDescription,
                     (uint8_t) (sizeof(gateDescription) - 1U)));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);

  TEST_ASSERT_TRUE(ConfigurationServiceGetActivePreemptConfig(&reloaded,
                                                              0U,
                                                              &preempt));
  TEST_ASSERT_EQUAL_UINT8(0x10U, preempt.control);
  TEST_ASSERT_EQUAL_UINT16(3U, preempt.delaySeconds);
  TEST_ASSERT_EQUAL_UINT16(8U, preempt.minimumDurationSeconds);
  TEST_ASSERT_EQUAL_UINT8(1U, preempt.trackPhases.length);
  TEST_ASSERT_EQUAL_UINT8(2U, preempt.trackPhases.values[0]);
  TEST_ASSERT_EQUAL_UINT8(1U, preempt.exitPhases.length);
  TEST_ASSERT_EQUAL_UINT8(1U, preempt.exitPhases.values[0]);
  TEST_ASSERT_EQUAL_UINT8(1U, preempt.sequenceNumber);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_PREEMPT_EXIT_TYPE_EXIT_PHASES,
                          preempt.exitType);
  TEST_ASSERT_TRUE(ConfigurationServiceGetPreemptQueueDelayWeight(&reloaded,
                                                                  0U,
                                                                  0U,
                                                                  &
                                                                  detectorWeight));
  TEST_ASSERT_EQUAL_UINT16(375U, detectorWeight);
  TEST_ASSERT_TRUE(ConfigurationServiceGetActivePreemptGateConfig(&reloaded,
                                                                  0U,
                                                                  &gate));
  TEST_ASSERT_EQUAL_UINT8((uint8_t) (sizeof(gateDescription) - 1U),
                          gate.descriptionLength);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(gateDescription,
                                gate.description,
                                gate.descriptionLength);
} /* test_preempt_configuration_persists_across_reload */

void test_input_mapping_configuration_persists_across_reload(void)
{
  ConfigurationService_t reloaded;
  IntersectionInputMappingConfig_t inputMapping;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseDetectorInput(&s_service, 0U,
                                                             9U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhasePedInput(&s_service, 3U, 0U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhasePedInput(&s_service, 1U, 4U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptInputSource(&s_service, 0U,
                                                             3U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptControlSource(&s_service,
                                                               1U,
                                                               5U));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);

  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveInputMappingConfig(&reloaded,
                                                                   &inputMapping));
  TEST_ASSERT_EQUAL_UINT8(9U, inputMapping.phaseDetectors[0]);
  TEST_ASSERT_EQUAL_UINT8(4U, inputMapping.phasePedInputs[1]);
  TEST_ASSERT_EQUAL_UINT8(0U, inputMapping.phasePedInputs[3]);
  TEST_ASSERT_EQUAL_UINT8(3U, inputMapping.preemptInputs[0]);
  TEST_ASSERT_EQUAL_UINT8(5U, inputMapping.preemptControls[1]);
}

void test_ring_sequence_data_persists_across_reload(void)
{
  ConfigurationService_t reloaded;
  IntersectionRingPlan_t ringPlan;
  const uint8_t reorderedSequence[] = { 2U, 1U, 3U, 4U };

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetRingSequenceData(&s_service,
                                                           0U,
                                                           reorderedSequence,
                                                           4U));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);

  TEST_ASSERT_EQUAL_UINT8(1U, ConfigurationServiceGetSequenceCount(&reloaded));
  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveRingPlan(&reloaded,
                                                         0U,
                                                         &ringPlan));
  TEST_ASSERT_EQUAL_UINT8(4U, ringPlan.phaseCount);
  TEST_ASSERT_EQUAL_UINT8(1U, ringPlan.phaseOrder[0]);
  TEST_ASSERT_EQUAL_UINT8(0U, ringPlan.phaseOrder[1]);
  TEST_ASSERT_EQUAL_UINT8(2U, ringPlan.phaseOrder[2]);
  TEST_ASSERT_EQUAL_UINT8(3U, ringPlan.phaseOrder[3]);
}

void test_single_sequence_cap_rejects_pattern_and_preempt_sequence_numbers(void)
{
  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_FALSE(ConfigurationServiceSetPatternSequenceNumber(&s_service,
                                                                 0U,
                                                                 2U));
  TEST_ASSERT_FALSE(ConfigurationServiceSetPreemptSequenceNumber(&s_service,
                                                                 0U,
                                                                 2U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPatternSequenceNumber(&s_service,
                                                                0U,
                                                                1U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptSequenceNumber(&s_service,
                                                                0U,
                                                                1U));
}

void test_out_of_range_input_mapping_fails_verify(void)
{
  IntersectionConfigErrorInfo_t errorInfo;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptInputSource(&s_service, 0U,
                                                             9U));
  TEST_ASSERT_FALSE(ConfigurationServiceVerify(&s_service));

  errorInfo = ConfigurationServiceGetLastError(&s_service);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONFIG_ERROR_PREEMPT_INPUT_SOURCE,
                        errorInfo.type);
  TEST_ASSERT_EQUAL_UINT16(1U, errorInfo.objectIndex);
}

void test_unsupported_channel_control_type_fails_runtime_support_verify(void)
{
  IntersectionConfigErrorInfo_t errorInfo;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetChannelControlType(
                     &s_service,
                     0U,
                     INTERSECTION_CHANNEL_CONTROL_TYPE_PED_OVERLAP));
  TEST_ASSERT_TRUE(ConfigurationServiceSetChannelControlSource(&s_service, 0U,
                                                               1U));
  TEST_ASSERT_FALSE(ConfigurationServiceVerify(&s_service));

  errorInfo = ConfigurationServiceGetLastError(&s_service);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONFIG_ERROR_CHANNEL_CONTROL_TYPE,
                        errorInfo.type);
  TEST_ASSERT_EQUAL_UINT16(1U, errorInfo.objectIndex);
}

void test_supported_overlap_type_passes_runtime_support_verify(void)
{
  const uint8_t includedPhases[] = { 1U };
  const uint8_t modifierPhases[] = { 2U };

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetOverlapType(
                     &s_service,
                     0U,
                     INTERSECTION_OVERLAP_TYPE_FYA_THREE_SECTION));
  TEST_ASSERT_TRUE(ConfigurationServiceSetOverlapIncludedPhases(&s_service,
                                                                0U,
                                                                includedPhases,
                                                                1U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetOverlapModifierPhases(&s_service,
                                                                0U,
                                                                modifierPhases,
                                                                1U));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
}

void test_minus_green_yellow_alternate_passes_runtime_support_verify(void)
{
  const uint8_t includedPhases[] = { 1U, 2U };
  const uint8_t modifierPhases[] = { 3U };

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(
    ConfigurationServiceSetOverlapType(&s_service,
                                       0U,
                                       INTERSECTION_OVERLAP_TYPE_MINUS_GREEN_YELLOW_ALTERNATE));
  TEST_ASSERT_TRUE(ConfigurationServiceSetOverlapIncludedPhases(&s_service,
                                                                0U,
                                                                includedPhases,
                                                                2U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetOverlapModifierPhases(&s_service,
                                                                0U,
                                                                modifierPhases,
                                                                1U));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
}

void test_unsupported_overlap_type_fails_runtime_support_verify(void)
{
  IntersectionConfigErrorInfo_t errorInfo;
  const uint8_t includedPhases[] = { 1U };

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetOverlapType(
                     &s_service,
                     0U,
                     INTERSECTION_OVERLAP_TYPE_TRANSIT_2));
  TEST_ASSERT_TRUE(ConfigurationServiceSetOverlapIncludedPhases(&s_service,
                                                                0U,
                                                                includedPhases,
                                                                1U));
  TEST_ASSERT_FALSE(ConfigurationServiceVerify(&s_service));

  errorInfo = ConfigurationServiceGetLastError(&s_service);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONFIG_ERROR_OVERLAP_TYPE, errorInfo.type);
  TEST_ASSERT_EQUAL_UINT16(1U, errorInfo.objectIndex);
}

void test_supported_preempt_exit_type_passes_runtime_support_verify(void)
{
  const uint8_t trackPhases[] = { 2U };
  const uint8_t exitPhases[] = { 1U };

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptControl(&s_service, 0U,
                                                         0x10U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptTrackPhases(&s_service,
                                                             0U,
                                                             trackPhases,
                                                             1U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptExitPhases(&s_service,
                                                            0U,
                                                            exitPhases,
                                                            1U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPreemptExitType(
                     &s_service,
                     0U,
                     INTERSECTION_PREEMPT_EXIT_TYPE_SHORT_SERVICE));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
}

void test_phase_maximum_initial_below_minimum_green_fails_verify(void)
{
  IntersectionConfigErrorInfo_t errorInfo;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseMinGreenDs(&s_service, 0U,
                                                          120U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseMaxInitialDs(&s_service, 0U,
                                                            110U));
  TEST_ASSERT_FALSE(ConfigurationServiceVerify(&s_service));

  errorInfo = ConfigurationServiceGetLastError(&s_service);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONFIG_ERROR_MAX_INITIAL,
                        errorInfo.type);
  TEST_ASSERT_EQUAL_UINT16(1U, errorInfo.objectIndex);
}

void test_extended_phase_configuration_persists_across_reload(void)
{
  ConfigurationService_t reloaded;
  IntersectionPhaseConfig_t phase;
  const uint8_t concurrency[] = { 5U, 6U };

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseOptions(
                     &s_service,
                     0U,
                     (uint16_t) (PHASE_OPTIONS_ENABLED
                                 | PHASE_OPTIONS_MIN_RECALL
                                 | PHASE_OPTIONS_GUARANTEED_PASS)));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseMaximum2Ds(&s_service, 0U,
                                                          470U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseMaximum3Ds(&s_service, 0U,
                                                          3210U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseStartup(
                     &s_service,
                     0U,
                     (uint8_t) INTERSECTION_PHASE_STARTUP_GREEN_NO_WALK));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseConcurrency(&s_service,
                                                           0U,
                                                           concurrency,
                                                           2U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhasePedAlternateClearSeconds(
                     &s_service,
                     0U,
                     18U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhasePedAlternateWalkSeconds(
                     &s_service,
                     0U,
                     11U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPhaseAltMinTimeTransitionSeconds(
                     &s_service,
                     0U,
                     9U));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);
  TEST_ASSERT_TRUE(ConfigurationServiceGetActivePhaseConfig(&reloaded,
                                                            0U,
                                                            &phase));
  TEST_ASSERT_EQUAL_UINT16((uint16_t) (PHASE_OPTIONS_ENABLED
                                       | PHASE_OPTIONS_MIN_RECALL
                                       | PHASE_OPTIONS_GUARANTEED_PASS),
                           phase.phaseOptions);
  TEST_ASSERT_EQUAL_UINT16(470U, phase.phaseMaximum2Ds);
  TEST_ASSERT_EQUAL_UINT16(3210U, phase.phaseMaximum3Ds);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_PHASE_STARTUP_GREEN_NO_WALK,
                          phase.startup);
  TEST_ASSERT_EQUAL_UINT8(2U, phase.concurrency.length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(concurrency,
                                phase.concurrency.values,
                                phase.concurrency.length);
  TEST_ASSERT_EQUAL_UINT16(18U, phase.pedAlternateClearSeconds);
  TEST_ASSERT_EQUAL_UINT16(11U, phase.pedAlternateWalkSeconds);
  TEST_ASSERT_EQUAL_UINT8(9U, phase.altMinTimeTransitionSeconds);
}

void test_detector_configuration_persists_across_reload(void)
{
  ConfigurationService_t reloaded;
  IntersectionVehicleDetectorConfig_t vehicleDetector;
  IntersectionPedestrianDetectorConfig_t pedestrianDetector;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVehicleDetectorOptions(&s_service,
                                                                1U,
                                                                (uint8_t) (
                                                                  VEHICLE_DETECTOR_OPTIONS_CALL
                                                                  | VEHICLE_DETECTOR_OPTIONS_QUEUE)));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVehicleDetectorCallPhase(&s_service,
                                                                   1U,
                                                                   1U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVehicleDetectorSwitchPhase(&s_service,
                                                                     1U,
                                                                     3U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVehicleDetectorDelayDs(&s_service,
                                                                 1U,
                                                                 25U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVehicleDetectorExtendDs(&s_service,
                                                                  1U,
                                                                  11U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVehicleDetectorQueueLimitSeconds(
                     &s_service,
                     1U,
                     9U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVehicleDetectorFailTimeSeconds(
                     &s_service,
                     1U,
                     17U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPedestrianDetectorCallPhase(
                     &s_service,
                     0U,
                     2U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPedestrianDetectorApsMinimumActuationDs(
                     &s_service,
                     0U,
                     12U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPedestrianDetectorOptions(
                     &s_service,
                     0U,
                     (uint8_t) (PED_DETECTOR_OPTIONS_ALT_TIMING
                                | PED_DETECTOR_OPTIONS_NON_LOCKING)));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);

  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveVehicleDetectorConfig(&reloaded,
                                                                      1U,
                                                                      &vehicleDetector));
  TEST_ASSERT_EQUAL_UINT8((uint8_t) (VEHICLE_DETECTOR_OPTIONS_CALL
                                     | VEHICLE_DETECTOR_OPTIONS_QUEUE),
                          vehicleDetector.options);
  TEST_ASSERT_EQUAL_UINT8(1U, vehicleDetector.callPhase);
  TEST_ASSERT_EQUAL_UINT8(3U, vehicleDetector.switchPhase);
  TEST_ASSERT_EQUAL_UINT16(25U, vehicleDetector.delayDs);
  TEST_ASSERT_EQUAL_UINT8(11U, vehicleDetector.extendDs);
  TEST_ASSERT_EQUAL_UINT8(9U, vehicleDetector.queueLimitSeconds);
  TEST_ASSERT_EQUAL_UINT8(17U, vehicleDetector.failTimeSeconds);

  TEST_ASSERT_TRUE(ConfigurationServiceGetActivePedestrianDetectorConfig(
                     &reloaded,
                     0U,
                     &pedestrianDetector));
  TEST_ASSERT_EQUAL_UINT8(2U, pedestrianDetector.callPhase);
  TEST_ASSERT_EQUAL_UINT8(12U, pedestrianDetector.apsMinimumActuationDs);
  TEST_ASSERT_EQUAL_UINT8((uint8_t) (PED_DETECTOR_OPTIONS_ALT_TIMING
                                     | PED_DETECTOR_OPTIONS_NON_LOCKING),
                          pedestrianDetector.options);
}

void test_vehicle_detector_delay_above_mib_limit_fails_verify(void)
{
  IntersectionConfigErrorInfo_t errorInfo;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVehicleDetectorDelayDs(&s_service,
                                                                 0U,
                                                                 2551U));
  TEST_ASSERT_FALSE(ConfigurationServiceVerify(&s_service));

  errorInfo = ConfigurationServiceGetLastError(&s_service);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONFIG_ERROR_VEHICLE_DETECTOR_DELAY,
                        errorInfo.type);
  TEST_ASSERT_EQUAL_UINT16(1U, errorInfo.objectIndex);
}

void test_vehicle_detector_pairing_setter_maintains_reciprocal_relationships(
  void)
{
  IntersectionVehicleDetectorConfig_t detector;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVehicleDetectorPairedDetector(
                     &s_service,
                     0U,
                     2U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVehicleDetectorPairedDetector(
                     &s_service,
                     1U,
                     3U));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveVehicleDetectorConfig(
                     &s_service,
                     0U,
                     &detector));
  TEST_ASSERT_EQUAL_UINT8(0U, detector.pairedDetector);
  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveVehicleDetectorConfig(
                     &s_service,
                     1U,
                     &detector));
  TEST_ASSERT_EQUAL_UINT8(3U, detector.pairedDetector);
  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveVehicleDetectorConfig(
                     &s_service,
                     2U,
                     &detector));
  TEST_ASSERT_EQUAL_UINT8(2U, detector.pairedDetector);
}

void test_detector_report_periods_persist_across_reload(void)
{
  ConfigurationService_t reloaded;
  IntersectionDetectorReportConfig_t detectorReports;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVolumeOccupancyPeriodSeconds(
                     &s_service,
                     15U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVolumeOccupancyPeriodV3Seconds(
                     &s_service,
                     65535U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPedestrianDetectorPeriodSeconds(
                     &s_service,
                     65534U));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);
  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveDetectorReportConfig(
                     &reloaded,
                     &detectorReports));
  TEST_ASSERT_EQUAL_UINT8(15U, detectorReports.volumeOccupancyPeriodSeconds);
  TEST_ASSERT_EQUAL_UINT16(65535U,
                           detectorReports.volumeOccupancyPeriodV3Seconds);
  TEST_ASSERT_EQUAL_UINT16(65534U,
                           detectorReports.pedestrianDetectorPeriodSeconds);
}

void test_invalid_detector_report_period_values_fail_verify(void)
{
  IntersectionConfigErrorInfo_t errorInfo;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetVolumeOccupancyPeriodV3Seconds(
                     &s_service,
                     3601U));
  TEST_ASSERT_FALSE(ConfigurationServiceVerify(&s_service));
  errorInfo = ConfigurationServiceGetLastError(&s_service);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONFIG_ERROR_VEHICLE_REPORT_PERIOD_V3,
                        errorInfo.type);

  ConfigurationServiceRollback(&s_service);

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetPedestrianDetectorPeriodSeconds(
                     &s_service,
                     3601U));
  TEST_ASSERT_FALSE(ConfigurationServiceVerify(&s_service));
  errorInfo = ConfigurationServiceGetLastError(&s_service);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONFIG_ERROR_PED_REPORT_PERIOD,
                        errorInfo.type);
}

void test_timebase_configuration_persists_across_reload(void)
{
  ConfigurationService_t reloaded;
  IntersectionTimebaseConfig_t timebase;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetTimebasePatternSyncMinutes(
                     &s_service,
                     1234U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetTimebaseActionPattern(&s_service,
                                                                0U,
                                                                255U));
  TEST_ASSERT_TRUE(
    ConfigurationServiceSetTimebaseActionAuxiliaryFunction(
      &s_service,
      0U,
      INTERSECTION_TIMEBASE_AUX_FUNCTION_DIMMING
      | INTERSECTION_TIMEBASE_AUX_FUNCTION_1));
  TEST_ASSERT_TRUE(ConfigurationServiceSetTimebaseActionSpecialFunction(
                     &s_service,
                     0U,
                     0xAAU));
  TEST_ASSERT_TRUE(ConfigurationServiceSetTimebaseActionEnabledLane(
                     &s_service,
                     0U,
                     7U));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);
  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveTimebaseConfig(&reloaded,
                                                               &timebase));
  TEST_ASSERT_EQUAL_UINT16(1234U, timebase.patternSyncMinutes);
  TEST_ASSERT_EQUAL_UINT8(255U, timebase.actions[0].pattern);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_TIMEBASE_AUX_FUNCTION_DIMMING
                          | INTERSECTION_TIMEBASE_AUX_FUNCTION_1,
                          timebase.actions[0].auxiliaryFunction);
  TEST_ASSERT_EQUAL_UINT8(0xAAU, timebase.actions[0].specialFunction);
  TEST_ASSERT_EQUAL_UINT8(7U, timebase.actions[0].enabledLane);
}

void test_unit_configuration_persists_across_reload(void)
{
  ConfigurationService_t reloaded;
  IntersectionUnitConfig_t unit;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUnitStartUpFlashSeconds(&s_service,
                                                                  15U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUnitAutoPedestrianClear(
                     &s_service,
                     (uint8_t)
                     INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_ENABLE));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUnitBackupTimeSeconds(&s_service,
                                                                321U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUnitRedRevertDs(&s_service, 12U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUnitStartUpFlashMode(
                     &s_service,
                     (uint8_t)
                     INTERSECTION_UNIT_STARTUP_FLASH_MODE_ALL_RED_FLASH_OVERRIDE));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUnitTimeSourceCommanded(
                     &s_service,
                     (uint8_t) UNIT_CLOCK_SOURCE_RTC_SQWR));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUnitElevationOffsetMeters(&s_service,
                                                                    18U));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);
  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveUnitConfig(&reloaded, &unit));
  TEST_ASSERT_EQUAL_UINT8(15U, unit.startUpFlashSeconds);
  TEST_ASSERT_EQUAL_UINT8(
    (uint8_t) INTERSECTION_UNIT_AUTO_PEDESTRIAN_CLEAR_ENABLE,
    unit.autoPedestrianClear);
  TEST_ASSERT_EQUAL_UINT16(321U, unit.backupTimeSeconds);
  TEST_ASSERT_EQUAL_UINT8(12U, unit.redRevertDs);
  TEST_ASSERT_EQUAL_UINT8(
    (uint8_t) INTERSECTION_UNIT_STARTUP_FLASH_MODE_ALL_RED_FLASH_OVERRIDE,
    unit.startUpFlashMode);
  TEST_ASSERT_EQUAL_UINT8((uint8_t) UNIT_CLOCK_SOURCE_RTC_SQWR,
                          unit.timeSourceCommanded);
  TEST_ASSERT_EQUAL_UINT8(18U, unit.elevationOffsetMeters);
}

void test_line_sync_time_source_persists_across_reload(void)
{
  ConfigurationService_t reloaded;
  IntersectionUnitConfig_t unit;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUnitTimeSourceCommanded(
                     &s_service,
                     (uint8_t) UNIT_CLOCK_SOURCE_LINE_SYNC));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);
  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveUnitConfig(&reloaded, &unit));
  TEST_ASSERT_EQUAL_UINT8((uint8_t) UNIT_CLOCK_SOURCE_LINE_SYNC,
                          unit.timeSourceCommanded);
}

void test_unit_elevation_offset_above_mib_limit_fails_verify(void)
{
  IntersectionConfigErrorInfo_t errorInfo;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUnitElevationOffsetMeters(&s_service,
                                                                    32U));
  TEST_ASSERT_FALSE(ConfigurationServiceVerify(&s_service));
  errorInfo = ConfigurationServiceGetLastError(&s_service);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONFIG_ERROR_UNIT_ELEVATION_OFFSET,
                        errorInfo.type);
}

void test_user_defined_backup_configuration_persists_across_reload(void)
{
  ConfigurationService_t reloaded;
  IntersectionUnitConfig_t unit;
  IntersectionUserDefinedBackupContentConfig_t content;
  static const uint32_t backupOid[] =
  {
    1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 4U, 14U, 0U
  };
  static const uint8_t description[] = "systemPatternControl";

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUnitUserDefinedBackupTimeSeconds(
                     &s_service,
                     11U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUserDefinedBackupContentOid(
                     &s_service,
                     0U,
                     backupOid,
                     (uint8_t) (sizeof(backupOid) / sizeof(backupOid[0]))));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUserDefinedBackupContentDescription(
                     &s_service,
                     0U,
                     description,
                     (uint8_t) (sizeof(description) - 1U)));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  ConfigurationServiceInit(&reloaded, &s_repoPort);
  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveUnitConfig(&reloaded, &unit));
  TEST_ASSERT_EQUAL_UINT32(11U, unit.userDefinedBackupTimeSeconds);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_USER_DEFINED_BACKUP_CONTENT_COUNT_MAX,
                          ConfigurationServiceGetUserDefinedBackupContentCount(
                            &reloaded));
  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveUserDefinedBackupContentConfig(
                     &reloaded,
                     0U,
                     &content));
  TEST_ASSERT_EQUAL_UINT8(sizeof(backupOid) / sizeof(backupOid[0]),
                          content.oidLength);
  TEST_ASSERT_EQUAL_UINT32_ARRAY(backupOid, content.oid, content.oidLength);
  TEST_ASSERT_EQUAL_UINT8(sizeof(description) - 1U, content.descriptionLength);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(description,
                                content.description,
                                content.descriptionLength);
}

void test_unit_backup_time_write_is_ignored_when_user_defined_backup_time_is_nonzero(
  void)
{
  IntersectionUnitConfig_t unit;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUnitUserDefinedBackupTimeSeconds(
                     &s_service,
                     15U));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUnitBackupTimeSeconds(&s_service,
                                                                777U));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_service));

  TEST_ASSERT_TRUE(ConfigurationServiceGetActiveUnitConfig(&s_service, &unit));
  TEST_ASSERT_EQUAL_UINT32(15U, unit.userDefinedBackupTimeSeconds);
  TEST_ASSERT_EQUAL_UINT16(0U, unit.backupTimeSeconds);
}

void test_user_defined_backup_content_oid_length_above_limit_fails_verify(void)
{
  IntersectionConfigErrorInfo_t errorInfo;
  uint32_t oid[INTERSECTION_USER_DEFINED_BACKUP_OID_COMPONENT_COUNT_MAX + 1U];
  uint8_t index;

  for (index = 0U; index < (uint8_t) (sizeof(oid) / sizeof(oid[0])); index++)
  {
    oid[index] = (uint32_t) (index + 1U);
  }

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUserDefinedBackupContentOid(
                     &s_service,
                     0U,
                     oid,
                     (uint8_t) (sizeof(oid) / sizeof(oid[0]))));
  TEST_ASSERT_FALSE(ConfigurationServiceVerify(&s_service));
  errorInfo = ConfigurationServiceGetLastError(&s_service);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONFIG_ERROR_UNIT_USER_DEFINED_BACKUP_OID,
                        errorInfo.type);
}

void test_user_defined_backup_content_description_length_above_limit_fails_verify(
  void)
{
  IntersectionConfigErrorInfo_t errorInfo;
  uint8_t description[INTERSECTION_USER_DEFINED_BACKUP_DESCRIPTION_MAX + 1U];
  uint8_t index;

  for (index = 0U; index < sizeof(description); index++)
  {
    description[index] = (uint8_t) ('A' + (index % 26U));
  }

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(ConfigurationServiceSetUserDefinedBackupContentDescription(
                     &s_service,
                     0U,
                     description,
                     (uint8_t) sizeof(description)));
  TEST_ASSERT_FALSE(ConfigurationServiceVerify(&s_service));
  errorInfo = ConfigurationServiceGetLastError(&s_service);
  TEST_ASSERT_EQUAL_INT(
    INTERSECTION_CONFIG_ERROR_UNIT_USER_DEFINED_BACKUP_DESCRIPTION,
    errorInfo.type);
}

void test_reserved_timebase_auxiliary_bits_fail_verify(void)
{
  IntersectionConfigErrorInfo_t errorInfo;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_service));
  TEST_ASSERT_TRUE(
    ConfigurationServiceSetTimebaseActionAuxiliaryFunction(&s_service,
                                                           0U,
                                                           0x80U));
  TEST_ASSERT_FALSE(ConfigurationServiceVerify(&s_service));
  errorInfo = ConfigurationServiceGetLastError(&s_service);
  TEST_ASSERT_EQUAL_INT(
    INTERSECTION_CONFIG_ERROR_TIMEBASE_ACTION_AUXILIARY_FUNCTION,
    errorInfo.type);
  TEST_ASSERT_EQUAL_UINT16(1U, errorInfo.objectIndex);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_empty_repository_uses_defaults_until_first_commit);
  RUN_TEST(test_first_commit_targets_slot_b_and_survives_reload);
  RUN_TEST(test_newer_valid_slot_wins_over_older_slot);
  RUN_TEST(test_corrupt_newer_slot_falls_back_to_previous_valid_slot);
  RUN_TEST(test_rollback_discards_candidate_changes);
  RUN_TEST(test_active_set_id_changes_after_committed_static_data_change);
  RUN_TEST(test_channel_and_overlap_configuration_persist_across_reload);
  RUN_TEST(test_coordination_configuration_persists_across_reload);
  RUN_TEST(test_preempt_configuration_persists_across_reload);
  RUN_TEST(test_input_mapping_configuration_persists_across_reload);
  RUN_TEST(test_ring_sequence_data_persists_across_reload);
  RUN_TEST(test_single_sequence_cap_rejects_pattern_and_preempt_sequence_numbers);
  RUN_TEST(test_out_of_range_input_mapping_fails_verify);
  RUN_TEST(test_unsupported_channel_control_type_fails_runtime_support_verify);
  RUN_TEST(test_supported_overlap_type_passes_runtime_support_verify);
  RUN_TEST(test_minus_green_yellow_alternate_passes_runtime_support_verify);
  RUN_TEST(test_unsupported_overlap_type_fails_runtime_support_verify);
  RUN_TEST(test_supported_preempt_exit_type_passes_runtime_support_verify);
  RUN_TEST(test_phase_maximum_initial_below_minimum_green_fails_verify);
  RUN_TEST(test_extended_phase_configuration_persists_across_reload);
  RUN_TEST(test_detector_configuration_persists_across_reload);
  RUN_TEST(test_vehicle_detector_delay_above_mib_limit_fails_verify);
  RUN_TEST(
    test_vehicle_detector_pairing_setter_maintains_reciprocal_relationships);
  RUN_TEST(test_detector_report_periods_persist_across_reload);
  RUN_TEST(test_invalid_detector_report_period_values_fail_verify);
  RUN_TEST(test_timebase_configuration_persists_across_reload);
  RUN_TEST(test_unit_configuration_persists_across_reload);
  RUN_TEST(test_line_sync_time_source_persists_across_reload);
  RUN_TEST(test_unit_elevation_offset_above_mib_limit_fails_verify);
  RUN_TEST(test_user_defined_backup_configuration_persists_across_reload);
  RUN_TEST(
    test_unit_backup_time_write_is_ignored_when_user_defined_backup_time_is_nonzero);
  RUN_TEST(test_user_defined_backup_content_oid_length_above_limit_fails_verify);
  RUN_TEST(
    test_user_defined_backup_content_description_length_above_limit_fails_verify);
  RUN_TEST(test_reserved_timebase_auxiliary_bits_fail_verify);

  return UNITY_END();
}
