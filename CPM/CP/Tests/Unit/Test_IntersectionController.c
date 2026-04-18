/*
 * Tests/Unit/Test_IntersectionController.c
 *
 * Unit tests for controller-step orchestration from module-bus snapshots
 * into engine runtime and dispatched output.
 */
#include "unity.h"

#include "Domain/Intersection/IntersectionController.h"

#include <string.h>

typedef struct
{
  ModuleBusSnapshot_t snapshot;
  uint8_t available;
} FakeModuleBusCtx_t;

typedef struct
{
  OutputDriverImage_t lastImage;
  uint8_t applyCount;
  uint8_t succeed;
} FakeOutputCtx_t;

typedef struct
{
  uint8_t forceAllRed;
  uint8_t setForceAllRedResult;
} FakeMmuCtx_t;

typedef struct
{
  uint8_t dimmingInputActive;
  uint8_t interconnectCommand;
  uint8_t interconnectInputsValid;
} FakeUnitInputCtx_t;

static IntersectionEngine_t s_engine;
static IntersectionOutputDispatcher_t s_dispatcher;
static IntersectionController_t s_controller;
static FakeModuleBusCtx_t s_moduleBusCtx;
static FakeOutputCtx_t s_outputCtx;
static FakeMmuCtx_t s_mmuCtx;
static FakeUnitInputCtx_t s_unitInputCtx;
static IModuleBusPort_t s_moduleBusPort;
static IUnitInputPort_t s_unitInputPort;
static IOutputDriverPort_t s_outputPort;
static IMmuPort_t s_mmuPort;

static void ReplicateSequencePlansFromBase(IntersectionConfig_t *config)
{
  uint8_t sequenceIndex;
  uint8_t ringIndex;

  if (config == NULL)
  {
    return;
  }

  for (sequenceIndex = 1U;
       sequenceIndex < INTERSECTION_SEQUENCE_COUNT_MAX;
       sequenceIndex++)
  {
    for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
    {
      config->sequencePlans[sequenceIndex][ringIndex] = config->rings[ringIndex];
    }
  }
}

static uint8_t FakeModuleBusRead(void *ctx, ModuleBusSnapshot_t *snapshot)
{
  FakeModuleBusCtx_t *moduleBusCtx = (FakeModuleBusCtx_t *) ctx;

  if ((moduleBusCtx == NULL) || (snapshot == NULL)
      || (moduleBusCtx->available == 0U))
  {
    return 0U;
  }

  *snapshot = moduleBusCtx->snapshot;

  return 1U;
}

static uint8_t FakeOutputApply(void *ctx, const OutputDriverImage_t *image)
{
  FakeOutputCtx_t *outputCtx = (FakeOutputCtx_t *) ctx;

  outputCtx->lastImage = *image;
  outputCtx->applyCount++;

  return outputCtx->succeed;
}

static uint8_t FakeMmuSetForceAllRed(void *ctx, uint8_t forceAllRed)
{
  FakeMmuCtx_t *mmuCtx = (FakeMmuCtx_t *) ctx;

  mmuCtx->forceAllRed = (uint8_t) (forceAllRed != 0U);

  return mmuCtx->setForceAllRedResult;
}

static uint8_t FakeMmuFilter(void *ctx,
                             const OutputDriverImage_t *requested,
                             OutputDriverImage_t *approved)
{
  FakeMmuCtx_t *mmuCtx = (FakeMmuCtx_t *) ctx;
  uint8_t channelIndex;

  *approved = *requested;

  if (mmuCtx->forceAllRed == 0U)
  {
    return 1U;
  }

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       ++channelIndex)
  {
    approved->channels[channelIndex] = OUTPUT_DRIVER_ASPECT_RED;
    approved->channelDimmed[channelIndex] = 0U;
    approved->channelDimAlternateHalfCycle[channelIndex] = 0U;
  }

  return 1U;
}

static uint8_t FakeGetDimmingInputActive(void *ctx)
{
  FakeUnitInputCtx_t *unitInputCtx = (FakeUnitInputCtx_t *) ctx;

  if (unitInputCtx == NULL)
  {
    return 0U;
  }

  return unitInputCtx->dimmingInputActive;
}

