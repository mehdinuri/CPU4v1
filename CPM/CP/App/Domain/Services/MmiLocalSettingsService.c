/* App/Domain/Services/MmiLocalSettingsService.c */
#include "MmiLocalSettingsService.h"

#include <string.h>

static uint8_t IsLeapYear(uint16_t year)
{
  return (uint8_t) (((year % 4U) == 0U)
                    && (((year % 100U) != 0U) || ((year % 400U) == 0U)));
}

static uint8_t DaysInMonth(uint16_t year, uint8_t month)
{
  static const uint8_t kMonthDays[12] = {
    31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U
  };

  if ((month == 0U) || (month > 12U))
  {
    return 0U;
  }

  if ((month == 2U) && (IsLeapYear(year) != 0U))
  {
    return 29U;
  }

  return kMonthDays[month - 1U];
}

static uint8_t ComputeWeekDay(uint16_t year, uint8_t month, uint8_t day)
{
  uint16_t adjustedYear = year;
  uint8_t adjustedMonth = month;
  uint16_t century;
  uint16_t yearOfCentury;
  uint16_t h;

  if (adjustedMonth < 3U)
  {
    adjustedMonth = (uint8_t) (adjustedMonth + 12U);
    adjustedYear = (uint16_t) (adjustedYear - 1U);
  }

  century = (uint16_t) (adjustedYear / 100U);
  yearOfCentury = (uint16_t) (adjustedYear % 100U);
  h = (uint16_t) ((day + ((13U * ((uint16_t) adjustedMonth + 1U)) / 5U)
                   + yearOfCentury + (yearOfCentury / 4U)
                   + (century / 4U) + (5U * century))
                  % 7U);

  return (uint8_t) (((h + 5U) % 7U) + 1U);
}

void MmiLocalSettingsServiceInit(MmiLocalSettingsService_t *service)
{
  if (service != NULL)
  {
    service->modemConfigPort = NULL;
    service->gpsPort = NULL;
    service->rtcPort = NULL;
    service->gpsTimeSyncPort = NULL;
    service->userSettingsPort = NULL;
    service->brokenInputSettingsPort = NULL;
    service->configurationService = NULL;
    service->globalTimeManagementService = NULL;
    service->userAuthService = NULL;
  }
}

void MmiLocalSettingsServiceBind(MmiLocalSettingsService_t *service,
                                 IModemConfigPort_t *modemConfigPort,
                                 IGpsPort_t *gpsPort,
                                 IRealtimeClockPort_t *rtcPort,
                                 IGpsTimeSyncPort_t *gpsTimeSyncPort,
                                 IUserSettingsPort_t *userSettingsPort,
                                 IBrokenInputSettingsPort_t *brokenInputSettingsPort,
                                 ConfigurationService_t *configurationService,
                                 GlobalTimeManagementService_t *globalTimeManagementService,
                                 UserAuthService_t *userAuthService)
{
  if (service != NULL)
  {
    service->modemConfigPort = modemConfigPort;
    service->gpsPort = gpsPort;
    service->rtcPort = rtcPort;
    service->gpsTimeSyncPort = gpsTimeSyncPort;
    service->userSettingsPort = userSettingsPort;
    service->brokenInputSettingsPort = brokenInputSettingsPort;
    service->configurationService = configurationService;
    service->globalTimeManagementService = globalTimeManagementService;
    service->userAuthService = userAuthService;
  }
}

