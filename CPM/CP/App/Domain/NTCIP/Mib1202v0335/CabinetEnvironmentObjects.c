/* App/Domain/NTCIP/Mib1202v0335/CabinetEnvironmentObjects.c
 *
 * Canonical 1202 cabinetEnvironment subtree backed by transactional cabinet
 * configuration plus live cabinet status ports.
 */
#include "CabinetEnvironmentObjects.h"

#include <stddef.h>

enum
{
  CABINET_ENV_TAG_MAX_ENVIRON_DEVICES = 1,
  CABINET_ENV_TAG_ENVIRON_DEVICE_NUMBER,
  CABINET_ENV_TAG_ENVIRON_DEVICE_TYPE,
  CABINET_ENV_TAG_ENVIRON_DEVICE_INDEX,
  CABINET_ENV_TAG_ENVIRON_DEVICE_DESCRIPTION,
  CABINET_ENV_TAG_ENVIRON_DEVICE_ON_STATUS,
  CABINET_ENV_TAG_ENVIRON_DEVICE_ERROR_STATUS,
  CABINET_ENV_TAG_MAX_TEMP_SENSORS,
  CABINET_ENV_TAG_TEMP_SENSOR_INDEX,
  CABINET_ENV_TAG_TEMP_SENSOR_DESCRIPTION,
  CABINET_ENV_TAG_TEMP_SENSOR_CURRENT_READING,
  CABINET_ENV_TAG_TEMP_SENSOR_HIGH_THRESHOLD,
  CABINET_ENV_TAG_TEMP_SENSOR_LOW_THRESHOLD,
  CABINET_ENV_TAG_TEMP_SENSOR_STATUS,
  CABINET_ENV_TAG_MAX_HUMIDITY_SENSORS,
  CABINET_ENV_TAG_HUMIDITY_SENSOR_INDEX,
  CABINET_ENV_TAG_HUMIDITY_SENSOR_DESCRIPTION,
  CABINET_ENV_TAG_HUMIDITY_SENSOR_CURRENT_READING,
  CABINET_ENV_TAG_HUMIDITY_SENSOR_THRESHOLD,
  CABINET_ENV_TAG_HUMIDITY_SENSOR_STATUS,
  CABINET_ENV_TAG_POWER_SOURCE,
  CABINET_ENV_TAG_LINE_VOLTS,
  CABINET_ENV_TAG_LED_MODE
};

enum
{
  CABINET_ENV_DEVICE_STATUS_TRUE = 1,
  CABINET_ENV_DEVICE_STATUS_FALSE = 2
};

enum
{
  CABINET_ENV_DEVICE_ERROR_OTHER = 1,
  CABINET_ENV_DEVICE_ERROR_NO_ERROR = 2,
  CABINET_ENV_DEVICE_ERROR_FAIL = 3,
  CABINET_ENV_DEVICE_ERROR_NOT_MONITORED = 4
};

enum
{
  CABINET_TEMP_SENSOR_STATUS_OTHER = 1,
  CABINET_TEMP_SENSOR_STATUS_NO_ERROR = 2,
  CABINET_TEMP_SENSOR_STATUS_FAIL = 3
};

enum
{
  CABINET_HUMIDITY_SENSOR_STATUS_OTHER = 1,
  CABINET_HUMIDITY_SENSOR_STATUS_NO_ERROR = 2,
  CABINET_HUMIDITY_SENSOR_STATUS_FAIL = 3
};

enum
{
  CABINET_ENV_POWER_SOURCE_UNKNOWN = 1,
  CABINET_ENV_POWER_SOURCE_OTHER = 2,
  CABINET_ENV_POWER_SOURCE_AC_LINE = 3
};