static uint8_t FakeGetInterconnectCommand(void *ctx)
{
  FakeUnitInputCtx_t *unitInputCtx = (FakeUnitInputCtx_t *) ctx;

  if (unitInputCtx == NULL)
  {
    return 0U;
  }

  return unitInputCtx->interconnectCommand;
}

static uint8_t FakeGetInterconnectInputsValid(void *ctx)
{
  FakeUnitInputCtx_t *unitInputCtx = (FakeUnitInputCtx_t *) ctx;

  if (unitInputCtx == NULL)
  {
    return 1U;
  }

  return unitInputCtx->interconnectInputsValid;
}

static IntersectionConfig_t MakeControllerConfig(void)
{
  IntersectionConfig_t config;
  uint8_t phaseIndex;
  uint8_t detectorIndex;

  IntersectionConfigInitDefaults(&config);
  config.phaseCount = 4U;
  config.ringCount = 2U;
  config.barrierCount = 2U;

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; ++phaseIndex)
  {
    config.phases[phaseIndex].phaseOptions = PHASE_OPTIONS_ENABLED;
    config.phases[phaseIndex].ring = (phaseIndex < 2U) ? 0U : 1U;
  }

  config.phases[0].walkSeconds = 1U;
  config.phases[0].pedClearSeconds = 2U;

  config.rings[0].phaseCount = 2U;
  config.rings[0].barrierPhaseCount = 1U;
  config.rings[0].phaseOrder[0] = 0U;
  config.rings[0].phaseOrder[1] = 1U;
  config.rings[1].phaseCount = 2U;
  config.rings[1].barrierPhaseCount = 1U;
  config.rings[1].phaseOrder[0] = 2U;
  config.rings[1].phaseOrder[1] = 3U;
  ReplicateSequencePlansFromBase(&config);

  config.vehicleDetectors[0].callPhase = 2U;
  config.vehicleDetectors[1].callPhase = 1U;
  config.pedestrianDetectors[0].callPhase = 1U;
  config.pedestrianDetectors[1].callPhase = 2U;

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

  config.inputMapping.preemptInputs[0] = 2U;
  config.inputMapping.preemptControls[0] = 1U;

  config.channels[0].controlSource = 1U;
  config.channels[0].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  config.channels[1].controlSource = 1U;
  config.channels[1].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN;

  return config;
} /* MakeControllerConfig */

void setUp(void)
{
  IntersectionConfig_t config = MakeControllerConfig();

  IntersectionEngineInit(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));

  memset(&s_moduleBusCtx, 0, sizeof(s_moduleBusCtx));
  memset(&s_outputCtx, 0, sizeof(s_outputCtx));
  memset(&s_mmuCtx, 0, sizeof(s_mmuCtx));
  memset(&s_unitInputCtx, 0, sizeof(s_unitInputCtx));
  s_unitInputCtx.interconnectInputsValid = 1U;
  s_moduleBusCtx.available = 0U;
  s_outputCtx.succeed = 1U;
  s_mmuCtx.setForceAllRedResult = 1U;

  s_moduleBusPort.ctx = &s_moduleBusCtx;
  s_moduleBusPort.ReadSnapshot = FakeModuleBusRead;
  s_unitInputPort.ctx = &s_unitInputCtx;
  s_unitInputPort.GetDimmingInputActive = FakeGetDimmingInputActive;
  s_unitInputPort.GetInterconnectCommand = FakeGetInterconnectCommand;
  s_unitInputPort.GetInterconnectInputsValid = FakeGetInterconnectInputsValid;
  s_outputPort.ctx = &s_outputCtx;
  s_outputPort.Apply = FakeOutputApply;
  s_mmuPort.ctx = &s_mmuCtx;
  s_mmuPort.SetForceAllRed = FakeMmuSetForceAllRed;
  s_mmuPort.FilterOutputImage = FakeMmuFilter;

  IntersectionOutputDispatcherInit(&s_dispatcher);
  IntersectionOutputDispatcherBind(&s_dispatcher,
                                   &s_engine,
                                   &s_mmuPort,
                                   &s_outputPort);

  IntersectionControllerInit(&s_controller);
  IntersectionControllerBind(&s_controller,
                             &s_engine,
                             &s_dispatcher,
                             NULL,
                             &s_moduleBusPort,
                             &s_unitInputPort,
                             &s_mmuPort);
}

