#include "unity.h"

#include <string.h>

#include "MockConfigRepositoryAdapter.h"

#include "Domain/Services/MmiLocalSettingsService.h"

typedef struct
{
  uint8_t modemType;
  uint8_t saveOk;
} TestModemCtx_t;

typedef struct
{
  uint8_t portType;
  uint8_t baudRateIndex;
  uint8_t saveOk;
} TestGpsCtx_t;

typedef struct
{
  UserSettingsFlags_t settings;
  uint8_t readOk;
  uint8_t saveOk;
} TestUserSettingsCtx_t;

typedef struct
{
  BrokenInputSettings_t settings;
  uint8_t readOk;
  uint8_t saveOk;
} TestBrokenInputCtx_t;

typedef struct
{
  UserAuthStoreRecord_t record;
  uint8_t loadOk;
  uint8_t saveOk;
} TestUserAuthStoreCtx_t;

static uint8_t TestGetModemType(void *ctx)
{
  return ((TestModemCtx_t *) ctx)->modemType;
}

static void TestSetModemType(void *ctx, uint8_t modemType)
{
  ((TestModemCtx_t *) ctx)->modemType = modemType;
}

static uint8_t TestSaveModemConfig(void *ctx)
{
  return ((TestModemCtx_t *) ctx)->saveOk;
}

static uint8_t TestIsValidModemType(void *ctx, uint8_t modemType)
{
  (void) ctx;
  return (uint8_t) (modemType < 7U);
}

static uint8_t TestGetPortType(void *ctx)
{
  return ((TestGpsCtx_t *) ctx)->portType;
}

static void TestSetPortType(void *ctx, uint8_t type)
{
  ((TestGpsCtx_t *) ctx)->portType = type;
}

static uint8_t TestGetBaudRateIndex(void *ctx)
{
  return ((TestGpsCtx_t *) ctx)->baudRateIndex;
}

static void TestSetBaudRateIndex(void *ctx, uint8_t index)
{
  ((TestGpsCtx_t *) ctx)->baudRateIndex = index;
}

static uint32_t TestIndexToBaudRate(void *ctx, uint8_t index)
{
  (void) ctx;
  return (index == 9U) ? 115200UL : 4800UL;
}

static uint8_t TestSaveGpsConfig(void *ctx)
{
  return ((TestGpsCtx_t *) ctx)->saveOk;
}

static uint8_t TestIsValidPortType(void *ctx, uint8_t type)
{
  (void) ctx;
  return (uint8_t) (type <= 2U);
}

static uint8_t TestIsValidBaudRateIndex(void *ctx, uint8_t index)
{
  (void) ctx;
  return (uint8_t) ((index >= 1U) && (index <= 11U));
}

static uint8_t TestReadUserSettings(void *ctx)
{
  return ((TestUserSettingsCtx_t *) ctx)->readOk;
}

static void TestGetUserSettings(void *ctx, UserSettingsFlags_t *settings)
{
  *settings = ((TestUserSettingsCtx_t *) ctx)->settings;
}

static void TestSetUserSettings(void *ctx, const UserSettingsFlags_t *settings)
{
  ((TestUserSettingsCtx_t *) ctx)->settings = *settings;
}

static uint8_t TestSaveUserSettings(void *ctx)
{
  return ((TestUserSettingsCtx_t *) ctx)->saveOk;
}

static uint8_t TestReadBrokenInput(void *ctx)
{
  return ((TestBrokenInputCtx_t *) ctx)->readOk;
}

static void TestGetBrokenInput(void *ctx, BrokenInputSettings_t *settings)
{
  *settings = ((TestBrokenInputCtx_t *) ctx)->settings;
}

static void TestSetBrokenInput(void *ctx, const BrokenInputSettings_t *settings)
{
  ((TestBrokenInputCtx_t *) ctx)->settings = *settings;
}

static uint8_t TestSaveBrokenInput(void *ctx)
{
  return ((TestBrokenInputCtx_t *) ctx)->saveOk;
}

static uint8_t TestLoadAuthRecord(void *ctx, UserAuthStoreRecord_t *record)
{
  TestUserAuthStoreCtx_t *store = (TestUserAuthStoreCtx_t *) ctx;

  if ((store->loadOk == 0U) || (record == NULL))
  {
    return 0U;
  }

  *record = store->record;
  return 1U;
}