static MmiProtocolStatus_t ReadModemSettings(
  const MmiLocalSettingsService_t *service,
  uint8_t *payload,
  uint16_t *payloadLength)
{
  MmiLocalModemSettingsV2_t settings;

  if ((service == NULL) || (service->modemConfigPort == NULL)
      || (payload == NULL) || (payloadLength == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  (void) memset(&settings, 0, sizeof(settings));
  settings.modemType = IModemConfigPort_GetModemType(service->modemConfigPort);
  (void) memcpy(payload, &settings, sizeof(settings));
  *payloadLength = (uint16_t) sizeof(settings);
  return MMI_PROTOCOL_V2_STATUS_OK;
}

static MmiProtocolStatus_t WriteModemSettings(
  const MmiLocalSettingsService_t *service,
  const uint8_t *payload,
  uint16_t payloadLength)
{
  MmiLocalModemSettingsV2_t settings;

  if ((service == NULL) || (service->modemConfigPort == NULL)
      || (payload == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (payloadLength != sizeof(settings))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  (void) memcpy(&settings, payload, sizeof(settings));
  if (IModemConfigPort_IsValidModemType(service->modemConfigPort,
                                        settings.modemType) == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  if (IModemConfigPort_GetModemType(service->modemConfigPort)
      != settings.modemType)
  {
    IModemConfigPort_SetModemType(service->modemConfigPort,
                                  settings.modemType);
    if (IModemConfigPort_SaveConfig(service->modemConfigPort) == 0U)
    {
      return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
    }
  }

  return MMI_PROTOCOL_V2_STATUS_OK;
}

static MmiProtocolStatus_t ReadGpsSettings(
  const MmiLocalSettingsService_t *service,
  uint8_t *payload,
  uint16_t *payloadLength)
{
  MmiLocalGpsSettingsV2_t settings;

  if ((service == NULL) || (service->gpsPort == NULL)
      || (payload == NULL) || (payloadLength == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  (void) memset(&settings, 0, sizeof(settings));
  settings.gpsPortType = IGpsPort_GetPortType(service->gpsPort);
  settings.gpsBaudRateIndex = IGpsPort_GetBaudRateIndex(service->gpsPort);
  (void) memcpy(payload, &settings, sizeof(settings));
  *payloadLength = (uint16_t) sizeof(settings);
  return MMI_PROTOCOL_V2_STATUS_OK;
}

static MmiProtocolStatus_t WriteGpsSettings(
  const MmiLocalSettingsService_t *service,
  const uint8_t *payload,
  uint16_t payloadLength)
{
  MmiLocalGpsSettingsV2_t settings;

  if ((service == NULL) || (service->gpsPort == NULL) || (payload == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (payloadLength != sizeof(settings))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  (void) memcpy(&settings, payload, sizeof(settings));
  if ((IGpsPort_IsValidPortType(service->gpsPort, settings.gpsPortType) == 0U)
      || (IGpsPort_IsValidBaudRateIndex(service->gpsPort,
                                        settings.gpsBaudRateIndex) == 0U))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  if ((IGpsPort_GetPortType(service->gpsPort) != settings.gpsPortType)
      || (IGpsPort_GetBaudRateIndex(service->gpsPort)
          != settings.gpsBaudRateIndex))
  {
    IGpsPort_SetPortType(service->gpsPort, settings.gpsPortType);
    IGpsPort_SetBaudRateIndex(service->gpsPort, settings.gpsBaudRateIndex);
    if (IGpsPort_SaveConfig(service->gpsPort) == 0U)
    {
      return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
    }
  }

  return MMI_PROTOCOL_V2_STATUS_OK;
}

static MmiProtocolStatus_t ReadUserFlags(
  const MmiLocalSettingsService_t *service,
  uint8_t *payload,
  uint16_t *payloadLength)
{
  UserSettingsFlags_t flags;
  MmiLocalUserFlagsV2_t settings;

  if ((service == NULL) || (service->userSettingsPort == NULL)
      || (payload == NULL) || (payloadLength == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (UserSettingsPort_Read(service->userSettingsPort) == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  (void) memset(&flags, 0, sizeof(flags));
  (void) memset(&settings, 0, sizeof(settings));
  UserSettingsPort_Get(service->userSettingsPort, &flags);
  settings.configFlag = flags.configFlag;
  settings.logFlag = flags.logFlag;
  settings.trafficCountsFlag = flags.trafficCountsFlag;
  settings.standbyInfoFlag = flags.standbyInfoFlag;
  (void) memcpy(payload, &settings, sizeof(settings));
  *payloadLength = (uint16_t) sizeof(settings);
  return MMI_PROTOCOL_V2_STATUS_OK;
}

static MmiProtocolStatus_t WriteUserFlags(
  const MmiLocalSettingsService_t *service,
  const uint8_t *payload,
  uint16_t payloadLength)
{
  MmiLocalUserFlagsV2_t settings;
  UserSettingsFlags_t flags;

  if ((service == NULL) || (service->userSettingsPort == NULL)
      || (payload == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (payloadLength != sizeof(settings))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  if (UserSettingsPort_Read(service->userSettingsPort) == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  (void) memcpy(&settings, payload, sizeof(settings));
  flags.configFlag = settings.configFlag;
  flags.logFlag = settings.logFlag;
  flags.trafficCountsFlag = settings.trafficCountsFlag;
  flags.standbyInfoFlag = settings.standbyInfoFlag;
  UserSettingsPort_Set(service->userSettingsPort, &flags);
  if (UserSettingsPort_Save(service->userSettingsPort) == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  return MMI_PROTOCOL_V2_STATUS_OK;
}

static MmiProtocolStatus_t ReadBrokenInputSettings(
  const MmiLocalSettingsService_t *service,
  uint8_t *payload,
  uint16_t *payloadLength)
{
  BrokenInputSettings_t flags;
  MmiLocalBrokenInputSettingsV2_t settings;

  if ((service == NULL) || (service->brokenInputSettingsPort == NULL)
      || (payload == NULL) || (payloadLength == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (BrokenInputSettingsPort_Read(service->brokenInputSettingsPort) == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  (void) memset(&flags, 0, sizeof(flags));
  (void) memset(&settings, 0, sizeof(settings));
  BrokenInputSettingsPort_Get(service->brokenInputSettingsPort, &flags);
  settings.loopInputFlag = flags.loopInputFlag;
  settings.digitalInputFlag = flags.digitalInputFlag;
  (void) memcpy(payload, &settings, sizeof(settings));
  *payloadLength = (uint16_t) sizeof(settings);
  return MMI_PROTOCOL_V2_STATUS_OK;
}

static MmiProtocolStatus_t WriteBrokenInputSettings(
  const MmiLocalSettingsService_t *service,
  const uint8_t *payload,
  uint16_t payloadLength)
{
  MmiLocalBrokenInputSettingsV2_t settings;
  BrokenInputSettings_t flags;

  if ((service == NULL) || (service->brokenInputSettingsPort == NULL)
      || (payload == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (payloadLength != sizeof(settings))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  if (BrokenInputSettingsPort_Read(service->brokenInputSettingsPort) == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  (void) memcpy(&settings, payload, sizeof(settings));
  flags.loopInputFlag = settings.loopInputFlag;
  flags.digitalInputFlag = settings.digitalInputFlag;
  BrokenInputSettingsPort_Set(service->brokenInputSettingsPort, &flags);
  if (BrokenInputSettingsPort_Save(service->brokenInputSettingsPort) == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  return MMI_PROTOCOL_V2_STATUS_OK;
}

static MmiProtocolStatus_t ReadAdminInfo(
  const MmiLocalSettingsService_t *service,
  uint8_t *payload,
  uint16_t *payloadLength)
{
  MmiLocalAdminInfoV2_t settings;

  if ((service == NULL) || (service->userAuthService == NULL)
      || (payload == NULL) || (payloadLength == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (UserAuthServiceIsAdminValid(service->userAuthService) == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  (void) memset(&settings, 0, sizeof(settings));
  settings.adminUsername =
    UserAuthServiceGetAdminUsername(service->userAuthService);
  settings.adminValidity =
    UserAuthServiceIsAdminValid(service->userAuthService);
  (void) memcpy(payload, &settings, sizeof(settings));
  *payloadLength = (uint16_t) sizeof(settings);
  return MMI_PROTOCOL_V2_STATUS_OK;
}

static MmiProtocolStatus_t ReadClockSettings(
  const MmiLocalSettingsService_t *service,
  uint8_t *payload,
  uint16_t *payloadLength)
{
  MmiLocalClockSettingsV2_t settings;
  RtcSnapshot_t snapshot;
  IntersectionGlobalTimeManagementConfig_t globalTimeManagement;

  if ((service == NULL) || (service->rtcPort == NULL)
      || (service->configurationService == NULL) || (payload == NULL)
      || (payloadLength == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if ((RealtimeClockReadSnapshot(service->rtcPort, &snapshot) == 0U)
      || (ConfigurationServiceGetActiveGlobalTimeManagementConfig(
            service->configurationService,
            &globalTimeManagement) == 0U))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  (void) memset(&settings, 0, sizeof(settings));
  settings.second = snapshot.Seconds;
  settings.minute = snapshot.Minutes;
  settings.hour = snapshot.Hours;
  settings.day = snapshot.Date;
  settings.month = snapshot.Month;
  settings.year = snapshot.Year;
  settings.century = snapshot.Century;
  settings.daylightSavingEnabled =
    (uint8_t) (globalTimeManagement.globalDaylightSaving == 20U);
  (void) memcpy(payload, &settings, sizeof(settings));
  *payloadLength = (uint16_t) sizeof(settings);
  return MMI_PROTOCOL_V2_STATUS_OK;
}

static MmiProtocolStatus_t WriteAdminPasswordChange(
  const MmiLocalSettingsService_t *service,
  const uint8_t *payload,
  uint16_t payloadLength)
{
  MmiLocalAdminPasswordChangeV2_t request;
  UserAuthChangeStatus_t status;

  if ((service == NULL) || (service->userAuthService == NULL)
      || (payload == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (payloadLength != sizeof(request))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  (void) memcpy(&request, payload, sizeof(request));
  status = UserAuthServiceChangeAdminPin(service->userAuthService,
                                         request.currentPassword,
                                         request.newPassword);
  switch (status)
  {
      case USER_AUTH_CHANGE_OK:
      {
        return MMI_PROTOCOL_V2_STATUS_OK;
      }

      case USER_AUTH_CHANGE_INVALID_CURRENT:
      case USER_AUTH_CHANGE_INVALID_NEW:
      {
        return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
      }

      case USER_AUTH_CHANGE_STORE_FAILED:
      case USER_AUTH_CHANGE_INTERNAL_ERROR:
      default:
      {
        return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
      }
  }
}

static MmiProtocolStatus_t WriteClockSettings(
  const MmiLocalSettingsService_t *service,
  const uint8_t *payload,
  uint16_t payloadLength)
{
  MmiLocalClockSettingsV2_t settings;
  RtcSnapshot_t snapshot;
  IntersectionGlobalTimeManagementConfig_t globalTimeManagement;
  uint8_t gpsOwnsTime = 0U;

  if ((service == NULL) || (service->rtcPort == NULL)
      || (service->configurationService == NULL) || (payload == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (payloadLength != sizeof(settings))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  (void) memcpy(&settings, payload, sizeof(settings));
  if ((settings.century == 0U) || (settings.month == 0U)
      || (settings.month > 12U) || (settings.day == 0U)
      || (settings.day > DaysInMonth((uint16_t) (((uint16_t) (settings.century - 1U) * 100U)
                                                 + settings.year),
                                     settings.month))
      || (settings.hour > 23U)
      || (settings.minute > 59U) || (settings.second > 59U)
      || (settings.daylightSavingEnabled > 1U))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  if (ConfigurationServiceGetActiveGlobalTimeManagementConfig(
        service->configurationService,
        &globalTimeManagement) == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  globalTimeManagement.globalDaylightSaving =
    (settings.daylightSavingEnabled != 0U) ? 20U : 1U;
  if (ConfigurationServiceCreateTransaction(service->configurationService) == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_BUSY;
  }

  if (ConfigurationServiceSetGlobalTimeManagementConfig(
        service->configurationService,
        &globalTimeManagement) == 0U)
  {
    ConfigurationServiceRollback(service->configurationService);
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  if (ConfigurationServiceVerify(service->configurationService) == 0U)
  {
    ConfigurationServiceRollback(service->configurationService);
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  if (ConfigurationServiceCommit(service->configurationService) == 0U)
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (service->globalTimeManagementService != NULL)
  {
    GlobalTimeManagementServiceHandleCommittedConfig(
      service->globalTimeManagementService,
      &globalTimeManagement);
  }

  snapshot.Century = settings.century;
  snapshot.Year = settings.year;
  snapshot.Month = settings.month;
  snapshot.Date = settings.day;
  snapshot.WeekDay = ComputeWeekDay((uint16_t) (((uint16_t) (settings.century - 1U) * 100U)
                                                + settings.year),
                                    settings.month,
                                    settings.day);
  snapshot.Hours = settings.hour;
  snapshot.Minutes = settings.minute;
  snapshot.Seconds = settings.second;

  if (service->gpsTimeSyncPort != NULL)
  {
    gpsOwnsTime = (uint8_t) ((GpsTimeSyncPort_IsGpsAlive(service->gpsTimeSyncPort)
                              != 0U)
                             && (GpsTimeSyncPort_IsRtcInitialUpdateDone(
                                   service->gpsTimeSyncPort) != 0U));
  }

  if ((gpsOwnsTime == 0U)
      && (RealtimeClockWriteSnapshot(service->rtcPort, &snapshot) == 0U))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if ((service->gpsTimeSyncPort != NULL)
      && (gpsOwnsTime != 0U))
  {
    GpsTimeSyncPort_InvalidateRtcInitialUpdate(service->gpsTimeSyncPort);
  }

  return MMI_PROTOCOL_V2_STATUS_OK;
}

MmiProtocolStatus_t MmiLocalSettingsServiceRead(
  const MmiLocalSettingsService_t *service,
  uint8_t resourceId,
  uint8_t *payload,
  uint16_t *payloadLength)
{
  switch ((MmiProtocolLocalResource_t) resourceId)
  {
      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_MODEM:
      {
        return ReadModemSettings(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_GPS:
      {
        return ReadGpsSettings(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_USER_FLAGS:
      {
        return ReadUserFlags(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_BROKEN_INPUT:
      {
        return ReadBrokenInputSettings(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN:
      {
        return ReadAdminInfo(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS:
      {
        return ReadClockSettings(service, payload, payloadLength);
      }

      default:
      {
        return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
      }
  }
}

MmiProtocolStatus_t MmiLocalSettingsServiceWrite(
  const MmiLocalSettingsService_t *service,
  uint8_t resourceId,
  const uint8_t *payload,
  uint16_t payloadLength)
{
  switch ((MmiProtocolLocalResource_t) resourceId)
  {
      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_MODEM:
      {
        return WriteModemSettings(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_GPS:
      {
        return WriteGpsSettings(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_USER_FLAGS:
      {
        return WriteUserFlags(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_BROKEN_INPUT:
      {
        return WriteBrokenInputSettings(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN:
      {
        return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
      }

      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN_PASSWORD_CHANGE:
      {
        return WriteAdminPasswordChange(service, payload, payloadLength);
      }

      case MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS:
      {
        return WriteClockSettings(service, payload, payloadLength);
      }

      default:
      {
        return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
      }
  }
}
