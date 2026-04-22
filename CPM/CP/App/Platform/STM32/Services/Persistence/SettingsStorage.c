/* App/Platform/STM32/Services/Persistence/SettingsStorage.c */
#include "SettingsStorage.h"

#include <string.h>

#include "PersistencePorts.h"
#include "defs.h"

#define TRAFFIC_COUNTS_PERIOD_DEFAULT 0x0FU

static tSUserSettings s_userSettings;
static uint8_t s_userSettingsLoaded;
static tSBrokenInputSettings s_brokenInputSettings;
static uint8_t s_brokenInputSettingsLoaded;
static tSServerSettings s_serverSettings;
static uint8_t s_serverSettingsLoaded;

static void UserSettingsApplyDefaults(tSUserSettings *settings)
{
  if (settings == NULL)
  {
    return;
  }

  (void) memset(settings, 0, sizeof(*settings));
  settings->fSettingsChanged = USER_SETTINGS_CHANGE_CONTROL_VLAUE;
  settings->fConfigFlag = TRUE;
  settings->fLogFlag = TRUE;
  settings->fTrafficCountsFlag = FALSE;
  settings->bTrafficCountsPeriod = TRAFFIC_COUNTS_PERIOD_DEFAULT;
  settings->fStandbyInfoFlag = FALSE;
}

static uint8_t UserSettingsIsValid(const tSUserSettings *settings)
{
  if (settings == NULL)
  {
    return FALSE;
  }

  if (settings->fSettingsChanged != USER_SETTINGS_CHANGE_CONTROL_VLAUE)
  {
    return FALSE;
  }

  return TRUE;
}

static void EnsureUserSettingsLoaded(void)
{
  if (s_userSettingsLoaded != 0U)
  {
    return;
  }

  if ((PersistenceRead(&g_persistencePort,
                       PERSIST_OBJECT_USER_SETTINGS,
                       0U,
                       &s_userSettings,
                       sizeof(s_userSettings)) == FALSE)
      || (UserSettingsIsValid(&s_userSettings) == FALSE))
  {
    UserSettingsApplyDefaults(&s_userSettings);
    (void) PersistenceWrite(&g_persistencePort,
                            PERSIST_OBJECT_USER_SETTINGS,
                            0U,
                            &s_userSettings,
                            sizeof(s_userSettings));
  }

  s_userSettingsLoaded = TRUE;
}

static void BrokenInputSettingsApplyDefaults(tSBrokenInputSettings *settings)
{
  if (settings == NULL)
  {
    return;
  }

  (void) memset(settings, 0, sizeof(*settings));
  settings->fAlreadySet = BROKEN_INPUT_SETTINGS_SET_CONTROL_VLAUE;
  settings->SFlags.fLoopBusy = TRUE;
  settings->SFlags.fDigitalBusy = FALSE;
}

static uint8_t BrokenInputSettingsIsValid(
  const tSBrokenInputSettings *settings)
{
  if (settings == NULL)
  {
    return FALSE;
  }

  return (uint8_t) (settings->fAlreadySet
                    == BROKEN_INPUT_SETTINGS_SET_CONTROL_VLAUE);
}

static void EnsureBrokenInputSettingsLoaded(void)
{
  if (s_brokenInputSettingsLoaded != 0U)
  {
    return;
  }

  if ((PersistenceRead(&g_persistencePort,
                       PERSIST_OBJECT_BROKEN_INPUT_SETTINGS,
                       0U,
                       &s_brokenInputSettings,
                       sizeof(s_brokenInputSettings)) == FALSE)
      || (BrokenInputSettingsIsValid(&s_brokenInputSettings) == FALSE))
  {
    BrokenInputSettingsApplyDefaults(&s_brokenInputSettings);
    (void) PersistenceWrite(&g_persistencePort,
                            PERSIST_OBJECT_BROKEN_INPUT_SETTINGS,
                            0U,
                            &s_brokenInputSettings,
                            sizeof(s_brokenInputSettings));
  }

  s_brokenInputSettingsLoaded = TRUE;
}

static void ServerSettingsApplyDefaults(tSServerSettings *settings)
{
  if (settings == NULL)
  {
    return;
  }

  (void) memset(settings, 0, sizeof(*settings));
  settings->fAlreadySet = SERVER_SETTINGS_SET_CONTROL_VLAUE;
  settings->SFlags.fMCSAvailable = TRUE;
  settings->SFlags.fNTCIPAvailable = FALSE;
}

static uint8_t ServerSettingsIsValid(const tSServerSettings *settings)
{
  if (settings == NULL)
  {
    return FALSE;
  }

  return (uint8_t) (settings->fAlreadySet == SERVER_SETTINGS_SET_CONTROL_VLAUE);
}

static void EnsureServerSettingsLoaded(void)
{
  if (s_serverSettingsLoaded != 0U)
  {
    return;
  }

  if ((PersistenceRead(&g_persistencePort,
                       PERSIST_OBJECT_SERVER_SETTINGS,
                       0U,
                       &s_serverSettings,
                       sizeof(s_serverSettings)) == FALSE)
      || (ServerSettingsIsValid(&s_serverSettings) == FALSE))
  {
    ServerSettingsApplyDefaults(&s_serverSettings);
    (void) PersistenceWrite(&g_persistencePort,
                            PERSIST_OBJECT_SERVER_SETTINGS,
                            0U,
                            &s_serverSettings,
                            sizeof(s_serverSettings));
  }

  s_serverSettingsLoaded = TRUE;
}

uint8_t UserSettingsSave(void)
{
  EnsureUserSettingsLoaded();

  return PersistenceWrite(&g_persistencePort,
                          PERSIST_OBJECT_USER_SETTINGS,
                          0U,
                          &s_userSettings,
                          sizeof(s_userSettings));
}

