/*
 * Tests/Unit/Test_IntersectionActivationService.c
 *
 * Unit tests for staged online activation of committed controller plans.
 */
#include "unity.h"

#include "Domain/Intersection/IntersectionActivationService.h"
#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionOutputDispatcher.h"

#include <string.h>

typedef struct
{
  ModuleBusSnapshot_t snapshot;
  uint8_t available;
  uint16_t configEpoch;
  uint8_t setConfigEpochCount;
} FakeModuleBusCtx_t;

typedef struct
{
  OutputDriverImage_t lastImage;
  uint8_t applyCount;
  uint8_t succeed;
  uint16_t configEpoch;
  uint8_t setConfigEpochCount;
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
static IntersectionActivationService_t s_activationService;
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

static uint8_t FakeModuleBusSetConfigEpoch(void *ctx, uint16_t configEpoch)
{
  FakeModuleBusCtx_t *moduleBusCtx = (FakeModuleBusCtx_t *) ctx;

  if (moduleBusCtx == NULL)
  {
    return 0U;
  }

  moduleBusCtx->configEpoch = configEpoch;
  moduleBusCtx->snapshot.configEpoch = configEpoch;
  moduleBusCtx->setConfigEpochCount++;

  return 1U;
}

static uint8_t FakeOutputApply(void *ctx, const OutputDriverImage_t *image)
{
  FakeOutputCtx_t *outputCtx = (FakeOutputCtx_t *) ctx;

  if ((outputCtx == NULL) || (image == NULL))
  {
    return 0U;
  }

  outputCtx->lastImage = *image;
  outputCtx->applyCount++;

  return outputCtx->succeed;
}

static uint8_t FakeOutputSetConfigEpoch(void *ctx, uint16_t configEpoch)
{
  FakeOutputCtx_t *outputCtx = (FakeOutputCtx_t *) ctx;

  if (outputCtx == NULL)
  {
    return 0U;
  }

  outputCtx->configEpoch = configEpoch;
  outputCtx->setConfigEpochCount++;

  return 1U;
}

static uint8_t FakeMmuSetForceAllRed(void *ctx, uint8_t forceAllRed)
{
  FakeMmuCtx_t *mmuCtx = (FakeMmuCtx_t *) ctx;

  if (mmuCtx == NULL)
  {
    return 0U;
  }

  mmuCtx->forceAllRed = (uint8_t) (forceAllRed != 0U);

  return mmuCtx->setForceAllRedResult;
}

static uint8_t FakeMmuFilter(void *ctx,
                             const OutputDriverImage_t *requested,
                             OutputDriverImage_t *approved)
{
  FakeMmuCtx_t *mmuCtx = (FakeMmuCtx_t *) ctx;
  uint8_t channelIndex;

  if ((requested == NULL) || (approved == NULL))
  {
    return 0U;
  }

  *approved = *requested;

  if ((mmuCtx == NULL) || (mmuCtx->forceAllRed == 0U))
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

  return (unitInputCtx != NULL) ? unitInputCtx->dimmingInputActive : 0U;
}

static uint8_t FakeGetInterconnectCommand(void *ctx)
{
  FakeUnitInputCtx_t *unitInputCtx = (FakeUnitInputCtx_t *) ctx;

  return (unitInputCtx != NULL) ? unitInputCtx->interconnectCommand : 0U;
}

static uint8_t FakeGetInterconnectInputsValid(void *ctx)
{
  FakeUnitInputCtx_t *unitInputCtx = (FakeUnitInputCtx_t *) ctx;

  return (unitInputCtx != NULL) ? unitInputCtx->interconnectInputsValid : 1U;
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

  config.vehicleDetectors[0].callPhase = 2U;
  config.vehicleDetectors[1].callPhase = 1U;
  config.pedestrianDetectors[0].callPhase = 1U;
  config.pedestrianDetectors[1].callPhase = 2U;

  for (detectorIndex = config.phaseCount;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       ++detectorIndex)
  {
    config.vehicleDetectors[detectorIndex].callPhase = 0U;
  }

  for (detectorIndex = config.phaseCount;
       detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       ++detectorIndex)
  {
    config.pedestrianDetectors[detectorIndex].callPhase = 0U;
  }

  config.channels[0].controlSource = 1U;
  config.channels[0].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  config.channels[1].controlSource = 3U;
  config.channels[1].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;

  return config;
}

void setUp(void)
{
  IntersectionConfig_t liveConfig = MakeControllerConfig();

  IntersectionEngineInit(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &liveConfig));
  IntersectionActivationServiceInit(&s_activationService);
  TEST_ASSERT_TRUE(IntersectionActivationServiceLoadCommittedLivePlan(
    &s_activationService,
    &liveConfig,
    1U));

  memset(&s_moduleBusCtx, 0, sizeof(s_moduleBusCtx));
  memset(&s_outputCtx, 0, sizeof(s_outputCtx));
  memset(&s_mmuCtx, 0, sizeof(s_mmuCtx));
  memset(&s_unitInputCtx, 0, sizeof(s_unitInputCtx));
  s_outputCtx.succeed = 1U;
  s_outputCtx.configEpoch = 1U;
  s_moduleBusCtx.configEpoch = 1U;
  s_mmuCtx.setForceAllRedResult = 1U;
  s_unitInputCtx.interconnectInputsValid = 1U;

  s_moduleBusPort.ctx = &s_moduleBusCtx;
  s_moduleBusPort.ReadSnapshot = FakeModuleBusRead;
  s_moduleBusPort.SetConfigEpoch = FakeModuleBusSetConfigEpoch;
  s_moduleBusPort.CommandDetectorReset = NULL;
  s_unitInputPort.ctx = &s_unitInputCtx;
  s_unitInputPort.GetDimmingInputActive = FakeGetDimmingInputActive;
  s_unitInputPort.GetInterconnectCommand = FakeGetInterconnectCommand;
  s_unitInputPort.GetInterconnectInputsValid = FakeGetInterconnectInputsValid;
  s_outputPort.ctx = &s_outputCtx;
  s_outputPort.Apply = FakeOutputApply;
  s_outputPort.SetConfigEpoch = FakeOutputSetConfigEpoch;
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
                             &s_activationService,
                             &s_moduleBusPort,
                             &s_unitInputPort,
                             &s_mmuPort);
  IntersectionControllerSetExpectedModuleBusConfigEpoch(&s_controller, 1U);
}

