/**
 ******************************************************************************
 * @file    lcd.c
 * @author  Okan KILIC - Teknotel Electronics
 * @version V1.0.0
 * @date    08/10/2011
 * @brief  LCD and Keypad Services
 ******************************************************************************
 */

#include "lcd.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "cmsis_os2.h"
#include "HardwarePorts.h"
#include "PersistencePorts.h"
#include "usart.h"
#include "usb.h"
#include "gps.h"
#include "Domain/Lcd/LcdEngine.h"
#include "Domain/Lcd/LcdLanguage.h"
#include "Domain/Lcd/LcdServiceRegistry.h"
#include "Domain/Lcd/LcdPageRegistry.h"
#include "Ports/ISystemPort.h"
#include "Ports/ICommsStatusPort.h"
#include "Ports/IUserPort.h"
#include "Ports/ILogRepositoryPort.h"
#include "Ports/IGpsPort.h"
#include "DomainServices.h"

static LcdEngine_t g_lcdEngine;

/* Page instances */
extern LcdPage_t LcdPage_Home;
extern LcdPage_t LcdPage_Login;
extern LcdPage_t LcdPage_Menu;
extern LcdPage_t LcdPage_Help;
extern LcdPage_t LcdPage_Logs;
extern LcdPage_t LcdPage_ConnectionLogs;
extern LcdPage_t LcdPage_Network;
extern LcdPage_t LcdPage_Settings;
extern LcdPage_t LcdPage_SettingsDateTime;
extern LcdPage_t LcdPage_SettingsGps;
extern LcdPage_t LcdPage_SettingsLanguage;
extern LcdPage_t LcdPage_OutputTest;

/* Registry instances */
static LcdServiceRegistry_t g_lcdServices;
static LcdPageRegistry_t g_lcdPages;

/* Initialization functions (opaque context pointers used here) */
extern void LcdPage_Home_Init(void *ctx,
                              const LcdServiceRegistry_t *s,
                              const LcdPageRegistry_t *p,
                              const IntersectionEngine_t *e);
extern void LcdPage_Login_Init(void *ctx,
                               const LcdServiceRegistry_t *s,
                               const LcdPageRegistry_t *p);
extern void LcdPage_Menu_Init(void *ctx,
                              const LcdServiceRegistry_t *s,
                              const LcdPageRegistry_t *p);
extern void LcdPage_Help_Init(void *ctx,
                              const LcdServiceRegistry_t *s,
                              const LcdPageRegistry_t *p);
extern void LcdPage_Logs_Init(void *ctx,
                              const LcdServiceRegistry_t *s,
                              const LcdPageRegistry_t *p);
extern void LcdPage_ConnectionLogs_Init(void *ctx,
                                        const LcdServiceRegistry_t *s,
                                        const LcdPageRegistry_t *p);
extern void LcdPage_Network_Init(void *ctx,
                                 const LcdServiceRegistry_t *s,
                                 const LcdPageRegistry_t *p);
extern void LcdPage_Settings_Init(void *ctx,
                                  const LcdServiceRegistry_t *s,
                                  const LcdPageRegistry_t *p);
extern void LcdPage_SettingsDateTime_Init(void *ctx,
                                          const LcdServiceRegistry_t *s,
                                          const LcdPageRegistry_t *p);
extern void LcdPage_SettingsGps_Init(void *ctx,
                                     const LcdServiceRegistry_t *s,
                                     const LcdPageRegistry_t *p);
extern void LcdPage_SettingsLanguage_Init(void *ctx,
                                          const LcdServiceRegistry_t *s,
                                          const LcdPageRegistry_t *p);
extern void LcdPage_OutputTest_Init(void *ctx,
                                    const LcdServiceRegistry_t *s,
                                    const LcdPageRegistry_t *p);

uint8_t bLanguage = LANGUAGE_TURKISH;
static uint8_t fLCDPowered = 1U;

/* ------------------------------------------------------------------
 * Legacy LCD compatibility helpers.
 * ------------------------------------------------------------------ */
static void HardwareSetupLCD(void)
{
  DisplayPowerOn(&g_lcdPort);
  DisplayClear(&g_lcdPort);
  fLCDPowered = 1U;
}

uint8_t LCDLanguageGet(void)
{
  return bLanguage;
}

