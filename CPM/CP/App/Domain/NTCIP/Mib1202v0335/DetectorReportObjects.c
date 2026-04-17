/* App/Domain/NTCIP/Mib1202v0335/DetectorReportObjects.c
 *
 * 1202 detector report groups (volume/occupancy and pedestrian sample
 * reporting) backed by DetectorReportService and canonical config.
 */
#include "DetectorReportObjects.h"

#include <stddef.h>

enum
{
  DETECTOR_REPORT_OBJECT_TAG_VEHICLE_SEQUENCE = 1,
  DETECTOR_REPORT_OBJECT_TAG_VEHICLE_PERIOD,
  DETECTOR_REPORT_OBJECT_TAG_VEHICLE_ACTIVE_COUNT,
  DETECTOR_REPORT_OBJECT_TAG_VEHICLE_VOLUME,
  DETECTOR_REPORT_OBJECT_TAG_VEHICLE_OCCUPANCY,
  DETECTOR_REPORT_OBJECT_TAG_VEHICLE_AVG_SPEED,
  DETECTOR_REPORT_OBJECT_TAG_VEHICLE_PERIOD_V3,
  DETECTOR_REPORT_OBJECT_TAG_VEHICLE_SAMPLE_TIME,
  DETECTOR_REPORT_OBJECT_TAG_VEHICLE_SAMPLE_DURATION,
  DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SEQUENCE,
  DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_PERIOD,
  DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_ACTIVE_COUNT,
  DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_VOLUME,
  DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_ACTUATIONS,
  DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SERVICES,
  DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SAMPLE_TIME,
  DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SAMPLE_DURATION
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
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 4U, 1U, 1U
};
static const uint32_t kDetectorOccupancyOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 4U, 1U, 2U
};
static const uint32_t kDetectorAvgSpeedOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 4U, 1U, 3U
};
static const uint32_t kVolumeOccupancyPeriodV3Oid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 5U, 0U
};
static const uint32_t kDetectorSampleTimeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 5U, 6U, 0U
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
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 4U, 1U, 1U
};
static const uint32_t kPedestrianDetectorActuationsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 4U, 1U, 2U
};
static const uint32_t kPedestrianDetectorServicesOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 4U, 1U, 3U
};
static const uint32_t kPedestrianDetectorSampleTimeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 5U, 0U
};
static const uint32_t kPedestrianDetectorSampleDurationOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 2U, 10U, 6U, 0U
};

