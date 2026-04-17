/* App/Domain/Intersection/DetectorReportService.c */
#include "DetectorReportService.h"

#include <stddef.h>
#include <string.h>

enum
{
  VEHICLE_REPORT_OCCUPANCY_FAULT_MAX_PRESENCE = 210U,
  VEHICLE_REPORT_OCCUPANCY_FAULT_NO_ACTIVITY = 211U,
  VEHICLE_REPORT_OCCUPANCY_FAULT_ERRATIC = 217U,
  PEDESTRIAN_REPORT_FAULT_OTHER = 209U,
  PEDESTRIAN_REPORT_FAULT_MAX_PRESENCE = 210U,
  PEDESTRIAN_REPORT_FAULT_NO_ACTIVITY = 211U,
  PEDESTRIAN_REPORT_FAULT_CONFIGURATION = 215U,
  PEDESTRIAN_REPORT_FAULT_COMMUNICATIONS = 216U,
  PEDESTRIAN_REPORT_FAULT_ERRATIC = 217U,
  DETECTOR_REPORT_SPEED_INVALID = 511U
};

static const IntersectionConfig_t *GetConfig(const DetectorReportService_t *service)
{
  if ((service == NULL) || (service->engine == NULL))
  {
    return NULL;
  }

  return IntersectionEngineGetConfig(service->engine);
}

static const IntersectionRuntime_t *GetRuntime(
  const DetectorReportService_t *service)
{
  if ((service == NULL) || (service->engine == NULL))
  {
    return NULL;
  }

  return IntersectionEngineGetRuntime(service->engine);
}

static uint8_t GetControllerSnapshot(const DetectorReportService_t *service,
                                     ModuleBusSnapshot_t *snapshot)
{
  if ((service == NULL) || (snapshot == NULL) || (service->controller == NULL))
  {
    return 0U;
  }

  return IntersectionControllerGetLastSnapshot(service->controller, snapshot);
}

