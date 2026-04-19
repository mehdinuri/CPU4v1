/* App/Domain/Lcd/LcdPage_Help.c */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdLanguage.h"
#include "LcdPageRegistry.h"
#include "LcdServiceRegistry.h"

#include <stdio.h>
#include <string.h>

#include "Ports/UserInputTypes.h"

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t *pages;
} HelpCtx_t;

static const char *SafetyActionToText(uint8_t lang, uint8_t safetyAction)
{
  switch ((CpMpSafetyAction_t) safetyAction)
  {
      case CPMP_SAFETY_ACTION_FLASH:
      {
        return "FLASH";
      }

      case CPMP_SAFETY_ACTION_DARK:
      {
        return (lang == LANGUAGE_TURKISH) ? "KARANLIK" : "DARK";
      }

      case CPMP_SAFETY_ACTION_NORMAL:
      default:
      {
        return (lang == LANGUAGE_TURKISH) ? "NORMAL" : "NORMAL";
      }
  }
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  HelpCtx_t *c = (HelpCtx_t *) ctx;
  char buf[21];
  uint8_t lang = ISystemPort_GetLanguage(c->services->system);
  MmiRuntimeSafetySummaryV2_t summary;
  MmiRuntimeRelaySummaryV2_t relay;
  MmiRuntimeDoorSummaryV2_t door;

  (void) e;
  (void) memset(&summary, 0, sizeof(summary));
  (void) memset(&relay, 0, sizeof(relay));
  (void) memset(&door, 0, sizeof(door));

  DisplayClear(display);
  if (c->services->runtimeCache != NULL)
  {
    (void) MmiSnapshotCacheGetSafetySummary(c->services->runtimeCache, &summary);
    (void) MmiSnapshotCacheGetRelaySummary(c->services->runtimeCache, &relay);
    (void) MmiSnapshotCacheGetDoorSummary(c->services->runtimeCache, &door);
  }

  DisplayWrite(display,
               0U,
               0U,
               (lang == LANGUAGE_TURKISH) ? "GUVENLIK DURUMU" : "SAFETY STATUS",
               20U);

  (void) snprintf(buf,
                  sizeof(buf),
                  "ACT:%-8s R:%02u",
                  SafetyActionToText(lang, summary.safetyAction),
                  summary.safetyReasonCode);
  DisplayWrite(display, 1U, 0U, &buf[0], (uint8_t) strlen(buf));

  (void) snprintf(buf,
                  sizeof(buf),
                  "P:%u A:%u F:%04lX",
                  summary.peerHealthy,
                  summary.authorityReady,
                  (unsigned long) (summary.globalFaultFlags & 0xFFFFUL));
  DisplayWrite(display, 2U, 0U, &buf[0], (uint8_t) strlen(buf));

  (void) snprintf(buf,
                  sizeof(buf),
                  "REL:%u USR:%u D:%c",
                  relay.permitOutputPower,
                  relay.userOutputPowerEnabled,
                  (door.open != 0U) ? 'O' : 'C');
  DisplayWrite(display, 3U, 0U, &buf[0], (uint8_t) strlen(buf));
}

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
