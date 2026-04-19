/* App/Domain/Lcd/LcdPage_OutputTest.c */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdPageRegistry.h"
#include "LcdServiceRegistry.h"

#include <stdio.h>
#include <string.h>

#include "Ports/IUserPort.h"
#include "Ports/UserInputTypes.h"

enum
{
  OUTPUT_TEST_EXIT_CHORD_TIMEOUT_MS = 3000U
};

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t *pages;
  uint8_t selectedChannel;
  uint32_t clearChordMs;
} OutputTestCtx_t;

static const char *AspectToText(OutputDriverAspect_t aspect)
{
  switch (aspect)
  {
      case OUTPUT_DRIVER_ASPECT_RED:
      {
        return "RED";
      }

      case OUTPUT_DRIVER_ASPECT_YELLOW:
      {
        return "YEL";
      }

      case OUTPUT_DRIVER_ASPECT_GREEN:
      {
        return "GRN";
      }

      case OUTPUT_DRIVER_ASPECT_DARK:
      default:
      {
        return "DRK";
      }
  }
}

static void StartOutputTest(OutputTestCtx_t *ctx)
{
  if ((ctx != NULL) && (ctx->services != NULL)
      && (ctx->services->maintenance != NULL))
  {
    (void) MmiMaintenanceServiceStartOutputTest(ctx->services->maintenance);
  }
}

static void StopOutputTest(OutputTestCtx_t *ctx)
{
  if ((ctx != NULL) && (ctx->services != NULL)
      && (ctx->services->maintenance != NULL))
  {
    (void) MmiMaintenanceServiceStopOutputTest(ctx->services->maintenance);
  }
}

static void OnEnter(void *ctx, LcdEngine_t *e)
{
  OutputTestCtx_t *c = (OutputTestCtx_t *) ctx;

  if (UserCanAccessConfiguration(c->services->user) == 0U)
  {
    LcdEngine_SwitchPage(e, c->pages->menu);
    return;
  }

  c->selectedChannel = 1U;
  c->clearChordMs = 0U;
  StartOutputTest(c);
}

static void OnUpdate(void *ctx, LcdEngine_t *e, uint32_t tickMs)
{
  OutputTestCtx_t *c = (OutputTestCtx_t *) ctx;

  (void) e;
  if (c->clearChordMs != 0U)
  {
    if (tickMs >= c->clearChordMs)
    {
      c->clearChordMs = 0U;
    }
    else
    {
      c->clearChordMs -= tickMs;
    }
  }
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  OutputTestCtx_t *c = (OutputTestCtx_t *) ctx;
  char buf[21];
  OutputDriverAspect_t aspect = OUTPUT_DRIVER_ASPECT_DARK;
  uint8_t forced = 0U;
  MmiRuntimeRelaySummaryV2_t relay;

  (void) e;
  (void) memset(&relay, 0, sizeof(relay));
  DisplayClear(display);

  if ((c->services != NULL) && (c->services->runtimeCache != NULL))
  {
    (void) MmiSnapshotCacheGetRelaySummary(c->services->runtimeCache, &relay);
  }

  if ((c->services != NULL) && (c->services->maintenance != NULL)
      && (c->services->maintenance->outputTestService != NULL))
  {
    forced = OutputTestServiceGetChannelAspect(
      c->services->maintenance->outputTestService,
      c->selectedChannel,
      &aspect);
  }

  DisplayWrite(display, 0U, 0U, "OUTPUT TEST", 11U);

  (void) snprintf(buf,
                  sizeof(buf),
                  "CH:%02u ASP:%s",
                  c->selectedChannel,
                  AspectToText((forced != 0U) ? aspect : OUTPUT_DRIVER_ASPECT_DARK));
  DisplayWrite(display, 1U, 0U, &buf[0], (uint8_t) strlen(buf));

  DisplayWrite(display, 2U, 0U, "L/R 1R 2Y 3G 0D", 17U);

  (void) snprintf(buf,
                  sizeof(buf),
                  "P:%u C CLR ENT EXIT",
                  relay.permitOutputPower);
  DisplayWrite(display, 3U, 0U, &buf[0], (uint8_t) strlen(buf));
}

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  OutputTestCtx_t *c = (OutputTestCtx_t *) ctx;

  if ((c->services == NULL) || (c->services->maintenance == NULL)
      || (c->services->maintenance->outputTestService == NULL))
  {
    return;
  }

  if (key == KEY_LEFT)
  {
    c->selectedChannel = (c->selectedChannel == 1U)
                         ? INTERSECTION_CHANNEL_COUNT_MAX
                         : (uint8_t) (c->selectedChannel - 1U);
    c->clearChordMs = 0U;
  }
  else if (key == KEY_RIGHT)
  {
    c->selectedChannel = (c->selectedChannel >= INTERSECTION_CHANNEL_COUNT_MAX)
                         ? 1U
                         : (uint8_t) (c->selectedChannel + 1U);
    c->clearChordMs = 0U;
  }
  else if (key == KEY_1)
  {
    (void) OutputTestServiceSetChannelAspect(
      c->services->maintenance->outputTestService,
      c->selectedChannel,
      OUTPUT_DRIVER_ASPECT_RED);
    c->clearChordMs = 0U;
  }
  else if (key == KEY_2)
  {
    (void) OutputTestServiceSetChannelAspect(
      c->services->maintenance->outputTestService,
      c->selectedChannel,
      OUTPUT_DRIVER_ASPECT_YELLOW);
    c->clearChordMs = 0U;
  }
  else if (key == KEY_3)
  {
    (void) OutputTestServiceSetChannelAspect(
      c->services->maintenance->outputTestService,
      c->selectedChannel,
      OUTPUT_DRIVER_ASPECT_GREEN);
    c->clearChordMs = 0U;
  }
  else if (key == KEY_0)
  {
    (void) OutputTestServiceSetChannelAspect(
      c->services->maintenance->outputTestService,
      c->selectedChannel,
      OUTPUT_DRIVER_ASPECT_DARK);
    c->clearChordMs = 0U;
  }
  else if (key == KEY_CLEAR)
  {
    (void) OutputTestServiceClearChannel(
      c->services->maintenance->outputTestService,
      c->selectedChannel);
    c->clearChordMs = OUTPUT_TEST_EXIT_CHORD_TIMEOUT_MS;
  }
  else if (key == KEY_ENTER)
  {
    if (c->clearChordMs != 0U)
    {
      StopOutputTest(c);
      c->clearChordMs = 0U;
      LcdEngine_SwitchPage(e, c->pages->home);
    }
  }
}

static OutputTestCtx_t s_outputTestCtx;
LcdPage_t LcdPage_OutputTest = {
  .ctx = &s_outputTestCtx,
  .OnEnter = OnEnter,
  .OnUpdate = OnUpdate,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_OutputTest_Init(OutputTestCtx_t *ctx,
                             const LcdServiceRegistry_t *services,
                             const LcdPageRegistry_t *pages)
{
  LcdPage_OutputTest.ctx = ctx;
  ctx->services = services;
  ctx->pages = pages;
  ctx->selectedChannel = 1U;
  ctx->clearChordMs = 0U;
}