void tearDown(void)
{
}

void test_controller_maps_physical_inputs_to_configured_phase_sources(void)
{
  IntersectionPhaseStatusGroup_t statusGroup;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));

  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  s_moduleBusCtx.snapshot.validMask =
    MODULE_BUS_SNAPSHOT_VALID_DETECTORS | MODULE_BUS_SNAPSHOT_VALID_PEDS;
  s_moduleBusCtx.snapshot.healthMask = s_moduleBusCtx.snapshot.validMask;
  s_moduleBusCtx.snapshot.detectorInputs = 0x0002U; /* physical detector 2 -> phase 1 */
  s_moduleBusCtx.snapshot.pedInputs = 0x01U;        /* physical ped input 1 -> phase 1 */

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  TEST_ASSERT_TRUE(
    IntersectionEngineGetPhaseStatusGroup(&s_engine, 1U, &statusGroup));
  TEST_ASSERT_BITS_HIGH(0x01U, statusGroup.vehCalls);
  TEST_ASSERT_BITS_HIGH(0x01U, statusGroup.walks);
  TEST_ASSERT_BITS_LOW(0x40U, statusGroup.vehCalls);
}

void test_controller_forces_flash_and_all_red_from_mmu_snapshot(void)
{
  const IntersectionRuntime_t *runtime;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));

  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  s_moduleBusCtx.snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS;
  s_moduleBusCtx.snapshot.healthMask = MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS;
  s_moduleBusCtx.snapshot.mmuStatus = MODULE_BUS_MMU_STATUS_FORCE_ALL_RED;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_NOT_NULL(runtime);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FLASH, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->mmuFlashActive);
  TEST_ASSERT_EQUAL_UINT8(1U, s_mmuCtx.forceAllRed);
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_RED,
                        s_outputCtx.lastImage.channels[0]);
}

void test_controller_clears_stale_detector_source(void)
{
  IntersectionPhaseStatusGroup_t statusGroup;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));

  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  s_moduleBusCtx.snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
  s_moduleBusCtx.snapshot.healthMask = MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
  s_moduleBusCtx.snapshot.staleMask = MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
  s_moduleBusCtx.snapshot.detectorInputs = 0x0002U;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  TEST_ASSERT_TRUE(
    IntersectionEngineGetPhaseStatusGroup(&s_engine, 1U, &statusGroup));
  TEST_ASSERT_BITS_LOW(0x01U, statusGroup.vehCalls);
}

void test_controller_rejects_protocol_mismatch_for_noncritical_source(void)
{
  IntersectionPhaseStatusGroup_t statusGroup;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));

  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion =
    (uint8_t) (MODULE_BUS_PROTOCOL_VERSION + 1U);
  s_moduleBusCtx.snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
  s_moduleBusCtx.snapshot.healthMask = MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
  s_moduleBusCtx.snapshot.detectorInputs = 0x0002U;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  TEST_ASSERT_TRUE(
    IntersectionEngineGetPhaseStatusGroup(&s_engine, 1U, &statusGroup));
  TEST_ASSERT_BITS_LOW(0x01U, statusGroup.vehCalls);
  TEST_ASSERT_EQUAL_UINT8(0U, s_mmuCtx.forceAllRed);
}

void test_controller_forces_flash_on_protocol_mismatch_for_mmu_source(void)
{
  const IntersectionRuntime_t *runtime;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));

  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion =
    (uint8_t) (MODULE_BUS_PROTOCOL_VERSION + 1U);
  s_moduleBusCtx.snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS;
  s_moduleBusCtx.snapshot.healthMask = MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS;
  s_moduleBusCtx.snapshot.mmuStatus = 0U;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_NOT_NULL(runtime);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FLASH, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->mmuFlashActive);
  TEST_ASSERT_EQUAL_UINT8(1U, s_mmuCtx.forceAllRed);
}

