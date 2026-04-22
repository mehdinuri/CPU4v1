/* App/Domain/Lcd/LcdPage_OutputPower.c */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdPageRegistry.h"
#include "LcdServiceRegistry.h"

#include <stdio.h>
#include <string.h>

#include "LcdLanguage.h"
#include "Ports/IUserPort.h"
#include "Ports/ISystemPort.h"
#include "Ports/UserInputTypes.h"

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t *pages;
} OutputPowerCtx_t;

static const char *StateToText(uint8_t on)
{
  return (on != 0U) ? "ON" : "OFF";
}

static const char *PeerStateToText(uint8_t valid, uint8_t on)
{
  if (valid == 0U)
  {
    return "?";
  }

  return StateToText(on);
}

static void OnEnter(void *ctx, LcdEngine_t *e)
{
  OutputPowerCtx_t *c = (OutputPowerCtx_t *) ctx;

  if (UserCanAccessConfiguration(c->services->user) == 0U)
  {
    LcdEngine_SwitchPage(e, c->pages->menu);
  }
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  OutputPowerCtx_t *c = (OutputPowerCtx_t *) ctx;
  MmiRuntimeRelaySummaryV2_t relay;
  char buf[21];
  uint8_t lang = LANGUAGE_ENGLISH;

  (void) e;
  (void) memset(&relay, 0, sizeof(relay));
  DisplayClear(display);

  if ((c->services != NULL) && (c->services->system != NULL))
  {
    lang = ISystemPort_GetLanguage(c->services->system);
  }

  if ((c->services != NULL) && (c->services->runtimeCache != NULL))
  {
    (void) MmiSnapshotCacheGetRelaySummary(c->services->runtimeCache, &relay);
  }

  DisplayWrite(display,
               0U,
               0U,
               (lang == LANGUAGE_TURKISH) ? "CIKIS GUCU" : "OUTPUT POWER",
               (lang == LANGUAGE_TURKISH) ? 10U : 12U);

  (void) snprintf(buf,
                  sizeof(buf),
                  "U:%s C:%s M:%s",
                  StateToText(relay.userOutputPowerEnabled),
                  StateToText(relay.reserved0[0]),
                  PeerStateToText(relay.reserved0[2], relay.reserved0[1]));
  DisplayWrite(display, 1U, 0U, &buf[0], (uint8_t) strlen(buf));

  (void) snprintf(buf,
                  sizeof(buf),
                  "E:%s G:%u S:%u",
                  StateToText(relay.permitOutputPower),
                  relay.relayDrive,
                  relay.safetyAction);
  DisplayWrite(display, 2U, 0U, &buf[0], (uint8_t) strlen(buf));

  DisplayWrite(display,
               3U,
               0U,
               (lang == LANGUAGE_TURKISH) ? "1 AC 0 KAPA C CIK" : "1 ON 0 OFF C EXIT",
               (lang == LANGUAGE_TURKISH) ? 17U : 17U);
}

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  OutputPowerCtx_t *c = (OutputPowerCtx_t *) ctx;

  if ((c->services == NULL) || (c->services->maintenance == NULL))
  {
    return;
  }

  if (key == KEY_1)
  {
    (void) MmiMaintenanceServiceRequestRelayState(c->services->maintenance, 1U);
  }
  else if (key == KEY_0)
  {
    (void) MmiMaintenanceServiceRequestRelayState(c->services->maintenance, 0U);
  }
  else if (key == KEY_CLEAR)
  {
    LcdEngine_SwitchPage(e, c->pages->menu);
  }
}

static OutputPowerCtx_t s_outputPowerCtx;
LcdPage_t LcdPage_OutputPower = {
  .ctx = &s_outputPowerCtx,
  .OnEnter = OnEnter,
  .OnUpdate = NULL,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_OutputPower_Init(OutputPowerCtx_t *ctx,
                              const LcdServiceRegistry_t *services,
                              const LcdPageRegistry_t *pages)
{
  LcdPage_OutputPower.ctx = ctx;
  ctx->services = services;
  ctx->pages = pages;
}