static NtcipError_t ValidateDatabaseWrite(const NtcipContext_t *context,
                                          const NtcipRequestContext_t *
                                          requestContext)
{
  if ((context == NULL) || (context->dbTransactionService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return NtcipDbTransactionServiceValidateDatabaseWrite(
    context->dbTransactionService,
    requestContext);
}

static NtcipError_t GetVehicleRowSample(const NtcipContext_t *context,
                                        const uint32_t *indexes,
                                        uint8_t indexCount,
                                        DetectorReportVehicleSample_t *sample)
{
  if ((context == NULL) || (sample == NULL) || (indexes == NULL)
      || (indexCount != 1U) || (indexes[0] == 0U)
      || (context->detectorReportService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (DetectorReportServiceGetVehicleSample(context->detectorReportService,
                                            (uint8_t) indexes[0],
                                            sample) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetPedestrianRowSample(const NtcipContext_t *context,
                                           const uint32_t *indexes,
                                           uint8_t indexCount,
                                           DetectorReportPedestrianSample_t *
                                           sample)
{
  if ((context == NULL) || (sample == NULL) || (indexes == NULL)
      || (indexCount != 1U) || (indexes[0] == 0U)
      || (context->detectorReportService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (DetectorReportServiceGetPedestrianSample(context->detectorReportService,
                                               (uint8_t) indexes[0],
                                               sample) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetDetectorReportObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  DetectorReportVehicleSample_t vehicleSample;
  DetectorReportPedestrianSample_t pedestrianSample;
  uint8_t u8Value = 0U;
  uint16_t u16Value = 0U;
  uint32_t u32Value = 0U;
  NtcipError_t error = NTCIP_ERROR_OK;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL)
      || (context->configurationService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_VOLUME:
      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_OCCUPANCY:
      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_AVG_SPEED:
      {
        error = GetVehicleRowSample(context, indexes, indexCount, &vehicleSample);
        break;
      }

      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_VOLUME:
      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_ACTUATIONS:
      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SERVICES:
      {
        error = GetPedestrianRowSample(context,
                                       indexes,
                                       indexCount,
                                       &pedestrianSample);
        break;
      }

      default:
      {
        break;
      }
  }

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_SEQUENCE:
      {
        if ((context->detectorReportService == NULL)
            || (DetectorReportServiceGetVehicleSequence(
                  context->detectorReportService,
                  &u8Value) == 0U))
        {
          u8Value = 0U;
        }

        NtcipValueSetUnsigned32(value, u8Value);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_PERIOD:
      {
        IntersectionDetectorReportConfig_t detectorReports;

        if (ConfigurationServiceGetActiveDetectorReportConfig(
              context->configurationService,
              &detectorReports) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value,
                                detectorReports.volumeOccupancyPeriodSeconds);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_ACTIVE_COUNT:
      {
        if ((context->detectorReportService == NULL)
            || (DetectorReportServiceGetVehicleActiveCount(
                  context->detectorReportService,
                  &u8Value) == 0U))
        {
          u8Value = 0U;
        }

        NtcipValueSetUnsigned32(value, u8Value);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_VOLUME:
      {
        NtcipValueSetUnsigned32(value, vehicleSample.volume);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_OCCUPANCY:
      {
        NtcipValueSetUnsigned32(value, vehicleSample.occupancy);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_AVG_SPEED:
      {
        NtcipValueSetUnsigned32(value, vehicleSample.averageSpeed);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_PERIOD_V3:
      {
        IntersectionDetectorReportConfig_t detectorReports;

        if (ConfigurationServiceGetActiveDetectorReportConfig(
              context->configurationService,
              &detectorReports) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value,
                                detectorReports.volumeOccupancyPeriodV3Seconds);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_SAMPLE_TIME:
      {
        if ((context->detectorReportService == NULL)
            || (DetectorReportServiceGetVehicleSampleTimeSeconds(
                  context->detectorReportService,
                  &u32Value) == 0U))
        {
          u32Value = 0U;
        }

        NtcipValueSetUnsigned32(value, u32Value);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_SAMPLE_DURATION:
      {
        if ((context->detectorReportService == NULL)
            || (DetectorReportServiceGetVehicleSampleDurationSeconds(
                  context->detectorReportService,
                  &u16Value) == 0U))
        {
          u16Value = 0U;
        }

        NtcipValueSetUnsigned32(value, u16Value);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SEQUENCE:
      {
        if ((context->detectorReportService == NULL)
            || (DetectorReportServiceGetPedestrianSequence(
                  context->detectorReportService,
                  &u8Value) == 0U))
        {
          u8Value = 0U;
        }

        NtcipValueSetUnsigned32(value, u8Value);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_PERIOD:
      {
        IntersectionDetectorReportConfig_t detectorReports;

        if (ConfigurationServiceGetActiveDetectorReportConfig(
              context->configurationService,
              &detectorReports) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(
          value,
          detectorReports.pedestrianDetectorPeriodSeconds);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_ACTIVE_COUNT:
      {
        if ((context->detectorReportService == NULL)
            || (DetectorReportServiceGetPedestrianActiveCount(
                  context->detectorReportService,
                  &u8Value) == 0U))
        {
          u8Value = 0U;
        }

        NtcipValueSetUnsigned32(value, u8Value);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_VOLUME:
      {
        NtcipValueSetUnsigned32(value, pedestrianSample.volume);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_ACTUATIONS:
      {
        NtcipValueSetUnsigned32(value, pedestrianSample.actuations);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SERVICES:
      {
        NtcipValueSetUnsigned32(value, pedestrianSample.services);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SAMPLE_TIME:
      {
        if ((context->detectorReportService == NULL)
            || (DetectorReportServiceGetPedestrianSampleTimeSeconds(
                  context->detectorReportService,
                  &u32Value) == 0U))
        {
          u32Value = 0U;
        }

        NtcipValueSetUnsigned32(value, u32Value);
        return NTCIP_ERROR_OK;
      }

      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SAMPLE_DURATION:
      {
        if ((context->detectorReportService == NULL)
            || (DetectorReportServiceGetPedestrianSampleDurationSeconds(
                  context->detectorReportService,
                  &u16Value) == 0U))
        {
          u16Value = 0U;
        }

        NtcipValueSetUnsigned32(value, u16Value);
        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetTestDetectorReportObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;

  (void) indexes;
  (void) indexCount;

  if ((context == NULL) || (descriptor == NULL) || (requestContext == NULL)
      || (value == NULL) || (context->configurationService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (value->type != NTCIP_VALUE_TYPE_UNSIGNED32)
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (ValidateDatabaseWrite(context, requestContext) != NTCIP_ERROR_OK)
  {
    return ValidateDatabaseWrite(context, requestContext);
  }

  switch (descriptor->tag)
  {
      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_PERIOD:
      {
        return (value->data.unsigned32 <= 255U) ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_PERIOD_V3:
      {
        return ((value->data.unsigned32 <= 3600U)
                || (value->data.unsigned32 == 65535U))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_PERIOD:
      {
        return ((value->data.unsigned32 <= 3600U)
                || (value->data.unsigned32 == 65534U)
                || (value->data.unsigned32 == 65535U))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetValueDetectorReportObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;

  (void) indexes;
  (void) indexCount;

  if ((context == NULL) || (descriptor == NULL) || (requestContext == NULL)
      || (value == NULL) || (context->configurationService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_PERIOD:
      {
        return (ConfigurationServiceSetVolumeOccupancyPeriodSeconds(
                  context->configurationService,
                  (uint8_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_REPORT_OBJECT_TAG_VEHICLE_PERIOD_V3:
      {
        return (ConfigurationServiceSetVolumeOccupancyPeriodV3Seconds(
                  context->configurationService,
                  (uint16_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      case DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_PERIOD:
      {
        return (ConfigurationServiceSetPedestrianDetectorPeriodSeconds(
                  context->configurationService,
                  (uint16_t) value->data.unsigned32) != 0U)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_GEN_ERROR;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static const NtcipObjectDescriptor_t kDetectorReportObjects[] = {
  { kVolumeOccupancySequenceOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_VEHICLE_SEQUENCE, GetDetectorReportObject, NULL,
    NULL },
  { kVolumeOccupancyPeriodOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_VEHICLE_PERIOD, GetDetectorReportObject,
    SetTestDetectorReportObject, SetValueDetectorReportObject },
  { kActiveVolumeOccupancyDetectorsOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_VEHICLE_ACTIVE_COUNT, GetDetectorReportObject,
    NULL, NULL },
  { kDetectorVolumeOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_VEHICLE_VOLUME, GetDetectorReportObject, NULL,
    NULL },
  { kDetectorOccupancyOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_VEHICLE_OCCUPANCY, GetDetectorReportObject,
    NULL, NULL },
  { kDetectorAvgSpeedOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_VEHICLE_AVG_SPEED, GetDetectorReportObject,
    NULL, NULL },
  { kVolumeOccupancyPeriodV3Oid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_VEHICLE_PERIOD_V3, GetDetectorReportObject,
    SetTestDetectorReportObject, SetValueDetectorReportObject },
  { kDetectorSampleTimeOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_VEHICLE_SAMPLE_TIME, GetDetectorReportObject,
    NULL, NULL },
  { kDetectorSampleDurationOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_VEHICLE_SAMPLE_DURATION, GetDetectorReportObject,
    NULL, NULL },
  { kPedestrianDetectorSequenceOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SEQUENCE, GetDetectorReportObject,
    NULL, NULL },
  { kPedestrianDetectorPeriodOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_PERIOD, GetDetectorReportObject,
    SetTestDetectorReportObject, SetValueDetectorReportObject },
  { kActivePedestrianDetectorsOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_ACTIVE_COUNT, GetDetectorReportObject,
    NULL, NULL },
  { kPedestrianDetectorVolumeOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_VOLUME, GetDetectorReportObject,
    NULL, NULL },
  { kPedestrianDetectorActuationsOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_ACTUATIONS, GetDetectorReportObject,
    NULL, NULL },
  { kPedestrianDetectorServicesOid, 15U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SERVICES, GetDetectorReportObject,
    NULL, NULL },
  { kPedestrianDetectorSampleTimeOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SAMPLE_TIME, GetDetectorReportObject,
    NULL, NULL },
  { kPedestrianDetectorSampleDurationOid, 13U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    DETECTOR_REPORT_OBJECT_TAG_PEDESTRIAN_SAMPLE_DURATION,
    GetDetectorReportObject, NULL, NULL }
};

void DetectorReportObjectsRegister(NtcipObjectDirectory_t *directory,
                                   NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.detectorReport",
    kDetectorReportObjects,
    (uint16_t) (sizeof(kDetectorReportObjects)
                / sizeof(kDetectorReportObjects[0])),
    context);
}