static uint8_t TestSaveAuthRecord(void *ctx, const UserAuthStoreRecord_t *record)
{
  TestUserAuthStoreCtx_t *store = (TestUserAuthStoreCtx_t *) ctx;

  if ((store->saveOk == 0U) || (record == NULL))
  {
    return 0U;
  }

  store->record = *record;
  return 1U;
}

typedef struct
{
  RtcSnapshot_t snapshot;
  uint8_t readOk;
  uint8_t writeOk;
  uint8_t writeCount;
} TestRtcCtx_t;

typedef struct
{
  uint8_t gpsAlive;
  uint8_t rtcInitialUpdateDone;
  uint8_t invalidated;
} TestGpsTimeSyncCtx_t;

static uint8_t TestRtcRead(void *ctx, RtcSnapshot_t *snapshot)
{
  TestRtcCtx_t *rtc = (TestRtcCtx_t *) ctx;

  if ((rtc->readOk == 0U) || (snapshot == NULL))
  {
    return 0U;
  }

  *snapshot = rtc->snapshot;
  return 1U;
}

static uint8_t TestRtcWrite(void *ctx, const RtcSnapshot_t *snapshot)
{
  TestRtcCtx_t *rtc = (TestRtcCtx_t *) ctx;

  if ((rtc->writeOk == 0U) || (snapshot == NULL))
  {
    return 0U;
  }

  rtc->snapshot = *snapshot;
  rtc->writeCount++;
  return 1U;
}

static uint8_t TestRtcReadMetadata(void *ctx,
                                   RtcMetadataId_t metadataId,
                                   uint32_t *value)
{
  (void) ctx;
  (void) metadataId;
  (void) value;
  return 0U;
}

static uint8_t TestRtcWriteMetadata(void *ctx,
                                    RtcMetadataId_t metadataId,
                                    uint32_t value)
{
  (void) ctx;
  (void) metadataId;
  (void) value;
  return 0U;
}

static uint8_t TestRtcApplyDst(void *ctx, RtcDstAdjustment_t adjustment)
{
  (void) ctx;
  (void) adjustment;
  return 0U;
}

static uint8_t TestGpsSyncIsAlive(void *ctx)
{
  return ((TestGpsTimeSyncCtx_t *) ctx)->gpsAlive;
}

static uint8_t TestGpsSyncIsRtcInitialUpdateDone(void *ctx)
{
  return ((TestGpsTimeSyncCtx_t *) ctx)->rtcInitialUpdateDone;
}

static void TestGpsSyncInvalidate(void *ctx)
{
  TestGpsTimeSyncCtx_t *gpsSync = (TestGpsTimeSyncCtx_t *) ctx;

  gpsSync->invalidated = 1U;
  gpsSync->rtcInitialUpdateDone = 0U;
}

