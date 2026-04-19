/*
 * Tests/Unit/Test_MmiSnapshotCache.c
 *
 * Read-only MMI runtime cache over the canonical controller state.
 */
#include "unity.h"

#include <string.h>

#include "Domain/Intersection/IntersectionOutputDispatcher.h"
#include "Domain/Services/MmiSnapshotCache.h"
#include "MockRTCAdapter.h"

static ConfigurationService_t s_configurationService;
static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
static IntersectionOutputDispatcher_t s_dispatcher;
static DetectorReportService_t s_detectorReportService;
static GlobalTimeManagementService_t s_globalTimeService;
static CpMpLinkService_t s_cpMpLinkService;
static MmiSnapshotCache_t s_cache;
static MockRTCAdapterCtx_t s_rtcCtx;
static IRealtimeClockPort_t s_rtcPort;

static void ReplicateSequencePlansFromBase(IntersectionConfig_t *config)
{
  uint8_t sequenceIndex;
  uint8_t ringIndex;

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

static IntersectionConfig_t MakeConfig(void)
{
  IntersectionConfig_t config;
  uint8_t phaseIndex;
  uint8_t detectorIndex;

  IntersectionConfigInitDefaults(&config);
  config.phaseCount = 4U;
  config.ringCount = 2U;
  config.barrierCount = 2U;
  config.globalTimeManagement.globalDaylightSaving = 2U;
  config.globalTimeManagement.controllerStandardTimeZoneSeconds = 3600;
  config.detectorReports.volumeOccupancyPeriodSeconds = 60U;
  config.detectorReports.pedestrianDetectorPeriodSeconds = 60U;

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; phaseIndex++)
  {
    config.phases[phaseIndex].phaseOptions = PHASE_OPTIONS_ENABLED;
    config.phases[phaseIndex].ring = (phaseIndex < 2U) ? 0U : 1U;
    config.phases[phaseIndex].minGreenDs = 50U;
    config.phases[phaseIndex].yellowChangeDs = 30U;
    config.phases[phaseIndex].redClearDs = 20U;
    config.phases[phaseIndex].walkSeconds = 0U;
    config.phases[phaseIndex].pedClearSeconds = 0U;
  }

  config.phases[0].concurrency.length = 1U;
  config.phases[0].concurrency.values[0] = 3U;
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
  config.vehicleDetectors[0].options =
    VEHICLE_DETECTOR_OPTIONS_VOLUME | VEHICLE_DETECTOR_OPTIONS_OCCUPANCY;
  config.vehicleDetectors[1].callPhase = 1U;
  config.pedestrianDetectors[0].callPhase = 1U;
  config.pedestrianDetectors[1].callPhase = 2U;
  config.inputMapping.preemptInputs[0] = 2U;
  config.inputMapping.preemptControls[0] = 1U;
  config.channels[0].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  config.channels[0].controlSource = 1U;
  config.ioMap.outputs[0].deviceType = (uint8_t) INTERSECTION_IO_MAP_DEVICE_FIO;
  config.ioMap.outputs[0].devicePin = 1U;
  config.ioMap.outputs[0].function =
    (uint8_t) INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_GREEN;
  config.ioMap.outputs[0].functionIndex = 1U;
  config.unit.startUpFlashSeconds = 5U;
  config.unit.startUpFlashMode =
    INTERSECTION_UNIT_STARTUP_FLASH_MODE_ALL_RED_CONTROLLER_FLASH;

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

void setUp(void)
{
  IntersectionConfig_t config = MakeConfig();

  memset(&s_configurationService, 0, sizeof(s_configurationService));
  memset(&s_engine, 0, sizeof(s_engine));
  memset(&s_controller, 0, sizeof(s_controller));
  memset(&s_dispatcher, 0, sizeof(s_dispatcher));
  memset(&s_detectorReportService, 0, sizeof(s_detectorReportService));
  memset(&s_globalTimeService, 0, sizeof(s_globalTimeService));
  memset(&s_cpMpLinkService, 0, sizeof(s_cpMpLinkService));
  memset(&s_cache, 0, sizeof(s_cache));

  s_configurationService.activeConfig = config;
  s_configurationService.activeGeneration = 11U;

  IntersectionEngineInit(&s_engine);
  s_engine.config = config;
  s_engine.configLoaded = 1U;
  s_engine.runtime.configLoaded = 1U;

  s_engine.runtime.mode = INTERSECTION_CONTROL_MODE_PREEMPT;
  s_engine.runtime.localFreeStatus = INTERSECTION_LOCAL_FREE_STATUS_COORD_FREE;
  s_engine.runtime.unitControlStatus =
    INTERSECTION_UNIT_CONTROL_STATUS_TIMEBASE;
  s_engine.runtime.coordPatternStatus = 3U;
  s_engine.runtime.actionPlanControl = 4U;
  s_engine.runtime.timebaseActionStatus = 5U;
  s_engine.runtime.preemptStatus = 6U;
  s_engine.runtime.mmuFlashActive = 1U;
  s_engine.runtime.startUpFlashActive = 1U;
  s_engine.runtime.dimmingActive = 1U;
  s_engine.runtime.coordCycleStatusSeconds = 12U;
  s_engine.runtime.coordSyncStatusSeconds = 34U;
  s_engine.runtime.monotonicTicks = 5678U;
  s_engine.activeSequenceNumber = 2U;
  s_engine.runtime.rings[0].activePhaseIndex = 0U;
  s_engine.runtime.rings[0].stage = INTERSECTION_RING_STAGE_GREEN;
  s_engine.runtime.rings[0].statusCode = INTERSECTION_RING_STATUS_EXTENSION;
  s_engine.runtime.rings[0].terminationReasonBits =
    INTERSECTION_RING_TERMINATION_GAP_OUT;
  s_engine.runtime.rings[0].stageElapsedTicks = 111U;
  s_engine.runtime.phases[0].interval = INTERSECTION_PHASE_INTERVAL_GREEN;
  s_engine.runtime.phases[0].pedInterval = INTERSECTION_PED_INTERVAL_WALK;
  s_engine.runtime.phases[0].detectorActive = 1U;
  s_engine.runtime.phases[0].callLatched = 1U;
  s_engine.runtime.phases[0].next = 1U;
  s_engine.runtime.phases[0].intervalElapsedTicks = 44U;
  s_engine.runtime.phases[0].pedIntervalElapsedTicks = 9U;
  s_engine.runtime.channels[0].aspect = INTERSECTION_OUTPUT_ASPECT_GREEN;
  s_engine.runtime.channels[0].dimmed = 1U;
  s_engine.runtime.overlaps[0].aspect = INTERSECTION_OUTPUT_ASPECT_FLASH_YELLOW;
  s_engine.runtime.vehicleDetectors[0].inputActive = 1U;
  s_engine.runtime.vehicleDetectors[0].recognitionActive = 1U;
  s_engine.runtime.vehicleDetectors[0].delayTimerTicks = 120U;
  s_engine.runtime.vehicleDetectors[0].extendTimerTicks = 230U;
  s_engine.runtime.pedestrianDetectors[0].inputActive = 1U;
  s_engine.runtime.pedestrianDetectors[0].alternateTimingRequest = 1U;

  IntersectionControllerInit(&s_controller);
  s_controller.engine = &s_engine;
  s_controller.outputDispatcher = &s_dispatcher;
  s_controller.lastSnapshotValid = 1U;
  s_controller.lastSnapshot.protocolVersion = MODULE_BUS_PROTOCOL_VERSION;
  s_controller.lastSnapshot.validMask =
    MODULE_BUS_SNAPSHOT_VALID_DETECTORS | MODULE_BUS_SNAPSHOT_VALID_PEDS;
  s_controller.lastSnapshot.healthMask = s_controller.lastSnapshot.validMask;
  s_controller.lastSnapshot.sequence = 9U;
  s_controller.lastSnapshot.configEpoch = 22U;
  s_controller.lastSnapshot.rawVehicleDetectorInputs = 0x00000001UL;
  s_controller.lastSnapshot.rawPedestrianInputs = 0x00000002UL;
  s_controller.lastSnapshot.preemptInputs = 0x03U;
  s_controller.lastSnapshot.preemptControls = 0x04U;
  s_controller.lastSnapshot.loadSwitchReds = 0x0001U;
  s_controller.lastSnapshot.loadSwitchYellows = 0x0002U;
  s_controller.lastSnapshot.loadSwitchGreens = 0x0004U;
  s_controller.lastSnapshot.vehicleDetectorAlarms[0] = 0xAAU;
  s_controller.lastSnapshot.vehicleDetectorReportedAlarms[0] = 0x55U;
  s_controller.lastSnapshot.pedestrianDetectorAlarms[0] = 0x11U;

  s_dispatcher.lastRequestedImage.channels[0] = OUTPUT_DRIVER_ASPECT_GREEN;
  s_dispatcher.lastAppliedImage.channels[0] = OUTPUT_DRIVER_ASPECT_FLASH_RED;
  s_dispatcher.lastRequestedImage.channelDimmed[0] = 1U;
  s_dispatcher.lastRequestedImage.channelDimAlternateHalfCycle[0] = 1U;

  s_detectorReportService.engine = &s_engine;
  s_detectorReportService.controller = &s_controller;
  s_detectorReportService.vehicleSamples[0].volume = 7U;
  s_detectorReportService.vehicleSamples[0].occupancy = 8U;
  s_detectorReportService.vehicleSamples[0].averageSpeed = 9U;
  s_detectorReportService.pedestrianSamples[0].volume = 3U;
  s_detectorReportService.pedestrianSamples[0].actuations = 4U;
  s_detectorReportService.pedestrianSamples[0].services = 5U;

  MockRTCAdapterInit(&s_rtcCtx);
  s_rtcPort = MockRTCAdapterCreatePort(&s_rtcCtx);
  SetRtc(21U, 26U, 4U, 19U, 1U, 10U, 30U, 0U);
  GlobalTimeManagementServiceInit(&s_globalTimeService);
  GlobalTimeManagementServiceBind(&s_globalTimeService, &s_engine, &s_rtcPort);

  s_cpMpLinkService.tickCount = 10U;
  s_cpMpLinkService.lastMpHeartbeatTick = 9U;
  s_cpMpLinkService.lastMpHeartbeatSeen = 1U;
  s_cpMpLinkService.configSetId = 7U;
  s_cpMpLinkService.lastMpConfigSetId = 7U;
  s_cpMpLinkService.configGeneration = 11U;
  s_cpMpLinkService.lastMpConfigGeneration = 11U;
  s_cpMpLinkService.lastMpConfigState = CPMP_CONFIG_STATE_APPLIED;
  s_cpMpLinkService.lastSafetyAction = CPMP_SAFETY_ACTION_DARK;
  s_cpMpLinkService.lastSafetyReasonCode = 0x42U;
  s_cpMpLinkService.lastFaultStatusValid = 1U;
  s_cpMpLinkService.lastFaultStatus.sequence = 5U;
  s_cpMpLinkService.lastFaultStatus.globalFlags =
    CPMP_FAULT_GLOBAL_FLAG_WATCHDOG | CPMP_FAULT_GLOBAL_FLAG_CONFIG_INVALID;
  s_cpMpLinkService.lastFaultStatus.channelFlags[0] =
    CPMP_FAULT_CHANNEL_FLAG_CONFLICT;
  s_cpMpLinkService.lastFaultStatus.configState = CPMP_CONFIG_STATE_APPLIED;

  MmiSnapshotCacheInit(&s_cache);
  MmiSnapshotCacheBind(&s_cache,
                       &s_configurationService,
                       &s_engine,
                       &s_controller,
                       &s_detectorReportService,
                       &s_globalTimeService,
                       &s_cpMpLinkService);
}

void tearDown(void)
{
}

void test_snapshot_cache_refresh_populates_summary_and_outputs(void)
{
  MmiRuntimeSummaryV2_t summary;
  MmiRuntimeChannelRecordV2_t channel;

  TEST_ASSERT_TRUE(MmiSnapshotCacheRefresh(&s_cache));
  TEST_ASSERT_TRUE(MmiSnapshotCacheGetSummary(&s_cache, &summary));
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_CONTROL_MODE_PREEMPT, summary.mode);
  TEST_ASSERT_EQUAL_UINT8(CPMP_SAFETY_ACTION_DARK, summary.safetyAction);
  TEST_ASSERT_EQUAL_UINT8(0x42U, summary.safetyReasonCode);
  TEST_ASSERT_EQUAL_UINT8(2U, summary.activeSequenceNumber);
  TEST_ASSERT_EQUAL_UINT32(5678U, summary.monotonicTicks);

  TEST_ASSERT_TRUE(MmiSnapshotCacheGetChannelRecord(&s_cache, 1U, &channel));
  TEST_ASSERT_EQUAL_UINT8(1U, channel.channelNumber);
  TEST_ASSERT_EQUAL_UINT8(OUTPUT_DRIVER_ASPECT_GREEN, channel.requestedAspect);
  TEST_ASSERT_EQUAL_UINT8(OUTPUT_DRIVER_ASPECT_FLASH_RED, channel.appliedAspect);
  TEST_ASSERT_EQUAL_UINT8(1U, channel.dimmed);
  TEST_ASSERT_EQUAL_UINT8(1U, channel.dimAlternateHalfCycle);
}

