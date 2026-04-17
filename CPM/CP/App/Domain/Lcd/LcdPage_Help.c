/* App/Domain/Lcd/LcdPage_Help.c
 */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdLanguage.h"
#include "LcdPageRegistry.h"
#include "LcdServiceRegistry.h"
#include "Ports/ISystemPort.h"
#include <stdio.h>
#include <string.h>

/* Dependencies */
#include "Ports/IIntersectionStatusPort.h"
#include "Ports/UserInputTypes.h"

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t    *pages;
} HelpCtx_t;

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  HelpCtx_t *c = (HelpCtx_t *) ctx;
  char buf[21];
  uint8_t bSetNo;
  uint8_t bSetTotal = IntersectionStatusGetSetTotal(c->services->intersection);
  uint8_t fShowHelpMessage = 0U;
  LcdSetRuntime_t SSetRuntime;
  uint8_t bEmergencySet = 0;
  uint8_t lang = ISystemPort_GetLanguage(c->services->system);

  (void) e;

  for (bSetNo = 0; bSetNo < bSetTotal; bSetNo++)
  {
    if (IntersectionStatusIsSetEmergent(c->services->intersection, bSetNo))
    {
      fShowHelpMessage = 1U;
      IntersectionStatusGetSetRuntime(c->services->intersection,
                                      bSetNo,
                                      &SSetRuntime);
      bEmergencySet = bSetNo;
      break;
    }
  }

  DisplayClear(display);

  if (fShowHelpMessage != 0U)
  {
    /* Line 1: HELP - SET X */
    sprintf(buf, "%s %d", Lcd_GetHelpStr(lang), bEmergencySet + 1);
    DisplayWrite(display, 0, 0, buf, (uint8_t) strlen(buf));

    /* Line 2: Event Name */
    const char *eventStr = Lcd_GetEventStr(SSetRuntime.bSigModeSource, lang, 1);

    DisplayWrite(display, 1, 0, eventStr, (uint8_t) strlen(eventStr));

    /* Line 3: Parameter */
    sprintf(buf, "%s %d",
            Lcd_GetSignalSourceParamStr(SSetRuntime.bSigModeSource,
                                        lang), (int) SSetRuntime.bParam1);
    DisplayWrite(display, 2, 0, buf, (uint8_t) strlen(buf));
  }
  else
  {
    const char *noEmerg = Lcd_GetNoEmergencyStr(lang);

    DisplayWrite(display, 0, 0, noEmerg, (uint8_t) strlen(noEmerg));
  }
} /* OnDraw */

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  HelpCtx_t *c = (HelpCtx_t *) ctx;

  if (key == KEY_CLEAR)
  {
    LcdEngine_SwitchPage(e, c->pages->menu);
  }
}

static HelpCtx_t s_helpCtx;
LcdPage_t LcdPage_Help = {
  .ctx = &s_helpCtx,
  .OnEnter = NULL,
  .OnUpdate = NULL,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_Help_Init(HelpCtx_t *ctx,
                       const LcdServiceRegistry_t *services,
                       const LcdPageRegistry_t *pages)
{
  LcdPage_Help.ctx = ctx;
  ctx->services = services;
  ctx->pages = pages;
}
