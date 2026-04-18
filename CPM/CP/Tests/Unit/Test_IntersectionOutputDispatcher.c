/*
 * Tests/Unit/Test_IntersectionOutputDispatcher.c
 *
 * Unit tests for dispatching the engine output image through MMU and output
 * driver ports.
 */
#include "unity.h"

#include "Domain/Intersection/IntersectionOutputDispatcher.h"

typedef struct
{
  OutputDriverImage_t lastImage;
  uint8_t applyCallCount;
  uint8_t succeed;
} FakeOutputDriverCtx_t;

typedef struct
{
  uint8_t forceAllRed;
} FakeMmuCtx_t;

static IntersectionEngine_t s_engine;
static IntersectionOutputDispatcher_t s_dispatcher;
static FakeOutputDriverCtx_t s_outputCtx;
static FakeMmuCtx_t s_mmuCtx;
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

static uint8_t FakeOutputApply(void *ctx, const OutputDriverImage_t *image)
{
  FakeOutputDriverCtx_t *outputCtx = (FakeOutputDriverCtx_t *) ctx;

  outputCtx->lastImage = *image;
  outputCtx->applyCallCount++;

  return outputCtx->succeed;
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
       channelIndex++)
  {
    approved->channels[channelIndex] = OUTPUT_DRIVER_ASPECT_RED;
    approved->channelDimmed[channelIndex] = 0U;
    approved->channelDimAlternateHalfCycle[channelIndex] = 0U;
  }

  return 1U;
}

static IntersectionConfig_t MakeDispatcherConfig(void)
{
  IntersectionConfig_t config;
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
  ReplicateSequencePlansFromBase(&config);

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

  config.channels[0].controlSource = 1U;
  config.channels[0].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;

  return config;
}

static IntersectionConfig_t MakeTransitDispatcherConfig(void)
{
  IntersectionConfig_t config = MakeDispatcherConfig();
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
  config.channels[0].controlType = INTERSECTION_CHANNEL_CONTROL_TYPE_QUEUE_JUMP;
  config.overlaps[0].type = INTERSECTION_OVERLAP_TYPE_TRANSIT_2;
  config.overlaps[0].includedPhases.length = 1U;
  config.overlaps[0].includedPhases.values[0] = 1U;

  return config;
}

void setUp(void)
{
  IntersectionConfig_t config = MakeDispatcherConfig();

  IntersectionEngineInit(&s_engine);
  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);

  s_outputCtx.applyCallCount = 0U;
  s_outputCtx.succeed = 1U;
  s_mmuCtx.forceAllRed = 0U;

  s_outputPort.ctx = &s_outputCtx;
  s_outputPort.Apply = FakeOutputApply;
  s_mmuPort.ctx = &s_mmuCtx;
  s_mmuPort.FilterOutputImage = FakeMmuFilter;

  IntersectionOutputDispatcherInit(&s_dispatcher);
  IntersectionOutputDispatcherBind(&s_dispatcher,
                                   &s_engine,
                                   &s_mmuPort,
                                   &s_outputPort);
}

void tearDown(void)
{
}

void test_dispatcher_applies_engine_output_image_through_output_port(void)
{
  OutputDriverImage_t requestedImage;
  OutputDriverImage_t appliedImage;

  TEST_ASSERT_TRUE(IntersectionOutputDispatcherDispatch(&s_dispatcher));
  TEST_ASSERT_EQUAL_UINT8(1U, s_outputCtx.applyCallCount);
  TEST_ASSERT_TRUE(IntersectionOutputDispatcherGetLastRequestedImage(
                     &s_dispatcher,
                     &
                     requestedImage));
  TEST_ASSERT_TRUE(IntersectionOutputDispatcherGetLastAppliedImage(
                     &s_dispatcher,
                     &appliedImage));
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_GREEN, requestedImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_GREEN, appliedImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_GREEN,
                        s_outputCtx.lastImage.channels[0]);
}

void test_dispatcher_allows_mmu_to_force_all_red_output(void)
{
  OutputDriverImage_t requestedImage;
  OutputDriverImage_t appliedImage;

  s_mmuCtx.forceAllRed = 1U;

  TEST_ASSERT_TRUE(IntersectionOutputDispatcherDispatch(&s_dispatcher));
  TEST_ASSERT_TRUE(IntersectionOutputDispatcherGetLastRequestedImage(
                     &s_dispatcher,
                     &
                     requestedImage));
  TEST_ASSERT_TRUE(IntersectionOutputDispatcherGetLastAppliedImage(
                     &s_dispatcher,
                     &appliedImage));
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_GREEN, requestedImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_RED, appliedImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_RED,
                        s_outputCtx.lastImage.channels[0]);
}

void test_dispatcher_preserves_channel_dimming_flags(void)
{
  OutputDriverImage_t appliedImage;
  IntersectionConfig_t config = MakeDispatcherConfig();

  config.channels[0].dimMask = 0x09U;
  config.timebase.actions[0].auxiliaryFunction =
    INTERSECTION_TIMEBASE_AUX_FUNCTION_DIMMING;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  TEST_ASSERT_TRUE(IntersectionEngineSetUnitControl(&s_engine, 0x80U));
  TEST_ASSERT_TRUE(IntersectionEngineSetActionPlanControl(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);

  TEST_ASSERT_TRUE(IntersectionOutputDispatcherDispatch(&s_dispatcher));
  TEST_ASSERT_TRUE(IntersectionOutputDispatcherGetLastAppliedImage(
                     &s_dispatcher,
                     &appliedImage));
  TEST_ASSERT_EQUAL_UINT8(1U, appliedImage.channelDimmed[0]);
  TEST_ASSERT_EQUAL_UINT8(1U, appliedImage.channelDimAlternateHalfCycle[0]);
  TEST_ASSERT_EQUAL_UINT8(1U, s_outputCtx.lastImage.channelDimmed[0]);
  TEST_ASSERT_EQUAL_UINT8(1U,
                          s_outputCtx.lastImage.channelDimAlternateHalfCycle[0]);
}

void test_dispatcher_converts_flash_green_output(void)
{
  OutputDriverImage_t appliedImage;
  IntersectionConfig_t config = MakeTransitDispatcherConfig();
  const IntersectionRuntime_t *runtime;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  IntersectionEngineTick(&s_engine);
  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualControlTimeout(&s_engine, 3U));

  for (uint32_t tickIndex = 0U; tickIndex < 100U; ++tickIndex)
  {
    IntersectionEngineTick(&s_engine);
  }

  TEST_ASSERT_TRUE(
    IntersectionEngineSetRemoteManualIntervalAdvance(&s_engine, 1U));
  IntersectionEngineTick(&s_engine);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_EQUAL_INT(INTERSECTION_PHASE_INTERVAL_YELLOW,
                        runtime->phases[0].interval);

  TEST_ASSERT_TRUE(IntersectionOutputDispatcherDispatch(&s_dispatcher));
  TEST_ASSERT_TRUE(IntersectionOutputDispatcherGetLastAppliedImage(
                     &s_dispatcher,
                     &appliedImage));
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_FLASH_GREEN,
                        appliedImage.channels[0]);
  TEST_ASSERT_EQUAL_INT(OUTPUT_DRIVER_ASPECT_FLASH_GREEN,
                        s_outputCtx.lastImage.channels[0]);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_dispatcher_applies_engine_output_image_through_output_port);
  RUN_TEST(test_dispatcher_allows_mmu_to_force_all_red_output);
  RUN_TEST(test_dispatcher_preserves_channel_dimming_flags);
  RUN_TEST(test_dispatcher_converts_flash_green_output);

  return UNITY_END();
}
