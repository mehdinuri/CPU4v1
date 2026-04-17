/*
 * Tests/Unit/Test_DetectorReportObjects.c
 *
 * Unit tests for 1202 detector report OIDs routed through the object
 * directory and backed by DetectorReportService.
 */
#include "unity.h"

#include "Domain/Intersection/DetectorReportService.h"
#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/NTCIP/NTCIP1202.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"
#include "MockConfigRepositoryAdapter.h"
#include "MockRTCAdapter.h"


static const uint32_t kDbCreateTransactionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 1U, 0U
};
static const uint32_t kDbTransactionIdOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 4U, 0U
};
static const uint32_t kVehicleDetectorOptionsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 2U, 1U
};
static const uint32_t kVehicleDetectorCallPhaseOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 2U, 1U, 4U, 1U
};
static const uint32_t kPedestrianDetectorOptionsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 9U, 1U
};
static const uint32_t kPedestrianDetectorCallPhaseOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 7U, 1U, 2U, 1U
};
static const uint32_t kVolumeOccupancySequenceOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 1U, 0U
};
static const uint32_t kVolumeOccupancyPeriodOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 2U, 0U
};
static const uint32_t kActiveVolumeOccupancyDetectorsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 3U, 0U
};
static const uint32_t kDetectorVolumeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 4U, 1U, 1U, 1U
};
static const uint32_t kDetectorOccupancyOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 4U, 1U, 2U, 1U
};
static const uint32_t kDetectorAvgSpeedOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 4U, 1U, 3U, 1U
};
static const uint32_t kVolumeOccupancyPeriodV3Oid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 5U, 0U
};
static const uint32_t kDetectorSampleDurationOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 7U, 0U
};
static const uint32_t kPedestrianDetectorSequenceOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 1U, 0U
};
static const uint32_t kPedestrianDetectorPeriodOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 2U, 0U
};
static const uint32_t kActivePedestrianDetectorsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 3U, 0U
};
static const uint32_t kPedestrianDetectorVolumeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 4U, 1U, 1U, 1U
};
static const uint32_t kPedestrianDetectorActuationsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 4U, 1U, 2U, 1U
};
static const uint32_t kPedestrianDetectorServicesOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 4U, 1U, 3U, 1U
};
static const uint32_t kPedestrianDetectorSampleDurationOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 6U, 0U
};

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_configurationService;
static IntersectionEngine_t s_engine;
static IntersectionController_t s_controller;
static DetectorReportService_t s_detectorReportService;
static MockRTCAdapterCtx_t s_rtcCtx;
static IRealtimeClockPort_t s_rtcPort;
static NtcipDbTransactionService_t s_dbTransactionService;
static NtcipObjectDirectory_t s_directory;
static NtcipContext_t s_ntcipContext;

static void SetRtc(uint8_t seconds)
{
  RtcSnapshot_t snapshot = { 21U, 26U, 4U, 17U, 5U, 12U, 0U, seconds };

  TEST_ASSERT_TRUE(RealtimeClockWriteSnapshot(&s_rtcPort, &snapshot));
}

static void ReloadEngineAndReports(void)
{
  TEST_ASSERT_TRUE(IntersectionEngineLoadConfig(
                     &s_engine,
                     ConfigurationServiceGetActiveConfig(
                       &s_configurationService)));
  DetectorReportServiceReset(&s_detectorReportService);
}

static void BeginTransaction(NtcipRequestContext_t *request,
                             uint8_t transactionId)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, transactionId);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbTransactionIdOid,
                                                     13U,
                                                     request,
                                                     &value));
  request->transactionIdValid = 1U;
  request->transactionId = transactionId;
}

static void VerifyAndCommitTransaction(NtcipRequestContext_t *request)
{
  NtcipValue_t value;

  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_VERIFY);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kDbCreateTransactionOid,
                                                     13U,
                                                     request,
                                                     &value));
  ReloadEngineAndReports();
}

static void AdvanceTicks(uint16_t tickCount)
{
  uint16_t tickIndex;

  for (tickIndex = 0U; tickIndex < tickCount; ++tickIndex)
  {
    IntersectionEngineTick(&s_engine);
    DetectorReportServiceStep(&s_detectorReportService);
  }
}