static const uint32_t kMaxCabinetEnvironDevicesOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 1U
};
static const uint32_t kCabinetEnvironDeviceNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 2U, 1U, 1U
};
static const uint32_t kCabinetEnvironDeviceTypeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 2U, 1U, 2U
};
static const uint32_t kCabinetEnvironDeviceIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 2U, 1U, 3U
};
static const uint32_t kCabinetEnvironDeviceDescriptionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 2U, 1U, 4U
};
static const uint32_t kCabinetEnvironDeviceOnStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 2U, 1U, 5U
};
static const uint32_t kCabinetEnvironDeviceErrorStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 2U, 1U, 6U
};
static const uint32_t kMaxCabinetTempSensorsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 3U
};
static const uint32_t kCabinetTempSensorIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 4U, 1U, 1U
};
static const uint32_t kCabinetTempSensorDescriptionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 4U, 1U, 2U
};
static const uint32_t kCabinetTempSensorCurrentReadingOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 4U, 1U, 3U
};
static const uint32_t kCabinetTempSensorHighThresholdOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 4U, 1U, 4U
};
static const uint32_t kCabinetTempSensorLowThresholdOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 4U, 1U, 5U
};
static const uint32_t kCabinetTempSensorStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 4U, 1U, 6U
};
static const uint32_t kMaxCabinetHumiditySensorsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 5U
};
static const uint32_t kCabinetHumiditySensorIndexOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 6U, 1U, 1U
};
static const uint32_t kCabinetHumiditySensorDescriptionOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 6U, 1U, 2U
};
static const uint32_t kCabinetHumiditySensorCurrentReadingOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 6U, 1U, 3U
};
static const uint32_t kCabinetHumidityThresholdOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 6U, 1U, 4U
};
static const uint32_t kCabinetHumiditySensorStatusOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 6U, 1U, 5U
};
static const uint32_t kAscPowerSourceOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 7U
};
static const uint32_t kAscLineVoltsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 8U
};
static const uint32_t kAtccLedModeOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 12U, 9U
};

