/* App/Domain/Lcd/LcdPage_Settings.c
 */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdLanguage.h"
#include "LcdPageRegistry.h"
#include "LcdServiceRegistry.h"
#include "Ports/ISystemPort.h"
#include "Ports/IRealtimeClockPort.h"
#include "Ports/IGpsPort.h"
#include "Ports/UserInputTypes.h"
#include <stdio.h>
#include <string.h>

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t    *pages;
  uint8_t scrollIndex;
  uint8_t selectedIndex;
} SettingsCtx_t;

static void OnEnter(void *ctx, LcdEngine_t *e)
{
  SettingsCtx_t *c = (SettingsCtx_t *) ctx;

  if (UserCanAccessConfiguration(c->services->user) == 0U)
  {
    LcdEngine_SwitchPage(e, c->pages->menu);
  }
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  SettingsCtx_t *c = (SettingsCtx_t *) ctx;
  char buf[21];

  (void) e;

  DisplayClear(display);

  uint8_t lang = ISystemPort_GetLanguage(c->services->system);

  for (uint8_t i = 0; i < 4; i++)
  {
    uint8_t entryIdx = c->scrollIndex + i;

    if (entryIdx >= 10)
    {
      break;
    }

    sprintf(buf, "%02d %c %s", entryIdx + 1,
            (entryIdx == c->selectedIndex) ? '>' : ' ',
            Lcd_GetSettingsMenuEntryStr(entryIdx, lang));
    DisplayWrite(display, i, 0, buf, (uint8_t) strlen(buf));
  }
}

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  SettingsCtx_t *c = (SettingsCtx_t *) ctx;

  if (key == KEY_DELETE_DOWN)
  {
    if (c->selectedIndex < 9)
    {
      c->selectedIndex++;
    }

    if (c->selectedIndex >= c->scrollIndex + 4)
    {
      c->scrollIndex++;
    }
  }
  else if (key == KEY_UP)
  {
    if (c->selectedIndex > 0)
    {
      c->selectedIndex--;
    }

    if (c->selectedIndex < c->scrollIndex)
    {
      c->scrollIndex--;
    }
  }
  else if (key == KEY_ENTER)
  {
    /* Switch to selected subpage */
    if (c->selectedIndex == 0)
    {
      LcdEngine_SwitchPage(e, c->pages->settingsDateTime);
    }
    else if (c->selectedIndex == 1)
    {
      LcdEngine_SwitchPage(e, c->pages->settingsLanguage);
    }
    else if (c->selectedIndex == 2)
    {
      LcdEngine_SwitchPage(e, c->pages->settingsGps);
    }
  }
  else if (key == KEY_CLEAR)
  {
    LcdEngine_SwitchPage(e, c->pages->menu);
  }
} /* OnInput */

static SettingsCtx_t s_settingsCtx;
LcdPage_t LcdPage_Settings = {
  .ctx = &s_settingsCtx,
  .OnEnter = OnEnter,
  .OnUpdate = NULL,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_Settings_Init(SettingsCtx_t *ctx,
                           const LcdServiceRegistry_t *services,
                           const LcdPageRegistry_t *pages)
{
  LcdPage_Settings.ctx = ctx;
  ctx->services = services;
  ctx->pages = pages;
  ctx->scrollIndex = 0;
  ctx->selectedIndex = 0;
}
