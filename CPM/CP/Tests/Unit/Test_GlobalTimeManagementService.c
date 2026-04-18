/*
 * Tests/Unit/Test_GlobalTimeManagementService.c
 *
 * Unit tests for canonical 1201 time management and 1202 timebase linkage.
 */
#include "unity.h"

#include "Domain/Intersection/GlobalTimeManagementService.h"
#include "MockRTCAdapter.h"

static IntersectionEngine_t s_engine;
static GlobalTimeManagementService_t s_service;
static MockRTCAdapterCtx_t s_rtcCtx;
static IRealtimeClockPort_t s_rtcPort;

static void SetRtc(uint8_t century,
                   uint8_t year,
                   uint8_t month,
                   uint8_t date,
                   uint8_t weekDay,
                   uint8_t hour,
                   uint8_t minute,
                   uint8_t second)
{
  RtcSnapshot_t snapshot = {
    century, year, month, date, weekDay, hour, minute, second
  };

  TEST_ASSERT_TRUE(RealtimeClockWriteSnapshot(&s_rtcPort, &snapshot));
}

static IntersectionConfig_t MakeBaseConfig(void)
{
  IntersectionConfig_t config;

  IntersectionConfigInitDefaults(&config);
  config.globalTimeManagement.globalDaylightSaving = 2U;
  config.globalTimeManagement.controllerStandardTimeZoneSeconds = 0;

  return config;
}

void setUp(void)
{
  IntersectionEngineInit(&s_engine);
  GlobalTimeManagementServiceInit(&s_service);
  MockRTCAdapterInit(&s_rtcCtx);
  s_rtcPort = MockRTCAdapterCreatePort(&s_rtcCtx);
  SetRtc(21U, 26U, 4U, 18U, 7U, 9U, 20U, 0U);
}

void tearDown(void)
{
}

void test_service_selects_timebase_action_from_current_day_plan_and_syncs_pattern(
  void)
{
  IntersectionConfig_t config = MakeBaseConfig();
  const IntersectionRuntime_t *runtime;
  uint8_t actionPlanControl = 0U;

  config.coordination.patterns[0].cycleTimeSeconds = 120U;
  config.timebase.patternSyncMinutes = 65535U;
  config.timebase.actions[0].pattern = 1U;
  config.globalTimeManagement.schedules[0].monthMask = (uint16_t) (1U << 4U);
  config.globalTimeManagement.schedules[0].dayMask = (uint8_t) (1U << 7U);
  config.globalTimeManagement.schedules[0].dateMask = (uint32_t) (1UL << 18U);
  config.globalTimeManagement.schedules[0].dayPlanNumber = 1U;
  config.globalTimeManagement.dayPlans[0][0].hour = 8U;
  config.globalTimeManagement.dayPlans[0][0].minute = 15U;
  config.globalTimeManagement.dayPlans[0][0].actionNumber = 1U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  GlobalTimeManagementServiceBind(&s_service, &s_engine, &s_rtcPort);

  GlobalTimeManagementServiceStep(&s_service);

  TEST_ASSERT_TRUE(IntersectionEngineGetActionPlanControl(&s_engine,
                                                          &actionPlanControl));
  TEST_ASSERT_EQUAL_UINT8(1U, actionPlanControl);
  runtime = IntersectionEngineGetRuntime(&s_engine);
  TEST_ASSERT_NOT_NULL(runtime);
  TEST_ASSERT_EQUAL_UINT8(1U, runtime->actionPlanControl);
  TEST_ASSERT_EQUAL_UINT8(60U, runtime->systemSyncControlSeconds);
}

void test_service_sets_global_time_using_standard_time_zone(void)
{
  IntersectionConfig_t config = MakeBaseConfig();
  uint32_t controllerLocalTime = 0U;
  uint32_t globalTime = 0U;

  config.globalTimeManagement.controllerStandardTimeZoneSeconds = 3600;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  GlobalTimeManagementServiceBind(&s_service, &s_engine, &s_rtcPort);

  TEST_ASSERT_TRUE(GlobalTimeManagementServiceSetGlobalTime(&s_service, 3600U));
  TEST_ASSERT_TRUE(GlobalTimeManagementServiceGetControllerLocalTime(
    &s_service,
    &controllerLocalTime));
  TEST_ASSERT_TRUE(GlobalTimeManagementServiceGetGlobalTime(&s_service,
                                                            &globalTime));

  TEST_ASSERT_EQUAL_UINT32(7200U, controllerLocalTime);
  TEST_ASSERT_EQUAL_UINT32(3600U, globalTime);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_service_selects_timebase_action_from_current_day_plan_and_syncs_pattern);
  RUN_TEST(test_service_sets_global_time_using_standard_time_zone);
  return UNITY_END();
}