void LCDLanguageSet(uint8_t bLang)
{
  bLanguage = bLang;
}

uint8_t LCDLanguageWrite(void)
{
  return 1U;
}

uint8_t LCDLanguageRead(void)
{
  return 1U;
}

void LCDSoftwareClose(void)
{
  if (fLCDPowered != 0U)
  {
    DisplayPowerOff(&g_lcdPort);
    fLCDPowered = 0U;
  }
}

void LCDSoftwareOpen(void)
{
  if (fLCDPowered == 0U)
  {
    DisplayPowerOn(&g_lcdPort);
    fLCDPowered = 1U;
  }
}

void LCDSOMeasurements(uint8_t bSSMNo, tpSMCSLCDStream pSScreen)
{
  (void) bSSMNo; (void) pSScreen;
}

void OpeningScreenFirstLine(char *buf)
{
  (void) buf;
}

void OpeningScreenSecondLine(char *buf)
{
  (void) buf;
}

void OpeningScreenThirdLine(char *buf)
{
  (void) buf;
}

void OpeningScreenFourthLine(char *buf)
{
  (void) buf;
}

uint8_t GetLCDPowerRelayRequest(void)
{
  return 0U;
}

uint8_t GetLCDPowerRelay(void)
{
  return 0U;
}

void SetLCDPowerRelay(uint8_t state)
{
  (void) state;
}

void SetLCDPowerRelayRequest(uint8_t state)
{
  (void) state;
}

void SetLCDState(uint8_t state)
{
  (void) state;
}

static ISystemPort_t s_lcdSystemPort;
static ICommsStatusPort_t s_lcdCommsPort;
static IUserPort_t s_lcdUserPort;
static ILogRepositoryPort_t s_lcdLogPort;
static IGpsPort_t s_lcdGpsPort;

static uint16_t LcdGetMainVoltage(void *ctx)
{
  uint16_t lineVoltageTenthsVrms = 0U;

  (void) ctx;
  if (UiPowerServiceGetLineVoltageTenthsVrms(&g_uiPowerService,
                                             1U,
                                             &lineVoltageTenthsVrms) == 0U)
  {
    return 0U;
  }

  return (uint16_t) ((lineVoltageTenthsVrms + 5U) / 10U);
}

static uint8_t LcdGetTimeSource(void *ctx)
{
  uint8_t currentSource = 0U;

  (void) ctx;
  if (UnitClockPortGetCurrentSource(&g_unitClockPort, &currentSource) == 0U)
  {
    return 0U;
  }

  switch (currentSource)
  {
      case UNIT_CLOCK_SOURCE_GNSS:
      {
        return 3U;
      }

      case UNIT_CLOCK_SOURCE_RTC_SQWR:
      {
        return 2U;
      }

      case UNIT_CLOCK_SOURCE_LINE_SYNC:
      {
        return 4U;
      }

      default:
      {
        return 0U;
      }
  }
}

static uint8_t LcdGetLanguage(void *ctx)
{
  (void) ctx; return bLanguage;
}

static void LcdSetLanguage(void *ctx, uint8_t lang)
{
  (void) ctx; bLanguage = lang;
}

static UserRole_t LcdLogin(void *ctx, uint16_t username, uint16_t password)
{
  (void) ctx;
  return UserAuthServiceLogin(&g_userAuthService, username, password);
}

static UserRole_t LcdGetActiveRole(void *ctx)
{
  (void) ctx;
  return UserAuthServiceGetActiveRole(&g_userAuthService);
}

static void LcdLogout(void *ctx)
{
  (void) ctx;
  UserAuthServiceLogout(&g_userAuthService);
}

static uint8_t LcdChangeAdminPassword(void *ctx,
                                      uint16_t currentPassword,
                                      uint16_t newPassword)
{
  UserAuthChangeStatus_t status;

  (void) ctx;
  status = UserAuthServiceChangeAdminPin(&g_userAuthService,
                                         currentPassword,
                                         newPassword);
  return (uint8_t) (status == USER_AUTH_CHANGE_OK);
}

static uint8_t LcdLogRead(void *ctx,
                          uint16_t index,
                          void *record,
                          uint32_t recordSize)
{
  (void) ctx;
  return LogRepositoryRead(&g_logRepositoryPort, index, record, recordSize);
}