static NtcipError_t ValidateDatabaseWrite(const NtcipContext_t *context,
                                          const NtcipRequestContext_t *request)
{
  if ((context == NULL) || (context->dbTransactionService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return NtcipDbTransactionServiceValidateDatabaseWrite(
    context->dbTransactionService,
    request);
}

static uint16_t DescriptionLength(const uint8_t *description)
{
  uint16_t length = 0U;

  if (description == NULL)
  {
    return 0U;
  }

  while ((length < INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX)
         && (description[length] != 0U))
  {
    length++;
  }

  return length;
}

static void SetDescription(uint8_t *target, const NtcipValue_t *value)
{
  uint16_t index;

  if ((target == NULL) || (value == NULL))
  {
    return;
  }

  for (index = 0U; index < INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX;
       index++)
  {
    target[index] = 0U;
  }

  for (index = 0U;
       (index < value->data.octetString.length)
       && (index < INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX);
       index++)
  {
    target[index] = value->data.octetString.bytes[index];
  }
}

static NtcipError_t GetCabinetEnvironmentConfig(
  const NtcipContext_t *context,
  IntersectionCabinetEnvironmentConfig_t *cabinetEnvironment)
{
  if ((context == NULL) || (cabinetEnvironment == NULL)
      || (context->configurationService == NULL)
      || (ConfigurationServiceGetActiveCabinetEnvironmentConfig(
            context->configurationService,
            cabinetEnvironment) == 0U))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetCandidateCabinetEnvironmentConfig(
  NtcipContext_t *context,
  IntersectionCabinetEnvironmentConfig_t *cabinetEnvironment)
{
  if ((context == NULL) || (cabinetEnvironment == NULL)
      || (context->configurationService == NULL)
      || (ConfigurationServiceGetCandidateCabinetEnvironmentConfig(
            context->configurationService,
            cabinetEnvironment) == 0U))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t SaveCandidateCabinetEnvironmentConfig(
  NtcipContext_t *context,
  const IntersectionCabinetEnvironmentConfig_t *cabinetEnvironment)
{
  if ((context == NULL) || (cabinetEnvironment == NULL)
      || (context->configurationService == NULL)
      || (ConfigurationServiceSetCabinetEnvironmentConfig(
            context->configurationService,
            cabinetEnvironment) == 0U))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetEnvironmentDeviceEntry(const uint32_t *indexes,
                                              uint8_t indexCount,
                                              uint8_t *deviceIndex)
{
  if ((indexes == NULL) || (indexCount != 2U) || (indexes[0] == 0U)
      || (indexes[0] > INTERSECTION_CABINET_ENVIRONMENT_DEVICE_COUNT_MAX)
      || (indexes[1] != 1U) || (deviceIndex == NULL))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *deviceIndex = (uint8_t) (indexes[0] - 1U);

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetTempSensorEntry(const uint32_t *indexes,
                                       uint8_t indexCount,
                                       uint8_t *sensorIndex)
{
  if ((indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (indexes[0] > INTERSECTION_CABINET_TEMP_SENSOR_COUNT_MAX)
      || (sensorIndex == NULL))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *sensorIndex = (uint8_t) (indexes[0] - 1U);

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetHumiditySensorEntry(const uint32_t *indexes,
                                           uint8_t indexCount,
                                           uint8_t *sensorIndex)
{
  if ((indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (indexes[0] > INTERSECTION_CABINET_HUMIDITY_SENSOR_COUNT_MAX)
      || (sensorIndex == NULL))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *sensorIndex = (uint8_t) (indexes[0] - 1U);

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetEnvironmentDeviceLiveState(const NtcipContext_t *context,
                                                  uint8_t deviceIndex,
                                                  uint32_t *onStatus,
                                                  uint32_t *errorStatus)
{
  if ((context == NULL) || (onStatus == NULL) || (errorStatus == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (deviceIndex)
  {
      case 0U:
      {
        if (context->doorSensorPort == NULL)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        *onStatus = (DoorSensorIsOpen(context->doorSensorPort) != 0U)
                    ? CABINET_ENV_DEVICE_STATUS_TRUE
                    : CABINET_ENV_DEVICE_STATUS_FALSE;
        *errorStatus = CABINET_ENV_DEVICE_ERROR_NO_ERROR;

        return NTCIP_ERROR_OK;
      }

      case 1U:
      {
        *onStatus = CABINET_ENV_DEVICE_STATUS_FALSE;
        *errorStatus = CABINET_ENV_DEVICE_ERROR_NOT_MONITORED;

        return NTCIP_ERROR_OK;
      }

      case 2U:
      {
        if (context->heaterPort == NULL)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        *onStatus = (HeaterGetState(context->heaterPort) != 0U)
                    ? CABINET_ENV_DEVICE_STATUS_TRUE
                    : CABINET_ENV_DEVICE_STATUS_FALSE;
        *errorStatus = CABINET_ENV_DEVICE_ERROR_NOT_MONITORED;

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_RANGE_ERROR;
      }
  }
}

static NtcipError_t GetCabinetEnvironmentObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionCabinetEnvironmentConfig_t cabinetEnvironment;
  uint8_t objectIndex = 0U;
  uint16_t lineVoltage = 0U;
  uint8_t powerSource = CABINET_ENV_POWER_SOURCE_UNKNOWN;
  uint32_t onStatus = 0U;
  uint32_t errorStatus = 0U;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case CABINET_ENV_TAG_MAX_ENVIRON_DEVICES:
      {
        NtcipValueSetUnsigned32(value,
                                INTERSECTION_CABINET_ENVIRONMENT_DEVICE_COUNT_MAX);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_MAX_TEMP_SENSORS:
      {
        NtcipValueSetUnsigned32(value, INTERSECTION_CABINET_TEMP_SENSOR_COUNT_MAX);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_MAX_HUMIDITY_SENSORS:
      {
        NtcipValueSetUnsigned32(value,
                                INTERSECTION_CABINET_HUMIDITY_SENSOR_COUNT_MAX);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_POWER_SOURCE:
      {
        if ((context->powerMonitorPort == NULL)
            || (PowerMonitorGetPrimarySource(context->powerMonitorPort,
                                             &powerSource) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, powerSource);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_LINE_VOLTS:
      {
        if ((context->powerMonitorPort == NULL)
            || (PowerMonitorGetLineVoltageTenthsVrms(context->powerMonitorPort,
                                                     &lineVoltage) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        NtcipValueSetUnsigned32(value, lineVoltage);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        break;
      }
  }

  error = GetCabinetEnvironmentConfig(context, &cabinetEnvironment);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case CABINET_ENV_TAG_ENVIRON_DEVICE_NUMBER:
      case CABINET_ENV_TAG_ENVIRON_DEVICE_TYPE:
      case CABINET_ENV_TAG_ENVIRON_DEVICE_INDEX:
      case CABINET_ENV_TAG_ENVIRON_DEVICE_DESCRIPTION:
      case CABINET_ENV_TAG_ENVIRON_DEVICE_ON_STATUS:
      case CABINET_ENV_TAG_ENVIRON_DEVICE_ERROR_STATUS:
      {
        error = GetEnvironmentDeviceEntry(indexes, indexCount, &objectIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        break;
      }

      case CABINET_ENV_TAG_TEMP_SENSOR_INDEX:
      case CABINET_ENV_TAG_TEMP_SENSOR_DESCRIPTION:
      case CABINET_ENV_TAG_TEMP_SENSOR_CURRENT_READING:
      case CABINET_ENV_TAG_TEMP_SENSOR_HIGH_THRESHOLD:
      case CABINET_ENV_TAG_TEMP_SENSOR_LOW_THRESHOLD:
      case CABINET_ENV_TAG_TEMP_SENSOR_STATUS:
      {
        error = GetTempSensorEntry(indexes, indexCount, &objectIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        break;
      }

      case CABINET_ENV_TAG_HUMIDITY_SENSOR_INDEX:
      case CABINET_ENV_TAG_HUMIDITY_SENSOR_DESCRIPTION:
      case CABINET_ENV_TAG_HUMIDITY_SENSOR_CURRENT_READING:
      case CABINET_ENV_TAG_HUMIDITY_SENSOR_THRESHOLD:
      case CABINET_ENV_TAG_HUMIDITY_SENSOR_STATUS:
      {
        error = GetHumiditySensorEntry(indexes, indexCount, &objectIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        break;
      }

      case CABINET_ENV_TAG_LED_MODE:
      {
        NtcipValueSetUnsigned32(value, cabinetEnvironment.atccLedMode);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }

  switch (descriptor->tag)
  {
      case CABINET_ENV_TAG_ENVIRON_DEVICE_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_ENVIRON_DEVICE_TYPE:
      {
        NtcipValueSetUnsigned32(value,
                                cabinetEnvironment.devices[objectIndex].type);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_ENVIRON_DEVICE_INDEX:
      {
        NtcipValueSetUnsigned32(value, 1U);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_ENVIRON_DEVICE_DESCRIPTION:
      {
        return NtcipValueSetOctetString(
          value,
          cabinetEnvironment.devices[objectIndex].description,
          DescriptionLength(cabinetEnvironment.devices[objectIndex].description));
      }

      case CABINET_ENV_TAG_ENVIRON_DEVICE_ON_STATUS:
      {
        error = GetEnvironmentDeviceLiveState(context,
                                              objectIndex,
                                              &onStatus,
                                              &errorStatus);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        NtcipValueSetUnsigned32(value, onStatus);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_ENVIRON_DEVICE_ERROR_STATUS:
      {
        error = GetEnvironmentDeviceLiveState(context,
                                              objectIndex,
                                              &onStatus,
                                              &errorStatus);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        NtcipValueSetUnsigned32(value, errorStatus);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_TEMP_SENSOR_INDEX:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_TEMP_SENSOR_DESCRIPTION:
      {
        return NtcipValueSetOctetString(
          value,
          cabinetEnvironment.temperatureSensors[objectIndex].description,
          DescriptionLength(
            cabinetEnvironment.temperatureSensors[objectIndex].description));
      }

      case CABINET_ENV_TAG_TEMP_SENSOR_CURRENT_READING:
      {
        NtcipValueSetSigned32(value, -128);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_TEMP_SENSOR_HIGH_THRESHOLD:
      {
        NtcipValueSetSigned32(
          value,
          cabinetEnvironment.temperatureSensors[objectIndex].highThreshold);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_TEMP_SENSOR_LOW_THRESHOLD:
      {
        NtcipValueSetSigned32(
          value,
          cabinetEnvironment.temperatureSensors[objectIndex].lowThreshold);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_TEMP_SENSOR_STATUS:
      {
        NtcipValueSetUnsigned32(value, CABINET_TEMP_SENSOR_STATUS_FAIL);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_HUMIDITY_SENSOR_INDEX:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_HUMIDITY_SENSOR_DESCRIPTION:
      {
        return NtcipValueSetOctetString(
          value,
          cabinetEnvironment.humiditySensors[objectIndex].description,
          DescriptionLength(
            cabinetEnvironment.humiditySensors[objectIndex].description));
      }

      case CABINET_ENV_TAG_HUMIDITY_SENSOR_CURRENT_READING:
      {
        NtcipValueSetUnsigned32(value, 101U);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_HUMIDITY_SENSOR_THRESHOLD:
      {
        NtcipValueSetUnsigned32(
          value,
          cabinetEnvironment.humiditySensors[objectIndex].threshold);

        return NTCIP_ERROR_OK;
      }

      case CABINET_ENV_TAG_HUMIDITY_SENSOR_STATUS:
      {
        NtcipValueSetUnsigned32(value, CABINET_HUMIDITY_SENSOR_STATUS_FAIL);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetTestCabinetEnvironmentObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionCabinetEnvironmentConfig_t cabinetEnvironment;
  uint8_t objectIndex = 0U;
  NtcipError_t error;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
      case CABINET_ENV_TAG_ENVIRON_DEVICE_TYPE:
      {
        if ((value->type != NTCIP_VALUE_TYPE_UNSIGNED32)
            || (GetEnvironmentDeviceEntry(indexes, indexCount, &objectIndex)
                != NTCIP_ERROR_OK)
            || (value->data.unsigned32 < 1U)
            || (value->data.unsigned32 > 5U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return ValidateDatabaseWrite(context, requestContext);
      }

      case CABINET_ENV_TAG_ENVIRON_DEVICE_DESCRIPTION:
      {
        if ((value->type != NTCIP_VALUE_TYPE_OCTET_STRING)
            || (GetEnvironmentDeviceEntry(indexes, indexCount, &objectIndex)
                != NTCIP_ERROR_OK)
            || (value->data.octetString.length
                > INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return ValidateDatabaseWrite(context, requestContext);
      }

      case CABINET_ENV_TAG_TEMP_SENSOR_DESCRIPTION:
      {
        if ((value->type != NTCIP_VALUE_TYPE_OCTET_STRING)
            || (GetTempSensorEntry(indexes, indexCount, &objectIndex)
                != NTCIP_ERROR_OK)
            || (value->data.octetString.length
                > INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return ValidateDatabaseWrite(context, requestContext);
      }

      case CABINET_ENV_TAG_TEMP_SENSOR_HIGH_THRESHOLD:
      case CABINET_ENV_TAG_TEMP_SENSOR_LOW_THRESHOLD:
      {
        if ((value->type != NTCIP_VALUE_TYPE_SIGNED32)
            || (GetTempSensorEntry(indexes, indexCount, &objectIndex)
                != NTCIP_ERROR_OK)
            || (value->data.signed32 < -128)
            || (value->data.signed32 > 127))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        error = GetCandidateCabinetEnvironmentConfig(context,
                                                     &cabinetEnvironment);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        if (((descriptor->tag == CABINET_ENV_TAG_TEMP_SENSOR_HIGH_THRESHOLD)
             && (value->data.signed32
                 < cabinetEnvironment.temperatureSensors[objectIndex]
                     .lowThreshold))
            || ((descriptor->tag == CABINET_ENV_TAG_TEMP_SENSOR_LOW_THRESHOLD)
                && (value->data.signed32
                    > cabinetEnvironment.temperatureSensors[objectIndex]
                        .highThreshold)))
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return ValidateDatabaseWrite(context, requestContext);
      }

      case CABINET_ENV_TAG_HUMIDITY_SENSOR_DESCRIPTION:
      {
        if ((value->type != NTCIP_VALUE_TYPE_OCTET_STRING)
            || (GetHumiditySensorEntry(indexes, indexCount, &objectIndex)
                != NTCIP_ERROR_OK)
            || (value->data.octetString.length
                > INTERSECTION_CABINET_ENVIRONMENT_DESCRIPTION_MAX))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return ValidateDatabaseWrite(context, requestContext);
      }

      case CABINET_ENV_TAG_HUMIDITY_SENSOR_THRESHOLD:
      {
        if ((value->type != NTCIP_VALUE_TYPE_UNSIGNED32)
            || (GetHumiditySensorEntry(indexes, indexCount, &objectIndex)
                != NTCIP_ERROR_OK)
            || (value->data.unsigned32 > 101U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return ValidateDatabaseWrite(context, requestContext);
      }

      case CABINET_ENV_TAG_LED_MODE:
      {
        if ((value->type != NTCIP_VALUE_TYPE_UNSIGNED32) || (indexCount != 0U)
            || (value->data.unsigned32 < 1U)
            || (value->data.unsigned32 > 3U))
        {
          return NTCIP_ERROR_RANGE_ERROR;
        }

        return ValidateDatabaseWrite(context, requestContext);
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }
}

static NtcipError_t SetValueCabinetEnvironmentObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionCabinetEnvironmentConfig_t cabinetEnvironment;
  uint8_t objectIndex = 0U;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  error = GetCandidateCabinetEnvironmentConfig(context, &cabinetEnvironment);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case CABINET_ENV_TAG_ENVIRON_DEVICE_TYPE:
      {
        error = GetEnvironmentDeviceEntry(indexes, indexCount, &objectIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        cabinetEnvironment.devices[objectIndex].type =
          (uint8_t) value->data.unsigned32;
        break;
      }

      case CABINET_ENV_TAG_ENVIRON_DEVICE_DESCRIPTION:
      {
        error = GetEnvironmentDeviceEntry(indexes, indexCount, &objectIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        SetDescription(cabinetEnvironment.devices[objectIndex].description, value);
        break;
      }

      case CABINET_ENV_TAG_TEMP_SENSOR_DESCRIPTION:
      {
        error = GetTempSensorEntry(indexes, indexCount, &objectIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        SetDescription(
          cabinetEnvironment.temperatureSensors[objectIndex].description,
          value);
        break;
      }

      case CABINET_ENV_TAG_TEMP_SENSOR_HIGH_THRESHOLD:
      {
        error = GetTempSensorEntry(indexes, indexCount, &objectIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        cabinetEnvironment.temperatureSensors[objectIndex].highThreshold =
          (int8_t) value->data.signed32;
        break;
      }

      case CABINET_ENV_TAG_TEMP_SENSOR_LOW_THRESHOLD:
      {
        error = GetTempSensorEntry(indexes, indexCount, &objectIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        cabinetEnvironment.temperatureSensors[objectIndex].lowThreshold =
          (int8_t) value->data.signed32;
        break;
      }

      case CABINET_ENV_TAG_HUMIDITY_SENSOR_DESCRIPTION:
      {
        error = GetHumiditySensorEntry(indexes, indexCount, &objectIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        SetDescription(cabinetEnvironment.humiditySensors[objectIndex]
                         .description,
                       value);
        break;
      }

      case CABINET_ENV_TAG_HUMIDITY_SENSOR_THRESHOLD:
      {
        error = GetHumiditySensorEntry(indexes, indexCount, &objectIndex);
        if (error != NTCIP_ERROR_OK)
        {
          return error;
        }

        cabinetEnvironment.humiditySensors[objectIndex].threshold =
          (uint8_t) value->data.unsigned32;
        break;
      }

      case CABINET_ENV_TAG_LED_MODE:
      {
        cabinetEnvironment.atccLedMode = (uint8_t) value->data.unsigned32;
        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }

  return SaveCandidateCabinetEnvironmentConfig(context, &cabinetEnvironment);
}

static const NtcipObjectDescriptor_t kCabinetEnvironmentObjects[] = {
  { kMaxCabinetEnvironDevicesOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_MAX_ENVIRON_DEVICES, GetCabinetEnvironmentObject, NULL,
    NULL },
  { kCabinetEnvironDeviceNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_ENVIRON_DEVICE_NUMBER, GetCabinetEnvironmentObject, NULL,
    NULL },
  { kCabinetEnvironDeviceTypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_ENVIRON_DEVICE_TYPE, GetCabinetEnvironmentObject,
    SetTestCabinetEnvironmentObject, SetValueCabinetEnvironmentObject },
  { kCabinetEnvironDeviceIndexOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_ENVIRON_DEVICE_INDEX, GetCabinetEnvironmentObject, NULL,
    NULL },
  { kCabinetEnvironDeviceDescriptionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN,
    2U, NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    CABINET_ENV_TAG_ENVIRON_DEVICE_DESCRIPTION, GetCabinetEnvironmentObject,
    SetTestCabinetEnvironmentObject, SetValueCabinetEnvironmentObject },
  { kCabinetEnvironDeviceOnStatusOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_ENVIRON_DEVICE_ON_STATUS, GetCabinetEnvironmentObject,
    NULL, NULL },
  { kCabinetEnvironDeviceErrorStatusOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN,
    2U, NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_ENVIRON_DEVICE_ERROR_STATUS, GetCabinetEnvironmentObject,
    NULL, NULL },
  { kMaxCabinetTempSensorsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_MAX_TEMP_SENSORS, GetCabinetEnvironmentObject, NULL, NULL },
  { kCabinetTempSensorIndexOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_TEMP_SENSOR_INDEX, GetCabinetEnvironmentObject, NULL,
    NULL },
  { kCabinetTempSensorDescriptionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    CABINET_ENV_TAG_TEMP_SENSOR_DESCRIPTION, GetCabinetEnvironmentObject,
    SetTestCabinetEnvironmentObject, SetValueCabinetEnvironmentObject },
  { kCabinetTempSensorCurrentReadingOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN,
    1U, NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_SIGNED32,
    CABINET_ENV_TAG_TEMP_SENSOR_CURRENT_READING, GetCabinetEnvironmentObject,
    NULL, NULL },
  { kCabinetTempSensorHighThresholdOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN,
    1U, NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_SIGNED32,
    CABINET_ENV_TAG_TEMP_SENSOR_HIGH_THRESHOLD, GetCabinetEnvironmentObject,
    SetTestCabinetEnvironmentObject, SetValueCabinetEnvironmentObject },
  { kCabinetTempSensorLowThresholdOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_SIGNED32,
    CABINET_ENV_TAG_TEMP_SENSOR_LOW_THRESHOLD, GetCabinetEnvironmentObject,
    SetTestCabinetEnvironmentObject, SetValueCabinetEnvironmentObject },
  { kCabinetTempSensorStatusOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_TEMP_SENSOR_STATUS, GetCabinetEnvironmentObject, NULL,
    NULL },
  { kMaxCabinetHumiditySensorsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_MAX_HUMIDITY_SENSORS, GetCabinetEnvironmentObject, NULL,
    NULL },
  { kCabinetHumiditySensorIndexOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_HUMIDITY_SENSOR_INDEX, GetCabinetEnvironmentObject, NULL,
    NULL },
  { kCabinetHumiditySensorDescriptionOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN,
    1U, NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    CABINET_ENV_TAG_HUMIDITY_SENSOR_DESCRIPTION, GetCabinetEnvironmentObject,
    SetTestCabinetEnvironmentObject, SetValueCabinetEnvironmentObject },
  { kCabinetHumiditySensorCurrentReadingOid, 14U,
    NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, CABINET_ENV_TAG_HUMIDITY_SENSOR_CURRENT_READING,
    GetCabinetEnvironmentObject, NULL, NULL },
  { kCabinetHumidityThresholdOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_HUMIDITY_SENSOR_THRESHOLD, GetCabinetEnvironmentObject,
    SetTestCabinetEnvironmentObject, SetValueCabinetEnvironmentObject },
  { kCabinetHumiditySensorStatusOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_HUMIDITY_SENSOR_STATUS, GetCabinetEnvironmentObject, NULL,
    NULL },
  { kAscPowerSourceOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_POWER_SOURCE, GetCabinetEnvironmentObject, NULL, NULL },
  { kAscLineVoltsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_LINE_VOLTS, GetCabinetEnvironmentObject, NULL, NULL },
  { kAtccLedModeOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    CABINET_ENV_TAG_LED_MODE, GetCabinetEnvironmentObject,
    SetTestCabinetEnvironmentObject, SetValueCabinetEnvironmentObject }
};

void CabinetEnvironmentObjectsRegister(NtcipObjectDirectory_t *directory,
                                       NtcipContext_t *context)
{
  NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.cabinetEnvironment",
    kCabinetEnvironmentObjects,
    (uint16_t) (sizeof(kCabinetEnvironmentObjects)
                / sizeof(kCabinetEnvironmentObjects[0])),
    context);
}