uint8_t UserSettingsRead(void)
{
  EnsureUserSettingsLoaded();
  return TRUE;
}

void UserSettingsInit(void)
{
  UserSettingsApplyDefaults(&s_userSettings);
  s_userSettingsLoaded = TRUE;
  (void) UserSettingsSave();
}

void UserSettingsSet(tpSUserSettings pSUserSettings)
{
  if (pSUserSettings == NULL)
  {
    return;
  }

  EnsureUserSettingsLoaded();
  s_userSettings = *pSUserSettings;
}

void UserSettingsGet(tpSUserSettings pSUserSettings)
{
  if (pSUserSettings == NULL)
  {
    return;
  }

  EnsureUserSettingsLoaded();
  *pSUserSettings = s_userSettings;
}

uint8_t IsUserSettingsChanged(void)
{
  EnsureUserSettingsLoaded();
  return (uint8_t) (s_userSettings.fSettingsChanged
                    == USER_SETTINGS_CHANGE_CONTROL_VLAUE);
}

uint8_t UserSettingsConfigFlagGet(void)
{
  EnsureUserSettingsLoaded();
  return s_userSettings.fConfigFlag;
}

uint8_t UserSettingsLogFlagGet(void)
{
  EnsureUserSettingsLoaded();
  return s_userSettings.fLogFlag;
}

uint8_t UserSettingsTrafficCountsFlagGet(void)
{
  EnsureUserSettingsLoaded();
  return s_userSettings.fTrafficCountsFlag;
}

uint8_t UserSettingsTrafficCountsPeriodGet(void)
{
  EnsureUserSettingsLoaded();
  return s_userSettings.bTrafficCountsPeriod;
}

uint8_t UserSettingsStandbyFlagGet(void)
{
  EnsureUserSettingsLoaded();
  return s_userSettings.fStandbyInfoFlag;
}

uint8_t BrokenInputSettingsSave(void)
{
  EnsureBrokenInputSettingsLoaded();

  return PersistenceWrite(&g_persistencePort,
                          PERSIST_OBJECT_BROKEN_INPUT_SETTINGS,
                          0U,
                          &s_brokenInputSettings,
                          sizeof(s_brokenInputSettings));
}

uint8_t BrokenInputSettingsRead(void)
{
  EnsureBrokenInputSettingsLoaded();
  return TRUE;
}

void BrokenInputSettingsInit(void)
{
  BrokenInputSettingsApplyDefaults(&s_brokenInputSettings);
  s_brokenInputSettingsLoaded = TRUE;
  (void) BrokenInputSettingsSave();
}

void BrokenInputSettingsSet(tpSBrokenInputSettings pSBrokenInputSettings)
{
  if (pSBrokenInputSettings == NULL)
  {
    return;
  }

  EnsureBrokenInputSettingsLoaded();
  s_brokenInputSettings = *pSBrokenInputSettings;
}

void BrokenInputSettingsGet(tpSBrokenInputSettings pSBrokenInputSettings)
{
  if (pSBrokenInputSettings == NULL)
  {
    return;
  }

  EnsureBrokenInputSettingsLoaded();
  *pSBrokenInputSettings = s_brokenInputSettings;
}

uint8_t IsBrokenInputSettingsSet(void)
{
  EnsureBrokenInputSettingsLoaded();
  return (uint8_t) (s_brokenInputSettings.fAlreadySet
                    == BROKEN_INPUT_SETTINGS_SET_CONTROL_VLAUE);
}

uint8_t BrokenInputSettingsLoopFlagGet(void)
{
  EnsureBrokenInputSettingsLoaded();
  return s_brokenInputSettings.SFlags.fLoopBusy;
}

uint8_t BrokenInputSettingsDigitalFlagGet(void)
{
  EnsureBrokenInputSettingsLoaded();
  return s_brokenInputSettings.SFlags.fDigitalBusy;
}

uint8_t ServerSettingsSave(void)
{
  EnsureServerSettingsLoaded();

  return PersistenceWrite(&g_persistencePort,
                          PERSIST_OBJECT_SERVER_SETTINGS,
                          0U,
                          &s_serverSettings,
                          sizeof(s_serverSettings));
}

uint8_t ServerSettingsRead(void)
{
  EnsureServerSettingsLoaded();
  return TRUE;
}

void ServerSettingsSet(tpSServerSettings pSServerSettings)
{
  if (pSServerSettings == NULL)
  {
    return;
  }

  EnsureServerSettingsLoaded();
  s_serverSettings = *pSServerSettings;
}

void ServerSettingsGet(tpSServerSettings pSServerSettings)
{
  if (pSServerSettings == NULL)
  {
    return;
  }

  EnsureServerSettingsLoaded();
  *pSServerSettings = s_serverSettings;
}

void ServerSettingsInit(void)
{
  ServerSettingsApplyDefaults(&s_serverSettings);
  s_serverSettingsLoaded = TRUE;
  (void) ServerSettingsSave();
}

uint8_t IsServerSettingsSet(void)
{
  EnsureServerSettingsLoaded();
  return (uint8_t) (s_serverSettings.fAlreadySet
                    == SERVER_SETTINGS_SET_CONTROL_VLAUE);
}

uint8_t ServerSettingsMCSAvailableGet(void)
{
  EnsureServerSettingsLoaded();
  return s_serverSettings.SFlags.fMCSAvailable;
}

uint8_t ServerSettingsNTCIPAvailableGet(void)
{
  EnsureServerSettingsLoaded();
  return s_serverSettings.SFlags.fNTCIPAvailable;
}