void test_controller_forces_flash_on_epoch_mismatch_for_load_switch_source(void)
{
  const IntersectionRuntime_t *runtime;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_GREEN,
                        s_outputCtx.lastImage.channels[0]);

  IntersectionControllerSetExpectedModuleBusConfigEpoch(&s_controller,
                                                        0x1234U);
  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  s_moduleBusCtx.snapshot.configEpoch = 0x1235U;
  s_moduleBusCtx.snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  s_moduleBusCtx.snapshot.healthMask = MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  s_moduleBusCtx.snapshot.loadSwitchReds = 0x0000U;
  s_moduleBusCtx.snapshot.loadSwitchYellows = 0x0000U;
  s_moduleBusCtx.snapshot.loadSwitchGreens = 0x0001U;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_NOT_NULL(runtime);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FLASH, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->mmuFlashActive);
  TEST_ASSERT_EQUAL_UINT8(1U, s_mmuCtx.forceAllRed);
}

void test_controller_accepts_matching_epoch_snapshot(void)
{
  IntersectionPhaseStatusGroup_t statusGroup;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));

  IntersectionControllerSetExpectedModuleBusConfigEpoch(&s_controller,
                                                        0x1234U);
  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  s_moduleBusCtx.snapshot.configEpoch = 0x1234U;
  s_moduleBusCtx.snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
  s_moduleBusCtx.snapshot.healthMask = MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
  s_moduleBusCtx.snapshot.detectorInputs = 0x0002U;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  TEST_ASSERT_TRUE(
    IntersectionEngineGetPhaseStatusGroup(&s_engine, 1U, &statusGroup));
  TEST_ASSERT_BITS_HIGH(0x01U, statusGroup.vehCalls);
  TEST_ASSERT_EQUAL_UINT8(0U, s_mmuCtx.forceAllRed);
}

void test_controller_step_fails_when_mmu_command_fails(void)
{
  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  s_moduleBusCtx.snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS;
  s_moduleBusCtx.snapshot.healthMask = MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS;
  s_moduleBusCtx.snapshot.mmuStatus = MODULE_BUS_MMU_STATUS_FORCE_ALL_RED;
  s_mmuCtx.setForceAllRedResult = 0U;

  TEST_ASSERT_FALSE(IntersectionControllerStep(&s_controller));
  TEST_ASSERT_FALSE(s_controller.lastStepOk);
}

void test_controller_ignores_detector_source_with_context_fault_mask(void)
{
  IntersectionPhaseStatusGroup_t statusGroup;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));

  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  s_moduleBusCtx.snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
  s_moduleBusCtx.snapshot.healthMask = MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
  s_moduleBusCtx.snapshot.contextFaultMask = MODULE_BUS_SNAPSHOT_VALID_DETECTORS;
  s_moduleBusCtx.snapshot.detectorInputs = 0x0002U;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  TEST_ASSERT_TRUE(
    IntersectionEngineGetPhaseStatusGroup(&s_engine, 1U, &statusGroup));
  TEST_ASSERT_BITS_LOW(0x01U, statusGroup.vehCalls);
  TEST_ASSERT_EQUAL_UINT8(0U, s_mmuCtx.forceAllRed);
}

void test_controller_forces_flash_on_mmu_context_fault_mask(void)
{
  const IntersectionRuntime_t *runtime;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));

  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  s_moduleBusCtx.snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS;
  s_moduleBusCtx.snapshot.healthMask = MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS;
  s_moduleBusCtx.snapshot.contextFaultMask = MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS;
  s_moduleBusCtx.snapshot.mmuStatus = 0U;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_NOT_NULL(runtime);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FLASH, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->mmuFlashActive);
  TEST_ASSERT_EQUAL_UINT8(1U, s_mmuCtx.forceAllRed);
}

void test_controller_forces_flash_on_load_switch_sequence_fault_mask(void)
{
  const IntersectionRuntime_t *runtime;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_GREEN,
                        s_outputCtx.lastImage.channels[0]);

  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  s_moduleBusCtx.snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  s_moduleBusCtx.snapshot.healthMask = MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  s_moduleBusCtx.snapshot.sequenceFaultMask =
    MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  s_moduleBusCtx.snapshot.loadSwitchReds = 0x0000U;
  s_moduleBusCtx.snapshot.loadSwitchYellows = 0x0000U;
  s_moduleBusCtx.snapshot.loadSwitchGreens = 0x0001U;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_NOT_NULL(runtime);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FLASH, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->mmuFlashActive);
  TEST_ASSERT_EQUAL_UINT8(1U, s_mmuCtx.forceAllRed);
}

