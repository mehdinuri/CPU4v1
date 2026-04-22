/* App/Adapters/STM32/UserSettingsAdapter.c */
#include "UserSettingsAdapter.h"

#include <string.h>

#include "SettingsStorage.h"

static uint8_t ReadSettings(void *ctx)
{
  (void) ctx;
  return UserSettingsRead();
}

static void GetSettings(void *ctx, UserSettingsFlags_t *settings)
{
  tSUserSettings legacySettings;

  (void) ctx;
  if (settings == NULL)
  {
    return;
  }

  (void) memset(&legacySettings, 0, sizeof(legacySettings));
  UserSettingsGet(&legacySettings);

  settings->configFlag = legacySettings.fConfigFlag;
  settings->logFlag = legacySettings.fLogFlag;
  settings->trafficCountsFlag = legacySettings.fTrafficCountsFlag;
  settings->standbyInfoFlag = legacySettings.fStandbyInfoFlag;
}

static void SetSettings(void *ctx, const UserSettingsFlags_t *settings)
{
  tSUserSettings legacySettings;

  (void) ctx;
  if (settings == NULL)
  {
    return;
  }

  (void) memset(&legacySettings, 0, sizeof(legacySettings));
  UserSettingsGet(&legacySettings);
  legacySettings.fSettingsChanged = USER_SETTINGS_CHANGE_CONTROL_VLAUE;
  legacySettings.fConfigFlag = settings->configFlag;
  legacySettings.fLogFlag = settings->logFlag;
  legacySettings.fTrafficCountsFlag = settings->trafficCountsFlag;
  legacySettings.fStandbyInfoFlag = settings->standbyInfoFlag;
  UserSettingsSet(&legacySettings);
}

static uint8_t SaveSettings(void *ctx)
{
  (void) ctx;
  return UserSettingsSave();
}

void UserSettingsAdapterInit(UserSettingsAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
  }
}

IUserSettingsPort_t UserSettingsAdapterCreatePort(UserSettingsAdapterCtx_t *ctx)
{
  IUserSettingsPort_t port;

  port.ctx = ctx;
  port.Read = ReadSettings;
  port.Get = GetSettings;
  port.Set = SetSettings;
  port.Save = SaveSettings;
  return port;
}