void tearDown(void)
{
}

void test_activation_service_load_committed_live_plan_sets_live_status(void)
{
  IntersectionActivationStatus_t status;

  TEST_ASSERT_TRUE(
    IntersectionActivationServiceGetStatus(&s_activationService, &status));
  TEST_ASSERT_EQUAL_UINT16(1U, status.committedSetId);
  TEST_ASSERT_EQUAL_UINT16(1U, status.liveSetId);
  TEST_ASSERT_EQUAL_UINT16(0U, status.pendingSetId);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_ACTIVATION_STATE_IDLE, status.state);
}

void test_activation_service_rejects_unsupported_runtime_config(void)
{
  IntersectionConfig_t unsupportedConfig = MakeControllerConfig();
  IntersectionActivationStatus_t status;

  unsupportedConfig.channels[0].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_QUEUE_JUMP;

  TEST_ASSERT_FALSE(IntersectionActivationServiceStageCommittedConfig(
    &s_activationService,
    &unsupportedConfig,
    2U));
  TEST_ASSERT_TRUE(
    IntersectionActivationServiceGetStatus(&s_activationService, &status));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_ACTIVATION_STATE_FAILED, status.state);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_ACTIVATION_ERROR_UNSUPPORTED_RUNTIME,
                        status.error);
}