static void InitTimeServices(ConfigurationService_t *configurationService,
                             MockConfigRepositoryAdapterCtx_t *repoCtx,
                             IConfigRepositoryPort_t *repoPort,
                             GlobalTimeManagementService_t *globalTimeService,
                             IRealtimeClockPort_t *rtcPort,
                             TestRtcCtx_t *rtcCtx)
{
  MockConfigRepositoryAdapterInit(repoCtx);
  *repoPort = MockConfigRepositoryAdapterCreatePort(repoCtx);
  ConfigurationServiceInit(configurationService, repoPort);

  rtcPort->ctx = rtcCtx;
  rtcPort->ReadSnapshot = TestRtcRead;
  rtcPort->WriteSnapshot = TestRtcWrite;
  rtcPort->ReadMetadata = TestRtcReadMetadata;
  rtcPort->WriteMetadata = TestRtcWriteMetadata;
  rtcPort->ApplyDstAdjustment = TestRtcApplyDst;

  GlobalTimeManagementServiceInit(globalTimeService);
  GlobalTimeManagementServiceBind(globalTimeService, NULL, rtcPort);
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_MmiLocalSettingsServiceReadGetsModemAndGps(void)
{
  TestModemCtx_t modemCtx = { 4U, 1U };
  TestGpsCtx_t gpsCtx = { 2U, 9U, 1U };
  TestUserSettingsCtx_t userSettingsCtx = { { 1U, 0U, 1U, 1U }, 1U, 1U };
  TestBrokenInputCtx_t brokenInputCtx = { { 1U, 0U }, 1U, 1U };
  TestUserAuthStoreCtx_t authStoreCtx = { { 0U }, 0U, 1U };
  TestRtcCtx_t rtcCtx = { { 21U, 26U, 4U, 19U, 7U, 15U, 16U, 17U }, 1U, 1U, 0U };
  TestGpsTimeSyncCtx_t gpsSyncCtx = { 0U, 0U, 0U };
  IModemConfigPort_t modemPort;
  IGpsPort_t gpsPort;
  IRealtimeClockPort_t rtcPort;
  IGpsTimeSyncPort_t gpsSyncPort;
  IUserSettingsPort_t userSettingsPort;
  IBrokenInputSettingsPort_t brokenInputPort;
  IUserAuthStorePort_t authStorePort;
  UserAuthService_t authService;
  MockConfigRepositoryAdapterCtx_t repoCtx;
  IConfigRepositoryPort_t repoPort;
  ConfigurationService_t configurationService;
  GlobalTimeManagementService_t globalTimeService;
  MmiLocalSettingsService_t service;
  uint8_t payload[8];
  uint16_t payloadLength = 0U;
  MmiLocalModemSettingsV2_t modemSettings;
  MmiLocalGpsSettingsV2_t gpsSettings;
  MmiLocalUserFlagsV2_t userFlags;
  MmiLocalBrokenInputSettingsV2_t brokenInput;
  MmiLocalAdminInfoV2_t adminInfo;
  MmiLocalClockSettingsV2_t clockSettings;

  modemPort.ctx = &modemCtx;
  modemPort.GetModemType = TestGetModemType;
  modemPort.SetModemType = TestSetModemType;
  modemPort.SaveConfig = TestSaveModemConfig;
  modemPort.IsValidModemType = TestIsValidModemType;

  gpsPort.ctx = &gpsCtx;
  gpsPort.GetPortType = TestGetPortType;
  gpsPort.SetPortType = TestSetPortType;
  gpsPort.GetBaudRateIndex = TestGetBaudRateIndex;
  gpsPort.SetBaudRateIndex = TestSetBaudRateIndex;
  gpsPort.IndexToBaudRate = TestIndexToBaudRate;
  gpsPort.SaveConfig = TestSaveGpsConfig;
  gpsPort.IsValidPortType = TestIsValidPortType;
  gpsPort.IsValidBaudRateIndex = TestIsValidBaudRateIndex;

  userSettingsPort.ctx = &userSettingsCtx;
  userSettingsPort.Read = TestReadUserSettings;
  userSettingsPort.Get = TestGetUserSettings;
  userSettingsPort.Set = TestSetUserSettings;
  userSettingsPort.Save = TestSaveUserSettings;

  brokenInputPort.ctx = &brokenInputCtx;
  brokenInputPort.Read = TestReadBrokenInput;
  brokenInputPort.Get = TestGetBrokenInput;
  brokenInputPort.Set = TestSetBrokenInput;
  brokenInputPort.Save = TestSaveBrokenInput;

  authStorePort.ctx = &authStoreCtx;
  authStorePort.Load = TestLoadAuthRecord;
  authStorePort.Save = TestSaveAuthRecord;

  UserAuthServiceInit(&authService);
  UserAuthServiceBind(&authService, &authStorePort);
  InitTimeServices(&configurationService,
                   &repoCtx,
                   &repoPort,
                   &globalTimeService,
                   &rtcPort,
                   &rtcCtx);

  gpsSyncPort.ctx = &gpsSyncCtx;
  gpsSyncPort.IsGpsAlive = TestGpsSyncIsAlive;
  gpsSyncPort.IsRtcInitialUpdateDone = TestGpsSyncIsRtcInitialUpdateDone;
  gpsSyncPort.InvalidateRtcInitialUpdate = TestGpsSyncInvalidate;

  MmiLocalSettingsServiceInit(&service);
  MmiLocalSettingsServiceBind(&service,
                              &modemPort,
                              &gpsPort,
                              &rtcPort,
                              &gpsSyncPort,
                              &userSettingsPort,
                              &brokenInputPort,
                              &configurationService,
                              NULL,
                              &authService);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceRead(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_MODEM,
                            &payload[0],
                            &payloadLength));
  TEST_ASSERT_EQUAL_UINT16(sizeof(modemSettings), payloadLength);
  (void) memcpy(&modemSettings, &payload[0], sizeof(modemSettings));
  TEST_ASSERT_EQUAL_UINT8(4U, modemSettings.modemType);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceRead(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_GPS,
                            &payload[0],
                            &payloadLength));
  TEST_ASSERT_EQUAL_UINT16(sizeof(gpsSettings), payloadLength);
  (void) memcpy(&gpsSettings, &payload[0], sizeof(gpsSettings));
  TEST_ASSERT_EQUAL_UINT8(2U, gpsSettings.gpsPortType);
  TEST_ASSERT_EQUAL_UINT8(9U, gpsSettings.gpsBaudRateIndex);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceRead(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_USER_FLAGS,
                            &payload[0],
                            &payloadLength));
  TEST_ASSERT_EQUAL_UINT16(sizeof(userFlags), payloadLength);
  (void) memcpy(&userFlags, &payload[0], sizeof(userFlags));
  TEST_ASSERT_EQUAL_UINT8(1U, userFlags.configFlag);
  TEST_ASSERT_EQUAL_UINT8(0U, userFlags.logFlag);
  TEST_ASSERT_EQUAL_UINT8(1U, userFlags.trafficCountsFlag);
  TEST_ASSERT_EQUAL_UINT8(1U, userFlags.standbyInfoFlag);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceRead(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_BROKEN_INPUT,
                            &payload[0],
                            &payloadLength));
  TEST_ASSERT_EQUAL_UINT16(sizeof(brokenInput), payloadLength);
  (void) memcpy(&brokenInput, &payload[0], sizeof(brokenInput));
  TEST_ASSERT_EQUAL_UINT8(1U, brokenInput.loopInputFlag);
  TEST_ASSERT_EQUAL_UINT8(0U, brokenInput.digitalInputFlag);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceRead(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN,
                            &payload[0],
                            &payloadLength));
  TEST_ASSERT_EQUAL_UINT16(sizeof(adminInfo), payloadLength);
  (void) memcpy(&adminInfo, &payload[0], sizeof(adminInfo));
  TEST_ASSERT_EQUAL_UINT16(USER_AUTH_ADMIN_USERNAME, adminInfo.adminUsername);
  TEST_ASSERT_EQUAL_UINT8(1U, adminInfo.adminValidity);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceRead(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS,
                            &payload[0],
                            &payloadLength));
  TEST_ASSERT_EQUAL_UINT16(sizeof(clockSettings), payloadLength);
  (void) memcpy(&clockSettings, &payload[0], sizeof(clockSettings));
  TEST_ASSERT_EQUAL_UINT8(17U, clockSettings.second);
  TEST_ASSERT_EQUAL_UINT8(16U, clockSettings.minute);
  TEST_ASSERT_EQUAL_UINT8(15U, clockSettings.hour);
  TEST_ASSERT_EQUAL_UINT8(19U, clockSettings.day);
  TEST_ASSERT_EQUAL_UINT8(4U, clockSettings.month);
  TEST_ASSERT_EQUAL_UINT8(26U, clockSettings.year);
  TEST_ASSERT_EQUAL_UINT8(21U, clockSettings.century);
  TEST_ASSERT_EQUAL_UINT8(20U, clockSettings.globalDaylightSaving);
}

