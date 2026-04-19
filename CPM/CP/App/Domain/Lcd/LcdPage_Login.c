/* App/Domain/Lcd/LcdPage_Login.c
 */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdLanguage.h"
#include "LcdPageRegistry.h"
#include "LcdServiceRegistry.h"
#include "Ports/IUserPort.h"
#include "Ports/ISystemPort.h"
#include "Ports/ICommsStatusPort.h"
#include "Ports/IRealtimeClockPort.h"
#include "Ports/IGpsPort.h"
#include "Ports/ILogRepositoryPort.h"
#include "Ports/UserInputTypes.h"
#include <stdio.h>
#include <string.h>

typedef enum
{
  LOGIN_STEP_USERNAME,
  LOGIN_STEP_PASSWORD
} LoginStep_t;

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t    *pages;
  LoginStep_t step;
  uint16_t username;
  uint16_t password;
  uint8_t digitIndex;
} LoginCtx_t;

static void OnEnter(void *ctx, LcdEngine_t *e)
{
  LoginCtx_t *c = (LoginCtx_t *) ctx;

  (void) e;
  UserLogout(c->services->user);
  c->step = LOGIN_STEP_USERNAME;
  c->username = 0;
  c->password = 0;
  c->digitIndex = 0;
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  LoginCtx_t *c = (LoginCtx_t *) ctx;
  char buf[21];

  (void) e;

  DisplayClear(display);

  uint8_t lang = ISystemPort_GetLanguage(c->services->system);

  if (c->step == LOGIN_STEP_USERNAME)
  {
    DisplayWrite(display, 0, 0, Lcd_GetLoginUserStr(lang), 20);
    sprintf(buf, "%04d", (int) c->username);
    DisplayWrite(display, 1, 0, buf, (uint8_t) strlen(buf));
  }
  else
  {
    DisplayWrite(display, 0, 0, Lcd_GetLoginPassStr(lang), 20);
    memset(buf, '*', c->digitIndex);
    buf[c->digitIndex] = '\0';
    DisplayWrite(display, 1, 0, buf, c->digitIndex);
  }
}

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  LoginCtx_t *c = (LoginCtx_t *) ctx;

  if (key <= KEY_9)
  {
    if (c->digitIndex < 4)
    {
      if (c->step == LOGIN_STEP_USERNAME)
      {
        c->username = (uint16_t) (c->username * 10 + (uint16_t) key);
      }
      else
      {
        c->password = (uint16_t) (c->password * 10 + (uint16_t) key);
      }

      c->digitIndex++;
    }
  }
  else if (key == KEY_ENTER)
  {
    if (c->step == LOGIN_STEP_USERNAME)
    {
      c->step = LOGIN_STEP_PASSWORD;
      c->digitIndex = 0;
    }
    else
    {
      if (UserLogin(c->services->user, c->username, c->password)
          != USER_ROLE_NONE)
      {
        LcdEngine_SwitchPage(e, c->pages->home);
      }
      else
      {
        c->password = 0;
        c->digitIndex = 0;
      }
    }
  }
  else if (key == KEY_CLEAR)
  {
    UserLogout(c->services->user);
    LcdEngine_SwitchPage(e, c->pages->home);
  }
}

static LoginCtx_t s_loginCtx;
LcdPage_t LcdPage_Login = {
  .ctx = &s_loginCtx,
  .OnEnter = OnEnter,
  .OnUpdate = NULL,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_Login_Init(LoginCtx_t *ctx,
                        const LcdServiceRegistry_t *services,
                        const LcdPageRegistry_t *pages)
{
  LcdPage_Login.ctx = ctx;
  ctx->services = services;
  ctx->pages = pages;
}