static uint8_t VehicleDetectorCollectsData(
  const IntersectionVehicleDetectorConfig_t *detector)
{
  if (detector == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((((detector->options
                       & (uint8_t) (VEHICLE_DETECTOR_OPTIONS_VOLUME
                                     | VEHICLE_DETECTOR_OPTIONS_OCCUPANCY))
                      != 0U)
                     || ((detector->options2
                          & VEHICLE_DETECTOR_OPTIONS2_SPEED_ENABLED)
                         != 0U)));
}

static uint8_t PedestrianDetectorCollectsData(
  const IntersectionPedestrianDetectorConfig_t *detector)
{
  return (uint8_t) ((detector != NULL) && (detector->callPhase != 0U));
}

static uint16_t ResolveCoordinatedCyclePeriodSeconds(
  const DetectorReportService_t *service)
{
  const IntersectionRuntime_t *runtime = GetRuntime(service);

  if ((runtime == NULL)
      || (runtime->mode != INTERSECTION_CONTROL_MODE_COORDINATED)
      || (runtime->coordCycleStatusSeconds == 0U))
  {
    return 0U;
  }

  return runtime->coordCycleStatusSeconds;
}

static uint16_t ResolveVehiclePeriodSeconds(const DetectorReportService_t *service)
{
  const IntersectionConfig_t *config = GetConfig(service);
  uint16_t cycleSeconds;

  if (config == NULL)
  {
    return 0U;
  }

  if (config->detectorReports.volumeOccupancyPeriodSeconds != 0U)
  {
    return config->detectorReports.volumeOccupancyPeriodSeconds;
  }

  if (config->detectorReports.volumeOccupancyPeriodV3Seconds == 0U)
  {
    return 0U;
  }

  if (config->detectorReports.volumeOccupancyPeriodV3Seconds <= 3600U)
  {
    return config->detectorReports.volumeOccupancyPeriodV3Seconds;
  }

  if (config->detectorReports.volumeOccupancyPeriodV3Seconds != 65535U)
  {
    return 0U;
  }

  cycleSeconds = ResolveCoordinatedCyclePeriodSeconds(service);

  return cycleSeconds;
}

static uint16_t ResolvePedestrianPeriodSeconds(
  const DetectorReportService_t *service)
{
  const IntersectionConfig_t *config = GetConfig(service);
  uint16_t cycleSeconds;

  if (config == NULL)
  {
    return 0U;
  }

  if (config->detectorReports.pedestrianDetectorPeriodSeconds == 0U)
  {
    return 0U;
  }

  if (config->detectorReports.pedestrianDetectorPeriodSeconds <= 3600U)
  {
    return config->detectorReports.pedestrianDetectorPeriodSeconds;
  }

  if (config->detectorReports.pedestrianDetectorPeriodSeconds == 65534U)
  {
    return ResolveVehiclePeriodSeconds(service);
  }

  if (config->detectorReports.pedestrianDetectorPeriodSeconds != 65535U)
  {
    return 0U;
  }

  cycleSeconds = ResolveCoordinatedCyclePeriodSeconds(service);

  return cycleSeconds;
}

static uint8_t VehicleRowIsActive(const DetectorReportService_t *service,
                                  uint8_t detectorIndex)
{
  const IntersectionConfig_t *config = GetConfig(service);

  if ((config == NULL)
      || (detectorIndex >= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX)
      || (ResolveVehiclePeriodSeconds(service) == 0U))
  {
    return 0U;
  }

  return VehicleDetectorCollectsData(&config->vehicleDetectors[detectorIndex]);
}

static uint8_t PedestrianRowIsActive(const DetectorReportService_t *service,
                                     uint8_t detectorIndex)
{
  const IntersectionConfig_t *config = GetConfig(service);

  if ((config == NULL)
      || (detectorIndex >= INTERSECTION_PED_INPUT_COUNT_MAX)
      || (ResolvePedestrianPeriodSeconds(service) == 0U))
  {
    return 0U;
  }

  return PedestrianDetectorCollectsData(&config->pedestrianDetectors[detectorIndex]);
}

static uint8_t ReadLocalEpochSeconds(const DetectorReportService_t *service,
                                     uint32_t *epochSeconds)
{
  RtcSnapshot_t snapshot;
  int32_t year;
  int32_t month;
  int32_t date;
  int32_t adjustedYear;
  int32_t era;
  int32_t yearOfEra;
  int32_t dayOfYear;
  int32_t dayOfEra;
  int64_t daysSinceEpoch;
  uint64_t seconds;

  if ((service == NULL) || (epochSeconds == NULL) || (service->rtcPort == NULL)
      || (RealtimeClockReadSnapshot(service->rtcPort, &snapshot) == 0U)
      || (snapshot.Century == 0U) || (snapshot.Month == 0U)
      || (snapshot.Month > 12U) || (snapshot.Date == 0U)
      || (snapshot.Date > 31U) || (snapshot.Hours > 23U)
      || (snapshot.Minutes > 59U) || (snapshot.Seconds > 59U))
  {
    return 0U;
  }

  year = ((int32_t) snapshot.Century - 1) * 100 + (int32_t) snapshot.Year;
  month = (int32_t) snapshot.Month;
  date = (int32_t) snapshot.Date;
  adjustedYear = year - (month <= 2);
  era = (adjustedYear >= 0) ? (adjustedYear / 400)
        : ((adjustedYear - 399) / 400);
  yearOfEra = adjustedYear - (era * 400);
  dayOfYear = ((153 * (month + ((month > 2) ? -3 : 9))) + 2) / 5 + date - 1;
  dayOfEra = (yearOfEra * 365) + (yearOfEra / 4) - (yearOfEra / 100)
             + dayOfYear;
  daysSinceEpoch = ((int64_t) era * 146097) + (int64_t) dayOfEra - 719468LL;
  seconds = ((uint64_t) daysSinceEpoch * 86400ULL)
            + ((uint64_t) snapshot.Hours * 3600ULL)
            + ((uint64_t) snapshot.Minutes * 60ULL)
            + (uint64_t) snapshot.Seconds;

  *epochSeconds = (uint32_t) seconds;

  return 1U;
}

static void ResetCurrentVehicleAccumulators(DetectorReportService_t *service)
{
  uint8_t detectorIndex;

  if (service == NULL)
  {
    return;
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       ++detectorIndex)
  {
    service->vehicleAccumulators[detectorIndex].volumeCount = 0U;
    service->vehicleAccumulators[detectorIndex].occupancyTicks = 0U;
  }
}

static void ResetCurrentPedestrianAccumulators(DetectorReportService_t *service)
{
  uint8_t detectorIndex;

  if (service == NULL)
  {
    return;
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       ++detectorIndex)
  {
    service->pedestrianAccumulators[detectorIndex].volumeCount = 0U;
    service->pedestrianAccumulators[detectorIndex].actuationCount = 0U;
    service->pedestrianAccumulators[detectorIndex].serviceCount = 0U;
  }
}

static uint8_t EncodeVolumeCount(uint32_t count)
{
  if (count > 254U)
  {
    return 255U;
  }

  return (uint8_t) count;
}

static uint8_t EncodePedestrianActuationCount(uint32_t count)
{
  if (count > 200U)
  {
    return 201U;
  }

  return (uint8_t) count;
}

static uint16_t EncodeVehicleAverageSpeed(void)
{
  return DETECTOR_REPORT_SPEED_INVALID;
}

static uint8_t EncodeOccupancyTicks(uint32_t occupancyTicks,
                                    uint16_t sampleDurationSeconds)
{
  uint32_t totalTicks;

  if (sampleDurationSeconds == 0U)
  {
    return 0U;
  }

  totalTicks = (uint32_t) sampleDurationSeconds * 100U;

  if (totalTicks == 0U)
  {
    return 0U;
  }

  return (uint8_t) (((occupancyTicks * 200U) + (totalTicks / 2U)) / totalTicks);
}

static uint8_t GetVehicleOccupancyFaultCode(const DetectorReportService_t *service,
                                            uint8_t detectorIndex)
{
  ModuleBusSnapshot_t snapshot;
  const IntersectionConfig_t *config = GetConfig(service);

  if ((config == NULL)
      || (detectorIndex >= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  if ((config->vehicleDetectors[detectorIndex].callPhase != 0U)
      && (GetControllerSnapshot(service, &snapshot) != 0U)
      && (ModuleBusSnapshotSourceReady(&snapshot,
                                       MODULE_BUS_SNAPSHOT_VALID_DETECTORS)
          == 0U))
  {
    return 0U;
  }

  return 0U;
}

static uint8_t GetPedestrianActuationFaultCode(
  const DetectorReportService_t *service,
  uint8_t detectorIndex)
{
  ModuleBusSnapshot_t snapshot;
  const IntersectionConfig_t *config = GetConfig(service);

  if ((config == NULL)
      || (detectorIndex >= INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return PEDESTRIAN_REPORT_FAULT_OTHER;
  }

  if ((config->pedestrianDetectors[detectorIndex].callPhase != 0U)
      && (GetControllerSnapshot(service, &snapshot) != 0U)
      && (ModuleBusSnapshotSourceReady(&snapshot,
                                       MODULE_BUS_SNAPSHOT_VALID_PEDS) == 0U))
  {
    return PEDESTRIAN_REPORT_FAULT_COMMUNICATIONS;
  }

  return 0U;
}

static void FinalizeVehicleSample(DetectorReportService_t *service,
                                  uint16_t sampleDurationSeconds)
{
  const IntersectionConfig_t *config = GetConfig(service);
  uint8_t detectorIndex;
  uint32_t sampleTimeSeconds;

  if ((service == NULL) || (config == NULL))
  {
    return;
  }

  service->vehicleSequence++;
  service->vehicleSampleDurationSeconds = sampleDurationSeconds;

  if (ReadLocalEpochSeconds(service, &sampleTimeSeconds) != 0U)
  {
    service->vehicleSampleTimeSeconds = sampleTimeSeconds;
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       ++detectorIndex)
  {
    const IntersectionVehicleDetectorConfig_t *detector =
      &config->vehicleDetectors[detectorIndex];
    const DetectorReportVehicleAccumulator_t *accumulator =
      &service->vehicleAccumulators[detectorIndex];
    DetectorReportVehicleSample_t *sample =
      &service->vehicleSamples[detectorIndex];
    uint8_t occupancyFaultCode = GetVehicleOccupancyFaultCode(service,
                                                              detectorIndex);

    if ((detector->options & VEHICLE_DETECTOR_OPTIONS_VOLUME) != 0U)
    {
      sample->volume = EncodeVolumeCount(accumulator->volumeCount);
    }
    else
    {
      sample->volume = 0U;
    }

    if (((detector->options
          & (uint8_t) (VEHICLE_DETECTOR_OPTIONS_VOLUME
                        | VEHICLE_DETECTOR_OPTIONS_OCCUPANCY)) != 0U)
        && (occupancyFaultCode != 0U))
    {
      sample->occupancy = occupancyFaultCode;
    }
    else if ((detector->options & VEHICLE_DETECTOR_OPTIONS_OCCUPANCY) != 0U)
    {
      sample->occupancy = EncodeOccupancyTicks(accumulator->occupancyTicks,
                                               sampleDurationSeconds);
    }
    else
    {
      sample->occupancy = 0U;
    }

    sample->averageSpeed = EncodeVehicleAverageSpeed();
  }
}

static void FinalizePedestrianSample(DetectorReportService_t *service,
                                     uint16_t sampleDurationSeconds)
{
  const IntersectionConfig_t *config = GetConfig(service);
  uint8_t detectorIndex;
  uint32_t sampleTimeSeconds;

  if ((service == NULL) || (config == NULL))
  {
    return;
  }

  service->pedestrianSequence++;
  service->pedestrianSampleDurationSeconds = sampleDurationSeconds;

  if (ReadLocalEpochSeconds(service, &sampleTimeSeconds) != 0U)
  {
    service->pedestrianSampleTimeSeconds = sampleTimeSeconds;
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       ++detectorIndex)
  {
    const IntersectionPedestrianDetectorConfig_t *detector =
      &config->pedestrianDetectors[detectorIndex];
    const DetectorReportPedestrianAccumulator_t *accumulator =
      &service->pedestrianAccumulators[detectorIndex];
    DetectorReportPedestrianSample_t *sample =
      &service->pedestrianSamples[detectorIndex];
    uint8_t faultCode = GetPedestrianActuationFaultCode(service, detectorIndex);

    if ((detector->options & PED_DETECTOR_OPTIONS_PRESENCE) != 0U)
    {
      sample->volume = EncodeVolumeCount(accumulator->volumeCount);
    }
    else
    {
      sample->volume = 0U;
    }

    if (faultCode != 0U)
    {
      sample->actuations = faultCode;
    }
    else
    {
      sample->actuations = EncodePedestrianActuationCount(
        accumulator->actuationCount);
    }

    sample->services = EncodeVolumeCount(accumulator->serviceCount);
  }
}

static void UpdatePhaseServiceCounts(DetectorReportService_t *service,
                                     const IntersectionConfig_t *config,
                                     const IntersectionRuntime_t *runtime,
                                     uint8_t pedCollectionActive)
{
  uint8_t phaseIndex;

  if ((service == NULL) || (config == NULL) || (runtime == NULL))
  {
    return;
  }

  for (phaseIndex = 0U; phaseIndex < config->phaseCount; ++phaseIndex)
  {
    uint8_t walkActive = (uint8_t) (runtime->phases[phaseIndex].pedInterval
                                    == INTERSECTION_PED_INTERVAL_WALK);

    if ((pedCollectionActive != 0U) && (walkActive != 0U)
        && (service->previousPhaseWalkActive[phaseIndex] == 0U))
    {
      uint8_t detectorIndex;

      for (detectorIndex = 0U; detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
           ++detectorIndex)
      {
        if ((PedestrianRowIsActive(service, detectorIndex) != 0U)
            && (config->pedestrianDetectors[detectorIndex].callPhase
                == (uint8_t) (phaseIndex + 1U)))
        {
          service->pedestrianAccumulators[detectorIndex].serviceCount++;
        }
      }
    }

    service->previousPhaseWalkActive[phaseIndex] = walkActive;
  }
}

static void UpdateVehicleAccumulators(DetectorReportService_t *service,
                                      const IntersectionConfig_t *config,
                                      const IntersectionRuntime_t *runtime,
                                      uint8_t vehicleCollectionActive)
{
  uint8_t detectorIndex;

  if ((service == NULL) || (config == NULL) || (runtime == NULL))
  {
    return;
  }

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       ++detectorIndex)
  {
    uint8_t recognitionActive =
      runtime->vehicleDetectors[detectorIndex].recognitionActive;
    DetectorReportVehicleAccumulator_t *accumulator =
      &service->vehicleAccumulators[detectorIndex];

    if ((vehicleCollectionActive != 0U)
        && (VehicleRowIsActive(service, detectorIndex) != 0U))
    {
      if (recognitionActive != 0U)
      {
        accumulator->occupancyTicks++;
      }

      if ((recognitionActive != 0U)
          && (accumulator->previousRecognitionActive == 0U))
      {
        accumulator->volumeCount++;
      }
    }

    accumulator->previousRecognitionActive = recognitionActive;
  }
}

static void UpdatePedestrianAccumulators(DetectorReportService_t *service,
                                         const IntersectionConfig_t *config,
                                         const IntersectionRuntime_t *runtime,
                                         uint8_t pedCollectionActive)
{
  uint8_t detectorIndex;

  if ((service == NULL) || (config == NULL) || (runtime == NULL))
  {
    return;
  }

  for (detectorIndex = 0U; detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       ++detectorIndex)
  {
    uint8_t active =
      (uint8_t) ((runtime->pedestrianDetectors[detectorIndex].inputActive != 0U)
                 || (runtime->pedestrianDetectors[detectorIndex].remoteActuation
                     != 0U));
    DetectorReportPedestrianAccumulator_t *accumulator =
      &service->pedestrianAccumulators[detectorIndex];

    if ((pedCollectionActive != 0U)
        && (PedestrianRowIsActive(service, detectorIndex) != 0U)
        && (active != 0U)
        && (accumulator->previousActive == 0U))
    {
      accumulator->actuationCount++;

      if ((config->pedestrianDetectors[detectorIndex].options
           & PED_DETECTOR_OPTIONS_PRESENCE) != 0U)
      {
        accumulator->volumeCount++;
      }
    }

    accumulator->previousActive = active;
  }
}

void DetectorReportServiceInit(DetectorReportService_t *service)
{
  uint8_t detectorIndex;

  if (service == NULL)
  {
    return;
  }

  memset(service, 0, sizeof(*service));

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       ++detectorIndex)
  {
    service->vehicleSamples[detectorIndex].averageSpeed =
      DETECTOR_REPORT_SPEED_INVALID;
  }
}

void DetectorReportServiceBind(DetectorReportService_t *service,
                               IntersectionEngine_t *engine,
                               IntersectionController_t *controller,
                               IRealtimeClockPort_t *rtcPort)
{
  if (service == NULL)
  {
    return;
  }

  service->engine = engine;
  service->controller = controller;
  service->rtcPort = rtcPort;
  DetectorReportServiceReset(service);
}

void DetectorReportServiceReset(DetectorReportService_t *service)
{
  uint8_t detectorIndex;

  if (service == NULL)
  {
    return;
  }

  service->activeVehiclePeriodSeconds = 0U;
  service->activePedestrianPeriodSeconds = 0U;
  service->vehicleSequence = 0U;
  service->pedestrianSequence = 0U;
  service->vehicleElapsedTicks = 0U;
  service->pedestrianElapsedTicks = 0U;
  service->vehicleSampleTimeSeconds = 0U;
  service->pedestrianSampleTimeSeconds = 0U;
  service->vehicleSampleDurationSeconds = 0U;
  service->pedestrianSampleDurationSeconds = 0U;
  memset(service->previousPhaseWalkActive, 0, sizeof(service->previousPhaseWalkActive));
  memset(service->vehicleAccumulators, 0, sizeof(service->vehicleAccumulators));
  memset(service->pedestrianAccumulators, 0, sizeof(service->pedestrianAccumulators));
  memset(service->vehicleSamples, 0, sizeof(service->vehicleSamples));
  memset(service->pedestrianSamples, 0, sizeof(service->pedestrianSamples));

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       ++detectorIndex)
  {
    service->vehicleSamples[detectorIndex].averageSpeed =
      DETECTOR_REPORT_SPEED_INVALID;
  }
}

void DetectorReportServiceStep(DetectorReportService_t *service)
{
  const IntersectionConfig_t *config = GetConfig(service);
  const IntersectionRuntime_t *runtime = GetRuntime(service);
  uint16_t vehiclePeriodSeconds;
  uint16_t pedPeriodSeconds;

  if ((service == NULL) || (config == NULL) || (runtime == NULL))
  {
    return;
  }

  vehiclePeriodSeconds = ResolveVehiclePeriodSeconds(service);
  pedPeriodSeconds = ResolvePedestrianPeriodSeconds(service);

  if (vehiclePeriodSeconds != service->activeVehiclePeriodSeconds)
  {
    service->activeVehiclePeriodSeconds = vehiclePeriodSeconds;
    service->vehicleElapsedTicks = 0U;
    ResetCurrentVehicleAccumulators(service);
  }

  if (pedPeriodSeconds != service->activePedestrianPeriodSeconds)
  {
    service->activePedestrianPeriodSeconds = pedPeriodSeconds;
    service->pedestrianElapsedTicks = 0U;
    ResetCurrentPedestrianAccumulators(service);
  }

  UpdatePhaseServiceCounts(service, config, runtime,
                           (uint8_t) (pedPeriodSeconds != 0U));
  UpdateVehicleAccumulators(service, config, runtime,
                            (uint8_t) (vehiclePeriodSeconds != 0U));
  UpdatePedestrianAccumulators(service, config, runtime,
                               (uint8_t) (pedPeriodSeconds != 0U));

  if (vehiclePeriodSeconds != 0U)
  {
    service->vehicleElapsedTicks++;

    if (service->vehicleElapsedTicks
        >= ((uint32_t) vehiclePeriodSeconds * 100U))
    {
      FinalizeVehicleSample(service, vehiclePeriodSeconds);
      service->vehicleElapsedTicks = 0U;
      ResetCurrentVehicleAccumulators(service);
    }
  }

  if (pedPeriodSeconds != 0U)
  {
    service->pedestrianElapsedTicks++;

    if (service->pedestrianElapsedTicks
        >= ((uint32_t) pedPeriodSeconds * 100U))
    {
      FinalizePedestrianSample(service, pedPeriodSeconds);
      service->pedestrianElapsedTicks = 0U;
      ResetCurrentPedestrianAccumulators(service);
    }
  }
}

uint8_t DetectorReportServiceGetVehicleSequence(
  const DetectorReportService_t *service,
  uint8_t *sequence)
{
  if ((service == NULL) || (sequence == NULL))
  {
    return 0U;
  }

  *sequence = service->vehicleSequence;

  return 1U;
}

uint8_t DetectorReportServiceGetVehicleActiveCount(
  const DetectorReportService_t *service,
  uint8_t *count)
{
  const IntersectionConfig_t *config = GetConfig(service);
  uint8_t detectorIndex;

  if ((config == NULL) || (count == NULL))
  {
    return 0U;
  }

  *count = 0U;

  for (detectorIndex = 0U;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       ++detectorIndex)
  {
    if (VehicleRowIsActive(service, detectorIndex) != 0U)
    {
      (*count)++;
    }
  }

  return 1U;
}

uint8_t DetectorReportServiceGetVehicleSampleTimeSeconds(
  const DetectorReportService_t *service,
  uint32_t *sampleTimeSeconds)
{
  if ((service == NULL) || (sampleTimeSeconds == NULL))
  {
    return 0U;
  }

  *sampleTimeSeconds = service->vehicleSampleTimeSeconds;

  return 1U;
}

uint8_t DetectorReportServiceGetVehicleSampleDurationSeconds(
  const DetectorReportService_t *service,
  uint16_t *sampleDurationSeconds)
{
  if ((service == NULL) || (sampleDurationSeconds == NULL))
  {
    return 0U;
  }

  *sampleDurationSeconds = service->vehicleSampleDurationSeconds;

  return 1U;
}

uint8_t DetectorReportServiceGetVehicleSample(
  const DetectorReportService_t *service,
  uint8_t detectorNumber,
  DetectorReportVehicleSample_t *sample)
{
  uint8_t detectorIndex;

  if ((service == NULL) || (sample == NULL) || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  detectorIndex = (uint8_t) (detectorNumber - 1U);

  if (VehicleRowIsActive(service, detectorIndex) == 0U)
  {
    return 0U;
  }

  *sample = service->vehicleSamples[detectorIndex];

  return 1U;
}

uint8_t DetectorReportServiceGetPedestrianSequence(
  const DetectorReportService_t *service,
  uint8_t *sequence)
{
  if ((service == NULL) || (sequence == NULL))
  {
    return 0U;
  }

  *sequence = service->pedestrianSequence;

  return 1U;
}

uint8_t DetectorReportServiceGetPedestrianActiveCount(
  const DetectorReportService_t *service,
  uint8_t *count)
{
  const IntersectionConfig_t *config = GetConfig(service);
  uint8_t detectorIndex;

  if ((config == NULL) || (count == NULL))
  {
    return 0U;
  }

  *count = 0U;

  for (detectorIndex = 0U; detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       ++detectorIndex)
  {
    if (PedestrianRowIsActive(service, detectorIndex) != 0U)
    {
      (*count)++;
    }
  }

  return 1U;
}

uint8_t DetectorReportServiceGetPedestrianSampleTimeSeconds(
  const DetectorReportService_t *service,
  uint32_t *sampleTimeSeconds)
{
  if ((service == NULL) || (sampleTimeSeconds == NULL))
  {
    return 0U;
  }

  *sampleTimeSeconds = service->pedestrianSampleTimeSeconds;

  return 1U;
}

uint8_t DetectorReportServiceGetPedestrianSampleDurationSeconds(
  const DetectorReportService_t *service,
  uint16_t *sampleDurationSeconds)
{
  if ((service == NULL) || (sampleDurationSeconds == NULL))
  {
    return 0U;
  }

  *sampleDurationSeconds = service->pedestrianSampleDurationSeconds;

  return 1U;
}

uint8_t DetectorReportServiceGetPedestrianSample(
  const DetectorReportService_t *service,
  uint8_t detectorNumber,
  DetectorReportPedestrianSample_t *sample)
{
  uint8_t detectorIndex;

  if ((service == NULL) || (sample == NULL) || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  detectorIndex = (uint8_t) (detectorNumber - 1U);

  if (PedestrianRowIsActive(service, detectorIndex) == 0U)
  {
    return 0U;
  }

  *sample = service->pedestrianSamples[detectorIndex];

  return 1U;
}