void test_snapshot_cache_refresh_populates_inputs_detectors_and_safety(void)
{
  MmiRuntimeRawInputsV2_t inputs;
  MmiRuntimeVehicleDetectorRecordV2_t vehicle;
  MmiRuntimePedestrianDetectorRecordV2_t ped;
  MmiRuntimeSafetySummaryV2_t safety;
  MmiRuntimeSafetyChannelRecordV2_t safetyChannel;

  TEST_ASSERT_TRUE(MmiSnapshotCacheRefresh(&s_cache));

  TEST_ASSERT_TRUE(MmiSnapshotCacheGetRawInputs(&s_cache, &inputs));
  TEST_ASSERT_EQUAL_HEX32(0x00000001UL, inputs.rawVehicleMask);
  TEST_ASSERT_EQUAL_HEX32(0x00000002UL, inputs.rawPedestrianMask);
  TEST_ASSERT_EQUAL_UINT8(9U, inputs.sequence);

  TEST_ASSERT_TRUE(MmiSnapshotCacheGetVehicleDetectorRecord(&s_cache,
                                                            1U,
                                                            &vehicle));
  TEST_ASSERT_EQUAL_UINT8(1U, vehicle.detectorNumber);
  TEST_ASSERT_EQUAL_UINT8(1U, vehicle.inputActive);
  TEST_ASSERT_EQUAL_UINT8(1U, vehicle.callPhase);
  TEST_ASSERT_EQUAL_UINT8(7U, vehicle.volume);
  TEST_ASSERT_EQUAL_UINT8(8U, vehicle.occupancy);
  TEST_ASSERT_EQUAL_UINT16(9U, vehicle.averageSpeed);
  TEST_ASSERT_EQUAL_UINT8(0xAAU, vehicle.alarm);
  TEST_ASSERT_EQUAL_UINT8(0x55U, vehicle.reportedAlarm);

  TEST_ASSERT_TRUE(MmiSnapshotCacheGetPedestrianDetectorRecord(&s_cache,
                                                               1U,
                                                               &ped));
  TEST_ASSERT_EQUAL_UINT8(2U, ped.callPhase);
  TEST_ASSERT_EQUAL_UINT8(3U, ped.volume);
  TEST_ASSERT_EQUAL_UINT8(4U, ped.actuations);
  TEST_ASSERT_EQUAL_UINT8(5U, ped.services);
  TEST_ASSERT_EQUAL_UINT8(0x11U, ped.alarm);

  TEST_ASSERT_TRUE(MmiSnapshotCacheGetSafetySummary(&s_cache, &safety));
  TEST_ASSERT_EQUAL_UINT8(1U, safety.peerHealthy);
  TEST_ASSERT_EQUAL_UINT8(1U, safety.authorityReady);
  TEST_ASSERT_EQUAL_UINT8(CPMP_SAFETY_ACTION_DARK, safety.safetyAction);
  TEST_ASSERT_EQUAL_UINT32(
    CPMP_FAULT_GLOBAL_FLAG_WATCHDOG | CPMP_FAULT_GLOBAL_FLAG_CONFIG_INVALID,
    safety.globalFaultFlags);

  TEST_ASSERT_TRUE(MmiSnapshotCacheGetSafetyChannelRecord(&s_cache,
                                                          1U,
                                                          &safetyChannel));
  TEST_ASSERT_EQUAL_UINT16(CPMP_FAULT_CHANNEL_FLAG_CONFLICT,
                           safetyChannel.faultFlags);
}

void test_snapshot_cache_refresh_populates_clock_summary(void)
{
  MmiRuntimeClockSummaryV2_t clockSummary;

  TEST_ASSERT_TRUE(MmiSnapshotCacheRefresh(&s_cache));
  TEST_ASSERT_TRUE(MmiSnapshotCacheGetClockSummary(&s_cache, &clockSummary));
  TEST_ASSERT_NOT_EQUAL(0U, clockSummary.globalTimeSeconds);
  TEST_ASSERT_NOT_EQUAL(0U, clockSummary.localTimeSeconds);
  TEST_ASSERT_NOT_EQUAL(0, clockSummary.globalLocalDifferentialSeconds);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_snapshot_cache_refresh_populates_summary_and_outputs);
  RUN_TEST(test_snapshot_cache_refresh_populates_inputs_detectors_and_safety);
  RUN_TEST(test_snapshot_cache_refresh_populates_clock_summary);
  return UNITY_END();
}
