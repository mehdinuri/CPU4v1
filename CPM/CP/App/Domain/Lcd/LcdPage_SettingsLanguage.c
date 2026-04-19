/* App/Domain/Lcd/LcdPage_SettingsLanguage.c
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
} LangCtx_t;

static const char *const pStrLanguages[LANGUAGES_MAX] = { "TURKCE", "ENGLISH" };

static void OnEnter(void *ctx, LcdEngine_t *e)
{
  LangCtx_t *c = (LangCtx_t *) ctx;

  if (UserCanAccessConfiguration(c->services->user) == 0U)
  {
    LcdEngine_SwitchPage(e, c->pages->menu);
  }
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  LangCtx_t *c = (LangCtx_t *) ctx;
  char buf[21];
  uint8_t currentLang = ISystemPort_GetLanguage(c->services->system);

  (void) e;

  DisplayClear(display);

  DisplayWrite(display,
               0,
               0,
               (currentLang
                == LANGUAGE_TURKISH) ? "MENu DiLi" : "MENU LANGUAGE",
               20);

  for (uint8_t i = 0; i < LANGUAGES_MAX; i++)
  {
    sprintf(buf, "%d %c %s", i + 1,
            (i == currentLang) ? '>' : ' ',
            pStrLanguages[i]);
    DisplayWrite(display, i + 1, 0, buf, (uint8_t) strlen(buf));
  }
}

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  LangCtx_t *c = (LangCtx_t *) ctx;

  if (key == KEY_1)
  {
    ISystemPort_SetLanguage(c->services->system, LANGUAGE_TURKISH);
  }
  else if (key == KEY_2)
  {
    ISystemPort_SetLanguage(c->services->system, LANGUAGE_ENGLISH);
  }
  else if (key == KEY_CLEAR)
  {
    LcdEngine_SwitchPage(e, c->pages->settings);
  }
}

static LangCtx_t s_langCtx;
LcdPage_t LcdPage_SettingsLanguage = {
  .ctx = &s_langCtx,
  .OnEnter = OnEnter,
  .OnUpdate = NULL,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_SettingsLanguage_Init(LangCtx_t *ctx,
                                   const LcdServiceRegistry_t *services,
                                   const LcdPageRegistry_t *pages)
{
  ctx->services = services;
  ctx->pages = pages;
  LcdPage_SettingsLanguage.ctx = ctx;
}
