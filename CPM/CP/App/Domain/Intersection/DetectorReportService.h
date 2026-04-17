/* App/Domain/Intersection/DetectorReportService.h
 *
 * Runtime detector and pedestrian sample reporting driven by the canonical
 * detector configuration, controller runtime, and RTC wall clock.
 */
#ifndef DETECTOR_REPORT_SERVICE_H
#define DETECTOR_REPORT_SERVICE_H

#include <stdint.h>

#include "Domain/Intersection/IntersectionController.h"
#include "Ports/IRealtimeClockPort.h"

typedef struct
{
  uint8_t volume;
  uint8_t occupancy;
  uint16_t averageSpeed;
} DetectorReportVehicleSample_t;

typedef struct
{
  uint8_t volume;
  uint8_t actuations;
  uint8_t services;
} DetectorReportPedestrianSample_t;

typedef struct
{
  uint32_t volumeCount;
  uint32_t occupancyTicks;
  uint8_t previousRecognitionActive;
  uint8_t reserved0;
  uint8_t reserved1;
  uint8_t reserved2;
} DetectorReportVehicleAccumulator_t;

typedef struct
{
  uint32_t volumeCount;
  uint32_t actuationCount;
  uint32_t serviceCount;
  uint8_t previousActive;
  uint8_t reserved0;
  uint8_t reserved1;
  uint8_t reserved2;
} DetectorReportPedestrianAccumulator_t;

typedef struct
{
  IntersectionEngine_t *engine;
  IntersectionController_t *controller;
  IRealtimeClockPort_t *rtcPort;
  uint16_t activeVehiclePeriodSeconds;
  uint16_t activePedestrianPeriodSeconds;
  uint8_t vehicleSequence;
  uint8_t pedestrianSequence;
  uint16_t reserved0;
  uint32_t vehicleElapsedTicks;
  uint32_t pedestrianElapsedTicks;
  uint32_t vehicleSampleTimeSeconds;
  uint32_t pedestrianSampleTimeSeconds;
  uint16_t vehicleSampleDurationSeconds;
  uint16_t pedestrianSampleDurationSeconds;
  uint8_t previousPhaseWalkActive[INTERSECTION_PHASE_COUNT_MAX];
  DetectorReportVehicleAccumulator_t vehicleAccumulators[
    INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX];
  DetectorReportPedestrianAccumulator_t pedestrianAccumulators[
    INTERSECTION_PED_INPUT_COUNT_MAX];
  DetectorReportVehicleSample_t vehicleSamples[
    INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX];
  DetectorReportPedestrianSample_t pedestrianSamples[
    INTERSECTION_PED_INPUT_COUNT_MAX];
} DetectorReportService_t;

void DetectorReportServiceInit(DetectorReportService_t *service);
void DetectorReportServiceBind(DetectorReportService_t *service,
                               IntersectionEngine_t *engine,
                               IntersectionController_t *controller,
                               IRealtimeClockPort_t *rtcPort);
void DetectorReportServiceReset(DetectorReportService_t *service);
void DetectorReportServiceStep(DetectorReportService_t *service);

uint8_t DetectorReportServiceGetVehicleSequence(
  const DetectorReportService_t *service,
  uint8_t *sequence);
uint8_t DetectorReportServiceGetVehicleActiveCount(
  const DetectorReportService_t *service,
  uint8_t *count);
uint8_t DetectorReportServiceGetVehicleSampleTimeSeconds(
  const DetectorReportService_t *service,
  uint32_t *sampleTimeSeconds);
uint8_t DetectorReportServiceGetVehicleSampleDurationSeconds(
  const DetectorReportService_t *service,
  uint16_t *sampleDurationSeconds);
uint8_t DetectorReportServiceGetVehicleSample(
  const DetectorReportService_t *service,
  uint8_t detectorNumber,
  DetectorReportVehicleSample_t *sample);

uint8_t DetectorReportServiceGetPedestrianSequence(
  const DetectorReportService_t *service,
  uint8_t *sequence);
uint8_t DetectorReportServiceGetPedestrianActiveCount(
  const DetectorReportService_t *service,
  uint8_t *count);
uint8_t DetectorReportServiceGetPedestrianSampleTimeSeconds(
  const DetectorReportService_t *service,
  uint32_t *sampleTimeSeconds);
uint8_t DetectorReportServiceGetPedestrianSampleDurationSeconds(
  const DetectorReportService_t *service,
  uint16_t *sampleDurationSeconds);
uint8_t DetectorReportServiceGetPedestrianSample(
  const DetectorReportService_t *service,
  uint8_t detectorNumber,
  DetectorReportPedestrianSample_t *sample);

#endif /* DETECTOR_REPORT_SERVICE_H */