void test_controller_forces_flash_on_load_switch_feedback_mismatch(void)
{
  const IntersectionRuntime_t *runtime;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_GREEN,
                        s_outputCtx.lastImage.channels[0]);

  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  s_moduleBusCtx.snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  s_moduleBusCtx.snapshot.healthMask = MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  s_moduleBusCtx.snapshot.loadSwitchReds = 0x0001U;
  s_moduleBusCtx.snapshot.loadSwitchYellows = 0x0000U;
  s_moduleBusCtx.snapshot.loadSwitchGreens = 0x0000U;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_NOT_NULL(runtime);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FLASH, runtime->mode);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->mmuFlashActive);
  TEST_ASSERT_EQUAL_UINT8(1U, s_mmuCtx.forceAllRed);
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_RED,
                        s_outputCtx.lastImage.channels[0]);
}

void test_controller_applies_local_dimming_input_to_output_image(void)
{
  IntersectionConfig_t config;

  config = MakeControllerConfig();
  config.channels[0].dimMask = 0x09U;
  config.timebase.actions[0].auxiliaryFunction =
    INTERSECTION_TIMEBASE_AUX_FUNCTION_DIMMING;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetActionPlanControl(&s_engine, 1U));

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  TEST_ASSERT_EQUAL_UINT8(0U, s_outputCtx.lastImage.channelDimmed[0]);

  s_unitInputCtx.dimmingInputActive = 1U;
  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  TEST_ASSERT_EQUAL_UINT8(1U, s_outputCtx.lastImage.channelDimmed[0]);
  TEST_ASSERT_EQUAL_UINT8(1U,
                          s_outputCtx.lastImage.channelDimAlternateHalfCycle[0]);
}

void test_controller_applies_interconnect_priority_over_timebase(void)
{
  IntersectionConfig_t config;
  const IntersectionRuntime_t *runtime;

  config = MakeControllerConfig();
  config.channels[0].flashMask = 0x04U;
  config.timebase.actions[0].pattern = 254U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetActionPlanControl(&s_engine, 1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetUnitControl(&s_engine, 0x40U));

  s_unitInputCtx.interconnectCommand = 255U;
  s_unitInputCtx.interconnectInputsValid = 1U;

  TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));
  runtime = IntersectionEngineGetRuntime(&s_engine);

  TEST_ASSERT_NOT_NULL(runtime);
  TEST_ASSERT_EQUAL_UINT8(7U, runtime->unitControlStatus);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_CONTROL_MODE_FLASH, runtime->mode);
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_FLASH_RED,
                        s_outputCtx.lastImage.channels[0]);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_controller_maps_physical_inputs_to_configured_phase_sources);
  RUN_TEST(test_controller_forces_flash_and_all_red_from_mmu_snapshot);
  RUN_TEST(test_controller_clears_stale_detector_source);
  RUN_TEST(test_controller_rejects_protocol_mismatch_for_noncritical_source);
  RUN_TEST(test_controller_forces_flash_on_protocol_mismatch_for_mmu_source);
  RUN_TEST(test_controller_forces_flash_on_epoch_mismatch_for_load_switch_source);
  RUN_TEST(test_controller_accepts_matching_epoch_snapshot);
  RUN_TEST(test_controller_step_fails_when_mmu_command_fails);
  RUN_TEST(test_controller_ignores_detector_source_with_context_fault_mask);
  RUN_TEST(test_controller_forces_flash_on_mmu_context_fault_mask);
  RUN_TEST(test_controller_forces_flash_on_load_switch_sequence_fault_mask);
  RUN_TEST(test_controller_forces_flash_on_load_switch_feedback_mismatch);
  RUN_TEST(test_controller_applies_local_dimming_input_to_output_image);
  RUN_TEST(test_controller_applies_interconnect_priority_over_timebase);

  return UNITY_END();
}