void test_activation_service_accepts_preempt_sequence_number_within_max_sequences(
  void)
{
  IntersectionConfig_t stagedConfig = MakeControllerConfig();
  IntersectionActivationStatus_t status;

  stagedConfig.preempts[0].control = 0x10U;
  stagedConfig.preempts[0].sequenceNumber = 1U;
  stagedConfig.preempts[0].exitType =
    (uint8_t) INTERSECTION_PREEMPT_EXIT_TYPE_EXIT_COORD;

  TEST_ASSERT_TRUE(IntersectionActivationServiceStageCommittedConfig(
    &s_activationService,
    &stagedConfig,
    2U));
  TEST_ASSERT_TRUE(
    IntersectionActivationServiceGetStatus(&s_activationService, &status));
  TEST_ASSERT_EQUAL_INT(INTERSECTION_ACTIVATION_STATE_STAGED, status.state);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_ACTIVATION_ERROR_NONE, status.error);
  TEST_ASSERT_EQUAL_UINT16(2U, status.pendingSetId);
}

void test_controller_soft_reload_switches_live_plan_and_epochs(void)
{
  IntersectionConfig_t stagedConfig = MakeControllerConfig();
  IntersectionActivationStatus_t status;
  const IntersectionConfig_t *liveConfig;
  uint8_t stepCount;

  stagedConfig.phases[0].maxGreenDs =
    (uint16_t) (stagedConfig.phases[0].maxGreenDs + 50U);

  s_moduleBusCtx.available = 1U;
  s_moduleBusCtx.snapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  s_moduleBusCtx.snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  s_moduleBusCtx.snapshot.healthMask = MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH;
  s_moduleBusCtx.snapshot.configEpoch = 1U;
  s_moduleBusCtx.snapshot.loadSwitchReds = 0x0003U;
  s_moduleBusCtx.snapshot.loadSwitchYellows = 0U;
  s_moduleBusCtx.snapshot.loadSwitchGreens = 0U;

  TEST_ASSERT_TRUE(IntersectionActivationServiceStageCommittedConfig(
    &s_activationService,
    &stagedConfig,
    2U));

  for (stepCount = 0U; stepCount < 40U; ++stepCount)
  {
    TEST_ASSERT_TRUE(IntersectionControllerStep(&s_controller));

    TEST_ASSERT_TRUE(
      IntersectionActivationServiceGetStatus(&s_activationService, &status));

    if ((status.state == INTERSECTION_ACTIVATION_STATE_IDLE)
        && (status.liveSetId == 2U))
    {
      break;
    }
  }

  TEST_ASSERT_TRUE(stepCount < 40U);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_ACTIVATION_STATE_IDLE, status.state);
  TEST_ASSERT_EQUAL_UINT16(2U, status.committedSetId);
  TEST_ASSERT_EQUAL_UINT16(2U, status.liveSetId);
  TEST_ASSERT_EQUAL_UINT16(0U, status.pendingSetId);
  TEST_ASSERT_EQUAL_UINT16(2U, s_controller.expectedModuleBusConfigEpoch);
  TEST_ASSERT_EQUAL_UINT16(2U, s_moduleBusCtx.configEpoch);
  TEST_ASSERT_EQUAL_UINT16(2U, s_outputCtx.configEpoch);
  TEST_ASSERT_EQUAL_UINT8(1U, s_moduleBusCtx.setConfigEpochCount);
  TEST_ASSERT_EQUAL_UINT8(1U, s_outputCtx.setConfigEpochCount);
  TEST_ASSERT_EQUAL_UINT8(0U, s_mmuCtx.forceAllRed);

  liveConfig = IntersectionEngineGetConfig(&s_engine);
  TEST_ASSERT_NOT_NULL(liveConfig);
  TEST_ASSERT_EQUAL_UINT16(stagedConfig.phases[0].maxGreenDs,
                           liveConfig->phases[0].maxGreenDs);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_activation_service_load_committed_live_plan_sets_live_status);
  RUN_TEST(test_activation_service_rejects_unsupported_runtime_config);
  RUN_TEST(
    test_activation_service_accepts_preempt_sequence_number_within_max_sequences);
  RUN_TEST(test_controller_soft_reload_switches_live_plan_and_epochs);

  return UNITY_END();
}
