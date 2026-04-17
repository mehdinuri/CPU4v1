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

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_dispatcher_applies_engine_output_image_through_output_port);
  RUN_TEST(test_dispatcher_allows_mmu_to_force_all_red_output);
  RUN_TEST(test_dispatcher_preserves_channel_dimming_flags);

  return UNITY_END();
}