static uint8_t LcdLogExists(void *ctx)
{
  (void) ctx;
  return LogRepositoryExists(&g_logRepositoryPort);
}

static uint8_t LcdLogIsIndexValid(void *ctx, uint16_t index)
{
  (void) ctx;
  return LogRepositoryIsIndexValid(&g_logRepositoryPort, index);
}

static uint16_t LcdLogGetWriteIndex(void *ctx)
{
  uint16_t latestIndex = 0xFFFFU;

  (void) ctx;
  (void) MmiEventLogServiceGetLatestIndex(&g_mmiEventLogService, &latestIndex);
  return latestIndex;
}

static uint8_t LcdReadCommsSnapshot(void *ctx, CommsStatusSnapshot_t *snapshot)
{
  (void) ctx;

  if (snapshot == NULL)
  {
    return 0U;
  }

  if (UiCommsIdentityServiceGetSnapshot(&g_uiCommsIdentityService, snapshot) != 0U)
  {
    return 1U;
  }

  (void) memset(snapshot, 0, sizeof(*snapshot));
  return 0U;
}

static uint8_t LcdGpsGetPortType(void *ctx)
{
  (void) ctx; return GpsPortGet();
}

static void LcdGpsSetPortType(void *ctx, uint8_t type)
{
  (void) ctx; GpsPortSet(type);
}

static uint8_t LcdGpsGetBaudRateIndex(void *ctx)
{
  (void) ctx; return GpsBaudRateIndexGet();
}

static void LcdGpsSetBaudRateIndex(void *ctx, uint8_t index)
{
  (void) ctx; GpsBaudRateIndexSet(index);
}

static uint32_t LcdGpsIndexToBaudRate(void *ctx, uint8_t index)
{
  (void) ctx; return GpsIndexToBaudRate(index);
}

static uint8_t LcdGpsSaveConfig(void *ctx)
{
  uint8_t ok;

  (void) ctx;
  ok = GpsPortWrite();
  ok = (uint8_t) (ok && GpsBaudRateIndexWrite());
  return ok;
}

static uint8_t LcdGpsIsValidPortType(void *ctx, uint8_t type)
{
  (void) ctx;
  return (uint8_t) (type <= GPS_PORT_TYPE_MAX);
}

static uint8_t LcdGpsIsValidBaudRateIndex(void *ctx, uint8_t index)
{
  (void) ctx;
  return (uint8_t) ((index >= GPS_MIN_BAUD_RATE_INDEX)
                    && (index <= GPS_MAX_BAUD_RATE_INDEX));
}

