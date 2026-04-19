/* App/Adapters/STM32/BrokenInputSettingsAdapter.c */
#include "BrokenInputSettingsAdapter.h"

#include <string.h>

#include "data.h"

static uint8_t ReadSettings(void *ctx)
{
  (void) ctx;
  return BrokenInputSettingsRead();
}

static void GetSettings(void *ctx, BrokenInputSettings_t *settings)
{
  tSBrokenInputSettings legacySettings;

  (void) ctx;
  if (settings == NULL)
  {
    return;
  }

  (void) memset(&legacySettings, 0, sizeof(legacySettings));
  BrokenInputSettingsGet(&legacySettings);
  settings->loopInputFlag = legacySettings.SFlags.fLoopBusy;
  settings->digitalInputFlag = legacySettings.SFlags.fDigitalBusy;
}

static void SetSettings(void *ctx, const BrokenInputSettings_t *settings)
{
  tSBrokenInputSettings legacySettings;

  (void) ctx;
  if (settings == NULL)
  {
    return;
  }

  (void) memset(&legacySettings, 0, sizeof(legacySettings));
  BrokenInputSettingsGet(&legacySettings);
  legacySettings.fAlreadySet = BROKEN_INPUT_SETTINGS_SET_CONTROL_VLAUE;
  legacySettings.SFlags.fLoopBusy = settings->loopInputFlag;
  legacySettings.SFlags.fDigitalBusy = settings->digitalInputFlag;
  BrokenInputSettingsSet(&legacySettings);
}

static uint8_t SaveSettings(void *ctx)
{
  (void) ctx;
  return BrokenInputSettingsSave();
}

void BrokenInputSettingsAdapterInit(BrokenInputSettingsAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
  }
}

IBrokenInputSettingsPort_t BrokenInputSettingsAdapterCreatePort(
  BrokenInputSettingsAdapterCtx_t *ctx)
{
  IBrokenInputSettingsPort_t port;

  port.ctx = ctx;
  port.Read = ReadSettings;
  port.Get = GetSettings;
  port.Set = SetSettings;
  port.Save = SaveSettings;
  return port;
}