void test_MmiLocalSettingsServiceWritePersistsValidatedSettings(void)
{
  TestModemCtx_t modemCtx = { 0U, 1U };
  TestGpsCtx_t gpsCtx = { 1U, 3U, 1U };
  TestUserSettingsCtx_t userSettingsCtx = { { 0U, 0U, 0U, 0U }, 1U, 1U };
  TestBrokenInputCtx_t brokenInputCtx = { { 1U, 1U }, 1U, 1U };
  TestUserAuthStoreCtx_t authStoreCtx = { { 0U }, 0U, 1U };
  TestRtcCtx_t rtcCtx = { { 21U, 26U, 4U, 19U, 7U, 15U, 16U, 17U }, 1U, 1U, 0U };
  TestGpsTimeSyncCtx_t gpsSyncCtx = { 0U, 0U, 0U };
  IModemConfigPort_t modemPort;
  IGpsPort_t gpsPort;
  IRealtimeClockPort_t rtcPort;
  IGpsTimeSyncPort_t gpsSyncPort;
  IUserSettingsPort_t userSettingsPort;
  IBrokenInputSettingsPort_t brokenInputPort;
  IUserAuthStorePort_t authStorePort;
  UserAuthService_t authService;
  MockConfigRepositoryAdapterCtx_t repoCtx;
  IConfigRepositoryPort_t repoPort;
  ConfigurationService_t configurationService;
  GlobalTimeManagementService_t globalTimeService;
  MmiLocalSettingsService_t service;
  MmiLocalModemSettingsV2_t modemSettings = { 5U, { 0U, 0U, 0U } };
  MmiLocalGpsSettingsV2_t gpsSettings = { 2U, 9U, { 0U, 0U } };
  MmiLocalUserFlagsV2_t userFlags = { 1U, 1U, 0U, 1U };
  MmiLocalBrokenInputSettingsV2_t brokenInput = { 0U, 1U, { 0U, 0U } };
  MmiLocalAdminPasswordChangeV2_t adminPasswordChange = {
    USER_AUTH_DEFAULT_ADMIN_PIN,
    2468U
  };
  MmiLocalClockSettingsV2_t clockSettings = { 50U, 59U, 18U, 21U, 4U, 26U, 21U, 2U };

  modemPort.ctx = &modemCtx;
  modemPort.GetModemType = TestGetModemType;
  modemPort.SetModemType = TestSetModemType;
  modemPort.SaveConfig = TestSaveModemConfig;
  modemPort.IsValidModemType = TestIsValidModemType;

  gpsPort.ctx = &gpsCtx;
  gpsPort.GetPortType = TestGetPortType;
  gpsPort.SetPortType = TestSetPortType;
  gpsPort.GetBaudRateIndex = TestGetBaudRateIndex;
  gpsPort.SetBaudRateIndex = TestSetBaudRateIndex;
  gpsPort.IndexToBaudRate = TestIndexToBaudRate;
  gpsPort.SaveConfig = TestSaveGpsConfig;
  gpsPort.IsValidPortType = TestIsValidPortType;
  gpsPort.IsValidBaudRateIndex = TestIsValidBaudRateIndex;

  userSettingsPort.ctx = &userSettingsCtx;
  userSettingsPort.Read = TestReadUserSettings;
  userSettingsPort.Get = TestGetUserSettings;
  userSettingsPort.Set = TestSetUserSettings;
  userSettingsPort.Save = TestSaveUserSettings;

  brokenInputPort.ctx = &brokenInputCtx;
  brokenInputPort.Read = TestReadBrokenInput;
  brokenInputPort.Get = TestGetBrokenInput;
  brokenInputPort.Set = TestSetBrokenInput;
  brokenInputPort.Save = TestSaveBrokenInput;

  authStorePort.ctx = &authStoreCtx;
  authStorePort.Load = TestLoadAuthRecord;
  authStorePort.Save = TestSaveAuthRecord;

  UserAuthServiceInit(&authService);
  UserAuthServiceBind(&authService, &authStorePort);
  InitTimeServices(&configurationService,
                   &repoCtx,
                   &repoPort,
                   &globalTimeService,
                   &rtcPort,
                   &rtcCtx);

  gpsSyncPort.ctx = &gpsSyncCtx;
  gpsSyncPort.IsGpsAlive = TestGpsSyncIsAlive;
  gpsSyncPort.IsRtcInitialUpdateDone = TestGpsSyncIsRtcInitialUpdateDone;
  gpsSyncPort.InvalidateRtcInitialUpdate = TestGpsSyncInvalidate;

  MmiLocalSettingsServiceInit(&service);
  MmiLocalSettingsServiceBind(&service,
                              &modemPort,
                              &gpsPort,
                              &rtcPort,
                              &gpsSyncPort,
                              &userSettingsPort,
                              &brokenInputPort,
                              &configurationService,
                              &globalTimeService,
                              &authService);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceWrite(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_MODEM,
                            (const uint8_t *) &modemSettings,
                            sizeof(modemSettings)));
  TEST_ASSERT_EQUAL_UINT8(5U, modemCtx.modemType);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceWrite(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_GPS,
                            (const uint8_t *) &gpsSettings,
                            sizeof(gpsSettings)));
  TEST_ASSERT_EQUAL_UINT8(2U, gpsCtx.portType);
  TEST_ASSERT_EQUAL_UINT8(9U, gpsCtx.baudRateIndex);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceWrite(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_USER_FLAGS,
                            (const uint8_t *) &userFlags,
                            sizeof(userFlags)));
  TEST_ASSERT_EQUAL_UINT8(1U, userSettingsCtx.settings.configFlag);
  TEST_ASSERT_EQUAL_UINT8(1U, userSettingsCtx.settings.logFlag);
  TEST_ASSERT_EQUAL_UINT8(0U, userSettingsCtx.settings.trafficCountsFlag);
  TEST_ASSERT_EQUAL_UINT8(1U, userSettingsCtx.settings.standbyInfoFlag);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceWrite(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_BROKEN_INPUT,
                            (const uint8_t *) &brokenInput,
                            sizeof(brokenInput)));
  TEST_ASSERT_EQUAL_UINT8(0U, brokenInputCtx.settings.loopInputFlag);
  TEST_ASSERT_EQUAL_UINT8(1U, brokenInputCtx.settings.digitalInputFlag);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceWrite(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN_PASSWORD_CHANGE,
                            (const uint8_t *) &adminPasswordChange,
                            sizeof(adminPasswordChange)));
  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceWrite(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS,
                            (const uint8_t *) &clockSettings,
                            sizeof(clockSettings)));
  TEST_ASSERT_EQUAL_UINT8(18U, rtcCtx.snapshot.Hours);
  TEST_ASSERT_EQUAL_UINT8(59U, rtcCtx.snapshot.Minutes);
  TEST_ASSERT_EQUAL_UINT8(50U, rtcCtx.snapshot.Seconds);
  TEST_ASSERT_EQUAL_UINT8(21U, rtcCtx.snapshot.Date);
  TEST_ASSERT_EQUAL_UINT8(4U, rtcCtx.snapshot.Month);
  TEST_ASSERT_EQUAL_UINT8(26U, rtcCtx.snapshot.Year);
  TEST_ASSERT_EQUAL_UINT8(21U, rtcCtx.snapshot.Century);
  TEST_ASSERT_EQUAL_UINT8(2U,
                          ConfigurationServiceGetActiveConfig(
                            &configurationService)->globalTimeManagement.globalDaylightSaving);
}

