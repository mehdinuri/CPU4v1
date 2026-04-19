/* App/Domain/Lcd/LcdPage_SettingsGps.c
 */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdLanguage.h"
#include "LcdServiceRegistry.h"
#include "LcdPageRegistry.h"
#include <stdio.h>
#include <string.h>

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t    *pages;
  uint8_t tempPort;
  uint8_t tempBaudIdx;
  uint8_t activeField;
} GpsSettingsCtx_t;

static const char *const pStrGpsPorts[3] = { "YOK", "DAHILI", "HARICI" };
static const char *const pStrGpsPortsEn[3] = { "NONE", "INTERNAL", "EXTERNAL" };

static void OnEnter(void *ctx, LcdEngine_t *e)
{
  GpsSettingsCtx_t *c = (GpsSettingsCtx_t *) ctx;

  if (UserCanAccessConfiguration(c->services->user) == 0U)
  {
    LcdEngine_SwitchPage(e, c->pages->menu);
    return;
  }

  c->tempPort = IGpsPort_GetPortType(c->services->gps);
  c->tempBaudIdx = IGpsPort_GetBaudRateIndex(c->services->gps);
  c->activeField = 0;
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  GpsSettingsCtx_t *c = (GpsSettingsCtx_t *) ctx;
  char buf[21];
  uint8_t lang = ISystemPort_GetLanguage(c->services->system);

  (void) e;

  DisplayClear(display);

  DisplayWrite(display,
               0,
               0,
               (lang == LANGUAGE_TURKISH) ? "GPS AYARLARI" : "GPS SETTINGS",
               20);

  /* Line 2: Port */
  const char *portStr = (lang
                         == LANGUAGE_TURKISH) ? pStrGpsPorts[c->tempPort
                                                             % 3]
                        : pStrGpsPortsEn[c->tempPort % 3];

  sprintf(buf, "PORT: %s %c", portStr, (c->activeField == 0) ? '^' : ' ');
  DisplayWrite(display, 1, 0, buf, (uint8_t) strlen(buf));

  /* Line 3: Baud */
  sprintf(buf, "BAUD: %lu %c",
          (unsigned long) IGpsPort_IndexToBaudRate(c->services->gps,
                                                   c->tempBaudIdx),
          (c->activeField == 1) ? '^' : ' ');
  DisplayWrite(display, 2, 0, buf, (uint8_t) strlen(buf));

  DisplayWrite(display,
               3,
               0,
               (lang == LANGUAGE_TURKISH) ? "KAYDET: ENTER" : "SAVE: ENTER",
               20);
}

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  GpsSettingsCtx_t *c = (GpsSettingsCtx_t *) ctx;

  if (key == KEY_ENTER)
  {
    IGpsPort_SetPortType(c->services->gps, c->tempPort);
    IGpsPort_SetBaudRateIndex(c->services->gps, c->tempBaudIdx);
    IGpsPort_SaveConfig(c->services->gps);
    LcdEngine_SwitchPage(e, c->pages->settings);
  }
  else if (key == KEY_UP)
  {
    if (c->activeField == 0)
    {
      c->tempPort = (c->tempPort + 1) % 3;
    }
    else
    {
      if (c->tempBaudIdx < 11)
      {
        c->tempBaudIdx++;
      }
    }
  }
  else if (key == KEY_DELETE_DOWN)
  {
    if (c->activeField == 0)
    {
      c->tempPort = (c->tempPort == 0) ? 2 : (c->tempPort - 1);
    }
    else
    {
      if (c->tempBaudIdx > 1)
      {
        c->tempBaudIdx--;
      }
    }
  }
  else if ((key == KEY_LEFT) || (key == KEY_RIGHT) )
  {
    c->activeField = (c->activeField == 0) ? 1 : 0;
  }
  else if (key == KEY_CLEAR)
  {
    LcdEngine_SwitchPage(e, c->pages->settings);
  }
} /* OnInput */

static GpsSettingsCtx_t s_gpsCtx;
LcdPage_t LcdPage_SettingsGps = {
  .ctx = &s_gpsCtx,
  .OnEnter = OnEnter,
  .OnUpdate = NULL,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_SettingsGps_Init(GpsSettingsCtx_t *ctx,
                              const LcdServiceRegistry_t *services,
                              const LcdPageRegistry_t *pages)
{
  ctx->services = services;
  ctx->pages = pages;
  LcdPage_SettingsGps.ctx = ctx;
}