void InitLCDTask(void)
{
  HardwareSetupLCD();

  /* Initialize Service Registry */
  g_lcdServices.system = &s_lcdSystemPort;
  g_lcdServices.comms = &s_lcdCommsPort;
  g_lcdServices.user = &s_lcdUserPort;
  g_lcdServices.logs = &s_lcdLogPort;
  g_lcdServices.rtc = &g_rtcPort;
  g_lcdServices.gps = &s_lcdGpsPort;
  g_lcdServices.maintenance = &g_mmiMaintenanceService;
  g_lcdServices.runtimeCache = &g_mmiSnapshotCache;

  s_lcdSystemPort.ctx = NULL;
  s_lcdSystemPort.GetMainVoltage = LcdGetMainVoltage;
  s_lcdSystemPort.GetTimeSource = LcdGetTimeSource;
  s_lcdSystemPort.GetLanguage = LcdGetLanguage;
  s_lcdSystemPort.SetLanguage = LcdSetLanguage;

  s_lcdCommsPort.ctx = NULL;
  s_lcdCommsPort.ReadSnapshot = LcdReadCommsSnapshot;

  s_lcdUserPort.ctx = NULL;
  s_lcdUserPort.Login = LcdLogin;
  s_lcdUserPort.GetActiveRole = LcdGetActiveRole;
  s_lcdUserPort.Logout = LcdLogout;
  s_lcdUserPort.ChangeAdminPassword = LcdChangeAdminPassword;

  s_lcdLogPort.ctx = NULL;
  s_lcdLogPort.Read = LcdLogRead;
  s_lcdLogPort.Exists = LcdLogExists;
  s_lcdLogPort.IsIndexValid = LcdLogIsIndexValid;
  s_lcdLogPort.GetWriteIndex = LcdLogGetWriteIndex;

  s_lcdGpsPort.ctx = NULL;
  s_lcdGpsPort.GetPortType = LcdGpsGetPortType;
  s_lcdGpsPort.SetPortType = LcdGpsSetPortType;
  s_lcdGpsPort.GetBaudRateIndex = LcdGpsGetBaudRateIndex;
  s_lcdGpsPort.SetBaudRateIndex = LcdGpsSetBaudRateIndex;
  s_lcdGpsPort.IndexToBaudRate = LcdGpsIndexToBaudRate;
  s_lcdGpsPort.SaveConfig = LcdGpsSaveConfig;
  s_lcdGpsPort.IsValidPortType = LcdGpsIsValidPortType;
  s_lcdGpsPort.IsValidBaudRateIndex = LcdGpsIsValidBaudRateIndex;

  /* Initialize Page Registry */
  g_lcdPages.home = &LcdPage_Home;
  g_lcdPages.login = &LcdPage_Login;
  g_lcdPages.menu = &LcdPage_Menu;
  g_lcdPages.help = &LcdPage_Help;
  g_lcdPages.logs = &LcdPage_Logs;
  g_lcdPages.connectionLogs = &LcdPage_ConnectionLogs;
  g_lcdPages.network = &LcdPage_Network;
  g_lcdPages.settings = &LcdPage_Settings;
  g_lcdPages.settingsDateTime = &LcdPage_SettingsDateTime;
  g_lcdPages.settingsGps = &LcdPage_SettingsGps;
  g_lcdPages.settingsLanguage = &LcdPage_SettingsLanguage;
  g_lcdPages.outputTest = &LcdPage_OutputTest;

  /* Initialize All Pages (Contexts are managed internally by pages for now) */
  LcdPage_Home_Init(LcdPage_Home.ctx,
                    &g_lcdServices,
                    &g_lcdPages,
                    &g_intersectionEngine);
  LcdPage_Login_Init(LcdPage_Login.ctx, &g_lcdServices, &g_lcdPages);
  LcdPage_Menu_Init(LcdPage_Menu.ctx, &g_lcdServices, &g_lcdPages);
  LcdPage_Help_Init(LcdPage_Help.ctx, &g_lcdServices, &g_lcdPages);
  LcdPage_Logs_Init(LcdPage_Logs.ctx, &g_lcdServices, &g_lcdPages);
  LcdPage_ConnectionLogs_Init(LcdPage_ConnectionLogs.ctx,
                              &g_lcdServices,
                              &g_lcdPages);
  LcdPage_Network_Init(LcdPage_Network.ctx, &g_lcdServices, &g_lcdPages);
  LcdPage_Settings_Init(LcdPage_Settings.ctx, &g_lcdServices, &g_lcdPages);
  LcdPage_SettingsDateTime_Init(LcdPage_SettingsDateTime.ctx,
                                &g_lcdServices,
                                &g_lcdPages);
  LcdPage_SettingsGps_Init(LcdPage_SettingsGps.ctx, &g_lcdServices,
                           &g_lcdPages);
  LcdPage_SettingsLanguage_Init(LcdPage_SettingsLanguage.ctx,
                                &g_lcdServices,
                                &g_lcdPages);
  LcdPage_OutputTest_Init(LcdPage_OutputTest.ctx,
                          &g_lcdServices,
                          &g_lcdPages);

  LcdEngine_Init(&g_lcdEngine, &g_lcdPort, &g_keypadPort);
  LcdEngine_SwitchPage(&g_lcdEngine, &LcdPage_Home);
} /* InitLCDTask */

void LCDTaskFunc(void *argument)
{
  uint32_t lastDoorChangeSequence;

  UNUSED(argument);
  InitLCDTask();
  lastDoorChangeSequence = UiDoorServiceGetChangeSequence(&g_uiDoorService);

  for (;;)
  {
    LcdEngine_Tick(&g_lcdEngine, osKernelGetTickCount());
    if (UiDoorServiceGetChangeSequence(&g_uiDoorService) != lastDoorChangeSequence)
    {
      lastDoorChangeSequence = UiDoorServiceGetChangeSequence(&g_uiDoorService);
      if (UiDoorServiceIsOpen(&g_uiDoorService) != 0U)
      {
        LCDSoftwareOpen();
      }
    }
    osDelay(LCD_KEY_SCAN_TIME);
  }
}
