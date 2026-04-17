/* App/Domain/Lcd/LcdPage_Menu.c
 */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdLanguage.h"
#include "LcdPageRegistry.h"
#include "LcdServiceRegistry.h"
#include "Ports/UserInputTypes.h"
#include "Ports/ISystemPort.h"
#include "Ports/ICommsStatusPort.h"
#include "Ports/IRealtimeClockPort.h"
#include "Ports/IGpsPort.h"
#include "Ports/ILogRepositoryPort.h"
#include "Ports/IIntersectionStatusPort.h"
#include <stdio.h>
#include <string.h>

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t    *pages;
  uint8_t scrollIndex;
  uint8_t selectedIndex;
} MenuCtx_t;

static const char *const pStrMenuEntries[LANGUAGES_MAX][6] = {
  { "YARDIM", "CiHAZ LOG", "BAgLANTI LOG", "TEST", "LOKAL/SUNUCU iP",
    "AYARLAR" },
  { "HELP", "DEVICE LOG", "CONNECTION LOG", "TEST", "LOCAL/SERVER IP",
    "SETTINGS" }
};

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  MenuCtx_t *c = (MenuCtx_t *) ctx;
  char buf[21];

  (void) e;

  DisplayClear(display);

  uint8_t lang = ISystemPort_GetLanguage(c->services->system);

  for (uint8_t i = 0; i < 4; i++)
  {
    uint8_t entryIdx = c->scrollIndex + i;

    if (entryIdx >= 6)
    {
      break;
    }

    sprintf(buf, "%02d %c %s", entryIdx + 1,
            (entryIdx == c->selectedIndex) ? '>' : ' ',
            pStrMenuEntries[lang][entryIdx]);
    DisplayWrite(display, i, 0, buf, (uint8_t) strlen(buf));
  }
}

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  MenuCtx_t *c = (MenuCtx_t *) ctx;

  if (key == KEY_DELETE_DOWN)
  {
    if (c->selectedIndex < 5)
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
      LcdEngine_SwitchPage(e, c->pages->help);
    }
    else if (c->selectedIndex == 1)
    {
      LcdEngine_SwitchPage(e, c->pages->logs);
    }
    else if (c->selectedIndex == 2)
    {
      LcdEngine_SwitchPage(e, c->pages->connectionLogs);
    }
    else if (c->selectedIndex == 4)
    {
      LcdEngine_SwitchPage(e, c->pages->network);
    }
    else if (c->selectedIndex == 5)
    {
      LcdEngine_SwitchPage(e, c->pages->settings);
    }
  }
  else if (key == KEY_CLEAR)
  {
    LcdEngine_SwitchPage(e, c->pages->home);
  }
} /* OnInput */

static MenuCtx_t s_menuCtx;
LcdPage_t LcdPage_Menu = {
  .ctx = &s_menuCtx,
  .OnEnter = NULL,
  .OnUpdate = NULL,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_Menu_Init(MenuCtx_t *ctx,
                       const LcdServiceRegistry_t *services,
                       const LcdPageRegistry_t *pages)
{
  LcdPage_Menu.ctx = ctx;
  ctx->services = services;
  ctx->pages = pages;
  ctx->scrollIndex = 0;
  ctx->selectedIndex = 0;
}
