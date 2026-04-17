/* App/Domain/Lcd/LcdPage_Home.c
 */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdLanguage.h"
#include "LcdServiceRegistry.h"
#include "LcdPageRegistry.h"
#include <stdio.h>
#include <string.h>

/* Dependencies (Domain Types) */
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/Intersection/IntersectionRuntime.h"
#include "Ports/UserInputTypes.h"

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t    *pages;
  const IntersectionEngine_t *intersectionEngine;
} HomeCtx_t;

/* Helper to map interval to a character: R, Y, G, - */
static char IntervalToChar(IntersectionPhaseInterval_t interval)
{
  switch (interval)
  {
      case INTERSECTION_PHASE_INTERVAL_GREEN:
      { return 'G'; }

      case INTERSECTION_PHASE_INTERVAL_YELLOW:
      { return 'Y'; }

      case INTERSECTION_PHASE_INTERVAL_RED:
      { return 'R'; }

      case INTERSECTION_PHASE_INTERVAL_RED_CLEAR:
      { return 'r'; }

      default:
      { return '-'; }
  }
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  HomeCtx_t *c = (HomeCtx_t *) ctx;
  char buf[41];
  const IntersectionRuntime_t *r =
    IntersectionEngineGetRuntime(c->intersectionEngine);
  uint8_t lang = ISystemPort_GetLanguage(c->services->system);

  (void) e;

  /* Line 1: System Info & Mode */
  const char *modeStr = "FREE";

  if (r->mode == INTERSECTION_CONTROL_MODE_COORDINATED)
  {
    modeStr = "CORD";
  }
  else if (r->mode == INTERSECTION_CONTROL_MODE_PREEMPT)
  {
    modeStr = "PREM";
  }
  else if (r->mode == INTERSECTION_CONTROL_MODE_FLASH)
  {
    modeStr = "FLSH";
  }

  sprintf(buf, "MAESTRO %-4s %3dV", modeStr,
          (int) ISystemPort_GetMainVoltage(c->services->system));
  DisplayWrite(display, 0, 0, buf, (uint8_t) strlen(buf));

  /* Line 2: Ring 1 (Phases 1-4) Status */
  sprintf(buf, "R1: %d %c%c%c%c %02d:%02d:%02d",
          (int) r->rings[0].activePosition,
          IntervalToChar(r->phases[0].interval),
          IntervalToChar(r->phases[1].interval),
          IntervalToChar(r->phases[2].interval),
          IntervalToChar(r->phases[3].interval),
          (int) (r->rings[0].stageElapsedTicks / 100),
          (int) (r->rings[0].stageElapsedTicks / 10) % 10,
          (int) (r->rings[0].stageElapsedTicks % 10));
  DisplayWrite(display, 1, 0, buf, (uint8_t) strlen(buf));

  /* Line 3: Ring 2 (Phases 5-8) Status */
  sprintf(buf, "R2: %d %c%c%c%c P:%02d S:%02d",
          (int) r->rings[1].activePosition,
          IntervalToChar(r->phases[4].interval),
          IntervalToChar(r->phases[5].interval),
          IntervalToChar(r->phases[6].interval),
          IntervalToChar(r->phases[7].interval),
          (int) r->systemPatternControl,
          (int) r->coordCycleStatusSeconds);
  DisplayWrite(display, 2, 0, buf, (uint8_t) strlen(buf));

  /* Line 4: Time & Comms */
  sprintf(buf, "12:00 MO A?G%dC%d %s",
          (int) CommsStatusGetModemAlive(c->services->comms),
          (int) CommsStatusGetAsynchConnected(c->services->comms),
          Lcd_GetAdvanceModeStr(ISystemPort_GetTimeSource(c->services->system),
                                lang));
  DisplayWrite(display, 3, 0, buf, (uint8_t) strlen(buf));
} /* OnDraw */

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  HomeCtx_t *c = (HomeCtx_t *) ctx;

  if (key == KEY_ENTER)
  {
    LcdEngine_SwitchPage(e, c->pages->login);
  }
}

/* Page instance and context - allocated here but could be injected */
static HomeCtx_t s_homeCtx;
LcdPage_t LcdPage_Home = {
  .ctx = &s_homeCtx,
  .OnEnter = NULL,
  .OnUpdate = NULL,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_Home_Init(HomeCtx_t *ctx,
                       const LcdServiceRegistry_t *services,
                       const LcdPageRegistry_t *pages,
                       const IntersectionEngine_t *engine)
{
  ctx->services = services;
  ctx->pages = pages;
  ctx->intersectionEngine = engine;
  LcdPage_Home.ctx = ctx;
}