void test_MmiLocalSettingsServiceRejectsInvalidValuesAndUnsupportedResources(void)
{
  TestModemCtx_t modemCtx = { 0U, 1U };
  TestGpsCtx_t gpsCtx = { 1U, 3U, 1U };
  TestUserSettingsCtx_t userSettingsCtx = { { 0U, 0U, 0U, 0U }, 1U, 1U };
  TestBrokenInputCtx_t brokenInputCtx = { { 1U, 1U }, 1U, 1U };
  TestUserAuthStoreCtx_t authStoreCtx = { { 0U }, 0U, 1U };
  TestRtcCtx_t rtcCtx = { { 21U, 26U, 4U, 19U, 7U, 15U, 16U, 17U }, 1U, 1U, 0U };
  TestGpsTimeSyncCtx_t gpsSyncCtx = { 1U, 1U, 0U };
  IModemConfigPort_t modemPort;
  IGpsPort_t gpsPort;
  IRealtimeClockPort_t rtcPort;
  IGpsTimeSyncPort_t gpsSyncPort;
  IUserSettingsPort_t userSettingsPort;
  IBrokenInputSettingsPort_t brokenInputPort;
  IUserAuthStorePort_t authStorePort;
  UserAuthService_t authService;
  MockConfigRepositoryAdapterCtx_t repoCtx;
  IConfigRepositoryPort_t repoPort;
  ConfigurationService_t configurationService;
  GlobalTimeManagementService_t globalTimeService;
  MmiLocalSettingsService_t service;
  MmiLocalModemSettingsV2_t modemSettings = { 99U, { 0U, 0U, 0U } };
  MmiLocalGpsSettingsV2_t gpsSettings = { 4U, 0U, { 0U, 0U } };
  MmiLocalAdminPasswordChangeV2_t adminPasswordChange = {
    USER_AUTH_DEFAULT_ADMIN_PIN,
    0U
  };
  MmiLocalClockSettingsV2_t invalidClockSettings = { 0U, 0U, 24U, 31U, 2U, 26U, 21U, 2U };
  MmiLocalClockSettingsV2_t gpsOwnedClockSettings = { 1U, 2U, 3U, 4U, 5U, 26U, 21U, 1U };
  uint8_t payload[8];
  uint16_t payloadLength = 0U;

  modemPort.ctx = &modemCtx;
  modemPort.GetModemType = TestGetModemType;
  modemPort.SetModemType = TestSetModemType;
  modemPort.SaveConfig = TestSaveModemConfig;
  modemPort.IsValidModemType = TestIsValidModemType;

  gpsPort.ctx = &gpsCtx;
  gpsPort.GetPortType = TestGetPortType;
  gpsPort.SetPortType = TestSetPortType;
  gpsPort.GetBaudRateIndex = TestGetBaudRateIndex;
  gpsPort.SetBaudRateIndex = TestSetBaudRateIndex;
  gpsPort.IndexToBaudRate = TestIndexToBaudRate;
  gpsPort.SaveConfig = TestSaveGpsConfig;
  gpsPort.IsValidPortType = TestIsValidPortType;
  gpsPort.IsValidBaudRateIndex = TestIsValidBaudRateIndex;

  userSettingsPort.ctx = &userSettingsCtx;
  userSettingsPort.Read = TestReadUserSettings;
  userSettingsPort.Get = TestGetUserSettings;
  userSettingsPort.Set = TestSetUserSettings;
  userSettingsPort.Save = TestSaveUserSettings;

  brokenInputPort.ctx = &brokenInputCtx;
  brokenInputPort.Read = TestReadBrokenInput;
  brokenInputPort.Get = TestGetBrokenInput;
  brokenInputPort.Set = TestSetBrokenInput;
  brokenInputPort.Save = TestSaveBrokenInput;

  authStorePort.ctx = &authStoreCtx;
  authStorePort.Load = TestLoadAuthRecord;
  authStorePort.Save = TestSaveAuthRecord;

  UserAuthServiceInit(&authService);
  UserAuthServiceBind(&authService, &authStorePort);
  InitTimeServices(&configurationService,
                   &repoCtx,
                   &repoPort,
                   &globalTimeService,
                   &rtcPort,
                   &rtcCtx);

  gpsSyncPort.ctx = &gpsSyncCtx;
  gpsSyncPort.IsGpsAlive = TestGpsSyncIsAlive;
  gpsSyncPort.IsRtcInitialUpdateDone = TestGpsSyncIsRtcInitialUpdateDone;
  gpsSyncPort.InvalidateRtcInitialUpdate = TestGpsSyncInvalidate;

  MmiLocalSettingsServiceInit(&service);
  MmiLocalSettingsServiceBind(&service,
                              &modemPort,
                              &gpsPort,
                              &rtcPort,
                              &gpsSyncPort,
                              &userSettingsPort,
                              &brokenInputPort,
                              &configurationService,
                              &globalTimeService,
                              &authService);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_INVALID_VALUE,
                          MmiLocalSettingsServiceWrite(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_MODEM,
                            (const uint8_t *) &modemSettings,
                            sizeof(modemSettings)));
  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_INVALID_VALUE,
                          MmiLocalSettingsServiceWrite(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_GPS,
                            (const uint8_t *) &gpsSettings,
                            sizeof(gpsSettings)));
  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceRead(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_USER_FLAGS,
                            &payload[0],
                            &payloadLength));
  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceRead(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_BROKEN_INPUT,
                            &payload[0],
                            &payloadLength));
  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceRead(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN,
                            &payload[0],
                            &payloadLength));
  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_INVALID_VALUE,
                          MmiLocalSettingsServiceWrite(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN_PASSWORD_CHANGE,
                            (const uint8_t *) &adminPasswordChange,
                            sizeof(adminPasswordChange)));
  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_INVALID_VALUE,
                          MmiLocalSettingsServiceWrite(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS,
                            (const uint8_t *) &invalidClockSettings,
                            sizeof(invalidClockSettings)));
  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiLocalSettingsServiceWrite(
                            &service,
                            MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS,
                            (const uint8_t *) &gpsOwnedClockSettings,
                            sizeof(gpsOwnedClockSettings)));
  TEST_ASSERT_EQUAL_UINT8(1U, gpsSyncCtx.invalidated);
  TEST_ASSERT_EQUAL_UINT8(15U, rtcCtx.snapshot.Hours);
  TEST_ASSERT_EQUAL_UINT8(16U, rtcCtx.snapshot.Minutes);
  TEST_ASSERT_EQUAL_UINT8(17U, rtcCtx.snapshot.Seconds);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_MmiLocalSettingsServiceReadGetsModemAndGps);
  RUN_TEST(test_MmiLocalSettingsServiceWritePersistsValidatedSettings);
  RUN_TEST(test_MmiLocalSettingsServiceRejectsInvalidValuesAndUnsupportedResources);
  return UNITY_END();
}
