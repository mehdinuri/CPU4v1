/*
 * Tests/Unit/Test_DetectorReportService.c
 *
 * Unit tests for detector and pedestrian sample reporting driven by the
 * controller runtime and RTC wall clock.
 */
#include "unity.h"

#include "Domain/Intersection/DetectorReportService.h"
#include "MockRTCAdapter.h"

#include <string.h>

static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
static DetectorReportService_t s_service;
static MockRTCAdapterCtx_t s_rtcCtx;
static IRealtimeClockPort_t s_rtcPort;

static void SetRtc(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
  RtcSnapshot_t snapshot = { 21U, 26U, 4U, 17U, 5U, hours, minutes, seconds };

  TEST_ASSERT_TRUE(RealtimeClockWriteSnapshot(&s_rtcPort, &snapshot));
}

static void AdvanceTicks(uint16_t tickCount)
{
  uint16_t tickIndex;

  for (tickIndex = 0U; tickIndex < tickCount; ++tickIndex)
  {
    IntersectionEngineTick(&s_engine);
    DetectorReportServiceStep(&s_service);
  }
}

static IntersectionConfig_t MakeVehicleReportConfig(void)
{
  IntersectionConfig_t config;
  uint8_t detectorIndex;

  IntersectionConfigInitDefaults(&config);
  config.detectorReports.volumeOccupancyPeriodSeconds = 1U;

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       ++detectorIndex)
  {
    config.vehicleDetectors[detectorIndex].callPhase = 0U;
    config.vehicleDetectors[detectorIndex].options = 0U;
    config.vehicleDetectors[detectorIndex].options2 = 0U;
  }

  config.vehicleDetectors[0].options = (uint8_t) (VEHICLE_DETECTOR_OPTIONS_VOLUME
                                                   | VEHICLE_DETECTOR_OPTIONS_OCCUPANCY);

  return config;
}

static IntersectionConfig_t MakePedestrianReportConfig(void)
{
  IntersectionConfig_t config;
  uint8_t detectorIndex;

  IntersectionConfigInitDefaults(&config);
  config.detectorReports.volumeOccupancyPeriodSeconds = 1U;
  config.detectorReports.pedestrianDetectorPeriodSeconds = 65534U;
  config.phases[0].startup = (uint8_t) INTERSECTION_PHASE_STARTUP_GREEN_WALK;

  for (detectorIndex = 0U; detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       ++detectorIndex)
  {
    config.pedestrianDetectors[detectorIndex].callPhase = 0U;
    config.pedestrianDetectors[detectorIndex].options = 0U;
  }

  config.pedestrianDetectors[0].callPhase = 1U;
  config.pedestrianDetectors[0].options = PED_DETECTOR_OPTIONS_PRESENCE;

  return config;
}

void setUp(void)
{
  IntersectionEngineInit(&s_engine);
  IntersectionControllerInit(&s_controller);
  DetectorReportServiceInit(&s_service);
  MockRTCAdapterInit(&s_rtcCtx);
  s_rtcPort = MockRTCAdapterCreatePort(&s_rtcCtx);
  SetRtc(12U, 0U, 0U);
}

void tearDown(void)
{
}

void test_vehicle_report_counts_volume_and_occupancy_over_sample_period(void)
{
  IntersectionConfig_t config = MakeVehicleReportConfig();
  DetectorReportVehicleSample_t sample;
  uint8_t sequence = 0U;
  uint8_t activeCount = 0U;
  uint16_t durationSeconds = 0U;
  uint32_t sampleTimeSeconds = 0U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  DetectorReportServiceBind(&s_service, &s_engine, &s_controller, &s_rtcPort);

  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 1U, 1U));
  AdvanceTicks(30U);
  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 1U, 0U));
  SetRtc(12U, 0U, 1U);
  AdvanceTicks(70U);

  TEST_ASSERT_TRUE(DetectorReportServiceGetVehicleSequence(&s_service, &sequence));
  TEST_ASSERT_TRUE(DetectorReportServiceGetVehicleActiveCount(&s_service,
                                                              &activeCount));
  TEST_ASSERT_TRUE(DetectorReportServiceGetVehicleSampleDurationSeconds(
                     &s_service,
                     &durationSeconds));
  TEST_ASSERT_TRUE(DetectorReportServiceGetVehicleSampleTimeSeconds(
                     &s_service,
                     &sampleTimeSeconds));
  TEST_ASSERT_TRUE(DetectorReportServiceGetVehicleSample(&s_service, 1U,
                                                         &sample));

  TEST_ASSERT_EQUAL_UINT8(1U, sequence);
  TEST_ASSERT_EQUAL_UINT8(1U, activeCount);
  TEST_ASSERT_EQUAL_UINT16(1U, durationSeconds);
  TEST_ASSERT_EQUAL_UINT8(1U, sample.volume);
  TEST_ASSERT_EQUAL_UINT8(60U, sample.occupancy);
  TEST_ASSERT_EQUAL_UINT16(511U, sample.averageSpeed);
  TEST_ASSERT_TRUE(sampleTimeSeconds > 0U);
}

void test_vehicle_cycle_length_period_does_not_sample_in_free_mode(void)
{
  IntersectionConfig_t config = MakeVehicleReportConfig();
  uint8_t activeCount = 99U;
  uint8_t sequence = 99U;

  config.detectorReports.volumeOccupancyPeriodSeconds = 0U;
  config.detectorReports.volumeOccupancyPeriodV3Seconds = 65535U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  DetectorReportServiceBind(&s_service, &s_engine, &s_controller, &s_rtcPort);

  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 1U, 1U));
  AdvanceTicks(150U);

  TEST_ASSERT_TRUE(DetectorReportServiceGetVehicleActiveCount(&s_service,
                                                              &activeCount));
  TEST_ASSERT_TRUE(DetectorReportServiceGetVehicleSequence(&s_service, &sequence));
  TEST_ASSERT_EQUAL_UINT8(0U, activeCount);
  TEST_ASSERT_EQUAL_UINT8(0U, sequence);
}

