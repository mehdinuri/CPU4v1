/* App/Domain/Lcd/LcdPage_Network.c
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
#include "Ports/ICommsStatusPort.h"
#include "Ports/UserInputTypes.h"

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t    *pages;
} NetworkCtx_t;

static void ReadCommsSnapshot(const LcdServiceRegistry_t *services,
                              CommsStatusSnapshot_t *snapshot)
{
  (void) memset(snapshot, 0, sizeof(*snapshot));
  if ((services != NULL) && (services->comms != NULL))
  {
    (void) CommsStatusReadSnapshot(services->comms, snapshot);
  }
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  NetworkCtx_t *c = (NetworkCtx_t *) ctx;
  char buf[21];
  CommsStatusSnapshot_t comms;
  uint8_t lang = ISystemPort_GetLanguage(c->services->system);

  (void) e;
  DisplayClear(display);
  ReadCommsSnapshot(c->services, &comms);

  /* Line 1: Local IP Label */
  DisplayWrite(display, 0, 0, Lcd_GetLocalIpStr(lang), 20);
  /* Line 2: Local IP Value */
  sprintf(buf, "%-20s", &comms.localIp[0]);
  DisplayWrite(display, 1, 0, buf, 20);

  /* Line 3: Server IP Label */
  DisplayWrite(display, 2, 0, Lcd_GetServerIpStr(lang), 20);
  /* Line 4: Server IP Value */
  sprintf(buf, "%-20s", &comms.remoteIp[0]);
  DisplayWrite(display, 3, 0, buf, 20);
}

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  NetworkCtx_t *c = (NetworkCtx_t *) ctx;

  if (key == KEY_CLEAR)
  {
    LcdEngine_SwitchPage(e, c->pages->menu);
  }
}

static NetworkCtx_t s_networkCtx;
LcdPage_t LcdPage_Network = {
  .ctx = &s_networkCtx,
  .OnEnter = NULL,
  .OnUpdate = NULL,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_Network_Init(NetworkCtx_t *ctx,
                          const LcdServiceRegistry_t *services,
                          const LcdPageRegistry_t *pages)
{
  LcdPage_Network.ctx = ctx;
  ctx->services = services;
  ctx->pages = pages;
}