void setUp(void)
{
  MockConfigRepositoryAdapterInit(&s_repoCtx);
  s_repoPort = MockConfigRepositoryAdapterCreatePort(&s_repoCtx);
  ConfigurationServiceInit(&s_configurationService, &s_repoPort);
  IntersectionEngineInit(&s_engine);
  IntersectionControllerInit(&s_controller);
  DetectorReportServiceInit(&s_detectorReportService);
  MockRTCAdapterInit(&s_rtcCtx);
  s_rtcPort = MockRTCAdapterCreatePort(&s_rtcCtx);
  SetRtc(0U);
  DetectorReportServiceBind(&s_detectorReportService,
                            &s_engine,
                            &s_controller,
                            &s_rtcPort);
  NtcipDbTransactionServiceInit(&s_dbTransactionService,
                                &s_configurationService);
  NtcipContextInit(&s_ntcipContext,
                   &s_configurationService,
                   &s_engine,
                   &s_controller,
                   &s_dbTransactionService);
  NtcipContextBindDetectorReportService(&s_ntcipContext,
                                        &s_detectorReportService);
  NtcipObjectDirectoryInit(&s_directory);
  Ntcip1201RegisterObjects(&s_directory, &s_ntcipContext);
  Ntcip1202RegisterObjects(&s_directory, &s_ntcipContext);
  ReloadEngineAndReports();
}

void tearDown(void)
{
}

void test_detector_report_period_scalars_are_transactional(void)
{
  NtcipRequestContext_t request = { 0x4444U, 0U, 0U };
  NtcipValue_t value;

  BeginTransaction(&request, 4U);
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVolumeOccupancyPeriodOid,
                                                     14U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 65534U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPedestrianDetectorPeriodOid,
                                                     14U,
                                                     &request,
                                                     &value));

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVolumeOccupancyPeriodOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVolumeOccupancyPeriodOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianDetectorPeriodOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(65534U, value.data.unsigned32);
}

void test_detector_report_objects_follow_runtime_samples(void)
{
  NtcipRequestContext_t request = { 0x4545U, 0U, 0U };
  NtcipValue_t value;

  BeginTransaction(&request, 5U);
  NtcipValueSetUnsigned32(&value,
                          (uint32_t) (VEHICLE_DETECTOR_OPTIONS_VOLUME
                                      | VEHICLE_DETECTOR_OPTIONS_OCCUPANCY));
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVehicleDetectorOptionsOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 0U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVehicleDetectorCallPhaseOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, PED_DETECTOR_OPTIONS_PRESENCE);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPedestrianDetectorOptionsOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPedestrianDetectorCallPhaseOid,
                                                     15U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 1U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVolumeOccupancyPeriodOid,
                                                     14U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 65534U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kPedestrianDetectorPeriodOid,
                                                     14U,
                                                     &request,
                                                     &value));
  NtcipValueSetUnsigned32(&value, 0U);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectorySetValue(&s_directory,
                                                     kVolumeOccupancyPeriodV3Oid,
                                                     14U,
                                                     &request,
                                                     &value));

  VerifyAndCommitTransaction(&request);

  TEST_ASSERT_TRUE(IntersectionEngineSetPedestrianDetectorInput(&s_engine,
                                                                1U,
                                                                1U));
  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 1U, 1U));
  AdvanceTicks(30U);
  TEST_ASSERT_TRUE(IntersectionEngineSetPedestrianDetectorInput(&s_engine,
                                                                1U,
                                                                0U));
  TEST_ASSERT_TRUE(IntersectionEngineSetVehicleDetectorInput(&s_engine, 1U, 0U));
  SetRtc(1U);
  AdvanceTicks(70U);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kVolumeOccupancySequenceOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kActiveVolumeOccupancyDetectorsOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kDetectorVolumeOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kDetectorOccupancyOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(60U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kDetectorAvgSpeedOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(511U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kDetectorSampleDurationOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);

  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianDetectorSequenceOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kActivePedestrianDetectorsOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(8U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianDetectorVolumeOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianDetectorActuationsOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianDetectorServicesOid,
                                                16U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(0U, value.data.unsigned32);
  TEST_ASSERT_EQUAL_INT(NTCIP_ERROR_OK,
                        NtcipObjectDirectoryGet(&s_directory,
                                                kPedestrianDetectorSampleDurationOid,
                                                14U,
                                                NULL,
                                                &value));
  TEST_ASSERT_EQUAL_UINT32(1U, value.data.unsigned32);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_detector_report_period_scalars_are_transactional);
  RUN_TEST(test_detector_report_objects_follow_runtime_samples);

  return UNITY_END();
}