void test_pedestrian_period_can_follow_vehicle_period_and_count_services(void)
{
  IntersectionConfig_t config = MakePedestrianReportConfig();
  DetectorReportPedestrianSample_t sample;
  uint8_t sequence = 0U;
  uint8_t activeCount = 0U;
  uint16_t durationSeconds = 0U;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  DetectorReportServiceBind(&s_service, &s_engine, &s_controller, &s_rtcPort);

  TEST_ASSERT_TRUE(IntersectionEngineSetPedestrianDetectorInput(&s_engine,
                                                                1U,
                                                                1U));
  AdvanceTicks(1U);
  TEST_ASSERT_TRUE(IntersectionEngineSetPedestrianDetectorInput(&s_engine,
                                                                1U,
                                                                0U));
  SetRtc(12U, 0U, 1U);
  AdvanceTicks(99U);

  TEST_ASSERT_TRUE(DetectorReportServiceGetPedestrianSequence(&s_service,
                                                              &sequence));
  TEST_ASSERT_TRUE(DetectorReportServiceGetPedestrianActiveCount(&s_service,
                                                                 &activeCount));
  TEST_ASSERT_TRUE(DetectorReportServiceGetPedestrianSampleDurationSeconds(
                     &s_service,
                     &durationSeconds));
  TEST_ASSERT_TRUE(DetectorReportServiceGetPedestrianSample(&s_service, 1U,
                                                            &sample));

  TEST_ASSERT_EQUAL_UINT8(1U, sequence);
  TEST_ASSERT_EQUAL_UINT8(1U, activeCount);
  TEST_ASSERT_EQUAL_UINT16(1U, durationSeconds);
  TEST_ASSERT_EQUAL_UINT8(1U, sample.volume);
  TEST_ASSERT_EQUAL_UINT8(1U, sample.actuations);
  TEST_ASSERT_EQUAL_UINT8(1U, sample.services);
}

void test_presence_pedestrian_detector_reports_without_creating_service(void)
{
  IntersectionConfig_t config = MakePedestrianReportConfig();
  DetectorReportPedestrianSample_t sample;
  uint8_t sequence = 0U;
  uint8_t activeCount = 0U;

  config.phases[0].startup = (uint8_t) INTERSECTION_PHASE_STARTUP_GREEN_NO_WALK;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  DetectorReportServiceBind(&s_service, &s_engine, &s_controller, &s_rtcPort);

  TEST_ASSERT_TRUE(IntersectionEngineSetPedestrianDetectorInput(&s_engine,
                                                                1U,
                                                                1U));
  AdvanceTicks(1U);
  TEST_ASSERT_TRUE(IntersectionEngineSetPedestrianDetectorInput(&s_engine,
                                                                1U,
                                                                0U));
  SetRtc(12U, 0U, 1U);
  AdvanceTicks(99U);

  TEST_ASSERT_TRUE(DetectorReportServiceGetPedestrianSequence(&s_service,
                                                              &sequence));
  TEST_ASSERT_TRUE(DetectorReportServiceGetPedestrianActiveCount(&s_service,
                                                                 &activeCount));
  TEST_ASSERT_TRUE(DetectorReportServiceGetPedestrianSample(&s_service, 1U,
                                                            &sample));

  TEST_ASSERT_EQUAL_UINT8(1U, sequence);
  TEST_ASSERT_EQUAL_UINT8(1U, activeCount);
  TEST_ASSERT_EQUAL_UINT8(1U, sample.volume);
  TEST_ASSERT_EQUAL_UINT8(1U, sample.actuations);
  TEST_ASSERT_EQUAL_UINT8(0U, sample.services);
}

void test_pedestrian_sample_returns_communications_fault_when_inputs_are_stale(void)
{
  IntersectionConfig_t config = MakePedestrianReportConfig();
  ModuleBusSnapshot_t snapshot;
  DetectorReportPedestrianSample_t sample;

  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(&s_engine, &config));
  DetectorReportServiceBind(&s_service, &s_engine, &s_controller, &s_rtcPort);

  memset(&snapshot, 0, sizeof(snapshot));
  snapshot.validMask = MODULE_BUS_SNAPSHOT_VALID_PEDS;
  snapshot.healthMask = 0U;
  snapshot.staleMask = MODULE_BUS_SNAPSHOT_VALID_PEDS;
  s_controller.lastSnapshot = snapshot;
  s_controller.lastSnapshotValid = 1U;

  SetRtc(12U, 0U, 1U);
  AdvanceTicks(100U);

  TEST_ASSERT_TRUE(DetectorReportServiceGetPedestrianSample(&s_service, 1U,
                                                            &sample));
  TEST_ASSERT_EQUAL_UINT8(216U, sample.actuations);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_vehicle_report_counts_volume_and_occupancy_over_sample_period);
  RUN_TEST(test_vehicle_cycle_length_period_does_not_sample_in_free_mode);
  RUN_TEST(test_pedestrian_period_can_follow_vehicle_period_and_count_services);
  RUN_TEST(
    test_presence_pedestrian_detector_reports_without_creating_service);
  RUN_TEST(
    test_pedestrian_sample_returns_communications_fault_when_inputs_are_stale);

  return UNITY_END();
}
